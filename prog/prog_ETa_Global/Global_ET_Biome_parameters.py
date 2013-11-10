#!/usr/bin/python
from math import *

def fc( ndvi ):
	"""Fraction of vegetation cover"""
	ndvimin = 0.05
	ndvimax = 0.95
	return ( ( ndvi - ndvimin ) / ( ndvimax - ndvimin ) )

def bdpc( ndvi, b1, b2, b3, b4 ):
	"""Biome dependent potential conductance (g0 in Zhang et al, 2009. WRR)"""
	return (1.0 / (b1 + b2 * exp(-b3*ndvi))+b4)

def mTday( tday, tclosemin, topenmax, topt, beta ):
	"""Temperature dependent reduction function of the biome dependent potential conductance""" 
	if tday <= tclosemin:
		return 0.01
	elif tday >= topenmax:
		return 0.01
	else:
		return ( exp( - ( (tday-topt)/beta )*( (tday-topt)/beta ) ) )

def mVPD( vpd, vpdclose, vpdopen ):
	"""VPD dependent reduction function of the biome dependent potential conductance""" 
	if vpd <= vpdopen:
		return 1.0
	elif vpd >= vpdclose:
		return 1.0
	else:
		return ( ( vpdclose - vpd ) / ( vpdclose - vpdopen ) )

def esat( tday ):
	"""From FAO56, output is in [Pa]"""
	return( 1000 * 0.6108 * exp( 17.27 * tday / (tday + 237.3) ) )

def vpdeficit( rh, tday ):
	"""From FAO56, vpd is esat - eact"""
	e_sat = esat( tday )
	return ( ( 1.0 - rh ) * e_sat )

def rhumidity( sh, tday, patm ):
	"""http://en.wikipedia.org/wiki/Humidity#Specific_humidity
	Output  [0.0-1.0]"""
	e_sat = esat( tday )
	print "e_sat\t=",e_sat,"\t[Pa]"
	b = ( 0.622 + sh ) * e_sat 
	return ( sh * patm / b )

def slopesvpcurve( tday ):
	"""From FAO56, output is in [Pa/C]"""
	e_sat = esat( tday )
	return( 4098.0 * e_sat / pow( tday + 237.3, 2 ) )

def rhoair( dem, tday ):
	"""Requires T in Kelvin"""
	t = tday+273.15
	b = ( ( t - ( 0.00627 * dem ) ) / t )
	return( 349.467 * pow( b, 5.26 ) / t )

