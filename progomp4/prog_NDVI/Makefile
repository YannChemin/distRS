ndvi: ndvi.c
	/usr/bin/gcc -o ndvi ndvi.c -I/usr/include/gdal/ -L/usr/lib -lgdal -lm -Wall -fopenmp -foffload=nvptx-none

clean:
	rm -rf ndvi
