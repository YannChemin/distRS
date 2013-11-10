sebs_eta: gdal_sebs_eta.c sebs_eta.c
#	gcc -o eta gdal_eta.c rs.c -I/c/msys/local/include   -L/c/msys/local/lib  -llibgdal -lm
	gcc -o sebs_eta gdal_sebs_eta.c sebs_eta.c -I/usr/include/gdal/ -L/usr/lib -lgdal -lm -fopenmp -Wall

clean:
	rm -rf sebs_eta
