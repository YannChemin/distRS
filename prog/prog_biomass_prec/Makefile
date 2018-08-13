bp: bp.c
#	gcc -o bp bp.c biomass.c solar_day.c rs.c -I/c/msys/local/include   -L/c/msys/local/lib  -llibgdal -lm
	gcc -o bp bp.c biomass.c solar_day.c -I/usr/include/gdal/ -L/usr/lib -lgdal -lm -Wall -fopenmp -Wno-unused-result

clean:
	rm -rf bp
