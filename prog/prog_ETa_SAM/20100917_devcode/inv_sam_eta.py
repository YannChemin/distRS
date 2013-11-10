# -*- coding: utf-8 -*-
#Usage: python -u sam_et.py

from math import *
from sam_eta_lib import *

# This is calibration code for the C factor of Albedo (see definition of C in Barrett et al., 1995)

# Data from Farm 5001 Summer (Corn, Harvested May 15th, 2010)
yyyyddd=[]
CornAge=[]
lst=[]
albedo=[]
ndvi=[]
rn=[]
sunza=[]
time=[]
e0=[]
yyyyddd=[2010023,2010025,2010026,2010027,2010028,2010029,2010041,2010046,2010051,2010052,2010054,2010060,2010061,2010062,2010066,2010070]
print len(yyyyddd)
#check here how many samples we got from instruments and set it as "number_of_days"
number_of_days=len(yyyyddd)

CornAge=[84,86,87,88,89,90,102,107,112,113,115,121,122,123,127,131]
lst=[306.36,314.04,318.8,307.84,310.68,307.94,308.4,306.26,305.78,309.14,300.88,302.58,302.6,306.82,298.06,298.24]
albedo=[0.194,0.194,0.194,0.194,0.194,0.194,0.208,0.208,0.211,0.211,0.211,0.19,0.19,0.19,0.177,0.177]
ndvi=[0.6346,0.6346,0.6346,0.6346,0.6346,0.6346,0.5942,0.5942,0.3832,0.3832,0.3832,0.3832,0.3832,0.3832,0.3642,0.3642]
sunza=[28.09,15.35,27.63,23.65,30.54,24.58,31.03,18.4,61.5,46.7,48.58,59.36,38.64,46.18,43.24,41.25]
time=[10.5,9.3,11,10.1,10.8,9.9,10.3,9.7,10.9,10,9.8,10.8,9.9,10.6,10.2,9.8]
#time=[11.0]*number_of_days
e0=[0.984,0.985,0.985,0.985,0.986,0.985,0.985,0.985,0.985,0.986,0.986,0.984,0.985,0.986,0.984,0.985]
print len(CornAge), len(lst), len(albedo), len(ndvi), len(sunza), len(time), len(e0)
# Meteorological data
wind_speed=[]
wind_height=[]
temperature_height=[]
atmo_pressure=[]
dem=[]
wind_speed=[3.385083,0.8810596,0.6670133,2.2169,0.8298947,3.614332,2.64063,3.22218,3.19117,5.657018,5.09192,3.059685,2.189741,1.668996,3.353387,2.50472]
atmo_pressure=[99.49069,99.59405,99.46934,99.55578,99.35571,99.50503,100.3306,99.40543,100.1859,99.94296,100.3547,100.1075,100.2296,100.1811,99.54597,101.4775]
print len(wind_speed), len(atmo_pressure)
wind_height=[5.0]*number_of_days
temperature_height=[5.0]*number_of_days
dem=[125]*number_of_days
# NDVI boundaries where lAI=0 or LAI=max from temporal statistics around crop season
ndvimin=[0.1]*number_of_days
ndvimax=[0.9]*number_of_days

