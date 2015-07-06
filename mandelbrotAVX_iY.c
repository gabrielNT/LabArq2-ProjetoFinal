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
-------------------------------------------------
3.Versao otimizada com inline assembly, com o conjunto de instrucoes AVX. Essa versao
paralelizou o loop em iY.
*/

#include <stdio.h>
#include <math.h>
#include <immintrin.h>

int main()
{
	/* screen ( integer) coordinate */
	int iX, iY;
	const int iXmax = 16384;
	const int iYmax = 16384;

	/* world ( double) coordinate = parameter plane*/
	// A variavel Cy foi extendida para um vetor, para aproveitar o uso dos registradores avx
	float Cx, _Cy[8];
	const float CxMin = -2.5;
	const float CxMax = 1.5;
	const float CyMin = -2.0;
	const float CyMax = 2.0;
	
	float PixelWidth = (CxMax - CxMin) / iXmax;
	float PixelHeight = (CyMax - CyMin) / iYmax;
	
	/* color component ( R or G or B) is coded from 0 to 255 */
	/* it is 24 bit color RGB file */
	const int MaxColorComponentValue = 255;
	FILE * fp;
	char *filename = "_simd_avx_iY.ppm";
	static unsigned char color[3];
	
	/* Z=Zx+Zy*i  ;   Z0 = 0 */
	double Zx, Zy;
	double Zx2, Zy2; /* Zx2=Zx*Zx;  Zy2=Zy*Zy  */
	
	int Iteration;
	const int IterationMax = 256;

	/* bail-out value , radius of circle ;  */
	const double EscapeRadius = 2;
	double ER2 = EscapeRadius*EscapeRadius;
	
	/*create new file,give it a name and open it in binary mode  */
	fp = fopen(filename, "wb"); /* b -  binary mode */
	
	/*write ASCII header to the file*/
	fprintf(fp, "P6\n %d\n %d\n %d\n", iXmax, iYmax, MaxColorComponentValue);
	
	// Gera um vetor com oito palavras de 32 bits, com valor PixelWidth por meio de funcoes intrinsecas
	__m256 PixelHeight256 = _mm256_set1_ps(PixelHeight);
	__m256 CyMin256 = _mm256_set1_ps(CyMin);

	/* compute and write image data bytes to the file*/
	// Loop paralelizado(são feitas 8 iteracoes simultaneamente)
	for (iY = 0; iY<iYmax/8; iY++)
	{
		// Gera os indices e coloca em simdIy
		float avxIy[8];
		for (int i = 0; i < 8; i++)
			avxIy[i] = iY * 8.0 + i;

		//Cy = CyMin + iY*PixelHeight
		_asm{
			vmovups ymm5, avxIy
			vmulps ymm5, ymm5, PixelHeight256
			vaddps ymm5, ymm5, CyMin256
			vmovups _Cy, ymm5
		}

		for (int i = 0; i < 8; i++){
			if (fabs(_Cy[i]) < PixelHeight / 2) _Cy[i] = 0.0; /* Main antenna */
			
			for (iX = 0; iX < iXmax; iX++)
			{
				Cx = CxMin + iX*PixelWidth;
				/* initial value of orbit = critical point Z= 0 */
				Zx = 0.0;
				Zy = 0.0;
				Zx2 = Zx*Zx;
				Zy2 = Zy*Zy;

				for (Iteration = 0; Iteration < IterationMax && ((Zx2 + Zy2) < ER2); Iteration++)
				{
					Zy = 2 * Zx*Zy + _Cy[i];
					Zx = Zx2 - Zy2 + Cx;
					Zx2 = Zx*Zx;
					Zy2 = Zy*Zy;
				};
				/* compute  pixel color (24 bit = 3 bytes) */
				if (Iteration == IterationMax)
				{ /*  interior of Mandelbrot set = black */
					color[0] = 0;
					color[1] = 0;
					color[2] = 0;
				}
				else
				{ /* exterior of Mandelbrot set = white */
					color[0] = ((IterationMax - Iteration) % 8) * 63;  /* Red */
					color[1] = ((IterationMax - Iteration) % 4) * 127;  /* Green */
					color[2] = ((IterationMax - Iteration) % 2) * 255;  /* Blue */
				};
				/*write color to the file*/
				fwrite(color, 1, 3, fp);
			}
		}
	}
	fclose(fp);

	return 0;
}