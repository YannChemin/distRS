# -*- coding: utf-8 -*-
"""
Created on Thu Jan 23 2019 09:45:01
WA             : WA with the partner from Thessaloniki University, testing
                 different approach to force the WARM model with EO data
DATE & VERSION : Finalized on Jan, 2020
INPUT          : yearly stacks of EVI and Acquisition-DOY from MOD&MYD13Q1
                 from 2000 to 2019
OUTPUT         : (i)  retrieve maps of PHENO OCCURRENCES and
                 (ii) statistics aggregated to a reference meteo grid of 25km
AUTHOR         : Giacinto Manfron | giacinto.manfron@ec.europa.eu
NOTES          : x
WEB REFERENCES : x
@author: manfrgi
"""

# --- Libraries :
import os
import numpy as np
import scipy as sp
import fnmatch
import time
import matplotlib.pyplot as plt
from osgeo import gdal

# --- Patches :
main_root = 'G:/0_TELEWORKING/WA_20190708_RS.Data.Assimilation.Into.Crop.Models/1_MODIS_TS_MODMYD13/'
ref_root  = main_root + '2_MODIS-PREPROCESSING/MasterGreece/' 
in_root   = main_root + '3_MODIS-MODMYD13-STACKING/'                           # Repository where I stored yearly 3D-Arrays of MODIS-EVI and MODIS-DOYqf.
out_root  = main_root + '4_MODISMODMYD13-TSANALYSIS/'
ref_rice  = ref_root  + 'Master_MYD13Q1_Resize_RiceMask.tif'                   # To reach a reference file, regarding the presence of rice fields (from Corine).
ref_mgrid = ref_root  + 'Master_MYD13Q1_Resize_MeteoGrid_Mask.tif'             # To reach a reference file, regarding the meteo-grid.
outf_path = out_root  + 'MODIS250m_TimeSeriesAnalyses_EstPhenoOcc_MeteoGridAggrStats_v2.csv'


# --- Parameters :
start_Y  = 2003
end_Y    = 2019
cnt1     = 1
Grid_cd  = [1060153, 1059152, 1059153, 1062154, 1060152]                       # 25km meteo grid codes


# --- P R O C E S S I N G :
print 'START processing: MODIS250_ts_Analysis.py', time.ctime()
print
    
# 01. OPEN THE 'RICE REFERENCE MASK' AND THE 'METEO-GRID REFERENCE MASK' :
ref_ds1   = gdal.Open(ref_rice)
msk_rice  = ref_ds1.ReadAsArray()
smt_px    = np.where(msk_rice != 0)
ref_geo   = ref_ds1.GetGeoTransform()  
ref_prj   = ref_ds1.GetProjection()
ref_lne   = msk_rice.shape[0]
ref_cln   = msk_rice.shape[1]
ref_ds2   = gdal.Open(ref_mgrid)
msk_mgrid = ref_ds2.ReadAsArray()

# 02. OPEN THE OUTPUT '.CSV' USED TO THEN WRITE (IN APPEND) PHENO-STATISTICS :
with open(outf_path,'a') as fd:
    Header_Descriptors = "YEAR,GRID_CODE,DOY_MIN_MEAN,DOY_MIN_MEDIAN,DOY_MIN_STD,DOY_SOS_MEAN,DOY_SOS_MEDIAN,DOY_SOS_STD,DOY_MAX_MEAN,DOY_MAX_MEDIAN,DOY_MAX_STD,DOY_EOS_MEAN,DOY_EOS_MEDIAN,DOY_EOS_STD"
    MyHeader = Header_Descriptors + '\n'
    fd.write(MyHeader)
    fd.close()

    
