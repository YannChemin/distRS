#!/usr/bin/python
import numpy as np
import pylab as pb
#Set output size
pb.rcParams['figure.figsize'] = 10, 8

# col0, col1, col2, col3, col4, col5, col6
#Filename, pix count, pix sum, pix min, pix max, pix avg, pix stdev
#argv[1],count,sum,min,max,average,stdev

#average is col5

data = np.genfromtxt('/home/yann/cgET/Asia/pk/Hakra_stats.csv', delimiter=',')
t = np.arange(data.shape[0])
pb.xlim([0,data.shape[0]])
pb.plot(t, data[:,5], 'b.', label="EF")
pb.xlabel('Time (days)')
pb.ylabel('Evaporative Fraction (-)')
pb.title('Mean Evaporative Fraction over Hakra System (2000-now)')
pb.grid(True)
pb.savefig("Hakra_ef_plot.png", dpi=100)
pb.show() 
