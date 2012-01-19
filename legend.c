#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<math.h>
#include"legend.h"

/* リファクタリング途中バージョン　乱数の読み込みとチェックまでできた　20080919 */
/*	ファイルからの原子データの読み込みも出来た　20080919 */
/* 原子データを読み込んで番号を振るところで、なんか数え間違えてるらしい←直した 20080920 */
/* なんとか自動ぶん回し出来た エッチング速すぎるので次は途中経過を確認せよ　20081004 */
/* 乱数の与え方が繰り返し一回ごとだったのでエッチング速すぎた　修正　20081005 */
/* Cygwinではぶん回しできるんだけど、Linux環境では2回目の途中でabort とりあえず、ゾンビに対して近傍扱いしないように修正　20081006 */
/* 判定の正誤を判断しやすくするために　各原子の全情報を出力するように修正。ファイルに出力できるんだけど、なぜかそれが標準出力にも出る。なんで？ ←直した */
/* 「エッチングされるたびに全体を再計算する」というアフォをやらずに済むように etch_or_not を修正（途中） 20081008 */

static double PROBABILITY_SUM;
static double PROBABILITY[7];

#define ITERATION  1500
#define MAX_ITERATION 1500

double distance[10000][10000]; /* n 番目の原子とm 番目の原子の距離 
				konoatari ga genkai nanode kokode motanai youni suru*/

// これを legend.h に持って行くにはどうしたらいいんだろう？

#define MAX_ATOMS 10000

typedef struct vector {
		double x;
		double y;
		double z;
		} VECTOR;
		
/*	仮読み用構造体	*/

	struct{
		char number[255];
		char element[255];
		char x[100];
		char y[100];
		char z[100];
		char neigh0[100];
		char neigh1[100];
		char neigh2[100];
		char neigh3[100];
		char type[255];
		} readed_apostle[MAX_ATOMS];

#define ALL_DICES 10000000

int DICE_NUMBER[ALL_DICES];
double DICE[MAX_ITERATION][MAX_ATOMS];
int BOX[10];


/* 本番用構造体 */

	struct{
		int number;			/* 「その使徒」番号 (0 - NUMBER_OF_Si)			*/

		char element[255];			/* 「その使徒」原子種			*/

		double x, y ,z;			/* 「その使徒」座標 Z座標が負なら「ゾンビ」とする）	*/

		int neighbor[4];		/* 「その使徒」のご近所さん番号　*/
					/*	メモリ化け防止のため
						一個多く宣言	*/

		int type;			/* 「その使徒」の状態
						no neighbor		:7
						kink			:6
						3-hydride	 	:5
						2-vertical hydride	:4
						2-horizontal hidride	:3
						1-(horizontal) hidride	:2
						terrace			:1
						表面にいない		:0
						etchされてゾンビ化	:-1
						ゴミなのでゾンビ扱い　	:-255 */

		} apostle[MAX_ATOMS];

//初期構造ファイル名

const char input[] = "test-first.xyz";

int check_probability();
int read_dices();
int read_structure();
int calc_neighbor( int );
int calc_boundary( int );
int etch_or_not(int , int );

int main(void){


/*	入出力ファイル	*/
	FILE *f_log, *frand, *fstructure, *finput, *fdiffer ;
	char log[] = "log.txt";
	char rand[] = "rand1000000.txt";
	char structure[] = "structure.xyz";
	char differ[] = "difference.txt";


/*	ログファイルを新規に開く	*/

	f_log=fopen(log,"w");
	fprintf(f_log,"\n// 200812041830 \n\n");
	fclose(f_log);
	
	fdiffer=fopen(differ,"w");
	fclose(fdiffer);

/*	エッチング確率	ヘッダに持っていきたいのだけど	*/

	PROBABILITY[6]	= 0.1000;	/* kinkのSiのetch確率 		*/
	PROBABILITY[5]	= 0.0500;	/* 3個のHで終端されたSiのetch確率 	*/
	PROBABILITY[4]	= 0.0100;	/* 横向きにH2個で終端されたSiのetch確率 */
	PROBABILITY[3]	= 0.0300;	/* 横と上向きにH2個で終端されたSiのetch確率 */
	PROBABILITY[2]	= 0.0050;	/* 1個のHで終端されたSiのetch確率	*/
	PROBABILITY[1]	= 0.0030;	/* terraceのSiのetch確率	*/

	f_log=fopen(log,"a");

		printf("// check probability\n\n");
		fprintf(f_log,"// check probability\n\n");


//	エッチング確率の合計が1以上だったら強制終了
	if( check_probability() == 1){

		fprintf(f_log,"PROBABILITY_SUM = %lf \n", PROBABILITY_SUM);
		exit(0);

	}

//	エッチング確率の合計が1以下だったら処理続行

		fprintf(f_log,"PROBABILITY_SUM = %lf \n", PROBABILITY_SUM);

		fclose(f_log);

// 乱数は今のところはファイルから読み込む　ゆくゆくは引数から取り込めるようにする


// 乱数を発生させるバージョンは　legend2008091729.c あたりに残ってる


/* 乱数を読み込む */

	read_dices();

//	構造を読み込む
//	構造を作るバージョンは　legend2008091729.c あたりに残ってる

	int atoms; 	// 1から数え始めた原子数

	atoms = read_structure();
	printf("read_structure\n");
	printf("atoms = %d\n", atoms);

// 初期構造から近傍原子情報・サイト情報を作る

	printf("calc_neighbor\n");
	calc_neighbor(atoms);
	
	printf("calc_boundary\n");
	calc_boundary(atoms);

// エッチングの繰り返し処理を行う

	int difference = 0;
	int i = 0;

// ITERATION回の繰り返し　始まり

	for( i = 0; i < ITERATION; i++){

		f_log = fopen(log,"a");
		fprintf(f_log,"\n+++ %d th iteration \n", i);
		fclose(f_log);
	
		printf("etch_or_not\n");
		difference = etch_or_not(i, atoms);

		f_log = fopen(log,"a");
		printf("difference = %d \n", difference);	
		fprintf(f_log,"difference = %d \n", difference);
		fclose(f_log);

/*  変化があったら近傍と境界の再計算 */

	if ( difference >= 1){
		
		printf("\n// etched \n\n");
		
		f_log = fopen(log,"a");
		fprintf(f_log,"\n// etched \n\n");
		fclose(f_log);

		fdiffer=fopen(differ,"a");
		fprintf(fdiffer,"%d\n",i);
		fclose(fdiffer);
		
		printf("calc_neighbor\n");
		calc_neighbor(atoms);
	
		printf("calc_boundary\n");
		calc_boundary(atoms);

	} else {
		
		printf("\n// not etched \n\n");
		
		f_log = fopen(log,"a");
		fprintf(f_log,"\n// not etched \n\n");
		fclose(f_log);
		
	}
		
//	fclose(f_log);
	
	// ITERATION回の繰り返し　終わり
	}
	
//	最後にファイル閉じる
//	fclose(f_log);

	return 0;

}


