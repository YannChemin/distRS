ndvi: ndvi.c
#	gcc -o eta gdal_eta.c rs.c -I/c/msys/local/include   -L/c/msys/local/lib  -llibgdal -lm
	gcc -o ndvi ndvi.c -I/usr/include/gdal/ -L/usr/lib -lgdal -lm -Wall -fopenmp -Wno-unused-result

clean:
	rm -rf ndvi
