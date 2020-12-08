# -*- coding: utf-8 -*-
"""
Created on Fri Oct 19 11:46:15 2018

https://gis.stackexchange.com/questions/263050/how-to-generate-a-cloud-mask-using-mod09gq-image-with-python

Voglio usare i MODIS 09GA per estrarne dati di cloud contamination e 
per poi mascherare i MODIS MOD09GQ, dai quali voglio calcolare l'NDVI sui soli
pixel cloud free, in cui si coltiva riso e che hanno valori di NDVI < 0

@author: manfrgi
"""
## -----------------------------------------------------------------------------
## -------------------------------   PART-1   ----------------------------------
## ----------------------------- MODIS DOWNLOAD --------------------------------
#
## ---------------------------------------------------------- Import libraries :
#import os
#import fnmatch
#import ogr
#import datetime
#import dateutil.rrule
#import numpy as np
#import pandas as pd
#import requests
#import time
#
## --------------------------------------------------- Set download parameters :
#hh        = '18'
#vv        = '04'
#coll      = 6
#prod      = 'MOD11A1' # MOD09GQ
#yyyy      = 2018
#DOY_start = 91
#DOY_end   = 181
#
## ----------------------------------------------------------------- Set roots :
## out_root = 'E:/ix_ToGreg_MODIS.rice.flood.mng.map/MODIS_imgs/' + str(prod) + '_' + str(yyyy) + '/'
#out_root = 'D:/WA_GIS_Matteo/ix_ToGreg_MODIS.rice.flood.mng.map/MODIS_LST/MOD11A1/'
#print 'Start processing : ', time.ctime()
#    
#for time_stamp in range(DOY_start, DOY_end + 1):
#    
#    if len(str(time_stamp)) == 3:
#        time_stamp = str(time_stamp)
#    elif len(str(time_stamp)) == 2:
#        time_stamp = '0' + str(time_stamp)
#    else:
#        time_stamp = '00' + str(time_stamp)
#
#    url = 'https://ladsweb.modaps.eosdis.nasa.gov/archive/allData/' + str(coll) + '/' + prod + '/' + str(yyyy) + '/' + time_stamp    
#    pag = (requests.get(url).content)
#        
#    fil_src = prod + '.A' + str(yyyy) + time_stamp + '.' + 'h' + str(hh) + 'v' + str(vv) + '.'    
#    pag_src = pag.find(fil_src)
#    
#    if pag_src != -1:
#        fil_dow = pag[pag_src:pag_src + 45]
#        url_dow = url + "/" + fil_dow    
#        str_out = out_root + fil_dow
#        rsponse = requests.get(url_dow, stream=True)
#        handler = open(str_out, "wb")
#        for chunk in rsponse.iter_content(chunk_size=512): 
#            if chunk:
#                handler.write(chunk)
#        handler.close()
#        print "   Downloaded : " + fil_dow + '  -  ' + time.ctime()[11:19]
#    else:
#        print "   --> DOWNLOAD ERROR: " + fil_src
#
#
#print 'End processing : ', time.ctime()



# -----------------------------------------------------------------------------
# -------------------------------   PART-2   ----------------------------------
# ------------------------- MOD09GA PREPROCESSING -----------------------------
# https://gis.stackexchange.com/questions/168910/gdal-python-extracting-projection-info-from-hdf-file

from osgeo import gdal, gdalconst
import numpy as np
import pylab as plt
import fnmatch
import time
import os

root_in = '//ies/d5/mars4cast/Users/manfrgi/GR/mod09ga/original/'
root_fl = fnmatch.filter(os.listdir(root_in), '*.hdf')

