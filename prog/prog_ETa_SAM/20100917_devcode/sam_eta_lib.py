# -*- coding: utf-8 -*-
# /usr/bin/python
# SAM_ET Library
# Charles Sturt University, 2010

from math import *

#satellite images:
#-----------------
#lst = land surface temperature (K)
#albedo = surface Albedo (-)
#lai = Leaf Area Index (-)
#ndvi = normalized difference vegetation index (-)
#fc = fraction of vegetation cover (-)
#sunza = sun zenith angle (rad)

# Various inputs
#rn = Net Radiation Instantaneous (W/m2)
#c = extinction rate of LAI value in rns()
#kg = soil type and moisture conditions coefficient in g0
#k = attenuation factor of NDVI range in f_c()

#----------------------------------------------------------
#CSU Corn functions for Coleambally Irrigation Area
#----------------------------------------------------------

# Tsoil(C): Temperature for Soil from MODIS 1Km (R2=0.6689)
def Tsoil(lst_c):
	 return(0.9456*lst_c-1.77)

# Tair(C): Temperature for air from MODIS 1Km (R2=0.6546)
def Tair(lst_c):
	 return(0.6832*lst_c+2.43)

# Tc: Canopy temperature
def tc(lst,fc,tsoil,e0):
	#Emissivity stats from MDB for MODIS 1Km
	esoil=0.968
	ecanopy=0.992
	tempval=(e0*pow(lst,4)+(fc-1)*esoil*pow(tsoil,4))/(ecanopy*fc)
	if(tempval<=0):
		return(-pow(abs(tempval),0.25))
	else:
		return(pow((e0*pow(lst,4)+(fc-1)*esoil*pow(tsoil,4))/(ecanopy*fc),0.25))

# Ts: Soil temperature Version 2
def ts(lst,fc,tcanopy,e0):
	#Emissivity stats from MDB for MODIS 1Km
	esoil=0.968
	ecanopy=0.992
	return(pow((e0*pow(lst,4)-fc*ecanopy*pow(tcanopy,4))/(esoil*(1-fc)),0.25))

# Roughness length for heat momentum
def z0m(CornAge):
	return(log(CornAge-24)/10 - 0.1*pow(CornAge,2)/pow(181,2))

# DeltaT: Temperature difference between Soil and Air for MODIS 1Km
#def deltaT(lst_c,albedo,ndvi,time):
	 #return(exp(log(lst_c)/exp(pow(pow(albedo,ndvi),time))))

# LAI function using MODIS 1Km input
def lai(ndvi):
	return(3.2658*log(ndvi)+6.9212)

#Rescale g0 from MODIS 1Km to field (R2=0.3195)
def g_modis2field(g_modis,rn_s,ndvi,ndvimax,ndvimin):
	ndviscaled=(ndvi-ndvimin)/(ndvimax-ndvimin)
	g_s=g_modis*ndviscaled
	g_s=0.3006*g_s+8.6387
	if( g_s > rn_s ):
		g_s = rn_s
	return(g_s)

#---------------------------
#Back to Generic Formulation
#---------------------------

#Net Radiation for bare soil
def rns(rn,c,lai,sunza):
	return(rn*exp(-c*lai/cos(sunza*pi/180)))

#Net Radiation for canopy
def rnc(rn,rns):
	return(rn-rns)

# Soil Heat Flux (Friedl, 2002)
def g_0(kg,rns,sunza):
	return(kg*rns*cos(sunza*pi/180))

#Fraction of vegetation cover (Jia et al, 2003)
def fc(k,ndvi,ndvimin,ndvimax):
	if(ndvi>0.1):
		fc=1-pow(((ndvi-ndvimax)/(ndvimin-ndvimax)),k)
	else:
		fc=0
	return(fc)

# Air density standard paramterization
def rohair(dem, lst, delta_T):
	tair = lst-delta_T
	b = ((tair - 0.00627*dem)/tair)
	return(349.467 * pow( b , 5.26)/ tair)

#Monin-Obukov Length
#usage: (psih,psim)=psi(h,rohair,wind_speed,temperature,vonK,g)
def psi(h,rohair,wind_speed,wind_height,temperature,vonK,g):
	if(temperature < 200):
		temperature=temperature+273.15
	if(h!=0.0):
		most_L = (-1004*rohair*pow(wind_speed,3)*temperature)/(vonK*g*h)
	else:
		most_L = -1000.0
	if(most_L < 0.0):
		most_x = pow((1-16*(wind_height/most_L)),0.25)
		psih = (2*log((1+pow(most_x,2))/2))
		psim = (2*log((1+most_x)/2)+log((1+pow(most_x,2))/2)-2*atan(most_x)+0.5*pi)
	else:
		psih = -5*2/most_L
		psim = -5*2/most_L
	return(psih,psim)

