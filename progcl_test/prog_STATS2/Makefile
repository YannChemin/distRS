stats: stats.c
#	gcc -o eta gdal_eta.c rs.c -I/c/msys/local/include   -L/c/msys/local/lib  -llibgdal -lm
	gcc -o stats stats.c -I/usr/include/gdal/ -L/usr/lib -lgdal -lm -Wall -fopenmp

clean:
	rm -rf stats
