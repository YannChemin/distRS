#/usr/bin/gnuplot
set yrange [0:100]
plot "log.dat" using 1:2 title 'Score [%]' with lines lt -1 lw 2 
reread