def ETa (biome_type, ndvi, tday, sh, patm, Rn, G, dem):
	"""ETa global model from Zhang et al 2010. 
	A continuous satellite derived global record of land surface evapotranspiration from 1983 to 2006.
	WRR 46"""
        b4 = 0.0 #except for WSV with NDVI > 0.64
	if biome_type == "BENF":
		tclosemin = -8.0
		topenmax = 40.0
		vpdclose = 2800.0
		vpdopen = 500.0
		topt = 12.0
		beta = 25.0
		k = 150.0
		ga = 0.03
		gtot  = 0.002
		gch = 0.08
		b1 = 208.3
		b2 = 8333.3
		b3 = 10.0
	if biome_type == "TENF":
		tclosemin = -8.0
		topenmax = 40.0
		vpdclose = 2800.0
		vpdopen = 500.0
		topt = 25.0
		beta = 25.0
		k = 200.0
		ga = 0.03
		gtot  = 0.004
		gch = 0.08
		b1 = 133.3
		b2 = 888.9
		b3 = 6.0
	if biome_type == "EBF":
		tclosemin = -8.0
		topenmax = 50.0
		vpdclose = 4000.0
		vpdopen = 500.0
		topt = 40.0
		beta = 40.0
		k = 300.0
		ga = 0.03
		gtot  = 0.006
		gch = 0.01
		b1 = 57.7
		b2 = 769.2
		b3 = 4.5
	if biome_type == "DBF":
		tclosemin = -6.0
		topenmax = 45.0
		vpdclose = 2800.0
		vpdopen = 650.0
		topt = 28.0
		beta = 25.0
		k = 200.0
		ga = 0.04
		gtot  = 0.002
		gch = 0.01
		b1 = 85.8
		b2 = 694.7
		b3 = 4
	if biome_type == "CSH":
		tclosemin = -8.0
		topenmax = 45.0
		vpdclose = 3300.0
		vpdopen = 500.0
		topt = 19.0
		beta = 20.0
		k = 400.0
		ga = 0.01
		gtot  = 0.001
		gch = 0.04
		b1 = 202.0
		b2 = 4040.4
		b3 = 6.5
	if biome_type == "OSH":
		tclosemin = -8.0
		topenmax = 40.0
		vpdclose = 3700.0
		vpdopen = 500.0
		topt = 10.0
		beta = 30.0
		k = 50.0
		ga = 0.005
		gtot  = 0.012
		gch = 0.04
		b1 = 178.6
		b2 = 178.6
		b3 = 8
	if biome_type == "WSV" and ndvi < 0.64:
		tclosemin = -8.0
		topenmax = 50.0
		vpdclose = 3200.0
		vpdopen = 500.0
		topt = 32.0
		beta = 28.0
		k = 900.0
		ga = 0.002
		gtot  = 0.0018
		gch = 0.04
		b1 = 0.2
		b2 = 24000
		b3 = 6.5
	if biome_type == "WSV" and ndvi > 0.64:
		tclosemin = -8.0
		topenmax = 50.0
		vpdclose = 3200.0
		vpdopen = 500.0
		topt = 32.0
		beta = 28.0
		k = 900.0
		ga = 0.002
		gtot  = 0.0018
		gch = 0.04
		b1 = 57.1
		b2 = 3333.3
		b3 = 8.0
		b4 = -0.01035
	if biome_type == "SV":
		tclosemin = -8.0
		topenmax = 40.0
		vpdclose = 5000.0
		vpdopen = 650.0
		topt = 32.0
		beta = 30.0
		k = 800.0
		ga = 0.001
		gtot  = 0.001
		gch = 0.04
		b1 = 790.9
		b2 = 8181.8
		b3 = 10.0
	if biome_type == "GRS":
		tclosemin = -8.0
		topenmax = 40.0
		vpdclose = 3800.0
		vpdopen = 650.0
		topt = 20.0
		beta = 30.0
		k = 500.0
		ga = 0.001
		gtot  = 0.001
		gch = 0.04
		b1 = 175.0
		b2 = 2000
		b3 = 6.0
	if biome_type == "CRP":
		tclosemin = -8.0
		topenmax = 45.0
		vpdclose = 3800.0
		vpdopen = 650.0
		topt = 20.0
		beta = 30.0
		k = 450.0
		ga = 0.005
		gtot  = 0.003
		gch = 0.04
		b1 = 105.0
		b2 = 300.0
		b3 = 3.0

	#Compute potential conductance for this biome and this NDVI
	g0 = bdpc(ndvi,b1,b2,b3,b4)
	#Preprocessing for Surface conductance (gs) in PM (FAO56), gc in this article
	mtday = mTday( tday, tclosemin, topenmax, topt, beta )
	#relative humidity
	rh = rhumidity( sh, tday, patm )
	print "rh\t=",rh,"\t[-]"
	vpd = vpdeficit( rh, tday )
	print "vpd\t=",vpd,"\t\t[Pa]"
	mvpd = mVPD( vpd, vpdclose, vpdopen )
	#Actually computing Surface conductance (gs) in PM (FAO56), gc in this article
	gs = g0 * mtday * mvpd
	print "rs\t=",1/gs,"\t[s/m]"
	#Fraction of vegetation cover
	fracveg = fc(ndvi)
	print "fc\t=", fracveg, "\t[-]"
	#preprocessing for soil Evaporation
	latent = 2.45 #MJ/Kg FAO56
	MaMw = 0.622 # - FAO56
	Cp = 1.013 * 0.001 # MJ/Kg/C FAO56
	psi = patm * Cp / (MaMw * latent) #psi = patm * 0.6647 / 1000
	print "psi\t=",psi,"\t[Pa/C]"
	gtotc = gtot * ((273.15+tday) / 293.13) * (101300.0 /  patm)
	Delta = slopesvpcurve( tday ) #slope in Pa/C
	print "Delta\t=",Delta,"\t[de/dt]"
	rho = rhoair( dem, tday )
	print "rho\t=",rho,"\t[kg/m3]"
	#soil Evaporation
	Esoil = pow(rh,vpd/k) * (Delta*(1-fracveg)*(Rn-G)+rho*Cp*vpd*ga) / (Delta+psi*ga/gtotc)
	#Canopy evapotranspiration
	Ecanopy = (Delta*fracveg*(Rn-G)+rho*Cp*vpd*ga) / (Delta+psi*(1.0+ga/gs))
	print "-------------------------------------------------------------"
	print "Esoil\t=", Esoil, "\t[mm/d]"
	print "Ecanopy\t=", Ecanopy, "\t[mm/d]"
	print "-------------------------------------------------------------"
	return( (1-fracveg) * Esoil + fracveg * Ecanopy )

