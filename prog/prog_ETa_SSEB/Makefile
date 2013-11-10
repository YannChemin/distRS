sseb_eta: gdal_sseb_eta.c sseb_eta.c
#	gcc -o eta gdal_eta.c rs.c -I/c/msys/local/include   -L/c/msys/local/lib  -llibgdal -lm
	gcc -o sseb_eta gdal_sseb_eta.c sseb_eta.c -I/usr/include/gdal/ -L/usr/lib -lgdal -lm -fopenmp -Wall


clean:
	rm -rf sseb_eta