/*	エッチング確率のチェック　合計が1を超えたらエラー終了	*/

int check_probability(){

	printf("// inside check probability\n");

	// PROBABILITY_SUMに各確率を足しこむ

	int i;
	for( i=0; i<7; i++){

		PROBABILITY_SUM = PROBABILITY_SUM + PROBABILITY[i];

	}

	if( PROBABILITY_SUM > 1.0 ){

		return 1;

	} else {

		return 0;

	}

}

int read_dices(){
	
	
	FILE *frand, *f_log;
	
	char log[] = "log.txt";
	char rand[] = "dice.txt";

	frand = fopen(rand, "r");
	f_log = fopen(log,"a");

	int i = 0;
	int j = 0;
	int k = 0;
	int rank = 0;

	char buffer[256];

	printf("\n// +++ inside read_dices\n");
	fprintf(f_log,"\n// +++inside read_dices\n\n");

	printf("\nALL_DICES = %d\n", ALL_DICES);
	
	int count1, count2;

	printf("// +++ initialize DICE_NUMBER[]\n\n");
	fprintf(f_log, "// +++ initialize DICE_NUMBER[]\n\n");

	for(count1 = 0; count1 < ALL_DICES; count1++){

		DICE_NUMBER[count1] = -1;
		
	}
	
	printf("// +++ initialize DICES[][]\n\n");
	fprintf(f_log, "// +++ initialize DICES[][]\n\n");
	
	for (count1 = 0; count1 < ALL_DICES / MAX_ATOMS; count1++){
		
		for(count2 = 0; count2 < ALL_DICES % MAX_ATOMS; count2++){

		DICE[count1][count2] = -1.0;
		
		}
	}

	printf("// +++ read DICE_NUMBER[], DICES[][]\n\n");
	fprintf(f_log, "// +++ initialize DICE_NUMBER[], DICES[][]\n\n");

	while( (NULL != fgets( buffer, 256, frand )) &&  i < ALL_DICES ){
		
		j = (int)i / MAX_ATOMS ;
	    k = i % MAX_ATOMS;
		
			sscanf( buffer, "%d %lf\n", &DICE_NUMBER[i], &DICE[j][k] );

			fprintf( f_log, "%d DICE[%d][%d] = %lf\n", DICE_NUMBER[i], j,k, DICE[j][k]);

			rank = (int)(DICE[j][k] * 10);

			BOX[rank]++;


	i++;

	}

	fprintf(f_log,"\n//check dices baratsuki \n\n");
	printf("\n//check dices baratsuki \n\n");

	for( i = 0 ; i < 10; i++){

		fprintf(f_log,"(%lf - %lf) %d\n", 
		(double)i/10,(double)(i+1)/10, BOX[i]);


	}
	
	fprintf(f_log,"\n//check dices baratsuki \n\n");
	printf("\n//after check dices baratsuki \n\n");


	fclose(frand);
	fclose(f_log);

	return 0;
	
}