def run():
	print "-------------------------------------------------------------"
	print "ETa (biome_type, ndvi, tday, sh, patm, Rn, G, dem)"
	print "-------------------------------------------------------------"
	print "biome_type\t= SV\t\t(Savannah)"
	print "ndvi\t\t= 0.3\t\t[-]"
	print "tday\t\t= 32.0\t\t[C]\t\t(from GCM)" 
	print "sh\t\t= 0.0189\t[-]\t\t(from GCM)"
	print "patm\t\t= 100994.2\t[Pa]\t\t(from GCM)"
	print "Rn\t\t= 20.0\t\t[MJ/m2/d]\t(from GCM eq. 236.4 W/m2)"
	print "G\t\t= 2.0\t\t[MJ/m2/d]\t(10% Rn eq. 23.64 W/m2)"
	print "dem\t\t= 5\t\t[m]\t\t(from SRTM 1Km)"
	print "-------------------------------------------------------------"
	eta = ETa ("SV", 0.3, 32.0, 0.0189, 100994.2, 20.0, 2.0, 5.0)
	print "-------------------------------------------------------------"
	print "ETa\t=",eta,"\t[mm/d]"
	print "-------------------------------------------------------------"



def info():
	print "Zhang, K., Kimball, J.S., Nemani, R.R., Running, S.W. (2010). A continuous satellite-derived global record of land surface evapotranspiration from 1983 to 2006. WRR 46, W09522"
	print "-------------------------------------------------------------------------------------------------------------------------------------------------------"
	print "to run test use the function run()"
	print "-------------------------------------------------------------------------------------------------------------------------------------------------------"
	print "to run with your data use the function ETa()"
	print "ETa (biome_type, ndvi, tday, sh, patm, Rn, G)"
	print "-------------------------------------------------------------"
	print "Biome_type\t as defined at the end of this info bubble"
	print "ndvi\t\t NDVI value\t\t [-]"
	print "tday\t\t day temperature\t [C]"
	print "sh\t\t specific humidity\t [-]"
	print "patm\t\t atmospheric pressure\t [Pa]"
	print "Rn\t\t day net radiation\t [MJ/m2/d]"
	print "G\t\t day soil heat flux\t [MJ/m2/d]"
	print "-------------------------------------------------------------------------------------------------------------------------------------------------------"
	print "IGBP Biome types used in this model"
	print "-------------------------------------------------------------------------------------------------------------------------------------------------------"
	print "Code\t Description"
	print "BENF\t Boreal Evergreen Needleleaf Forest"
	print "TENF\t Temperate Evergreen Needleleaf Forest"
	print "EBF\t Evergreen Broadleaf Forest"
	print "DBF\t Deciduous Broadleaf Forest"
	print "CSH\t Closed Shrubland"
	print "OSH\t Open Shrubland"
	print "WSV\t Woody Savannah"
	print "SV\t Savannah"
	print "GRS\t Grassland"
	print "CRP\t Cropland"
	print "-------------------------------------------------------------------------------------------------------------------------------------------------------"
	print "IGBP Biome types and configuration of internal parameters of the model"
	print "-------------------------------------------------------------------------------------------------------------------------------------------------------"
	print "#Code	Description	TcloseMinC	TopenMaxC	VPDClosePa	VPDOpenPa	ToptC	BetaC	kPa	GaMs-1	GtotMs-1	GchMs-1	B1Sm-1	B2Sm-1	B3	b1	b2	b3	b4"
	print "#BENF	Boreal Evergreen Needleleaf Forest	-8	40	2800	500	12	25	150	0.03	0.002	0.08	208.3	8333.3	10"				
	print "#TENF	Temperate Evergreen Needleleaf Forest	-8	40	2800	500	25	25	200	0.03	0.004	0.08	133.3	888.9	6"				
	print "#EBF	Evergreen Broadleaf Forest	-8	50	4000	500	40	40	300	0.03	0.006	0.01	57.7	769.2	4.5"				
	print "#DBF	Deciduous Broadleaf Forest	-6	45	2800	650	28	25	200	0.04	0.002	0.01	85.8	694.7	4"				
	print "#MF	Mixed Forest"																	
	print "#CSH	Closed Shrubland	-8	45	3300	500	19	20	400	0.01	0.001	0.04	202	4040.4	6.5"				
	print "#OSH	Open Shrubland	-8	40	3700	500	10	30	50	0.005	0.012	0.04	178.6	178.6	8"				
	print "#WSV	Woody Savannah	-8	50	3200	500	32	28	900	0.002	0.0018	0.04	0.2	24000	6.5	57.1	3333.3	8	-0.01035"
	print "#SV	Savannah	-8	40	5000	650	32	30	800	0.001	0.001	0.04	790.9	8181.8	10"				
	print "#GRS	Grassland	-8	40	3800	650	20	30	500	0.001	0.001	0.04	175	2000	6"				
	print "#CRP	Cropland	-8	45	3800	650	20	30	450	0.005	0.003	0.04	105	3000	3"				
	print "															#For WSV when NDVI>0.64"
		