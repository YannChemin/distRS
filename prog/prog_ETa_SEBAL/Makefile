sebal_eta: gdal_sebal_eta.c sebal_eta.c
#	gcc -o eta gdal_eta.c sebal_eta.c -I/c/msys/local/include   -L/c/msys/local/lib  -llibgdal -lm
	gcc -o sebal_eta gdal_sebal_eta.c sebal_eta.c -I/usr/include/gdal/ -L/usr/lib -lgdal -lm -fopenmp -Wall

clean:
	rm -rf sebal_eta
