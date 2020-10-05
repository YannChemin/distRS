ndvi: ndvi.c
#	gcc -o eta gdal_eta.c rs.c -I/c/msys/local/include   -L/c/msys/local/lib  -llibgdal -lm
	#/usr/bin/gcc -o ndvi ndvi.c -I/usr/include/gdal/ -L/usr/lib -lgdal -lm -Wall -fopenmp
	/usr/local/Cellar/gcc/9.3.0_1/bin/gcc-9 -fopenmp -o ndvi ndvi.c -I/usr/local/Cellar/gcc/9.3.0_1/lib/gcc/9/gcc/x86_64-apple-darwin18/9.3.0/include -L/usr/local/Cellar/gcc/9.3.0_1/lib/gcc/9 -I/Library/Frameworks/GDAL.framework/unix/include -L/Library/Frameworks/GDAL.framework/unix/lib -L/usr/lib -lgdal -lm -Wall -fopenmp

clean:
	rm -rf ndvi