#-----------------------------------------------------------
#Output from flux tower
#-----------------------------------------------------------
rn=[769.8926,619.0639,635.9927,742.8115,703.0481,762.594,731.7601,472.5808,648.711,647.6872,666.8344,626.1292,592.1609,574.5297,627.7044,613.3814]
g0=[25.70766,40.62577,44.3186,33.74405,27.57054,29.81707,5.167445,3.311021,16.77248,21.56444,8.804884,3.640177,9.577145,13.23416,16.03292,24.57652]
h0=[204.1521,95.79371,101.6697,127.078,121.893,115.835,142.0093,41.97649,120.6414,179.2991,216.711,203.6637,141.8283,158.7953,136.4691,161.9877]
le0=[540.03284,482.64442,490.0044,581.98945,553.58456,616.94193,584.583355,427.293289,511.29712,446.82366,441.318516,418.825323,440.755455,402.50024,475.20238,426.81718]
evapfr0=[0.7256702077,0.8343924699,0.8281660461,0.8207815068,0.8195454487,0.841923244,0.8045544515,0.9105493431,0.8090931377,0.7136358691,0.6706667486,0.6728236282,0.756552944,0.7170914631,0.7768915105,0.7248873005]
#UStar=[0.4649038,0.1162947,0.1042695,0.1918768,0.09978294,0.2696953,0.2876776,0.3190177,0.5456132,0.5431139,0.5540669,0.6547168,0.5586902,0.4640418,0.7290381,0.6128985]
#RhoAir=[1.164708,1.147039,1.130105,1.147882,1.149999,1.149586,1.141073,1.161161,1.155649,1.136143,1.196143,1.186352,1.185566,1.177592,1.165128,1.204528]
RhoAir=[]
UStar=[]
#----------------------------------------------------------
#CSU Corn functions for Coleambally Irrigation Area
#----------------------------------------------------------
lst_c=[]
tsoil=[]
tair=[]
z0_m=[]
canopy_height=[]
la_i=[]
kg=[]
k=[]
c=[]
leaf_area=[]
leaf_perimeter=[]
rn_s=[]
rn_c=[]
f_c=[]
tcanopy=[]
kb_1=[]
ra_h=[]
r_s=[]
h_s=[]
h_c=[]
psi_h=[]
psi_m=[]
h=[]
diff_h0=[]
DeltaT=[]
#UStar=[]
#g0=[]
g_s=g0

# Tsoil: Temperature for Soil from MODIS 1Km
lst_c	=[x-273.15 for x in lst]
tsoil 	= map(Tsoil,lst_c)
# Tair: Temperature for air from MODIS 1Km
tair	= map(Tair,lst_c)
#tsoil=[35.69066,38.4907,40.17904,37.37024,35.2098,34.24701,28.00031,23.65871,28.76191,31.44437,26.34717,21.74035,24.07526,25.21222,24.03826,22.15769]
#tair=[23.17016,28.41712,32.50686,27.92518,27.00311,27.53454,31.30686,23.33671,27.23524,31.98536,18.68676,19.92192,20.5698,22.25774,22.44436,19.49637]
# Roughness length for heat momentum
z0_m 	= map(z0m,CornAge)
# Canopy_height is z0m/0.136
canopy_height = [x/0.136 for x in z0_m]
# LAI function using MODIS 1Km input
la_i	= map(lai,ndvi)
# kg for g0()
kg	= [1.0]*number_of_days
# k for fc()
k 	= [0.4631]*number_of_days
# C for Rns (LAI attenuation equation)
c	= [0.5]*number_of_days
# Leaf area and perimeter (ratio is important so we use average at finalmax values for rs())
# The ratio leaf_area/leaf_perimeter was taken as 3.2184 and was created from knowledge scattered in
# Maddonni et al. (2001), Jordan-Meille et al. (2004) and Garcia (2010).
leaf_area = [560]*number_of_days
leaf_perimeter = [174]*number_of_days
#---------------------------
#Back to Generic Formulation
#---------------------------
#von Karman Constant
vonK	= [0.41]*number_of_days
#Earth gravitional acceleration
g	= [9.81]*number_of_days
#mass density of water
roh_w 	= [1000.0]*number_of_days
#Specific Heat
cp 	= [1004.16]*number_of_days
#blending height in meters (Monin and Obukhov, 1954)
blending_height = [50]*number_of_days

c_best= [0.21000000000000002, 0.27000000000000007, 0.24000000000000005, 0.26000000000000006, 0.25000000000000006, 0.29000000000000009, 0.27000000000000007, 0.3000000000000001, 0.22000000000000003, 0.24000000000000005, 0.21000000000000002, 0.21000000000000002, 0.3000000000000001, 0.24000000000000005, 0.3000000000000001, 0.27000000000000007]
rn_s	= map(rns,rn,c,la_i,sunza)
g0	= map(g_0,kg,rn_s,sunza)
g0	= map(g_modis2field,g0,rn_s,ndvi,ndvimax,ndvimin)
print "c,",c_best
print "rn_s,",rn_s
print "rn,",rn
print "g0,",g0