cnt=0
for x in root_fl:
    print cnt+1, ' on ', len(root_fl)
    
    stringa = root_in + x                                # Set the input string
    hdf_ds = gdal.Open(stringa)                        # open the HDF container
    
    # Open the subdataset ":MODIS_Grid_1km_2D:state_1km_1:" :
    QA_flgs = gdal.Open(hdf_ds.GetSubDatasets()[1][0], gdal.GA_ReadOnly).ReadAsArray()
    sizes   = np.shape(QA_flgs)
    QA_cld  = QA_flgs
    QA_cls  = QA_flgs
    
    # Derive cloud and cloud shadow QA informations : 
    for l in range(sizes[0]):
        for c in range(sizes[1]):
            mask_bit_01 = 0b0000000000000011
            mask_bit_2  = 0b0000000000000100
            QA_val_b01  = (QA_flgs[l,c] & mask_bit_01)
            QA_val_b2   = (QA_flgs[l,c] &  mask_bit_2) >> 2 
            QA_cld[l,c] = QA_val_b01
            QA_cls[l,c] = QA_val_b2
    
    # Produce a noise mask :           
    QA_Sum  = QA_cld + QA_cls       # Cloud-clear pixel == 0 and No-shadow == 0
    QA_mask = np.where(QA_Sum == 0, 1, 0)                     # 1 = Cloud free
    QA_flgs = QA_Sum = QA_cld = QA_cls = None
    
    # Upsample the QA info, from 1km to 250m to further cope with NDVI imgs :
    k_1000to250 = 4
    k_1000to500 = 2
    
    QA_mask_res = QA_mask.repeat(k_1000to500, axis=0).repeat(k_1000to500, axis=1)
    QA_mask_res = QA_mask_res.astype(np.int8)
    
        
    # Convert QA-data from .hdf to .tif and save them  :
    driver  = gdal.GetDriverByName("GTiff")
    out_tif = stringa.split('006.')[0] + 'QA-Cloud.Res500.tif'
    dst_ds  = driver.Create(out_tif, len(QA_mask_res[0]), len(QA_mask_res[0]), 1, gdal.GDT_Byte)
    dat_ref = gdal.Open(hdf_ds.GetSubDatasets()[11][0])
    dt_tran = dat_ref.GetGeoTransform()  
    dt_proj = dat_ref.GetProjection()
    dst_ds.SetGeoTransform(dt_tran)
    dst_ds.SetProjection(dt_proj)                          
    dst_ds.GetRasterBand(1).WriteArray(QA_mask_res)
    dst_ds = QA_flgs = QA_mask_res = None                     
    cnt = cnt+1
    print time.ctime()[11:19], ' - ', out_tif
    print

print 'Done!'        





## -----------------------------------------------------------------------------
## -------------------------------   PART-3   ----------------------------------
## --------------------------- MOD09GA PROCESSING ------------------------------
#
# ---------------------------------------------------------- Import libraries :
#import os
#import fnmatch
#import ogr
#import datetime
#import dateutil.rrule
#import numpy as np
#import pandas as pd
#import requests
#import time
#from osgeo import gdal, gdalconst
#import pylab as plt
#
#DOY_start = 90
#DOY_end   = 150
#k_len = DOY_end - DOY_start +1
#
#main_folder = 'I:/ix_ToGreg_MODIS.rice.flood.mng.map/MODIS_imgs/'
#
#fld_CLOUD = main_folder + 'MOD09GA_2003/'
#fld_NDVI  = main_folder + 'MOD09GQ_2003/'
#
#
#
#cnt = 0
#for time_stamp in range(DOY_start, DOY_end + 1):
#    print cnt+1, ' on ', k_len
#    
#    if len(str(time_stamp)) == 3:
#        time_stamp = str(time_stamp)
#    if len(str(time_stamp)) == 2:
#        time_stamp = '0' + str(time_stamp)
#    
#    pattern    = '*A2003' + time_stamp + '*'
#    file_CLOUD = fnmatch.filter(os.listdir(fld_CLOUD), pattern + '.tif')
#    file_NDVI  = fnmatch.filter(os.listdir(fld_NDVI), pattern + '.hdf')
#
#    # Open the cloud data :
#    CLOUD_mask = gdal.Open(fld_CLOUD + file_CLOUD[0]).ReadAsArray().astype(float)
#
#    # Open the RED and NIR bands and compute NDVI index :
#    hdf_ds = gdal.Open(fld_NDVI + file_NDVI[0])
#    red  = gdal.Open(hdf_ds.GetSubDatasets()[1][0]).ReadAsArray().astype(float)
#    nir  = gdal.Open(hdf_ds.GetSubDatasets()[2][0]).ReadAsArray().astype(float)
#    ndvi = (nir - red) / (nir + red)
#    NDVI_masked = np.where(CLOUD_mask == 0, 0, ndvi)
#
#    # Save the NDVI image as a geotiff :
#    driver  = gdal.GetDriverByName("GTiff")
#    out_tif = fld_NDVI + file_NDVI[0]
#    out_tif = out_tif.replace(out_tif[-17:], 'NDVI.ClMasked.tif')
#    
#    dst_ds  = driver.Create(out_tif, len(NDVI_masked[0]), len(NDVI_masked[0]), 1, gdal.GDT_Float32)
#    QA_flgs = gdal.Open(fld_CLOUD + file_CLOUD[0])
#    QA_tran = QA_flgs.GetGeoTransform()  
#    QA_proj = QA_flgs.GetProjection()
#    dst_ds.SetGeoTransform(QA_tran)
#    dst_ds.SetProjection(QA_proj)                          
#    dst_ds.GetRasterBand(1).WriteArray(NDVI_masked)
#    dst_ds = QA_flgs = QA_mask_res = None                     
#    
#    cnt = cnt+1
#    print time.ctime()[11:19], ' - ', out_tif



