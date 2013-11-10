hist: hist.c
#	gcc -o eta gdal_eta.c rs.c -I/c/msys/local/include   -L/c/msys/local/lib  -llibgdal -lm
	gcc -o hist hist.c -I/usr/include/gdal/ -L/usr/lib -lgdal -lm -Wall -fopenmp

clean:
	rm -rf hist
