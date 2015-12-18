alb: alb.c
	gcc -o alb alb.c -lm -I/usr/include/gdal -L/usr/lib -lgdal -lm -fopenmp -Wall -O2 -mavx
	#/sw/lib/gcc4.9/bin/gcc-4 -o alb alb.c -lm -I/sw/include/gdal1 -L/sw/lib -lgdal -lm -fopenmp -Wall

clean:
	rm -rf alb
