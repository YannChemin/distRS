lst: lst.c arrays.c
#	gcc -o lst lst.c arrays.c -I/c/msys/local/include   -L/c/msys/local/lib  -llibgdal -lm
	gcc -o lst lst.c arrays.c -I/usr/include/gdal/ -L/usr/lib -lgdal -lm -Wall -fopenmp

clean:
	rm -rf lst
