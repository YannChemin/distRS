metric_eta: gdal_metric_eta.c metric_eta.c
#	gcc -o eta gdal_eta.c rs.c -I/c/msys/local/include   -L/c/msys/local/lib  -llibgdal -lm
	gcc -o metric_eta gdal_metric_eta.c metric_eta.c -I/usr/include/gdal/ -L/usr/lib -lgdal -lproj -lm -fopenmp -mavx -O2 -Wall


clean:
	rm -rf metric_eta
