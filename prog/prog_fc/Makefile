fc: fc.c
#	gcc -o eta gdal_eta.c rs.c -I/c/msys/local/include   -L/c/msys/local/lib  -llibgdal -lm
	#gcc -o fc fc.c -I/usr/include/gdal/ -L/usr/lib -lgdal -lm -Wall -fopenmp
	/usr/local/Cellar/gcc/10.2.0/bin/gcc-10 -fopenmp -o fc fc.c -I/usr/local/Cellar/gcc/10.2.0/lib/gcc/10/gcc/x86_64-apple-darwin18/10.2.0/include -L/usr/local/Cellar/gcc/10.2.0/lib/gcc/10 -I/Library/Frameworks/GDAL.framework/unix/include -L/Library/Frameworks/GDAL.framework/unix/lib -L/usr/lib -lgdal -lm -Wall -fopenmp


clean:
	rm -rf fc