# -----------------------------------------------------------------------------
# -------------------------------   PART-4   ----------------------------------
# ---------------------- MOD09GA and  NDVI and LWSI @ 500m --------------------
## NOTES on MOD09GQ :
## blue = band_3_MODIS = [band-13]
## nir  = band_2_MODIS = [band-12]   
## red  = band_1_MODIS = [band-11]
## swir = band_7_MODIS = [band-17]
## ndvi = (nir - red) / (nir + red) 
## ndfi = (red - swir) / (nir + swir)
#
#import os
#import fnmatch
#import ogr
#import datetime
#import dateutil.rrule
#import numpy as np
#import pandas as pd
#import requests
#import time
#from osgeo import gdal, gdalconst
#import pylab as plt
#
#DOY_start = 90
#DOY_end   = 150
#k_len = DOY_end - DOY_start + 1
#
#main_folder = 'I:/ix_ToGreg_MODIS.rice.flood.mng.map/MODIS_imgs/MOD09GA_2003/'
#
#cnt = 0
#for time_stamp in range(DOY_start, DOY_end + 1):
#    print cnt+1, ' on ', k_len
#    
#    if len(str(time_stamp)) == 3:
#        time_stamp = str(time_stamp)
#    if len(str(time_stamp)) == 2:
#        time_stamp = '0' + str(time_stamp)
#    
#    # Select files :
#    pattern  = '*A2003' + time_stamp + '*'
#    file_hdf = fnmatch.filter(os.listdir(main_folder), pattern + '.hdf')
#    file_cld = fnmatch.filter(os.listdir(main_folder), pattern + '.QA-Cloud.Res500.tif')
#
#    # Open the cloud data (1=cloud-free / 0=cloud) :
#    cld_mask = gdal.Open(main_folder + file_cld[0]).ReadAsArray().astype(float)
#    
#    # Open the RED, NIR and SWIR bands and compute NDVI and NDFI indices :
#    hdf_ds = gdal.Open(main_folder + file_hdf[0])     
#    blue = gdal.Open(hdf_ds.GetSubDatasets()[13][0]).ReadAsArray().astype(float) # b3
#    red  = gdal.Open(hdf_ds.GetSubDatasets()[11][0]).ReadAsArray().astype(float) # b1
#    nir  = gdal.Open(hdf_ds.GetSubDatasets()[12][0]).ReadAsArray().astype(float) # b2
#    swir = gdal.Open(hdf_ds.GetSubDatasets()[17][0]).ReadAsArray().astype(float) # b7  
#    ndvi = (nir - red) / (nir + red)
#    ndfi = (red - swir) / (red + swir)
#
#    #RiceFlood = np.where(((ndvi <= 0.4) & (ndfi > 0.) & (cld_mask == 1) & (blue <= 1800)), 1, 0) # Valuta se eliminare la maschera cloud !
#    #RiceFlood = np.where(((ndvi <= 0.4) & (ndfi > 0.32) & (blue <= 1800)), 1, 0)
#    RiceFlood = np.where(((ndvi <= 0.4) & (ndfi > 0.) & (blue <= 1800)), 1, 0)
#    
#    # Geo-references :
#    ref_file = gdal.Open(main_folder + file_cld[0])
#    tr       = ref_file.GetGeoTransform()  
#    pr       = ref_file.GetProjection()   
#    
#    # Save the NDVI image as a geotiff :
#    driver  = gdal.GetDriverByName("GTiff")
#    out_tif = file_cld[0].replace('QA-Cloud.Res500', 'NDVI')    
#    out_tif = main_folder + out_tif
#    dst_ds  = driver.Create(out_tif, len(ndvi[0]), len(ndvi[0]), 1, gdal.GDT_Float32)    
#    dst_ds.SetGeoTransform(tr)
#    dst_ds.SetProjection(pr)                          
#    dst_ds.GetRasterBand(1).WriteArray(ndvi)
#    dst_ds = None
#    print time.ctime()[11:19], ' - ', out_tif
#    
#    # Save the NDFI image as a geotiff :
#    driver  = gdal.GetDriverByName("GTiff")
#    out_tif = file_cld[0].replace('QA-Cloud.Res500', 'NDFI')    
#    out_tif = main_folder + out_tif
#    dst_ds  = driver.Create(out_tif, len(ndfi[0]), len(ndfi[0]), 1, gdal.GDT_Float32)    
#    dst_ds.SetGeoTransform(tr)
#    dst_ds.SetProjection(pr)                          
#    dst_ds.GetRasterBand(1).WriteArray(ndfi)
#    dst_ds = None
#    print time.ctime()[11:19], ' - ', out_tif
#    
#    # Save the RiceFlood image as a geotiff :
#    driver  = gdal.GetDriverByName("GTiff")
#    out_tif = file_cld[0].replace('QA-Cloud.Res500', 'RiceFloodMask.NDFIgt0')    
#    out_tif = main_folder + out_tif
#    dst_ds  = driver.Create(out_tif, len(RiceFlood[0]), len(RiceFlood[0]), 1, gdal.GDT_Byte)   
#    dst_ds.SetGeoTransform(tr)
#    dst_ds.SetProjection(pr)                          
#    dst_ds.GetRasterBand(1).WriteArray(RiceFlood)
#    dst_ds = None                     
#    
#    cnt = cnt+1
#    print time.ctime()[11:19], ' - ', out_tif
#    print
#
#print 'Done!'




