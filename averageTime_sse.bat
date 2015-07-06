@echo off

cl  mandelbrotSSE_iX.c
ECHO Start Measure - mandelbrotSSE_iX %Time% >> timer_sse.txt
FOR /L %%G IN (1,1,5) DO (
	mandelbrotSSE_iX.exe
)
ECHO Stop Measure - mandelbrotSSE_iX %TIME% >> timer_sse.txt

cl  mandelbrotSSE_iX_1.c
ECHO Start Measure - mandelbrotSSE_iX_1 %Time% >> timer_sse.txt
FOR /L %%G IN (1,1,5) DO (
	mandelbrotSSE_iX_1.exe
)
ECHO Stop Measure - mandelbrotSSE_iX_1 %TIME% >> timer_sse.txt

cl  mandelbrotSSE_iX_2.c
ECHO Start Measure - mandelbrotSSE_iX_2 %Time% >> timer_sse.txt
FOR /L %%G IN (1,1,5) DO (
	mandelbrotSSE_iX_2.exe
)
ECHO Stop Measure - mandelbrotSSE_iX_2 %TIME% >> timer_sse.txt

cl  mandelbrotSSE_iX_3.c
ECHO Start Measure - mandelbrotSSE_iX_3 %Time% >> timer_sse.txt
FOR /L %%G IN (1,1,5) DO (
	mandelbrotSSE_iX_3.exe
)
ECHO Stop Measure - mandelbrotSSE_iX_3 %TIME% >> timer_sse.txt



pause