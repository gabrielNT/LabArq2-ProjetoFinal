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
-------------------------------
3.Versao otimizada com inline assembly, com o conjunto de instrucoes SSE4_2. Essa versao
paralelizou o loop em iX(versão preliminar 2).
*/

#include <stdio.h>
#include <math.h>
#include <xmmintrin.h>

int main()
{
	/* screen ( integer) coordinate */
	int iX, iY;
	const int iXmax = 16384;
	const int iYmax = 16384;

	/* world ( double) coordinate = parameter plane*/
	// A variavel Cx foi extendida para um vetor, para aproveitar o uso dos registradores sse
	float _Cx[4], Cy;
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
	char *filename = "_simd_sse_iX_2.ppm";
	static unsigned char color[3];

	// Foi criado um vetor auxiliar onde sera feita parte do calculo de color[]
	int vetorCoresAux[4];

	/* Z=Zx+Zy*i  ;   Z0 = 0 */
	float Zx, Zy;
	float Zx2, Zy2; /* Zx2=Zx*Zx;  Zy2=Zy*Zy  */

	// A variavel Iteration foi extendida para um vetor, para aproveitar o uso dos registradores sse
	int Iteration[4];
	const int IterationMax = 256;

	/* bail-out value , radius of circle ;  */
	const float EscapeRadius = 2;
	float ER2 = EscapeRadius*EscapeRadius;

	/*create new file,give it a name and open it in binary mode  */
	fp = fopen(filename, "wb"); /* b -  binary mode */

	/*write ASCII header to the file*/
	fprintf(fp, "P6\n %d\n %d\n %d\n", iXmax, iYmax, MaxColorComponentValue);

	// Gera uma quatro palavras de 32 bits, com valor PixelWidth por meio de funcoes intrinsecas
	__m128 PixelWidth128 = _mm_set1_ps(PixelWidth);
	__m128 CxMin128 = _mm_set1_ps(CxMin);
	__m128 IterationMax128 = _mm_set1_ps(IterationMax);

	/* compute and write image data bytes to the file*/
	for (iY = 0; iY<iYmax; iY++)
	{
		Cy = CyMin + iY*PixelHeight;

		if (fabs(Cy)< PixelHeight / 2) Cy = 0.0; /* Main antenna */

		// Loop paralelizado(são feitas 4 iteracoes simultaneamente)
		for (iX = 0; iX<iXmax / 4; iX++)
		{
			float simdIx[4];
			for (int i = 0; i < 4; i++)
				simdIx[i] = iX * 4.0 + i;


			_asm{
				// xmm5 = CxMin + iX*PixelWidth
				movups xmm5, simdIx
					mulps xmm5, PixelWidth128
					addps xmm5, CxMin128
					movups _Cx, xmm5
			}

			// Apos recarregar na memoria, realiza essas operacoes em C
			// Nao e possivel paralelizar essa parte, pois cada iteracao do loop depende da anterior
			for (int i = 0; i < 4; i++){
				Zx = 0;
				Zy = 0;
				Zx2 = 0;
				Zy2 = 0;

				for (Iteration[i] = 0; Iteration[i] < IterationMax && ((Zx2 + Zy2) < ER2); Iteration[i]++)
				{
					Zy = 2 * Zx * Zy + Cy;
					Zx = Zx2 - Zy2 + _Cx[i];
					Zx2 = Zx * Zx;
					Zy2 = Zy * Zy;
				}
			}

			// Volta para asm, realizando apenas parte do calculo
			_asm{
				// xmm1 = color[0], xmm2 = color[1], xmm3 = color[2]
				movups xmm0, Iteration
					movups xmm4, IterationMax128
					movups xmm5, xmm0

					xorps xmm1, xmm1
					xorps xmm2, xmm2
					xorps xmm3, xmm3

					cmpneqps xmm0, xmm4
					psubd xmm4, xmm5
					andps xmm0, xmm4

					movups vetorCoresAux, xmm0
			}

			// Volta para C para poder realizar operacao modulo
			for (int i = 0; i < 4; i++){
				color[0] = (char)((vetorCoresAux[i] % 8) * 63);  /* Red */
				color[1] = (char)((vetorCoresAux[i] % 4) * 127);  /* Green */
				color[2] = (char)((vetorCoresAux[i] % 2) * 255);  /* Blue */

				/*write color to the file*/
				fwrite(color, 1, 3, fp);

			};
		}
	}
	fclose(fp);

	return 0;
}