# -----------------------------------------------------------------------------
# -------------------------------   PART-5   ----------------------------------
# --------------------------- STACK MODIS data ------------------------------
import os
import fnmatch
import ogr
import datetime
import dateutil.rrule
import numpy as np
import pandas as pd
import requests
import time
from osgeo import gdal, gdalconst
    
# --- Input folder parsing and NDVI data retrieving :
warea   = 'D:/WA_GIS_Matteo/ix_ToGreg_MODIS.rice.flood.mng.map/MODIS_LST/1_MOD11A1/QualityIndicator_Int8/'
pattern = '*NighttimeLSTQualityIndicators*'                                 # !!!
str_res = []
for root, dirs, files in os.walk(warea):
    for name in files:
        if fnmatch.fnmatch(name, pattern):
            str_res.append(os.path.join(root, name))
str_res = np.sort(str_res)

# --- Create the output string :
geo_rf = str_res[0]
geo_rf2= str_res[-1]
out_pt = geo_rf[0:110] + 'to' + geo_rf2[102:122] + 'STACK' + geo_rf[135:] # 154 or 156      # !!!

# --- Use the first image as a slave for data and geo references :            
ref_geo = gdal.Open(geo_rf)
geo     = ref_geo.GetGeoTransform()  
proj    = ref_geo.GetProjection()
slave   = gdal.Open(geo_rf)
sl_data = slave.ReadAsArray()

# --- Create the driver of the output NDVI stack :
# (IMPORTANT NOTE: during driver creation of 3D-array, you have to feed in the order: [Lines], [Columns] and [Bands]).
driver = gdal.GetDriverByName("GTiff")
dst_ds = driver.Create(out_pt, sl_data.shape[0], sl_data.shape[1], len(str_res), gdal.GDT_Byte)          # !!!    
dst_ds.SetGeoTransform(geo)
dst_ds.SetProjection(proj)
geo_rf = slave = sl_data = None

# --- Save band-wise the output NDVI stack :
for bnd in range(len(str_res)):
    print 'Writing on disk image N.', (bnd+1), ' on ', len(str_res)    
    ImageOfTheStack = gdal.Open(str_res[bnd])
    Image_dt = ImageOfTheStack.ReadAsArray()
    dst_ds.GetRasterBand(bnd+1).WriteArray(Image_dt)
    dst_ds.GetRasterBand(bnd+1).FlushCache()

dst_ds = out_pt = None

print '--------------------------> End stacking : ',  time.ctime()



