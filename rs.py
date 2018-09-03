from math import *

def rh(PW,Pa,Ta,dem):
	"""
	https://www.researchgate.net/publication/227247013_High-resolution_Surface_Relative_Humidity_Computation_Using_MODIS_Image_in_Peninsular_Malaysia/
	PW <- MOD05_L2 product
	Pa <- MOD07 product
	Pa <- 1013.3-0.1038*dem
	Ta <- MOD07 product
	Ta <- -0.0065*dem+TaMOD07 (if dem>400m)
	
	https://ladsweb.modaps.eosdis.nasa.gov/archive/allData/61/MOD05_L2/
	https://ladsweb.modaps.eosdis.nasa.gov/archive/allData/61/MOD07_L2/
	"""
	#Specific Humidity
	q=0.001*(-0.0762*PW*PW+1.753*PW+12.405)
	ta=-0.0065*dem+Ta
	a=17.2694*ta/(ta+238.3)
	return(q*Pa/(380*exp(a)))