int read_structure(){

//	構造を読み込む

	FILE *finput, *f_log;
	
	char log[] = "log.txt";
	char buffer[256];

	f_log=fopen(log,"a");
	finput = fopen(input, "r");
	
	fprintf(f_log,"\n// +++ inside read_structure \n\n");
	printf("\n// +++ inside read_structure \n\n");
	
	fprintf(f_log,"\n// +++ initialize array \n\n");
	printf("\n// +++ initialize array \n\n");

	int i = 0;

// 配列の初期化

	for( i = 0; i < MAX_ATOMS ; i++){

	printf("%d th initialize array\n\n", i );
	fprintf(f_log,"%d th initialize array\n\n", i );

		strcpy(readed_apostle[i].element, "XXX");
		strcpy(readed_apostle[i].x, "XXX");
		strcpy(readed_apostle[i].y, "XXX");
		strcpy(readed_apostle[i].z, "XXX");

		apostle[i].number = -255;
		strcpy( apostle[i].element, "ZZZ");
		apostle[i].x = -255.0;
		apostle[i].y = -255.0;
		apostle[i].z = -255.0;
		apostle[i].neighbor[0] = -255;
		apostle[i].neighbor[1] = -255;
		apostle[i].neighbor[2] = -255;
		apostle[i].neighbor[3] = -255;
		apostle[i].type = -255;

	}



	fprintf( f_log, "\n// +++ initial atoms in input file \n");

	
	i = 0;
	
	//ヘッダを飛ばす
	fgets( buffer, 256, finput );
	fgets( buffer, 256, finput );
	
	//内容を読む
	
	while( NULL != fgets( buffer, 256, finput ) ){

		printf("%s", buffer );


		sscanf( buffer, "%s\t%s\t%s\t%s\n", 
						readed_apostle[i].element,
						readed_apostle[i].x,
						readed_apostle[i].y,
						readed_apostle[i].z);

		fprintf( f_log, "i=%d %s %s %s %s\n",
						i,
						readed_apostle[i].element,
						readed_apostle[i].x,
						readed_apostle[i].y,
						readed_apostle[i].z);


//	計算用の原子データをセット	入力にない要素は初期化
	
	
		apostle[i].number = i;
		strcpy( apostle[i].element, readed_apostle[i].element);
		apostle[i].x = atof(readed_apostle[i].x);
		apostle[i].y = atof(readed_apostle[i].y);
		apostle[i].z = atof(readed_apostle[i].z);
		apostle[i].neighbor[0] = -255;
		apostle[i].neighbor[1] = -255;
		apostle[i].neighbor[2] = -255;
		apostle[i].neighbor[3] = -255;
		apostle[i].type = -255;

	
	i++;

	}

	int atoms = i ; //　1から数え始めた原子数
	int j = 0;
	
	fprintf(f_log, "atoms = %d\n", atoms);

	fprintf( f_log, "\n// +++ initial atoms for calculation \n");

	for( i=0; i < atoms  ; i++){

		fprintf(f_log, "%d %d %s %lf %lf %lf %d %d %d %d %d\n",
				i,
				apostle[i].number,
				apostle[i].element,
				apostle[i].x,
				apostle[i].y,
				apostle[i].z,
				apostle[i].neighbor[0],
				apostle[i].neighbor[1],
				apostle[i].neighbor[2],
				apostle[i].neighbor[3],
				apostle[i].type);


	}

	fprintf( f_log, "\n// +++ the end of read_structure \n");
	printf("the end of read_structure\n");

	fclose(f_log);
	
	return atoms;

}

