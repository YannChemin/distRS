#!/usr/bin/env python3

###########################################################
# Use of this file is strictly forbidden !!!!!!!!!!!!!!!!!#
# Copyright Yann Chemin, Freelancer, France, 2016         #
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!#

import xml.etree.ElementTree as ET
from osgeo import gdal, gdal_array, osr
import os, sys, glob
import numpy as np
from scipy.interpolate import interp1d

# For debug only
safefile="/Users/dnd/snap/Cozmin/S1A_IW_SLC__1SDV_20200509T055041_20200509T055108_032484_03C30B_EF9F.SAFE"
out="/Users/dnd/snap/noise_"

# Parse arguments
import argparse
parser = argparse.ArgumentParser()
parser.add_argument(dest = "safedir", type = argparse.FileType('r'), help = safefile)
parser.add_argument(dest = "outbase", type = str, help = out)
args = parser.parse_args()
# import arguments into processing
safefile = args.safedir
out = args.outbase 

# For GDAL to raise Exceptions
gdal.UseExceptions()


################################################                     
# Go into measurement subdir and open each file 
################################################
srcs = glob.glob(safefile+"/measurement/*.tiff")
print(safefile+"/measurement/*.tiff")
for srcfile in srcs:
    print(srcfile)
    ds = gdal.Open(srcfile)
    dr = ds.GetDriver()
    outf = out+os.path.basename(srcfile)
    ds_new = dr.Create(outf,ds.RasterXSize,ds.RasterYSize,ds.RasterCount,gdal.GDT_Float32)
    ds_new.SetGeoTransform(ds.GetGeoTransform())
    srs = osr.SpatialReference()
    srs.ImportFromEPSG(4326)
    ds_new.SetProjection(srs.ExportToWkt())
    #ds_new.SetProjection(ds.GetProjection())
    ds_new.SetMetadata(ds.GetMetadata())
    gcp_count = ds.GetGCPCount()
    if gcp_count != 0:
        try:
            ds_new.SetGCPs(ds.GetGCPs(), ds.GetGCPProjection())
        except:
            ds_new.SetGCPs(ds.GetGCPs(), srs.ExportToWkt())
    ds = None

    npimg = np.array(ds_new.GetRasterBand(1).ReadAsArray())
    # Clean up pixels to 0
    npimg [ npimg != 0 ] = 0
    # Clean all pixels to np.nan
    # npimg.fill(np.nan)
    # Shape of the numpy array
    print(npimg.shape)
    #create xmlfile name
    xmlfile = safefile+"/annotation/calibration/noise-"+os.path.basename(srcfile)[:-4]+"xml"
    print(xmlfile)
    tree = ET.parse(xmlfile)
    root = tree.getroot()
    #print(root.tag)
    #print(root.attrib)
    # Load line numbers
    l = []
    for dss in root.findall('noiseRangeVectorList'):
        #print(dss.tag, dss.attrib)
        for sub in dss:
            #print(sub.tag, sub.attrib)
            for val in sub.findall('line'):
                l.append(int(val.text))

    rows=l[-1]
    print("rows =",rows)
    # Load pixels
    p = []
    # Load noise
    n = []
    for dss in root.findall('noiseRangeVectorList'):
        #print(dss.tag, dss.attrib)
        for sub in dss:
            #print(sub.tag, sub.attrib)
            for val in sub.findall('pixel'):
                p.append([ int(x) for x in (val.text).split(' ') ])
                #print(p[-1])
            for val in sub.findall('noiseRangeLut'):
                n.append([ float(x) for x in (val.text).split(' ') ])
            #################################
            # Interpolate 1D Noise RangeLUT
            #################################
            f = interp1d(p[-1], n[-1], fill_value="extrapolate")
            xnew = np.linspace(0, npimg.shape[1], npimg.shape[1], endpoint=True)
            noise1d = f(xnew)
            #print(noise1d[0],noise1d.shape, npimg.shape)
            for j in range(npimg.shape[1]):
                #print("col[%d] = %d" % (j,p[-1][j]))
                for i in range(len(l)):
                    #print("row[%d] = %d" % (i, l[i]))
                    #print("npimg[%d][%d] = %f" % (l[i],p[-1][j],n[-1][i]))
                    #Write directly as it comes
                    npimg[l[i]][j] = noise1d[j]
    
    ##########################################
    # Interpolate 1D for each npimg.shape[0]
    ##########################################
    for j in range(npimg.shape[1]):
        #x = np.linspace(0, npimg.shape[1], len(l), endpoint=True)
        #print(npimg[:][j])
        #print(len(npimg[:][j]))
        #print(npimg.shape)
        # gather only values from noiseRangeLUT to create model of interp1d
        temparray = np.copy(npimg[:,j])
        tarray = temparray [ temparray != 0 ]
        #print(tarray)
        #print(tarray.shape)
        x = np.linspace(0, npimg.shape[0], tarray.shape[0], endpoint=True)
        f = interp1d(x, tarray, fill_value="extrapolate")
        xnew = np.linspace(0, npimg.shape[0], npimg.shape[0], endpoint=True)
        noise1d = f(xnew)
        #print(noise1d)
        #print(noise1d.shape)
        for i in range(npimg.shape[0]):
            npimg[i][j] = noise1d[i]

    # write the data
    ds_new.GetRasterBand(1).WriteArray(npimg, 0, 0)

    # flush data to disk, set the NoData value and calculate stats
    ds_new.GetRasterBand(1).FlushCache()
    ds_new.GetRasterBand(1).SetNoDataValue(-99)

    # Write to disk
    dr_new = None
    #cols=p[-1][-1]
    #print("cols =",cols)
    #print(p[0])
    #print(n[0])


