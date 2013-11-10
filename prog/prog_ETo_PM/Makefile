pm_eto: gdal_pm_eto.c pm_eto.c
#	gcc -o eta gdal_eta.c rs.c -I/c/msys/local/include   -L/c/msys/local/lib  -llibgdal -lm
	gcc -o pm_eto gdal_pm_eto.c pm_eto.c -I/usr/include/gdal/ -L/usr/lib -lgdal -lm -fopenmp -Wall


clean:
	rm -rf pm_eto
