ndvi: main.c arrays.c
	#gcc -o ndvi main.c arrays.c -L/usr/lib64 -L/opt/AMDAPP/lib/x86_64 -lOpenCL -I/opt/AMDAPP/include -I. -I/usr/include/gdal -lgdal -lm -Wall
	gcc -o ndvi main.c arrays.c -L/usr/lib64 -L/usr/lib/CL -lOpenCL -I/usr/lib/x86_64-linux-gnu -I. -I/usr/include/gdal -lgdal -lm -Wall -fno-stack-protector

clean:
	rm -rf ndvi