#aerodynamic resistance to heat momentum (soil)
def rs(wind_speed,wind_height,tsoil,CornAge,canopy_height,leaf_area,leaf_perimeter,lai,vonK,g,rohair,hc):
	# Roughness length for heat momentum
	z0_m 	= z0m(CornAge)
	#disp = displacement height (m)
	#if(canopy_height<1.0):
	disp = 0.65*canopy_height
	#else:
		#disp = 0
	#Monin-Obukov Length
	(psih,psim)=psi(hc,rohair,wind_speed,wind_height,tsoil,vonK,g)
	#Canopy based friction velocity
	uc = wind_speed * (log((canopy_height-disp)/z0_m)/(log((wind_height-disp)/z0_m)-psim))
	#Wind speed just above the soil surface (Goudriaan, 1977)
	s = 4*leaf_area/leaf_perimeter
	a = 0.28*pow(lai,2/3)*pow(canopy_height,1/3)*pow(s,-1/3)
	us = uc*exp(-a*(1-0.05/canopy_height))
	return(1/(0.004+0.012*us))

#aerodynamic resistance to heat momentum (canopy)
def rah(wind_speed, wind_height, tair, temperature_height, canopy_height, CornAge, vonK, g, kb1, rohair, hc):
	# Roughness length for heat momentum
	z0_m 	= z0m(CornAge)
	#disp = displacement height (m)
	if(canopy_height<1.0):
		disp = 0.65*canopy_height
	else:
		disp = 0
	#Monin-Obukov Length
	(psih,psim)=psi(hc,rohair,wind_speed,wind_height,tair,vonK,g)
	#Rah output
	ra_h=(log((wind_height-disp)/z0_m)-psim)*(log((temperature_height-disp)/(z0_m*exp(kb1)))-psih)/(pow(vonK,2)*wind_speed)
	if (ra_h < 0.1):
		ra_h = 0.1
	return(ra_h)

def ustar(wind_speed,vonK,wind_height,z0_m,blending_height,canopy_height,CornAge,psim):
	# Roughness length for heat momentum
	z0_m 	= z0m(CornAge)
	#disp = displacement height (m)
	if(canopy_height<1.0):
		disp = 0.65*canopy_height
	else:
		disp = 0
	return((wind_speed*vonK/log((wind_height-disp)/z0_m))/vonK*log((blending_height-disp)/z0_m)*vonK)/(log((blending_height-disp)/z0_m)-psim)

def rah1(wind_height,z0_m,kb1,psih,u_star,vonK):
	tempval=wind_height/(z0_m/exp(kb1))
	#print "z0m/exp(kb1)=",z0_m/exp(kb1),"Ln(tempval)=",log(tempval)
	if(tempval<0.01):
		return((log(0.01)-psih)/(u_star*vonK))
	else:
		return((log(wind_height/(z0_m/exp(kb1)))-psih)/(u_star*vonK))

#From SEBS (Su, 2002)
def kb1(wind_speed,wind_height,canopy_height,lai,fc,tair,atmopressure,z0):
	#After Brutsaert, 1982
	#u_zref = wind speed at reference height
	#zref = reference height, 2h<= zref <=hst=(0.1~0.15)hi, (=za)
	#h = canopy height
	#z0 = z0_m =  z02h(h)*h
	#lai = canopy total leaf area index
	#fc = Fractional canopy cover (-)
	#tair = ambient air temperature (oC)
	#atmopressure = ambient air pressure (Pa)
	#nec = within-canopy wind speed profile extinction coefficient (-)
	#re_star = roughness Reynolds Number
	c_d = 0.2 #foliage drag coefficient
	c_t = 0.00005 #heat transfer coefficient
	k = 0.4 #von Karman's constant
	Pr = 0.7 #Prandtl's number
	#Yann's modification from SEBS
	hs = 0.002 #0.009~0.024 Height of soil roughness obstacles
	zref = 2*wind_height
	if(tair < 200):
		tair=tair+273.15
	atmopressure = atmopressure * 1000.0 #Comes in kPa in flux tower
	fs = 1.0-fc #soil fraction
	if (fc == 0.0):
		canopy_height = hs # use smooth bare soil roughness
	#u_h0 = wind_height*log(2.446)/log((zref-0.667*canopy_height)/z0)#wind speed at canopy height
	u_h0=wind_speed
	#print "u_h0=",u_h0
	ust2u_h = 0.32-0.264/exp(15.1*c_d*lai)#Ustar2u_h(h)
	ustarh = ust2u_h*u_h0
	#kinematic viscosity of the air (m2/s) Massman, 1999b)
	nu0 = 1.327 *pow(10,-5)*(101325.0/atmopressure)*pow((tair/273.15),1.81)
	#Within Canopy wind speed profile extinction coefficient: nec)
	nec = c_d*lai/(2.0*pow(ust2u_h,2))
	#roughness Reynolds Number
	re_star=hs*ustarh/nu0
	#first term
	if (nec != 0.0):
		kb1_1=(fc*fc)*k*c_d/(4.0*c_t*ust2u_h*(1.0-exp(-nec/2.0)))
	else:
		kb1_1 = 0.0
	#second term
	c_t_star=pow(Pr,-2/3)*pow(re_star,-1/2)
	kb1_2 = 2*fc*fs*(k*ust2u_h*(z0/canopy_height))/c_t_star
	#third term
	#soil roughness Reynolds Number star
	re_star=hs*(wind_height*k/log(zref/hs))/nu0
	#soil heat transfer number kB-1 (Brutsaert, 1982)
	kb1soil=pow(2.46*(re_star),0.25)-log(7.4)
	kb1_3=kb1soil*fs*fs
	if((kb1_1+kb1_2+kb1_3)>700):
		return(700)
	else:
		return(kb1_1+kb1_2+kb1_3)