for yyyy in range(start_Y,end_Y+1):
    print 'Analyzing data for:', yyyy, time.ctime()[11:19]
    
    # 02. OPEN THE OUTPUT '.CSV' TO STORE (IN APPEND) THE SMOOTHED TIME SERIES :
    outf_pat2 = out_root  + 'MODIS250m_TimeSeriesAnalyses_TimeSeries_250m.pxLev_' + str(yyyy) + '.csv'
    Header_Descriptors2 = "YEAR,GRID_CODE,TS,LIN,COL,"
    for d in range(1,366):                                               # 1 to 365
        Header_Descriptors2 = Header_Descriptors2 + str(d) + ','
    Header_Descriptors2 = Header_Descriptors2[:-1]    
        
    with open(outf_pat2,'a') as fd2:
        MyHeader2 = Header_Descriptors2 + '\n'
        fd2.write(MyHeader2)
        fd2.close()
        
    # 02. OPEN INPUT STACKS :
    ptrn_0  = '*.' + str(yyyy) + 'EVI' + '.*.tif'
    in_EVI  = fnmatch.filter(os.listdir(in_root), ptrn_0)
    EVI_ds  = gdal.Open(in_root + in_EVI[0])
    EVI_3d  = EVI_ds.ReadAsArray()   
    ptrn_1  = '*.' + str(yyyy) + 'DOY' + '.*.tif'
    in_DOY  = fnmatch.filter(os.listdir(in_root), ptrn_1)
    DOY_ds  = gdal.Open(in_root + in_DOY[0])
    DOY_3d  = DOY_ds.ReadAsArray()
    ref_bnd = EVI_3d.shape[0]
    
    # 02. OPEN ONE OUTPUT ARRAY FOR EACH OF THE EIGHT RESULTING PHENOMAPS
    map_MAXval = np.zeros([msk_rice.shape[0],msk_rice.shape[1]], dtype=float)
    map_MAXdoy = np.zeros([msk_rice.shape[0],msk_rice.shape[1]], dtype=float)
    map_MINval = np.zeros([msk_rice.shape[0],msk_rice.shape[1]], dtype=float)
    map_MINdoy = np.zeros([msk_rice.shape[0],msk_rice.shape[1]], dtype=float)
    map_SOSval = np.zeros([msk_rice.shape[0],msk_rice.shape[1]], dtype=float)
    map_SOSdoy = np.zeros([msk_rice.shape[0],msk_rice.shape[1]], dtype=float)
    map_EOSval = np.zeros([msk_rice.shape[0],msk_rice.shape[1]], dtype=float)
    map_EOSdoy = np.zeros([msk_rice.shape[0],msk_rice.shape[1]], dtype=float)
    
    # 03. LOOP THROUGH RICE RELATED PIXELS AND SMOOTH EVI PROFILES :
    for px in range(len(smt_px[0])):
        
        # . Pick up EVI & DOY pixel (rice related) profiles :
        lne = smt_px[0][px]
        col = smt_px[1][px]
        m_grid  = str(Grid_cd[msk_mgrid[lne,col]-1])
        EVI_raw = EVI_3d[:,lne,col]
        DOY_raw = DOY_3d[:,lne,col]
        px_info = 'Line_' + str(lne) + '_Column_' + str(col)                
        if px % 500 == 0: 
            print str(px+1) + ' on ' + str(len(smt_px[0])) + ' ' + px_info + ' ' + time.ctime()[11:19]
        
        # . Use DOY info to sort chronologically EVI values :
        DOY_sort = np.argsort(DOY_raw)
        EVI_sorted = EVI_raw[DOY_sort]
        DOY_sorted = DOY_raw[DOY_sort]
            
        # . Here I need DOY data as Integer to be used as indices and I need to
        #   remove nan, which are stored by defoult at the end of the array :
        DOY_sorted_cln = DOY_sorted[np.logical_not(np.isnan(DOY_sorted))].astype(int)      
        EVI_sorted_cln = EVI_sorted[0:len(DOY_sorted_cln)]
        
        # . Here I need to place DOY and EVI values on a daily time line :
        EVI_raw_y  = np.arange(np.nanmax(DOY_sorted)+1)[1:] * 0
        AllThe_pos = np.arange(len(EVI_sorted_cln))
        #   (Here -1 is to pass from DOY to index)
        EVI_raw_y[DOY_sorted_cln-1] = EVI_sorted_cln[AllThe_pos]
        
        # . Bring EVI_raw_y to the lenght of 365 days if it is < 365 :
        if len(EVI_raw_y) < 365:
            add_d = 365 - len(EVI_raw_y)
            for d in range(add_d):
                EVI_raw_y = np.append(EVI_raw_y, 0)
        
        # . In the time series to be smoothed I need at least 3 clean values :           
        if len(np.where(EVI_raw_y != 0)[0]) > 2:     
            # 04. WHITTAKER SMOOTHING OF EVI_raw, ON A DAILY TIME STAMP :        
            lmbda = 100000
            m  = len(EVI_raw_y) 
            d1 = -1 * np.ones((m), dtype='d')
            d2 =  3 * np.ones((m), dtype='d')
            d3 = -3 * np.ones((m), dtype='d')
            d4 =      np.ones((m), dtype='d')
            D  = sp.sparse.diags([d1,d2,d3,d4], [0,1,2,3], shape=(m-3, m), format="coo")
            w  = np.where(EVI_raw_y == 0, 0, 1)        
            W  = sp.sparse.diags(w, 0, shape=(m,m), format="coo") 
            t  = W + lmbda * (D.transpose()).dot(D)
            C  = np.linalg.cholesky(t.toarray()).transpose()       
            z  = np.linalg.solve(C,np.linalg.solve(C.transpose(),(w*np.asarray(EVI_raw_y))))  
            
            # . Bring z (EVI smooth) to the lenght of 365 days if it is > 365 :
            if len(z) > 365:
                z = z[0:365]
                EVI_raw_y = EVI_raw_y[0:365]
    
            # 05. INDIVIDUATION OF PHENOLOGICAL OCCURRENCES :
            # . Relative MIN and relative MAX identification :
            shift_1 = np.where((z - np.roll(z, 1) > 0), 1, 0)
            shift_2 = np.where((z - np.roll(z,-1) > 0), 1, 0)
            loc_MAXMIN = shift_1 + shift_2 # 2 = Loc_MAX and 0 == Loc_MIN
            
            # . Agronomical MAX - Proxy of crop heading :
            #   - in case of more than 1 values, takes the higher. (MAX.1)
            #   - is accepted only into the DOY 153 to DOY 244 time window. (MAX.2)
            MAX_val = z[loc_MAXMIN == 2]
            
            if len(np.atleast_1d(MAX_val)) > 1:                                # (MAX.1)
                MAX_val = np.sort(z[loc_MAXMIN == 2])[-1]
                
            MAX_doy = np.where(z == MAX_val)[0][0] + 1
            
            if (MAX_doy > 153) and (MAX_doy < 255):                            # (MAX.2)        
                # . Agronomical MIN - Proxy of crop sowing :
                #   - search the average VI value in a 60 DOYs time window starting
                #     120 days before heading and ending 60 DOYs before heading.               
                MIN_ref = np.average(z[(MAX_doy - 121) : (MAX_doy - 61)])
                MIN_val = z[np.where(z[(MAX_doy - 121) : (MAX_doy-61)] > MIN_ref)[0][0] + 60]
                MIN_doy = np.where(z == MIN_val)[0][0] + 1
                 
                # . Agronomical SOS - proxy of Start of Season :
                #   - search the occurrence of +20% the EVI value at the MIN
                #   - If the profile has large plateau values, it became impossible
                #     to find "1.2 * MIN_val" in the profile, then I put all the 
                #     occurrences of these cases == 999. (SOS.1)
                if z[MIN_doy-1 :].max() < 1.2 * MIN_val:                  # (SOS.1)
                    MAX_doy = MAX_val = MIN_doy = MIN_val = SOS_val = SOS_doy = EOS_val = EOS_doy = 999                
                else:
                    SOS_val = z[np.where(z[MIN_doy-1 :] > (1.2 * MIN_val)) + MIN_doy-1][0][0]
                    SOS_doy = np.where(z == SOS_val)[0][0] + 1
        
                    # . Agronomical EOS - proxy of Crop Maturity :
                    #   - search a decrease in EVI signal corresponding to 50% the 
                    #     sowing-heading seasonal increase (i.e. MAX_val and MIN_val).  
                    EOS_treshold = MAX_val - ((MAX_val-MIN_val)/2)
                    if z[MAX_doy-1 :].min() > EOS_treshold:                  # (EOS.1)
                        MAX_doy = MAX_val = MIN_doy = MIN_val = SOS_val = SOS_doy = EOS_val = EOS_doy = 999
                    else:
                        EOS_val = z[np.where(z[MAX_doy-1 :] < EOS_treshold) + MAX_doy-1][0][0]
                        EOS_doy = np.where(z == EOS_val)[0][0] + 1  
                                        
                # . Update output phenomaps :            
                map_MINval[lne,col] = MIN_val
                map_MINdoy[lne,col] = MIN_doy
                map_SOSval[lne,col] = SOS_val
                map_SOSdoy[lne,col] = SOS_doy 
                map_MAXval[lne,col] = MAX_val
                map_MAXdoy[lne,col] = MAX_doy            
                map_EOSval[lne,col] = EOS_val
                map_EOSdoy[lne,col] = EOS_doy
            
            if (MAX_doy <= 153) and (MAX_doy >= 255):            
                map_MINval[lne,col] = 999
                map_MINdoy[lne,col] = 999
                map_SOSval[lne,col] = 999
                map_SOSdoy[lne,col] = 999 
                map_MAXval[lne,col] = 999
                map_MAXdoy[lne,col] = 999                       
                map_EOSval[lne,col] = 999
                map_EOSdoy[lne,col] = 999
                          
            # If a time series analyses is performed, save raw and smooth TSs :
            if map_MINval[lne,col] != 999:
                
                RAW_profile = str(yyyy) + ',' + m_grid + ',EVI_Raw,' + str(lne) + ',' + str(col) + ','
                for r in (EVI_raw_y):
                    RAW_profile = RAW_profile + str(r) + ','
                RAW_print = RAW_profile + '\n'
                with open(outf_pat2,'a') as fd2:
                    fd2.write(RAW_print)
                    fd2.close()                    
                
                z = np.round(z/10000,3)
                
                SMT_profile = str(yyyy) + ',' + m_grid + ',EVI_Smt,' + str(lne) + ',' + str(col) + ','
                for s in (z):
                    SMT_profile = SMT_profile + str(s) + ','
                SMT_print = SMT_profile + '\n'
                with open(outf_pat2,'a') as fd2:
                    fd2.write(SMT_print)
                    fd2.close()                    

                
    # 06. DERIVE YEARLY OUTPUT STATISTICS AGGREGATED AT METEO-GRID LEVEL :
    ref_mgrid_val = np.unique(msk_mgrid)[1:]
    
    for id_grid in ref_mgrid_val:
        print id_grid
        
        info_to_print = [yyyy, Grid_cd[id_grid-1]]
        
        # --- MINdoy :
        smpls = map_MINdoy[np.where(msk_mgrid == id_grid)]
        smpls = smpls[smpls != 0]
        smpls = smpls[smpls != 999]
        info_to_print.append(np.mean(smpls).round(1))
        info_to_print.append(np.median(smpls).round(1))
        info_to_print.append(np.std(smpls).round(1))
        
        # --- SOSdoy :
        smpls = map_SOSdoy[np.where(msk_mgrid == id_grid)]
        smpls = smpls[smpls != 0]
        smpls = smpls[smpls != 999]
        info_to_print.append(np.mean(smpls).round(1))
        info_to_print.append(np.median(smpls).round(1))
        info_to_print.append(np.std(smpls).round(1))
        
        # --- MAXdoy :
        smpls = map_MAXdoy[np.where(msk_mgrid == id_grid)]
        smpls = smpls[smpls != 0]
        smpls = smpls[smpls != 999]
        info_to_print.append(np.mean(smpls).round(1))
        info_to_print.append(np.median(smpls).round(1))
        info_to_print.append(np.std(smpls).round(1))
        
        # --- EOSdoy :
        smpls = map_EOSdoy[np.where(msk_mgrid == id_grid)]
        smpls = smpls[smpls != 0]
        smpls = smpls[smpls != 999]
        info_to_print.append(np.mean(smpls).round(1))
        info_to_print.append(np.median(smpls).round(1))
        info_to_print.append(np.std(smpls).round(1))
        
        str_to_print = str()
        for e in info_to_print:
            str_to_print = str_to_print + str(e) + ','
        
        str_to_print = str_to_print[0:-1] + '\n'

        with open(outf_path, 'a') as fd:
            fd.write(str_to_print)
            fd.close()

    # 06. SAVE THE 8 OUTPUT MAPS :    
    # MAP-MINval : #1
    outstr_MINval = out_root + str(yyyy) + '_MAP-MINval.tif' #2
    driver = gdal.GetDriverByName("GTiff")
    dst_ds = driver.Create(outstr_MINval, ref_cln, ref_lne, 1, gdal.GDT_Int16) #1
    dst_ds.SetGeoTransform(ref_geo)
    dst_ds.SetProjection(ref_prj)
    dst_ds.GetRasterBand(1).WriteArray(map_MINval) #1
    dst_ds.FlushCache() 
    dst_ds = map_MINval = None #1
    
    # MAP-MINdoy :
    outstr_MINdoy = out_root + str(yyyy) + '_MAP-MINdoy.tif'
    driver = gdal.GetDriverByName("GTiff")
    dst_ds = driver.Create(outstr_MINdoy, ref_cln, ref_lne, 1, gdal.GDT_Int16)
    dst_ds.SetGeoTransform(ref_geo)
    dst_ds.SetProjection(ref_prj)
    dst_ds.GetRasterBand(1).WriteArray(map_MINdoy)
    dst_ds.FlushCache() 
    dst_ds = map_MINdoy = None

    # MAP-SOSval :
    outstr_SOSval = out_root + str(yyyy) + '_MAP-SOSval.tif'
    driver = gdal.GetDriverByName("GTiff")
    dst_ds = driver.Create(outstr_SOSval, ref_cln, ref_lne, 1, gdal.GDT_Int16)
    dst_ds.SetGeoTransform(ref_geo)
    dst_ds.SetProjection(ref_prj)
    dst_ds.GetRasterBand(1).WriteArray(map_SOSval)
    dst_ds.FlushCache() 
    dst_ds = map_SOSval = None
    
    # MAP-SOSdoy :
    outstr_SOSdoy = out_root + str(yyyy) + '_MAP-SOSdoy.tif'
    driver = gdal.GetDriverByName("GTiff")
    dst_ds = driver.Create(outstr_SOSdoy, ref_cln, ref_lne, 1, gdal.GDT_Int16)
    dst_ds.SetGeoTransform(ref_geo)
    dst_ds.SetProjection(ref_prj)
    dst_ds.GetRasterBand(1).WriteArray(map_SOSdoy)
    dst_ds.FlushCache() 
    dst_ds = map_SOSdoy = None
       
    # MAP-MAXval :
    outstr_MAXval = out_root + str(yyyy) + '_MAP-MAXval.tif'
    driver = gdal.GetDriverByName("GTiff")
    dst_ds = driver.Create(outstr_MAXval, ref_cln, ref_lne, 1, gdal.GDT_Int16)
    dst_ds.SetGeoTransform(ref_geo)
    dst_ds.SetProjection(ref_prj)
    dst_ds.GetRasterBand(1).WriteArray(map_MAXval)
    dst_ds.FlushCache() 
    dst_ds = map_MAXval = None      
           
    # MAP-MAXdoy :
    outstr_MAXdoy = out_root + str(yyyy) + '_MAP-MAXdoy.tif'
    driver = gdal.GetDriverByName("GTiff")
    dst_ds = driver.Create(outstr_MAXdoy, ref_cln, ref_lne, 1, gdal.GDT_Int16)
    dst_ds.SetGeoTransform(ref_geo)
    dst_ds.SetProjection(ref_prj)
    dst_ds.GetRasterBand(1).WriteArray(map_MAXdoy)
    dst_ds.FlushCache() 
    dst_ds = map_MAXdoy = None       
        
    # MAP-EOSval :
    outstr_EOSval = out_root + str(yyyy) + '_MAP-EOSval.tif'
    driver = gdal.GetDriverByName("GTiff")
    dst_ds = driver.Create(outstr_EOSval, ref_cln, ref_lne, 1, gdal.GDT_Int16)
    dst_ds.SetGeoTransform(ref_geo)
    dst_ds.SetProjection(ref_prj)
    dst_ds.GetRasterBand(1).WriteArray(map_EOSval)
    dst_ds.FlushCache() 
    dst_ds = map_EOSval = None      
        
    # MAP-EOSdoy :
    outstr_EOSdoy = out_root + str(yyyy) + '_MAP-EOSdoy.tif'
    driver = gdal.GetDriverByName("GTiff")
    dst_ds = driver.Create(outstr_EOSdoy, ref_cln, ref_lne, 1, gdal.GDT_Int16)
    dst_ds.SetGeoTransform(ref_geo)
    dst_ds.SetProjection(ref_prj)
    dst_ds.GetRasterBand(1).WriteArray(map_EOSdoy)
    dst_ds.FlushCache() 
    dst_ds = map_EOSdoy = None     
         
    
    print
    
print 'Doneeeeeeeeee!!!'