int calc_neighbor( int the_MAX_ATOMS ){

	FILE *f_log;
	
	char log[] = "log.txt";
	char buffer[256];

	f_log=fopen(log,"a");
	
	fprintf(f_log,"\n// inside calc_neighbor \n\n");
	printf("\n// inside calc_neighbor \n\n");

//	原子　来てるので次は近傍計算をすればいい

	int num1 = 0;  /* 原子カウンタ1	*/
	int num2 = 0;  /* 原子カウンタ2 */
	int cnt = 0; /* ご近所さん（4つまで）カウンタ　*/

	double distance = 0.0; /* 原子間距離 */

//	int the_MAX_ATOMS = MAX_ATOMS;

	/*	ここも　100→MAX_ATOMS　としたい	*/

	int neighbors[MAX_ATOMS] ;	/*	ご近所さん人数	*/
	int neigh[MAX_ATOMS][4] ;	/*	サイト計算で使うご近所さんデータ	*/

	int kinks[MAX_ATOMS];  /* kinkさん候補　全原子数を超えることはありえねー　*/

/*	neigh[MAX_ATOMS][4] を初期化する	*/

		for( num1 = 0; num1 < MAX_ATOMS; num1++){
			
			for ( cnt = 0; cnt < 4; cnt++) {

			apostle[num1].neighbor[cnt] = -255;

			}
		}

/* 4つのご近所さんを計算する    */

	for( num1 = 0; num1 < the_MAX_ATOMS ; num1++ ){

	cnt = 0;

	for( num2 = 0; num2 < the_MAX_ATOMS ; num2++ ){

    // ゾンビにご近所さんはいない
    if( apostle[num1].type != -1){


	distance = sqrt(  pow(apostle[num1].x - apostle[num2].x, 2)
			+ pow(apostle[num1].y - apostle[num2].y, 2)
			+ pow(apostle[num1].z - apostle[num2].z, 2));

	    if( (distance != 0.0 ) && (distance < ((ATOMIC_DISTANCE_Si_Si ) * 1.01) ) && (cnt < 4) ){

		    apostle[num1].neighbor[cnt] = num2;

		    cnt++;

    }
    }

	}
	
	}

	int i;
	
	fprintf(f_log,"\n// after culc-neighbor (distance) \n\n");
	
	for( i=0; i < the_MAX_ATOMS ; i++){

		fprintf(f_log, "%d %s %lf %lf %lf %d %d %d %d %d\n",
				apostle[i].number,
				apostle[i].element,
				apostle[i].x,
				apostle[i].y,
				apostle[i].z,
				apostle[i].neighbor[0],
				apostle[i].neighbor[1],
				apostle[i].neighbor[2],
				apostle[i].neighbor[3],
				apostle[i].type);

	}

	fprintf(f_log,"\n// kokokara site keisan\n\n");

/*	まずご近所さん人数とkinkさん候補の初期化		*/

	for( i=0 ; i<MAX_ATOMS ; i++){

		neighbors[i] = 0;
		kinks[i] = -255;

	}

	int j;
	int k;

/*	各原子についてサイト計算するループはじまり	*/

	for( num1 = 0; num1 < the_MAX_ATOMS ; num1++){

	fprintf( f_log,"---\nThe %d th story\n", num1);

/*	4人のご近所さんについて調べるループはじまり　*/
	for ( cnt = 0; cnt < 4; cnt++) {

/*	ご近所さんデータ拝見 */
		fprintf( f_log, "apostle[%d].neighbor[%d] = %d\n",
							num1, cnt, apostle[num1].neighbor[cnt]);
							
/*	まずご近所さん人数を数える	*/

		if(apostle[num1].neighbor[cnt]  >= 0){
			neighbors[num1]++;
		
		}
		
/*	ご近所さんの人数を確認	*/
	fprintf( f_log, "neighbors[%d] = %d atoms\n",num1,neighbors[num1]);

/*	4人のご近所さんについて調べるループ終わり　*/	
	}
	
/* ここからサイト計算をする	境界による再計算は 後で　culc_bounary　関数でおこなう */


switch ( neighbors[num1] ) {
	case 4:
		apostle[num1].type = 0;
		fprintf( f_log, "apostle[%d].type = %d (Si inside)\n", num1,apostle[num1].type);

		break;
	case 3:

//		printf("apostle[%d].type = terrace or 1-(horizontal) hidride\n", num1);

		/* 上向き△になってるのが terrace　これがデフォ	*/
		/* ご近所さんが全部自分より下にある	*/
			apostle[num1].type = 1;
			fprintf( f_log, "apostle[%d].type = %d (terrace)\n", num1, apostle[num1].type);

		for( j=0 ; j<3 ; j++ ){

		if( apostle[num1].z < apostle[apostle[num1].neighbor[j]].z ){

		/*	値の比較は変なことになってないらしい。*/
//			printf("%lf < %lf \n", apostle[num1].z, apostle[apostle[num1].neighbor[j]].z );
//			printf("apostle[%d].neighbor[%d] is over myself\n", num1, j);	


		/* 下向き▽になってるのが 1-(horizontal) hidride	*/
		/*　ご近所さんのどれかが自分より上にあったらこれ	*/
			apostle[num1].type = 2;
			fprintf(  f_log, "apostle[%d].type = %d (1-(horizontal) hidride)\n", num1, apostle[num1].type);

			break;
		} 

		}

		break;
	case 2:
//		printf("apostle[%d].type = 2-horizontal hidride or 2-vertical hydride\n",num1);

		/* 上向き△になってるのが 2-vertical hydride　デフォ	*/
		/* ご近所さん Si が全部自分より下にある	*/

			apostle[num1].type = 4;
			fprintf(  f_log, "apostle[%d].type = %d (2-vertical hydride) \n", num1, apostle[num1].type);

		for( j=0 ; j<1 ; j++ ){

		if( apostle[num1].neighbor[j] >= 0 && apostle[num1].y < apostle[apostle[num1].neighbor[j]].y ){

		/* 下向き▽になってるのが 2-horizontal hidride	*/
		/*　ご近所さん Si のどれかが自分より上にあったらこれ	*/

			apostle[num1].type = 3;
			fprintf( f_log, "apostle[%d].type = %d (2-horizontal hidride) \n", num1, apostle[num1].type);

		for( k=0 ; k<3 ; k++){
		/* さらにkinkであるかどうかを判定	*/
//		printf("kink or not \n");

		/* そのご近所さん　Si　は自分より上ですか？	*/
		
		if( apostle[num1].neighbor[k] >= 0 && apostle[num1].y < apostle[apostle[num1].neighbor[k]].y ){

//		printf("kink's smell \n");

		/* 自分より上にある　ご近所さん　Si　は 1-(horizontal) hidride　ですか？　*/

			if( apostle[apostle[num1].neighbor[k]].type == 2){

		/* そいつは kink である	*/
			fprintf( f_log,"this is a kink. \n");
			apostle[num1].type = 6;
			fprintf( f_log, "apostle[%d].type = %d (kink) \n", num1, apostle[num1].type );
			}

		/* kinkかどうかの判定ループ終わり	*/
		}

		/* kinkくさいかどうかの判定終わり　*/
		}

		/*　2-horizontal hidride　かどうかのif文終わり 	*/
		}

		}
		
		break;
	case 1:
		apostle[num1].type = 5;
		fprintf( f_log, "apostle[%d].type = %d (3-hydride) \n", num1, apostle[num1].type );
		break;
	case 0:
		apostle[num1].type = 7;
		fprintf( f_log, "apostle[%d].type = %d (no neighbor) \n", num1, apostle[num1].type );
		break;
	default:
		fprintf( f_log,"Who are you %d th?\n", num1);
		break;
}
	

/*	各原子についてサイト計算するループ終わり	*/
	}

/* 結果の確認	*/

		fprintf(f_log, "\n\n the result of culc_neighbor \n");
		
		for( i =0; i  < the_MAX_ATOMS  ; i++){

		fprintf(f_log, "%d %d %s %lf %lf %lf %d %d %d %d %d\n",
				i,
				apostle[i].number,
				apostle[i].element,
				apostle[i].x,
				apostle[i].y,
				apostle[i].z,
				apostle[i].neighbor[0],
				apostle[i].neighbor[1],
				apostle[i].neighbor[2],
				apostle[i].neighbor[3],
				apostle[i].type);


		}



	fclose(f_log);

	return 0;
}


