@echo off

call "c:\Program Files (x86)\Microsoft Visual Studio 12.0\vc\vcvarsall.bat" x86

cl  mandelbrot.c
ECHO Start Measure - mandelbrot %Time% >> timer.txt
FOR /L %%G IN (1,1,5) DO (
	mandelbrot.exe
)
ECHO Stop Measure - mandelbrot %TIME% >> timer.txt

cl  mandelbrotSSE_iX.c
ECHO Start Measure - mandelbrotSSE_iX %Time% >> timer.txt
FOR /L %%G IN (1,1,5) DO (
	mandelbrotSSE_iX.exe
) 
ECHO Stop Measure - mandelbrotSSE_iX %TIME% >> timer.txt

cl  mandelbrotSSE_iY.c
ECHO Start Measure - mandelbrotSSE_iY %Time% >> timer.txt
FOR /L %%G IN (1,1,5) DO (
	mandelbrotSSE_iY.exe
) 
ECHO Stop Measure - mandelbrotSSE_iY %TIME% >> timer.txt

cl  mandelbrotSSE_iX_iY.c
ECHO Start Measure - mandelbrotSSE_iX_iY %Time% >> timer.txt
FOR /L %%G IN (1,1,5) DO (
	mandelbrotSSE_iX_iY.exe
) 
ECHO Stop Measure - mandelbrotSSE__iX_iY %TIME% >> timer.txt


cl  mandelbrotAVX_iX.c
ECHO Start Measure - mandelbrotAVX_iX %Time% >> timer.txt
FOR /L %%G IN (1,1,5) DO (
	mandelbrotAVX_iX.exe
) 
ECHO Stop Measure - mandelbrotSSEAVX_iX %TIME% >> timer.txt

cl /arch:AVX  mandelbrotAVX_iY.c
ECHO Start Measure - mandelbrotAVX_iY %Time% >> timer.txt
FOR /L %%G IN (1,1,5) DO (
	mandelbrotAVX_iY.exe
) 
ECHO Stop Measure - mandelbrotAVX_iY %TIME% >> timer.txt

cl /arch:AVX  mandelbrotAVX_iX_iY.c
ECHO Start Measure - mandelbrotAVX_iX_iY %Time% >> timer.txt
FOR /L %%G IN (1,1,5) DO (
	mandelbrotAVX_iX_iY.exe
) 
ECHO Stop Measure - mandelbrotAVX_iX_iY %TIME% >> timer.txt

pause