def sens_h(rn,c,la_i,sunza,rn_s,k,ndvi,ndvimin,ndvimax,lst_c,f_c,e0,wind_speed,wind_height,canopy_height,atmo_pressure,z0_m,tair,temperature_height,CornAge,vonK,g,RhoAir,rn_c,DeltaT,cp,ra_h,kb_1,psi_h,UStar,leaf_area,leaf_perimeter,tsoil,r_s,g0,h_c,h_s,diff_h0):
	rn_s	= map(rns,rn,c,la_i,sunza)
	g0	= map(g_0,kg,rn_s,sunza)
	g_s	= map(g_modis2field,g0,rn_s,ndvi,ndvimax,ndvimin)
	print "c,",c
	print "rn_s,",rn_s
	print "rn,",rn
	print "g0,",g_s
	rn_c	= map(rnc,rn,rn_s)
	f_c 	= map(fc,k,ndvi,ndvimin,ndvimax)
	tcanopy	= map(tc,tair,f_c,tsoil,e0)
	kb_1	= map(kb1,wind_speed,wind_height,canopy_height,la_i,f_c,tair,atmo_pressure,z0_m)
	DeltaT	= map(deltaT_SEBS,blending_height,wind_height,canopy_height,CornAge,kb_1,f_c,tsoil,dem)
	RhoAir = map(rohair,dem,lst,DeltaT)
	ra_h	= map(rah,wind_speed,wind_height,tair,temperature_height,canopy_height,CornAge,vonK,g,kb_1,RhoAir,rn_c)
	for i in range (0,3,1):
		DeltaT	= map(deltaT_SEBS,blending_height,wind_height,canopy_height,CornAge,kb_1,f_c,tsoil,dem)
		RhoAir = map(rohair,dem,lst,DeltaT)
		tair	= map(plus,tsoil,DeltaT)
		h_c	= map(hc,[0]*number_of_days,DeltaT,RhoAir,cp,ra_h,rn_c)
		psi_h	= map(psih,h_c,RhoAir,wind_speed,wind_height,tair,vonK,g)
		psi_m	= map(psim,h_c,RhoAir,wind_speed,wind_height,tair,vonK,g)
		UStar	= map(ustar,wind_speed,vonK,wind_height,z0_m,blending_height,canopy_height,CornAge,psi_m)
		ra_h	= map(rah1,wind_height,z0_m,kb_1,psi_h,UStar,vonK)

	h_c	= map(hc,[0]*number_of_days,DeltaT,RhoAir,cp,ra_h,rn_c)
	r_s	= map(rs,wind_speed,wind_height,tair,CornAge,canopy_height,leaf_area,leaf_perimeter,la_i,vonK,g,RhoAir,h_c)
	h_s	= map(hs,RhoAir,cp,tsoil,tair,ra_h,r_s,rn_s,g_s)
	h	= map(plus,h_c,h_s)
	#print "c,",c
	print "h0,",h0
	print "h,",h
	diff_h0	= map(minus,h0,h)
	print "diff_h0,",diff_h0
	return(diff_h0)

#-------------------------------------
# Parameters to try
#-------------------------------------
#c_i	= 0.2 #Attenuation coefficient of LAI for RN split to RNs

#c_best=[-1]*number_of_days
#diff_h0_best=[100000]*number_of_days
#for i in range(0,1,1):
	#fix this FT sample and run the model
	#c_i	= c_i + 0.01
	#c	= [c_i]*number_of_days
c= [0.21000000000000002, 0.27000000000000007, 0.24000000000000005, 0.26000000000000006, 0.25000000000000006, 0.29000000000000009, 0.27000000000000007, 0.3000000000000001, 0.22000000000000003, 0.24000000000000005, 0.21000000000000002, 0.21000000000000002, 0.3000000000000001, 0.24000000000000005, 0.3000000000000001, 0.27000000000000007]
diff_h0=sens_h(rn,c,la_i,sunza,rn_s,k,ndvi,ndvimin,ndvimax,lst_c,f_c,e0,wind_speed,wind_height,canopy_height,atmo_pressure,z0_m,tair,temperature_height,CornAge,vonK,g,RhoAir,rn_c,DeltaT,cp,ra_h,kb_1,psi_h,UStar,leaf_area,leaf_perimeter,tsoil,r_s,g0,h_c,h_s,diff_h0)
	#for j in range(0,number_of_days,1):
		#if (abs(diff_h0[j])<abs(diff_h0_best[j])):
			#diff_h0_best[j]=diff_h0[j]
			#c_best[j]=c[j]

#print "c_best=",c_best
#print "diff_h0_best=",diff_h0_best