int calc_boundary( int the_MAX_ATOMS ){

	FILE *f_log;
	
	char log[] = "log.txt";
	char buffer[256];

	f_log=fopen(log,"a");
	
	fprintf(f_log,"\n// inside calc_boundary \n\n");
	printf("\n// inside calc_boundary \n\n");	

	int num;
	int cnt;
	int bound[100][3];
	int bounds[100];

/* 来ている原子の情報を調べる	*/

		int i;
		
		for( i =0; i  < the_MAX_ATOMS  ; i++){

		fprintf(f_log, "%d %d %s %lf %lf %lf %d %d %d %d %d\n",
				i,
				apostle[i].number,
				apostle[i].element,
				apostle[i].x,
				apostle[i].y,
				apostle[i].z,
				apostle[i].neighbor[0],
				apostle[i].neighbor[1],
				apostle[i].neighbor[2],
				apostle[i].neighbor[3],
				apostle[i].type);


		}

/*	原子全員の境界情報を調べる	*/

/*	bound[100][3] と bounds[100] 初期化	*/

		for( num = 0; num < 100 ; num++){

			bounds[num] = 0;

/*	初期化されてるってば
			printf("bounds[%d] = %d\n",
				num,bounds[num]);
*/

			for ( cnt = 0; cnt < 3; cnt++) {

			bound[num][cnt] = 0;

/*	初期化されてるってば
			printf("bound[%d][%d] = %d\n",
				num,cnt,bound[num][cnt]);
*/

			}
		}

/*	3軸方向に関して境界であるかどうかの判定	*/


		for( num = 0; num < the_MAX_ATOMS; num++){


/*	原子番号確認　*/
	fprintf( f_log, "---\n%d th story\n",num);
//			printf("---\n%d th story\n",num);



/*	読んだデータのテスト　ここから	
			printf("apostle[%d].x = %lf\n",
				num,apostle[num].x);
			printf("apostle[%d].y = %lf\n",
				num,apostle[num].y);
			printf("apostle[%d].z = %lf\n",
				num,apostle[num].z);

			printf("apostle[%d].type(before culc_boundary) = %d\n",
				num,apostle[num].type);
	読んだデータのテスト　ここまで */

/*	Si-Si間距離の半分以下の距離に○面があったら境界　でいい？ */

		if( apostle[num].x < (ATOMIC_DISTANCE_Si_Si * 0.5) * 0.9999  ){

			bound[num][0] = 1;
			bounds[num]++;
/* x境界　確認	*/
			fprintf( f_log, "x:boundary\n");
			fprintf( f_log, "%lf < %lf\n", 
				apostle[num].x, (ATOMIC_DISTANCE_Si_Si * 0.5) * 0.9999);
			fprintf( f_log, "bound[%d][0] = %d\n", num,bound[num][0]);

		}

		if( apostle[num].y < (ATOMIC_DISTANCE_Si_Si * 0.5) * 0.9999 ){

			bound[num][1] = 1;
			bounds[num]++;
/* y境界　確認	*/

			fprintf( f_log, "apostle[%d].y = %lf\n", num, apostle[num].y);
			fprintf( f_log, "%lf < %lf\n", 
				apostle[num].y, (ATOMIC_DISTANCE_Si_Si * 0.5) * 0.9999);
			fprintf( f_log, "bound[%d][1] = %d\n",	num,bound[num][1]);
		}

		if( apostle[num].z < (ATOMIC_DISTANCE_Si_Si * 0.5) * 0.9999 ){

			bound[num][2] = 1;
			bounds[num]++;


/* z境界　確認	*/
			fprintf( f_log, "apostle[%d].z = %lf\n", num, apostle[num].z);
			fprintf( f_log, "%lf < %lf\n", 
				apostle[num].z, (ATOMIC_DISTANCE_Si_Si * 0.5) * 0.9999);
			fprintf( f_log, "bound[%d][2] = %d\n",num,bound[num][2]);
		}

/*	bounds[]に何が入ってるか 境界数	 */
			fprintf( f_log, "bounds[%d] = %d\n", num,bounds[num]);

/*	境界によって状態が変わるのだ	*/

	fprintf( f_log, "--judge the site\n");
	
	DebugPrint("culc_boundary's scenario starts\n");

	int i;

	switch( bounds[num] ) {

		case 3:
		/*	原点あたりにいる子	*/
		/*      △でも▽でもバルクの中	*/
		if( apostle[num].type == 5){

			DebugPrint("scenario A\n");
			fprintf( f_log, "scenario A\n");
			apostle[num].type = 0;

		} else if ( apostle[num].type == 2) {
		/*	2-hidradeはありえないはず */
		/*	1-(horizontal)hidradeにしている場合は▽でバルクの中 */
			DebugPrint("scenario B\n");
			fprintf( f_log, "scenario B\n");
			apostle[num].type = 0;

 		} else {

		/*	近所に誰もいなければ 1-horizontal hydride	*/
			DebugPrint("scenario C\n");
			fprintf( f_log, "scenario C\n");
			apostle[num].type = 2;

		}
			break;
		case 2:
		DebugPrint("scenario 2 start\n");

		if(bound[num][2] == 1 ){
		/*  x軸・z軸　または　y軸・z軸　が境界の場合	*/

			if( apostle[num].type == 5){
		/* naoko　に　3-hydride と判定されてて	*/

				if( fabs(apostle[num].x - apostle[apostle[num].neighbor[0]].x) < 0.1 
				    || fabs(apostle[num].y - apostle[apostle[num].neighbor[0]].y) < 0.1){
		/* お隣さんと　x座標またはy座標　が同じ 	*/
				/*
					printf("x: fabs(%lf - %lf)= %lf\n",
							apostle[num].x,
							apostle[apostle[num].neighbor[0]].x,
							fabs(apostle[num].x - apostle[apostle[num].neighbor[0]].x));
					printf("y: fabs(%lf - %lf)= %lf\n",
							apostle[num].y,
							apostle[apostle[num].neighbor[0]].y,
							fabs(apostle[num].y - apostle[apostle[num].neighbor[0]].y));
				*/
					apostle[num].type = 4;
					DebugPrint("scenario D\n");
					fprintf( f_log, "scenario D\n");

			/* お隣さんのz座標の方が高かったらバルク */
					if(apostle[num].z - apostle[apostle[num].neighbor[0]].z < 0){
					apostle[num].type = 0;
					DebugPrint("scenario (D+0.5)\n");
					fprintf( f_log, "scenario (D+0.5)\n");
					}

				}else{
		/* それ以外	*/
				/*
					printf("x: fabs(%lf - %lf)= %lf\n",
							apostle[num].x,
							apostle[apostle[num].neighbor[0]].x,
							fabs(apostle[num].x - apostle[apostle[num].neighbor[0]].x));
					printf("y: fabs(%lf - %lf)= %lf\n",
							apostle[num].y,
							apostle[apostle[num].neighbor[0]].y,
							fabs(apostle[num].y - apostle[apostle[num].neighbor[0]].y));
				*/
					apostle[num].type = 2;
					DebugPrint("scenario (D+1)\n");
					fprintf( f_log, "scenario (D+1)\n");

				}

			}else if( apostle[num].type == 4 || apostle[num].type == 3) {

		/*	2-hidradeだったらバルク */

				apostle[num].type = 0;
				DebugPrint("scenario E\n");
				fprintf( f_log, "scenario E\n");
				
 			}else if ( apostle[num].type == 2) {

		/*	1-(horizontal)hidradeの場合は場所による	*/

				if(apostle[num].x < (ATOMIC_DISTANCE_Si_Si /1.732)
					|| apostle[num].y < (ATOMIC_DISTANCE_Si_Si /1.732)){

				/*	x「か」yが原点の近くならバルク */
					apostle[num].type = 0;
					DebugPrint("scenario F\n");
				fprintf( f_log, "scenario F\n");
				
				}else{

				/*	それ以外はそのまま */
				 	apostle[num].type = 2;
					DebugPrint("scenario G\n");
					fprintf( f_log, "scenario G\n");
				}

 			} else {

		/*	近所に誰もいなければ 1-horizontal hydride	*/
				apostle[num].type = 2;
				DebugPrint("scenario H\n");
				fprintf( f_log, "scenario H\n");
			}

		}else{
		/*  x軸・y軸が境界のケース てっぺんに一個だけ残ってるとか	*/
		/*　▽で 3-hidride だったらそのまま	*/
				apostle[num].type = 5;
				DebugPrint("scenario I\n");
				fprintf( f_log, "scenario I\n");
		/*　△で 3-hidride だったら terrace	*/
			/* ご近所さんその0のx座標またはy座標が自分のより大ならこの形	*/

			if( apostle[num].x < apostle[apostle[num].neighbor[0]].x
				|| apostle[num].y < apostle[apostle[num].neighbor[0]].y ){

				apostle[num].type = 1;
				DebugPrint("scenario J\n");
				fprintf( f_log, "scenario J\n");
			}
		}

			break;
		case 1:

			DebugPrint("scenario 1\n");

			if( apostle[num].type == 1 || apostle[num].type == 2){
			/* 1-hidrided ならばバルク	*/
				apostle[num].type = 0;
				DebugPrint("scenario K\n");
				fprintf( f_log, "scenario K\n");

			}else if( bound[num][2] == 1){
			/* z軸にだけ接している時　*/

			if( apostle[num].type == 7 ){
			/* 近所がいない時　*/
				if( apostle[num].z > (ATOMIC_DISTANCE_Si_Si /1.732) * 0.5){
				/* z座標が0.5より大だったら terrace */
					apostle[num].type = 1;
					DebugPrint("scenario L\n");
					fprintf( f_log, "scenario L\n");
				
				}else{
				/* z座標が0.5より小だったら 3-hidride */
					apostle[num].type = 5;
					DebugPrint("scenario M\n");
					fprintf( f_log, "scenario M\n");
				}
			}

			/* 3-hidride → 2-vertical hydride */
				if( apostle[num].type == 5 ){
					apostle[num].type = 4;
					DebugPrint("scenario (N-1)\n");
					fprintf( f_log, "scenario (N-1)\n");
				}

			/* 2-vertical hydride　→　1-(horizontal) hidride */
				if( apostle[num].type == 4 ){
//					do nothing
//					apostle[num].type = 2;
					DebugPrint("scenario N\n");
					fprintf( f_log, "scenario N\n");
				}

			/* 2-horizontal hidride	→ 1-(horizontal) hidride　*/
				if( apostle[num].type == 3 ){
					apostle[num].type = 2;
					DebugPrint("scenario (N+1)\n");
					fprintf( f_log, "scenario (N+1)\n");
				}

			/* 1-(horizontal) hidride　→　ありえない　*/
			/* terrace　→　ありえない　*/
			/* 表面にいない　→　ありえないと思うがそのまま　*/
				if( apostle[num].type == 0 ){
					apostle[num].type = 0;
					DebugPrint("scenario O\n");
					fprintf( f_log, "scenario O\n");
				}

			}else if( bound[num][2] == 0){
			/* z軸に接していない時 横だけ見る */ 

			/* 近所がいない時　ここでは考えてない　*/
			/* 3-hidride → 2-vertical hydride */
                                if( apostle[num].type == 5 ){
                                        apostle[num].type = 4;
                                        DebugPrint("scenario P-1\n");
                                        fprintf( f_log, "scenario P-1\n");
                                }

			/* 2-horizontal hidride	→　バルク */
				else if( apostle[num].type == 3 ){
					apostle[num].type = 0;
					DebugPrint("scenario P\n");
					fprintf( f_log, "scenario P\n");
				}
			/* 2-vertical hydride　→　terrace */
				else if( apostle[num].type == 4 ){
					apostle[num].type = 1;
					DebugPrint("scenario Q\n");
					fprintf( f_log, "scenario Q\n");
				}
			/* 1-(horizontal) hidride　→　バルク　*/
				else if( apostle[num].type == 2 ){
					apostle[num].type = 0;
					DebugPrint("scenario R\n");
					fprintf( f_log, "scenario R\n");
				}
			/* terrace　→　ありえないと思うがそのまま　*/
				else if( apostle[num].type == 1 ){
					apostle[num].type = 1;
					DebugPrint("scenario S\n");
					fprintf( f_log, "scenario S\n");
				}
			/* 表面にいない　→　ありえないと思うがそのまま　*/
				else if( apostle[num].type == 0 ){
					apostle[num].type = 0;
					DebugPrint("scenario T\n");
					fprintf( f_log, "scenario T\n");
				}
			}

			break;
		default:
		/*	境界ではない	*/
			DebugPrint("scenario Z\n");
			DebugPrint("Not on boundary\n");
			fprintf( f_log, "scenario Z\n");
		/*	naokoさんのtype判定がそのまんま	*/

			break;
		}

		/*	ゴミが入っていたときだけゴミゾンビ化	*/
			if(apostle[num].z < 0){

				fprintf( f_log, "gomi iri kiken\n");
				apostle[num].type = -255;
			}

		fprintf( f_log, "apostle[%d].type(after culc_boundary) = %d\n",num,apostle[num].type);

		}


		for( i=0; i < the_MAX_ATOMS  ; i++){

		fprintf(f_log, "%d %d %s %lf %lf %lf %d %d %d %d %d \n",
				i,
				apostle[i].number,
				apostle[i].element,
				apostle[i].x,
				apostle[i].y,
				apostle[i].z,
				apostle[i].neighbor[0],
				apostle[i].neighbor[1],
				apostle[i].neighbor[2],
				apostle[i].neighbor[3],
				apostle[i].type);


	}

	DebugPrint("return from calc_boundary\n");

	fclose(f_log);

	return 0;
}

