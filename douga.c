/*************************************************************************************************/
/*                                          douga.c                                    */
/*       xyz$B7A<0%G!<%?$+$i(Bgnuplot$B$G(B3D$BIA2h$9$k!#%*%j%8%J%k!"$"$j$,$H!dJ!0f7/(B          */
/*************************************************************************************************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <string.h>

/* $BF~=PNO%G!<%?%U%!%$%k4V3V(B */
#define INITIALSTEP	 	 		 0 			/* $B3+;O%U%!%$%k%9%F%C%W?t(B  */
#define FINALSTEP 				 1499 		/* $B=*N;%U%!%$%k%9%F%C%W?t(B */
#define DSTEP 		 			   1  		/* $B%9%F%C%W?t9o$_(B */ 


#define TIMESTEP 							1.0e-16     	 /* time step [s] */

/* $B=PNO%0%i%U(B */
#define SIZEX								 1.0				/* 0.6 */
#define SIZEY								 1.0				/* 0.6 */
#define RATIO								 0.0				/* '0.0'= $B;XDj$J$7(B, otehr = $B;XDj%5%$%:(B */
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

#define FILENAME					0  	/* '0' = $B=PNO%U%!%$%k$J$7$G%0%i%U2=(B, '1' = 00001$B$+$iO"HV$N?t;z(B(png), '4 = step(png)'*/

/*************************************************************************************************/
/*                                     main                                              */
/*************************************************************************************************/
int main(void)
{
	int i, j, k, namen0, namen1, datastep, order;
	int n = 1;
	char infile[50] = "output"; //$BFI$_9~$`(Bxyz$B%G!<%?%U%!%$%k$N%U%!%$%kL>$NF,H>J,(B
	char outfile[50] = ""; // png$B%G!<%?$N=PNO%U%!%$%kL>(B
	FILE *gp, *f_in, *f_out;

		for(datastep = INITIALSTEP; datastep <= FINALSTEP; datastep += DSTEP){
			gp = popen("/usr/bin/gnuplot", "w"); /* gnuplot$B$N>l=j(B */

			if(gp == NULL){
				printf("Can't find gnuplot !\n"); //$B5!G=$7$J$$$G%Q%9$,%3%^%s%I$H$7$FG'<1$5$l$F$$$J$$$H$G$k!#(B
				exit(-1);
			}

		/* $BF~NO%U%!%$%k$H=PNO%U%!%$%k(B */

		sprintf(infile,"output%d.xyz", datastep);
		sprintf(outfile,"%05d.png",datastep);

		f_in = fopen(infile, "r");
			if(f_in == NULL){
				printf("Can't open %s !\n", infile); //$B5!G=$7$J$$$G%Q%9$,%3%^%s%I$H$7$FG'<1$5$l$F$$$J$$$H$G$k!#(B
				exit(-1);
			}
			
		f_out = fopen(outfile,"w");
			if(f_in == NULL){
				printf("Can't open %s !\n", outfile); //$B5!G=$7$J$$$G%Q%9$,%3%^%s%I$H$7$FG'<1$5$l$F$$$J$$$H$G$k!#(B
				exit(-1);
			}


		/* gnuplot$B$G=PNO$9$k?^$N@_Dj(B */
			fprintf(gp, "set ticslevel 0\n");			// XY$BJ?LL$r:GDcLL$K$9$k(B
			fprintf(gp, "set size %f, %f\n", SIZEX, SIZEY);			// $B%0%i%U$NBg$-$5!J$3$l$G%i%Y%k$NBg$-$5$rD4@0!K(B
			fprintf(gp, "set view %f, %f\n", VIEWX, VIEWY);				// $B%0%i%U$NPmbWJ}8~(B	

			fprintf(gp, "set xrange [%lf:%lf]\n", XRANGEL,XRANGEH);
			fprintf(gp, "set yrange [%lf:%lf]\n", YRANGEL,YRANGEH);
			fprintf(gp, "set zrange [%lf:%lf]\n", ZRANGEL, ZRANGEH);

			/* $B=PNO%U%!%$%k$N3HD%;R@_Dj(B */
			fprintf(gp, "set terminal png\n");
			fprintf(gp, "set output \"%s\"\n", outfile);

			fprintf(gp, "set title \"%d th\"\n", datastep);
			fprintf(gp, 
					"splot \"< awk \'{if( $4 < 1.2) print $0}\' %s\" every ::2 using 2:3:4 pt 1,\"<awk \'{if ($4 > 1.2 && $4 < 4.8) print $0}\' %s\" every ::2 using 2:3:4 pt 2, \"<awk \'{if ($4 > 4.8 ) print $0}\' %s\" every ::2 using 2:3:4 pt 3\n" 
					, infile, infile, infile);

			fflush(gp); // $B%P%C%U%!%U%i%C%7%e(B($B%0%i%U$NMM;R3NG'MQ(B)
			fprintf(gp, "pause -1\n"); // $B%3%^%s%I$G%]!<%:2r=|(B($B%0%i%U$NMM;R3NG'MQ(B)
			

			pclose(f_in);
			pclose(f_out);
			pclose(gp); //gnuplot$B$rJD$8$J$$$GO"B3$7$F%0%i%U$r=PNO$9$k$H4X78$J$$J8;zNs$r=PNO$7%0%i%U2=$K<:GT$9$k(B
		}

	return 0;
}
