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

def nd(x1, x2):
    return ((x1 - x2) / (x1 + x2))

def nddi(ndvi, ndwi):
    # NDDI: Normalized Difference Drought Index {
    result=nd(ndvi, ndwi)
    result[result < 0]=0
    result[result > 2]=2
    return (result)

def drought(ndvi, ndwi):
    # DROUGHT where drought = 1, no drought=0
    res=((ndvi < 0.5 and ndwi < 0.3) * 2) + ((ndvi > 0.6 and ndwi > 0.4) * 1) - 1
    return (!res)

def ndwi(green, nir):
    # NDWI: Normalized Difference Water Index
    # Stuart McFeeters. 1996. The Use of Normalized Difference Water Index in the Delination of
    # Open Water Features. International Journal of Remote Sensing 27(14):3025-3033
    result=nd(green, nir)
    result[result < -1]=-1
    result[result > 1]=1
    return (result)

def mndwi(green, swir):
    # MNDWI: Modified Normalized Difference Water Index
    # Hanqui XU. 2006. Modification of Normalized Difference Water Index to Enhance Open Water
    #	Features om Remotely Sensed Imagery. International Journal of Remote Sensing 17(7):1425-1432
    result=nd(green, swir)
    result[result < -1]=-1
    result[result > 1]=1
    return (result)

def lswi(nir, swir):
    # LSWI: Land Surface Water Index
    result=nd(nir, swir)
    result[result < -1]=-1
    result[result > 1]=1
    return (result)

def water(ndvi, albedo):
    # water: generic water mapping tool
    return ((ndvi < 0.1) and (albedo < 0.1))

def waterModis(ndvi, band7):
"""
# water.modis: Terra-MODIS water mapping tool
# Xiao X., Boles S., Liu J., Zhuang D., Frokling S., Li C., Salas W., Moore III B. (2005).
# Mapping paddy rice agriculture in southern China using multi-temporal MODIS images.
# Remote Sensing of Environment 95:480-492.
#
# Roy D.P., Jin Y., Lewis P.E., Justice C.O. (2005).
# Prototyping a global algorithm for systematic fire-affected
# area mapping using MODIS time series data.
# Remote Sensing of Environment 97:137-162.
"""
if((ndvi < 0.1) and (band7 < 0.04)):
    return(1)
else:
    return(0)

}