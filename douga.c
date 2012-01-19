/*************************************************************************************************/
/*                                          douga.c                                    */
/*       xyz形式データからgnuplotで3D描画する。オリジナル、ありがと＞福井君          */
/*************************************************************************************************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <string.h>

/* 入出力データファイル間隔 */
#define INITIALSTEP	 	 		 0 			/* 開始ファイルステップ数  */
#define FINALSTEP 				 1499 		/* 終了ファイルステップ数 */
#define DSTEP 		 			   1  		/* ステップ数刻み */ 


#define TIMESTEP 							1.0e-16     	 /* time step [s] */

/* 出力グラフ */
#define SIZEX								 1.0				/* 0.6 */
#define SIZEY								 1.0				/* 0.6 */
#define RATIO								 0.0				/* '0.0'= 指定なし, otehr = 指定サイズ */
#define VIEWX								80.0   			/* 57 60.0 74.0*/
#define VIEWY								50.0		  	/* 50 312.0 320.0*/
#define XLABELX							 0.0				/* -2.0 */
#define XLABELY							 -1.0				/* -1.0 */
#define YLABELX							 0.0				/* -0.6 */
#define YLABELY							 -0.5				/* -0.4 */
#define ZLABELX							 10.0				/* 5.0 */
#define ZLABELY							 9.0				/* 5.0 */

#define XRANGEH 					15.0
#define XRANGEL						0.0
#define YRANGEH 					15.0
#define YRANGEL						0.0
#define ZRANGEH 					10.0
#define ZRANGEL						0.0

#define FILENAME					0  	/* '0' = 出力ファイルなしでグラフ化, '1' = 00001から連番の数字(png), '4 = step(png)'*/

/*************************************************************************************************/
/*                                     main                                              */
/*************************************************************************************************/
int main(void)
{
	int i, j, k, namen0, namen1, datastep, order;
	int n = 1;
	char infile[50] = "output"; //読み込むxyzデータファイルのファイル名の頭半分
	char outfile[50] = ""; // pngデータの出力ファイル名
	FILE *gp, *f_in, *f_out;

		for(datastep = INITIALSTEP; datastep <= FINALSTEP; datastep += DSTEP){
			gp = popen("/usr/bin/gnuplot", "w"); /* gnuplotの場所 */

			if(gp == NULL){
				printf("Can't find gnuplot !\n"); //機能しないでパスがコマンドとして認識されていないとでる。
				exit(-1);
			}

		/* 入力ファイルと出力ファイル */

		sprintf(infile,"output%d.xyz", datastep);
		sprintf(outfile,"%05d.png",datastep);

		f_in = fopen(infile, "r");
			if(f_in == NULL){
				printf("Can't open %s !\n", infile); //機能しないでパスがコマンドとして認識されていないとでる。
				exit(-1);
			}
			
		f_out = fopen(outfile,"w");
			if(f_in == NULL){
				printf("Can't open %s !\n", outfile); //機能しないでパスがコマンドとして認識されていないとでる。
				exit(-1);
			}


		/* gnuplotで出力する図の設定 */
			fprintf(gp, "set ticslevel 0\n");			// XY平面を最低面にする
			fprintf(gp, "set size %f, %f\n", SIZEX, SIZEY);			// グラフの大きさ（これでラベルの大きさを調整）
			fprintf(gp, "set view %f, %f\n", VIEWX, VIEWY);				// グラフの俯瞰方向	

			fprintf(gp, "set xrange [%lf:%lf]\n", XRANGEL,XRANGEH);
			fprintf(gp, "set yrange [%lf:%lf]\n", YRANGEL,YRANGEH);
			fprintf(gp, "set zrange [%lf:%lf]\n", ZRANGEL, ZRANGEH);

			/* 出力ファイルの拡張子設定 */
			fprintf(gp, "set terminal png\n");
			fprintf(gp, "set output \"%s\"\n", outfile);

			fprintf(gp, "set title \"%d th\"\n", datastep);
			fprintf(gp, 
					"splot \"< awk \'{if( $4 < 1.2) print $0}\' %s\" every ::2 using 2:3:4 pt 1,\"<awk \'{if ($4 > 1.2 && $4 < 4.8) print $0}\' %s\" every ::2 using 2:3:4 pt 2, \"<awk \'{if ($4 > 4.8 ) print $0}\' %s\" every ::2 using 2:3:4 pt 3\n" 
					, infile, infile, infile);

			fflush(gp); // バッファフラッシュ(グラフの様子確認用)
			fprintf(gp, "pause -1\n"); // コマンドでポーズ解除(グラフの様子確認用)
			

			pclose(f_in);
			pclose(f_out);
			pclose(gp); //gnuplotを閉じないで連続してグラフを出力すると関係ない文字列を出力しグラフ化に失敗する
		}

	return 0;
}
