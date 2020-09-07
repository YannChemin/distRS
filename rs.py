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


def esat(tamean):
	"""
	esat: saturated vapour pressure
	tamean: air temperature daily mean
	"""
	return(610.78*exp(17.2694*tamean/(tamean+238.3))

def eact(esat,rh):
	"""
	eact: actual vapour pressure
	esat: saturated vapour pressure
	rh: relative humidity
	"""
	return(0.01*esat*rh)

def s2a_bbalb(b2,b3,b4,b5,b6,b7,b8,b8a,b11,b12):
    #bandwidth
    b2w=b2*98.0
    b3w=b3*45.0
    b4w=b4*38.0
    b5w=b5*19.0
    b6w=b6*18.0
    b7w=b7*28.0
    b8aw=b8a*33.0
    b11w=b11*143.0
    b12w=b12*242.0
    #weighted sum
    wsum=b2w+b3w+b4w+b5w+b6w+b7w+b8w+b8aw+b11w+b12w
    return(wsum/809.0)

def s2b_bbalb(b2,b3,b4,b5,b6,b7,b8,b8a,b11,b12):
    #bandwidth
    b2w=b2*98.0
    b3w=b3*46.0
    b4w=b4*39.0
    b5w=b5*20.0
    b6w=b6*18.0
    b7w=b7*28.0
    b8aw=b8a*32.0
    b11w=b11*141.0
    b12w=b12*238.0
    #weighted sum
    wsum=b2w+b3w+b4w+b5w+b6w+b7w+b8w+b8aw+b11w+b12w
    return(wsum/793.0)
