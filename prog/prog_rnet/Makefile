r_net: gdal_r_net.c r_net.c
#	gcc -o eta gdal_eta.c rs.c -I/c/msys/local/include   -L/c/msys/local/lib  -llibgdal -lm
	gcc -o r_net gdal_r_net.c r_net.c -I/usr/include/gdal/ -L/usr/lib -lgdal -lm -fopenmp -Wall

clean:
	rm -rf r_net