# -----------------------------------------------------------------------------
# -------------------------------   PART-6   ----------------------------------
# ----------- MOD MYD 11A2 EXTRACT DAYNIGHT TIME LST AND FLAGS ----------------
# https://gis.stackexchange.com/questions/168910/gdal-python-extracting-projection-info-from-hdf-file

# Layer 0 = Daytime Land Surface Temperature, Kelvin, 16-bit unsigned integer, 	
#           0, 7500 to 65535, scale factor 0.02

# Layer 4 = Nighttime Land Surface Temperature,	Kelvin, 16-bit unsigned integer, 	
#           0, 7500 to 65535, scale factor 0.02

#from   osgeo import gdal, gdalconst
#import numpy as np
#import pylab as plt
#import fnmatch
#import time
#import os
#
#root_in = 'D:/WA_GIS_Matteo/ix_ToGreg_MODIS.rice.flood.mng.map/MODIS_LST/1_MOD11A1/'
#
#root_fl = np.sort(fnmatch.filter(os.listdir(root_in), '*.hdf'))
#Mandatory_QA_flags = 'on'
#
#print time.ctime()
#
#cnt=0
#for x in root_fl:
#    print cnt+1, ' on ', len(root_fl)
#    
#    stringa = root_in + x                                # Set the input string
#    hdf_ds  = gdal.Open(stringa)                       # open the HDF container
#    
#    # Open the subdataset "Daytime Land Surface Temperature" and scale it :
#    dt_lst = gdal.Open(hdf_ds.GetSubDatasets()[0][0], gdal.GA_ReadOnly).ReadAsArray()
#    dt_lst = dt_lst * 0.02
#    sizes  = np.shape(dt_lst)
#    MandatoryQAflags_day = dt_lst * 0
#    
#    if Mandatory_QA_flags == 'on':
#        QA_flgs = gdal.Open(hdf_ds.GetSubDatasets()[1][0], gdal.GA_ReadOnly).ReadAsArray()
#        
#        # Derive Mandatory QA flags informations :
#        # I would keep only the case: bit 0 = 0 and bit 1 = 0
#        for l in range(sizes[0]):
#            for c in range(sizes[1]):
#                mask_bit = 0b0000000000000011                                  # This retrieve the 4 combinations of 2 bits
#                QA_val   = (QA_flgs[l,c] & mask_bit)
#                MandatoryQAflags_day[l,c] = QA_val
#
#        dt_lst_QAmask = np.where(MandatoryQAflags_day == 0, dt_lst, 0)        
#    
#    # Open the subdataset "Nighttime Land Surface Temperature" and scale it :
#    nt_lst = gdal.Open(hdf_ds.GetSubDatasets()[4][0], gdal.GA_ReadOnly).ReadAsArray()
#    nt_lst = nt_lst * 0.02 
#    sizes  = np.shape(nt_lst)
#    MandatoryQAflags_night = nt_lst * 0   
#
#    if Mandatory_QA_flags == 'on':
#        QA_flgs = gdal.Open(hdf_ds.GetSubDatasets()[5][0], gdal.GA_ReadOnly).ReadAsArray()
#        
#        # Derive Mandatory QA flags informations :
#        # I would keep only the case: bit 0 = 0 and bit 1 = 0
#        for l in range(sizes[0]):
#            for c in range(sizes[1]):
#                mask_bit = 0b0000000000000011
#                QA_val   = (QA_flgs[l,c] & mask_bit)
#                MandatoryQAflags_night[l,c] = QA_val
#
#        nt_lst_QAmask = np.where(MandatoryQAflags_night == 0, nt_lst, 0)
#
#    sizes  = np.shape(nt_lst)    
#        
#    # Save "Daytime Land Surface Temperature" in a geotiff file format :
#    driver  = gdal.GetDriverByName("GTiff")
#    out_tif = stringa.replace('.hdf', '.DaytimeLST.tif')
#    dst_ds  = driver.Create(out_tif, sizes[1], sizes[1], 1, gdal.GDT_Float32)
#    dat_ref = gdal.Open(hdf_ds.GetSubDatasets()[0][0])                  # Slave
#    dt_tran = dat_ref.GetGeoTransform()  
#    dt_proj = dat_ref.GetProjection()
#    dst_ds.SetGeoTransform(dt_tran)
#    dst_ds.SetProjection(dt_proj)                          
#    dst_ds.GetRasterBand(1).WriteArray(dt_lst)
#    dst_ds = dt_lst = dat_ref = None                     
#
#    # Save "Nighttime Land Surface Temperature" in a geotiff file format :
#    driver  = gdal.GetDriverByName("GTiff")
#    out_tif = stringa.replace('.hdf', '.NighttimeLST.tif')
#    dst_ds  = driver.Create(out_tif, sizes[1], sizes[1], 1, gdal.GDT_Float32)
#    dat_ref = gdal.Open(hdf_ds.GetSubDatasets()[0][0])                  # Slave
#    dt_tran = dat_ref.GetGeoTransform()  
#    dt_proj = dat_ref.GetProjection()
#    dst_ds.SetGeoTransform(dt_tran)
#    dst_ds.SetProjection(dt_proj)                          
#    dst_ds.GetRasterBand(1).WriteArray(nt_lst)
#    dst_ds = nt_lst = dat_ref = None                     
#
#    if Mandatory_QA_flags == 'on':
#        # Save "MandatoryQAflags" of "Daytime Land Surface Temperature" as tif:
#        driver  = gdal.GetDriverByName("GTiff")
#        out_tif = stringa.replace('.hdf', '.DaytimeLST_MandatoryQAflags.tif')
#        dst_ds  = driver.Create(out_tif, sizes[1], sizes[1], 1, gdal.GDT_Byte)
#        dat_ref = gdal.Open(hdf_ds.GetSubDatasets()[0][0])              # Slave
#        dt_tran = dat_ref.GetGeoTransform()  
#        dt_proj = dat_ref.GetProjection()
#        dst_ds.SetGeoTransform(dt_tran)
#        dst_ds.SetProjection(dt_proj)                          
#        dst_ds.GetRasterBand(1).WriteArray(MandatoryQAflags_day)
#        dst_ds = MandatoryQAflags_day = dat_ref = None         
#
#        # Save the masked version of "Daytime Land Surface Temperature" as tif:
#        driver  = gdal.GetDriverByName("GTiff")
#        out_tif = stringa.replace('.hdf', '.DaytimeLST_Masked-MandQAflags.tif')
#        dst_ds  = driver.Create(out_tif, sizes[1], sizes[1], 1, gdal.GDT_Float32)
#        dat_ref = gdal.Open(hdf_ds.GetSubDatasets()[0][0])              # Slave
#        dt_tran = dat_ref.GetGeoTransform()  
#        dt_proj = dat_ref.GetProjection()
#        dst_ds.SetGeoTransform(dt_tran)
#        dst_ds.SetProjection(dt_proj)                          
#        dst_ds.GetRasterBand(1).WriteArray(dt_lst_QAmask)
#        dst_ds = dt_lst_QAmask = dat_ref = None
#
#        # Save "MandatoryQAflags" of "Nightime Land Surface Temperature" as tif:
#        driver  = gdal.GetDriverByName("GTiff")
#        out_tif = stringa.replace('.hdf', '.NighttimeLST_MandatoryQAflags.tif')
#        dst_ds  = driver.Create(out_tif, sizes[1], sizes[1], 1, gdal.GDT_Byte)
#        dat_ref = gdal.Open(hdf_ds.GetSubDatasets()[0][0])              # Slave
#        dt_tran = dat_ref.GetGeoTransform()  
#        dt_proj = dat_ref.GetProjection()
#        dst_ds.SetGeoTransform(dt_tran)
#        dst_ds.SetProjection(dt_proj)                          
#        dst_ds.GetRasterBand(1).WriteArray(MandatoryQAflags_night)
#        dst_ds = MandatoryQAflags_night = dat_ref = None 
#
#        # Save the masked version of "Nighttime Land Surface Temperature" as tif:
#        driver  = gdal.GetDriverByName("GTiff")
#        out_tif = stringa.replace('.hdf', '.NighttimeLST_Masked-MandQAflags.tif')
#        dst_ds  = driver.Create(out_tif, sizes[1], sizes[1], 1, gdal.GDT_Float32)
#        dat_ref = gdal.Open(hdf_ds.GetSubDatasets()[0][0])              # Slave
#        dt_tran = dat_ref.GetGeoTransform()  
#        dt_proj = dat_ref.GetProjection()
#        dst_ds.SetGeoTransform(dt_tran)
#        dst_ds.SetProjection(dt_proj)                          
#        dst_ds.GetRasterBand(1).WriteArray(nt_lst_QAmask)
#        dst_ds = nt_lst_QAmask = dat_ref = None
#
#
#    cnt = cnt+1
#
#    print time.ctime()[11:19], ' - ', out_tif[-28:-19]
#    print
#
#
#print 'Done!' 