int etch_or_not(int iteration_number, int the_MAX_ATOMS ){

	FILE *f_log;
	FILE *f_output;
	FILE *f_output_all;
	
	char log[] = "log.txt";
	char output_xyz[256];
	char output_all[256];
	
	int etched = 0;

	f_log=fopen(log,"a");

	printf("\n// inside etch_or_not \n\n");

	fprintf(f_log,"\n// inside etch_or_not \n\n");
	
	printf("iteration_number = %d\n\n",iteration_number);

//	全原子に対してエッチングするかどうか判定

	int i;
	int etch_or_not = 0;	/* エッチングされる:1 エッチングされない:0 */
	double p_etch = 0.0;	/*	エッチング確率初期化	*/

	for( i=0; i < the_MAX_ATOMS  ; i++){

//	乱数の確認

	fprintf( f_log, "this DICE = %dth %lf\n", DICE_NUMBER[iteration_number * MAX_ATOMS + i], DICE[iteration_number][i] );

	fprintf(f_log, "%d %d %s %lf %lf %lf %d %d %d %d %d \n",
				i,
				apostle[i].number,
				apostle[i].element,
				apostle[i].x,
				apostle[i].y,
				apostle[i].z,
				apostle[i].neighbor[0],
				apostle[i].neighbor[1],
				apostle[i].neighbor[2],
				apostle[i].neighbor[3],
				apostle[i].type);

//	PROBABILITY[6]	= 0.1500;	/* kinkのSiのetch確率 		*/
//	PROBABILITY[5]	= 0.1000;	/* 3個のHで終端されたSiのetch確率 	*/
//	PROBABILITY[4]	= 0.0500;	/* 横向きにH2個で終端されたSiのetch確率 */
//	PROBABILITY[3]	= 0.0800;	/* 横と上向きにH2個で終端されたSiのetch確率 */
//	PROBABILITY[2]	= 0.0100;	/* 1個のHで終端されたSiのetch確率	*/
//	PROBABILITY[1]	= 0.0050;	/* terraceのSiのetch確率	*/


	switch (apostle[i].type)
	{
	case 7:
		p_etch =	PROBABILITY[1]; //　境界に張り付いて残った原子をとりあえずterrace扱い
		break;
	case 6:
		p_etch = 	PROBABILITY[6];
		break;
	case 5:
		p_etch = 	PROBABILITY[5];
		break;
	case 4:
		p_etch = 	PROBABILITY[4];
		break;
	case 3:
		p_etch = 	PROBABILITY[3];
		break;
	case 2:
		p_etch = 	PROBABILITY[2];
		break;
	case 1:
		p_etch =	PROBABILITY[1];
		break;
	case 0:
		p_etch = 	0;
		break;
	case -1:
		p_etch = 	0;
		break;
	case -255:
		p_etch = 	0;
		break;		
	default:
		break;
	}

/*	念のためエッチング確率の中身	*/
	fprintf(f_log,"p_etch=%lf, DICE[%d][%d]=%lf\n", p_etch, iteration_number, i , DICE[iteration_number][i] );

	int do_etch = 0;

	if (DICE[iteration_number][i] <= p_etch )
	{
		fprintf(f_log,"Let's etching.\n");

	/*	エッチングされる場合の処理 */
	/*	なんだってゾンビになるのだ	*/

		etched++;

		apostle[i].x = -5.0;
		apostle[i].y = -5.0;
		apostle[i].z = -5.0;
		apostle[i].neighbor[0] = -255;
		apostle[i].neighbor[1] = -255;
		apostle[i].neighbor[2] = -255;
		apostle[i].neighbor[3] = -255;
		apostle[i].type = -1;

        fprintf(f_log, "%d %d %s %lf %lf %lf %d %d %d %d %d \n",
                i,
                apostle[i].number,
                apostle[i].element,
                apostle[i].x,
                apostle[i].y,
                apostle[i].z,
                apostle[i].neighbor[0],
                apostle[i].neighbor[1],
                apostle[i].neighbor[2],
                apostle[i].neighbor[3],
                apostle[i].type);

	}
	else
	{
		fprintf(f_log,"Let's nothing.\n");
	}


// 原子全部に対して繰り返すforループの終わり
	}


/* 結果を出力する */

	sprintf(output_xyz, "output%d.xyz", iteration_number);
	printf("output_xyz = %s\n", output_xyz);
	
	sprintf(output_all, "output_all_%d.txt", iteration_number);
	printf("output_all = %s\n", output_all);


		f_output=fopen(output_xyz,"w");
		f_output_all=fopen(output_all,"w");

		fprintf(f_output,"%d\n\n", the_MAX_ATOMS);

 		for( i=0; i < the_MAX_ATOMS  ; i++){


			fprintf(f_output, "%s %lf %lf %lf\n",

					apostle[i].element,
					apostle[i].x,
					apostle[i].y,
					apostle[i].z);

			fprintf(f_output_all, "%d %s %lf %lf %lf %d %d %d %d %d\n",

                	apostle[i].number,
                	apostle[i].element,
                	apostle[i].x,
                	apostle[i].y,
                	apostle[i].z,
                	apostle[i].neighbor[0],
                	apostle[i].neighbor[1],
                	apostle[i].neighbor[2],
                	apostle[i].neighbor[3],
                	apostle[i].type);

		}


	DebugPrint("return from etch_or_not\n");

	fclose(f_output);
	fclose(f_output_all);
	fclose(f_log);

	return etched;

}

