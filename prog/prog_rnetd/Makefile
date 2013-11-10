r_netd: gdal_r_netd.c r_netd.c
#	gcc -o eta gdal_eta.c rs.c -I/c/msys/local/include   -L/c/msys/local/lib  -llibgdal -lm
	gcc -o r_netd gdal_r_netd.c r_netd.c -I/usr/include/gdal/ -L/usr/lib -lgdal -lm -fopenmp -Wall

clean:
	rm -rf r_netd