## -----------------------------------------------------------------------------
## -------------------------------   PART-7   ----------------------------------
## ------ MOD MYD 11A2 EXTRACT Daytime & Nighttime LST Quality Indicators ------
#
#from   osgeo import gdal, gdalconst
#import numpy as np
#import pylab as plt
#import fnmatch
#import time
#import os
#
#root_in = 'D:/WA_GIS_Matteo/ix_ToGreg_MODIS.rice.flood.mng.map/MODIS_LST/2_MYD11A1/'
#root_fl = np.sort(fnmatch.filter(os.listdir(root_in), '*.hdf'))
#
#print time.ctime()
#
#cnt=0
#for x in root_fl:
#    print cnt+1, ' on ', len(root_fl)
#    
#    stringa = root_in + x                                # Set the input string
#    hdf_ds  = gdal.Open(stringa)                       # open the HDF container
#    
#    # Open the subdataset "Daytime LST Quality Indicators" :
#    dt_QIflag = gdal.Open(hdf_ds.GetSubDatasets()[1][0], gdal.GA_ReadOnly).ReadAsArray()
#
#    # Open the subdataset "Nighttime LST Quality indicators" :
#    nt_QIflag = gdal.Open(hdf_ds.GetSubDatasets()[5][0], gdal.GA_ReadOnly).ReadAsArray()
#    sizes  = np.shape(nt_QIflag)
#
#    # Save "Daytime LST Quality Indicators" in a geotiff file format :
#    driver  = gdal.GetDriverByName("GTiff")
#    out_tif = stringa.replace('.hdf', '.DaytimeLSTQualityIndicators.tif')
#    dst_ds  = driver.Create(out_tif, sizes[1], sizes[1], 1, gdal.GDT_Byte)
#    dat_ref = gdal.Open(hdf_ds.GetSubDatasets()[0][0])                  # Slave
#    dt_tran = dat_ref.GetGeoTransform()  
#    dt_proj = dat_ref.GetProjection()
#    dst_ds.SetGeoTransform(dt_tran)
#    dst_ds.SetProjection(dt_proj)                          
#    dst_ds.GetRasterBand(1).WriteArray(dt_QIflag)
#    dst_ds = dt_QIflag = dat_ref = None                     
#
#    # Save "Nighttime Land Surface Temperature" in a geotiff file format :
#    driver  = gdal.GetDriverByName("GTiff")
#    out_tif = stringa.replace('.hdf', '.NighttimeLSTQualityIndicators.tif')
#    dst_ds  = driver.Create(out_tif, sizes[1], sizes[1], 1, gdal.GDT_Byte)
#    dat_ref = gdal.Open(hdf_ds.GetSubDatasets()[0][0])                  # Slave
#    dt_tran = dat_ref.GetGeoTransform()  
#    dt_proj = dat_ref.GetProjection()
#    dst_ds.SetGeoTransform(dt_tran)
#    dst_ds.SetProjection(dt_proj)                          
#    dst_ds.GetRasterBand(1).WriteArray(nt_QIflag)
#    dst_ds = nt_QIflag = dat_ref = None  
#
#    cnt = cnt + 1
#
#print 'Done!', time.ctime()





######################################################################## NOTES:
## https://gis.stackexchange.com/questions/57834/how-to-get-raster-corner-coordinates-using-python-gdal-bindings
#from osgeo import ogr
#from osgeo import osr
#
#src = gdal.Open(hdf_ds.GetSubDatasets()[1][0])
#ulx, xres, xskew, uly, yskew, yres  = src.GetGeoTransform()
#lrx = ulx + (src.RasterXSize * xres)
#lry = uly + (src.RasterYSize * yres)
#
## Setup the source projection - you can also import from epsg, proj4...
#source = osr.SpatialReference()
#source.ImportFromWkt(src.GetProjection())
#
## The target projection
#target = osr.SpatialReference()
#target.ImportFromEPSG(4326)
#
## Create the transform - this can be used repeatedly
#transform = osr.CoordinateTransformation(source, target)
#
## Transform the point. You can also create an ogr geometry and use the more generic `point.Transform()`
#transform.TransformPoint(ulx, uly)