@echo off

cl  mandelbrot.c
cl  mandelbrotSSE_iX.c
cl  mandelbrotSSE_iX_1.c
cl  mandelbrotSSE_iX_2.c
cl  mandelbrotSSE_iX_3.c
cl  mandelbrotSSE_iY.c
cl  mandelbrotSSE_iX_iY.c
cl /arch:AVX  mandelbrotAVX_iX.c
cl /arch:AVX  mandelbrotAVX_iY.c
cl /arch:AVX  mandelbrotAVX_iX_iY.c

echo Executando...

mandelbrot
mandelbrotSSE_iX
mandelbrotSSE_iX_1
mandelbrotSSE_iX_2
mandelbrotSSE_iX_3
mandelbrotSSE_iY
mandelbrotSSE_iX_iY
mandelbrotAVX_iX
mandelbrotAVX_iY
mandelbrotAVX_iX_iY

pause