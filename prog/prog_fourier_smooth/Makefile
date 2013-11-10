fourier: fourier.c main_fourier.c fillin.c movavg.c
	gcc -o fourier main_fourier.c fourier.c fillin.c movavg.c -I/usr/include/gdal/ -L/usr/lib -lgdal -lm -fopenmp -Wall


clean:
	rm -rf fourier
