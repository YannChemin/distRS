# -*- coding: utf-8 -*-
"""
Created on Fri Oct 19 11:46:15 2018

@author: manfrgi
"""

# ---------------------------------------------------------- Import libraries :
import os
import fnmatch
import ogr
import datetime
import dateutil.rrule
import numpy as np
import pandas as pd
import requests
import time


# --------------------------------------------------- Set download parameters :
hh        = '18'
vv        = '04'
coll      = 6
prod      = 'MYD09GA'
yyyy      = 2003
DOY_start = 90
DOY_end   = 150

# ----------------------------------------------------------------- Set roots :
out_root  = 'I:/ix_ToGreg_MODIS.rice.flood.mng.map/MODIS_imgs/' + str(prod) + '_' + str(yyyy) + '/'

# ---------------------------------------------------------------- Processing :
print 'Start processing : ', time.ctime()
    
for time_stamp in range(DOY_start, DOY_end + 1):
    
    if len(str(time_stamp)) == 3:
        time_stamp = str(time_stamp)
    elif len(str(time_stamp)) == 2:
        time_stamp = '0' + str(time_stamp)
    else:
        time_stamp = '00' + str(time_stamp)

    url = 'https://ladsweb.modaps.eosdis.nasa.gov/archive/allData/' + str(coll) + '/' + prod + '/' + str(yyyy) + '/' + time_stamp    
    pag = (requests.get(url).content)
        
    fil_src = prod + '.A' + str(yyyy) + time_stamp + '.' + 'h' + str(hh) + 'v' + str(vv) + '.'    
    pag_src = pag.find(fil_src)
    
    if pag_src != -1:
        fil_dow = pag[pag_src:pag_src + 45]
        url_dow = url + "/" + fil_dow    
        str_out = out_root + fil_dow
        rsponse = requests.get(url_dow, stream=True)
        handler = open(str_out, "wb")
        for chunk in rsponse.iter_content(chunk_size=512): 
            if chunk:
                handler.write(chunk)
        handler.close()
        print "   Downloaded : " + fil_dow + '  -  ' + time.ctime()[11:19]
    else:
        print "   --> DOWNLOAD ERROR: " + fil_src


print 'End processing : ', time.ctime()