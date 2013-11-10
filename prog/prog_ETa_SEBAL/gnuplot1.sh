#/usr/bin/gnuplot
set yrange [0:100]
plot "logold4.dat" using 1:2 title 'iter3     U2m=4     Score [%]' with lines , \
     "logold3.dat" using 1:2 title 'iter10   U2m=4     Score [%]' with lines , \
     "logold2.dat" using 1:2 title 'iter20   U2m=4     Score [%]' with lines , \
     "logold1.dat" using 1:2 title 'iter100 U2m=4     Score [%]' with lines , \
     "logold.dat" using 1:2 title 'iter100 U2m=20  Score [%]' with lines , \
     "log.dat" using 1:2 title 'iter100 U2m=100 Score [%]' with lines lt -1 lw 2 
reread