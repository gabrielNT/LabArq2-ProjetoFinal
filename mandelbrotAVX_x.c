/*
Modified from: http://rosettacode.org/wiki/Mandelbrot_set#PPM_non_interactive
c program:
--------------------------------
1. draws Mandelbrot set for Fc(z)=z*z +c
using Mandelbrot algorithm ( boolean escape time )
-------------------------------
2. technique of creating ppm file is  based on the code of Claudio Rocchini
http://en.wikipedia.org/wiki/Image:Color_complex_plot.jpg
create 24 bit color graphic file ,  portable pixmap file = PPM
see http://en.wikipedia.org/wiki/Portable_pixmap
to see the file use external application ( graphic viewer)
*/
#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <time.h>

int main()
{
	//Variaveis para medir o tempo
	clock_t begin, end;
	double time_spent;

	begin = clock();

	/* screen ( integer) coordinate */
	int iX, iY;
	const int iXmax = 16384;
	const int iYmax = 16384;
	/* world ( double) coordinate = parameter plane*/
	float _Cx[8], Cy;
	const float CxMin = -2.5;
	const float CxMax = 1.5;
	const float CyMin = -2.0;
	const float CyMax = 2.0;
	/* */
	float PixelWidth = (CxMax - CxMin) / iXmax;
	float PixelHeight = (CyMax - CyMin) / iYmax;
	/* color component ( R or G or B) is coded from 0 to 255 */
	/* it is 24 bit color RGB file */
	const int MaxColorComponentValue = 255;
	FILE * fp;
	char *filename = "mandelbrot.ppm";
	static unsigned char color[3];
	//int vetorCoresAux[8];
	/* Z=Zx+Zy*i  ;   Z0 = 0 */
	float Zx, Zy;
	float Zx2, Zy2; /* Zx2=Zx*Zx;  Zy2=Zy*Zy  */
	/*  */
	int Iteration[8];
	//float IterationF[8];
	const int IterationMax = 256;
	//const float IterationMaxF = 256.0;
	/* bail-out value , radius of circle ;  */
	const float EscapeRadius = 2;
	float ER2 = EscapeRadius*EscapeRadius;
	/*create new file,give it a name and open it in binary mode  */
	fp = fopen(filename, "wb"); /* b -  binary mode */
	/*write ASCII header to the file*/
	fprintf(fp, "P6\n %d\n %d\n %d\n", iXmax, iYmax, MaxColorComponentValue);

	// Funcao intrinseca: carrega float em variável para AVX (256 bits)
	__m256 PixelWidth256 = _mm256_set1_ps(PixelWidth);
	__m256 CxMin256 = _mm256_set1_ps(CxMin);
	__m256 IterationMax256 = _mm256_set1_ps(IterationMax);
	
	for (iY = 0; iY<iYmax; iY++){
		
		Cy = CyMin + iY*PixelHeight;
		
		if (fabs(Cy)< PixelHeight / 2) Cy = 0.0; /* Main antenna */
			for (iX = 0; iX<iXmax / 8; iX++){
				float avxIx[8];
				for (int i = 0; i < 8; i++)
					avxIx[i] = iX * 8.0 + i;

				_asm{
						vmovups ymm5, avxIx
						vmulps ymm5, ymm5, PixelWidth256
						vaddps ymm5, ymm5, CxMin256    // ymm5 = CxMin + iX*PixelWidth
						vmovups _Cx, ymm5
				}

				for (int i = 0; i < 8; i++){
					Zx = 0;
					Zy = 0;
					Zx2 = 0;
					Zy2 = 0;

					for (Iteration[i] = 0; Iteration[i] < IterationMax && ((Zx2 + Zy2) < ER2); Iteration[i]++){
						Zy = 2 * Zx * Zy + Cy;
						Zx = Zx2 - Zy2 + _Cx[i];
						Zx2 = Zx * Zx;
						Zy2 = Zy * Zy;
					}

					if (Iteration[i] == IterationMax){ /*  interior of Mandelbrot set = black */
						color[0] = 0;
						color[1] = 0;
						color[2] = 0;
					}
					else{ /* exterior of Mandelbrot set = white */
						color[0] = ((IterationMax - Iteration[i]) % 8) * 63;  /* Red */
						color[1] = ((IterationMax - Iteration[i]) % 4) * 127;  /* Green */
						color[2] = ((IterationMax - Iteration[i]) % 2) * 255;  /* Blue */
					};
					fwrite(color, 1, 3, fp);
				}
			}
	}

	fclose(fp);

	end = clock();
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;;
	printf("%f", time_spent);

	return 0;
}