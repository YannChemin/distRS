#/usr/bin/python
import numpy as np
import pylab as pb


#ETa
eta_sseb = np.genfromtxt("/home/yann/DATA/stats/eta_sseb.csv", delimiter=",", usecols = (6), autostrip=True)
eta_sebal = np.genfromtxt("/home/yann/DATA/stats/eta_sebal.csv", delimiter=",", usecols = (6), autostrip=True)
ta = np.genfromtxt("/home/yann/DATA/stats/ta.csv", delimiter=",", usecols = (6), autostrip=True)
eto_pm = np.genfromtxt("/home/yann/DATA/stats/eto_pm.csv", delimiter=",", usecols = (6), autostrip=True)
et_potd = np.genfromtxt("/home/yann/DATA/stats/et_potd.csv", delimiter=",", usecols = (6), autostrip=True)

eta_sseb[eta_sseb>10] = np.nan
eta_sebal[eta_sebal>10] = np.nan
ta[ta>5] = np.nan
eto_pm[eto_pm>10] = np.nan
et_potd[et_potd>10] = np.nan

#Plot ET
pb.figure(num=1,figsize = (11,5),dpi=80)
t = np.arange(len(eta_sseb))
pb.plot(t, eta_sseb, 'b-', label="ETa SSEB")
pb.plot(t, eta_sebal, 'g-', label="ETa SEBAL")
pb.plot(t, ta, 'r-', label="Ta")
pb.plot(t, eto_pm, 'y-', label="ETo PM")
pb.plot(t, et_potd, 'm-', label="ET Potential")
pb.legend(loc=(.75, 0.1))
pb.xlabel('Time (days)')
pb.ylabel('ETo/ETa/Ta/ETpot (mm/d)')
pb.title('Daily Evapotranspiration')
pb.grid(True)
pb.savefig('ET.png', dpi=300)


# Evapfr
evapfr_sseb = np.genfromtxt("/home/yann/DATA/stats/evapfr_sseb.csv", delimiter=",", usecols = (6), autostrip=True)
evapfr_sebal = np.genfromtxt("/home/yann/DATA/stats/evapfr_sebal.csv", delimiter=",", usecols = (6), autostrip=True)
#evapfr_metric = np.genfromtxt("/home/yann/DATA/stats/evapfr_metric.csv", delimiter=",", usecols = (6), autostrip=True)

evapfr_sseb[evapfr_sseb>1] = np.nan
evapfr_sebal[evapfr_sebal>1] = np.nan
#evapfr_metric[evapfr_metric>1] = np.nan

# Plot Evapfr
pb.figure(num=2,figsize = (11,5),dpi=80)
t = np.arange(len(evapfr_sseb))

pb.plot(t, evapfr_sseb, 'b-', label="SSEB")
pb.plot(t, evapfr_sebal, 'g-', label="SEBAL")
#pb.plot(t, evapfr_metric, 'r-', label="METRIC")
pb.legend(loc=(.75, 0.1))
pb.xlabel('Time (days from 2000.03.01)')
pb.ylabel('Evaporative Fraction (-)')
pb.title('Daily Evaporative Fraction')
pb.grid(True)
pb.savefig('Evapfr.png', dpi=300)
pb.show()


##Ta
#ta1 = np.genfromtxt("/home/yann/DATA/stats/part1/ta.csv", delimiter=",", usecols = (6), autostrip=True)
##ta1[ta1>5] = np.nan
#ta2 = np.genfromtxt("/home/yann/DATA/stats/part2/ta.csv", delimiter=",", usecols = (6), autostrip=True)
##ta2[ta2>5] = np.nan
#ta3 = np.genfromtxt("/home/yann/DATA/stats/part3/ta.csv", delimiter=",", usecols = (6), autostrip=True)
##ta3[ta3>5] = np.nan
#ta4 = np.genfromtxt("/home/yann/DATA/stats/part4/ta.csv", delimiter=",", usecols = (6), autostrip=True)
##ta4[ta4>5] = np.nan

##Plot Ta
#pb.figure(num=1,figsize = (11,5),dpi=80)
#t = np.arange(len(ta1))
#pb.plot(t, ta1, 'r-', label="Ta part 1")
#pb.plot(t, ta2, 'g-', label="Ta part 2")
#pb.plot(t, ta3, 'b-', label="Ta part 3")
#pb.plot(t, ta4, 'm-', label="Ta part 4")
#pb.legend(loc=(.75, 0.1))
#pb.xlabel('Time (days)')
#pb.ylabel('Ta (mm/d)')
#pb.title('Daily Transpiration (2010)')
#pb.grid(True)
#pb.savefig('/home/yann/DATA/stats/Ta.png', dpi=300)
#pb.show()