#From SEBS (Su, 2002)
def deltaT_SEBS(blending_height,wind_height,canopy_height,CornAge,kb1,fc,lst_c,dem):
	#t_s = Tsoil(lst_c)
	t_s=lst_c #is tsoil right now
	# Roughness length for heat momentum
	z0_m 	= z0m(CornAge)
	#disp = displacement height (m)
	if(canopy_height<1.0):
		disp = 0.65*canopy_height
	else:
		disp = 0
	#blending_height_SEBS=200
	t_c_1 = log((blending_height-disp)) / (z0_m/exp(kb1))
	t_c_2 = log((wind_height-disp)) / (z0_m/exp(kb1))
	t_c = t_c_1 / t_c_2
	t_pbl = (t_s*(1.0-fc)+t_c*fc)/pow((1.0-dem/44331.0),1.5029)

	t0 = lst_c / pow((1.0-dem/44331.0),1.5029)
	#print "tc=",t_c,"t_pbl=",t_pbl,"t0=",t0,"t0-t_pbl=",(t0-t_pbl)
	delta=t0-t_pbl
	return(delta)

#vegetation canopy Sensible heat flux of evaporation
def hc(tair, tcanopy, rohair, cp, rah,rn_c):
	#print "deltaT", (tcanopy-tair)
	h_c = rohair*cp*(tcanopy-tair)/rah
	if( h_c > rn_c ):
		h_c = 0
	if( h_c < 0 ):
		h_c = rn_c
	return(h_c)

#Soil Sensible heat flux of evaporation
def hs(rohair, cp, tsoil, tair, rah, rs, rn_s, g_s):
	h_s = rohair*cp*(tsoil-tair)/(rah+rs)
	if( h_s > (rn_s - g_s) ):
		h_s = 0
	if( h_s < 0 ):
		h_s = rn_s-g_s

	return(h_s)

#Monin-Obukov Length
def psim(h,rohair,wind_speed,wind_height,temperature,vonK,g):
	if(temperature < 200):
		temperature=temperature+273.15
	if(h!=0.0):
		most_L = (-1004*rohair*pow(wind_speed,3)*temperature)/(vonK*g*h)
	else:
		most_L = -1000.0
	if(most_L < 0.0):
		most_x = pow((1-16*(wind_height/most_L)),0.25)
		psim = (2*log((1+most_x)/2)+log((1+pow(most_x,2))/2)-2*atan(most_x)+0.5*pi)
	else:
		psim = -5*2/most_L
	return(psim)

def psih(h,rohair,wind_speed,wind_height,temperature,vonK,g):
	if(temperature < 200):
		temperature=temperature+273.15
	if(h!=0.0):
		most_L = (-1004*rohair*pow(wind_speed,3)*temperature)/(vonK*g*h)
	else:
		most_L = -1000.0
	if(most_L < 0.0):
		most_x = pow((1-16*(wind_height/most_L)),0.25)
		psih = (2*log((1+pow(most_x,2))/2))
	else:
		psih = -5*2/most_L
	return(psih)

#Evaporative Fraction
def evap_fr(rn_c, rn_s, g_s, h):
	return( (rn_c + rn_s - g_s - h)/(rn_c + rn_s - g_s) )

#------------------------------------------------------------------------------------------
#Basic functions for list operations
def plus(a,b):
	return(a+b)

def minus(a,b):
	return(a-b)

def minus2(a,b,c):
	result=a-b-c
	if(result<0):
		result=0
	return(result)

#misc functions
def tcanop(h_c,ra_h,roh_air,cp,tair):
	return(h_c*ra_h/(roh_air*cp)+tair)

def hsoil(roh_air,cp,tsoil,tair,ra_h,r_s):
	return(roh_air*cp*(tsoil-tair)/(ra_h+r_s))

def tsoi(ra_h,r_s,roh_air,cp,h_s,tair):
	return((ra_h+r_s)/(roh_air*cp)*h_s+tair)
