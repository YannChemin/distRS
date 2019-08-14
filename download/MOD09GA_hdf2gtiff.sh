#!/bin/bash
#cutline is here 
cutlineV='/data/GR/Greece_Rice_5_SimUnits.shp'
NODATA=-28672
NODATAQC=787410671
Root=/data/mod09ga/original
xmin=5374759
ymin=2016255
xmax=5450835
ymax=2117293
outRoot=/data/GR/mod09ga
mkdir -p $outRoot
for file in $outRoot/HDF4*.tif
do
	rm -f $file&
done
LAEA='EPSG:3035'
WGS84='EPSG:4326'

#Define number of (virtual) cores
ncores=`grep -c 'processor' /proc/cpuinfo`
echo "ncores=" $ncores
#Dive into the directory of the date of overpass
for year in 2001 2002 2003 2004 2005 2006 2007 2008 2009 2010 2011 2012 2013 2014 #2015 2016 2017 2018 2019
do
	for doy in 001 002 003 004 005 006 007 008 009 010 011 012 013 014 015 016 017 018 019 020 021 022 023 024 025 026 027 028 029 030 031 032 033 034 035 036 037 038 039 040 041 042 043 044 045 046 047 048 049 050 051 052 053 054 055 056 057 058 059 060 061 062 063 064 065 066 067 068 069 070 071 072 073 074 075 076 077 078 079 080 081 082 083 084 085 086 087 088 089 090 091 092 093 094 095 096 097 098 099 100 101 102 103 104 105 106 107 108 109 110 111 112 113 114 115 116 117 118 119 120 121 122 123 124 125 126 127 128 129 130 131 132 133 134 135 136 137 138 139 140 141 142 143 144 145 146 147 148 149 150 151 152 153 154 155 156 157 158 159 160 161 162 163 164 165 166 167 168 169 170 171 172 173 174 175 176 177 178 179 180 181 182 183 184 185 186 187 188 189 190 191 192 193 194 195 196 197 198 199 200 201 202 203 204 205 206 207 208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 223 224 225 226 227 228 229 230 231 232 233 234 235 236 237 238 239 240 241 242 243 244 245 246 247 248 249 250 251 252 253 254 255 256 257 258 259 260 261 262 263 264 265 266 267 268 269 270 271 272 273 274 275 276 277 278 279 280 281 282 283 284 285 286 287 288 289 290 291 292 293 294 295 296 297 298 299 300 301 302 303 304 305 306 307 308 309 310 311 312 313 314 315 316 317 318 319 320 321 322 323 324 325 326 327 328 329 330 331 332 333 334 335 336 337 338 339 340 341 342 343 344 345 346 347 348 349 350 351 352 353 354 355 356 357 358 359 360 361 362 363 364 365 366 
	do
		echo $year$doy
		file=$(ls MOD09GA.A$year$doy.h19v04*.hdf)
		SDS14=$(gdalinfo $file | grep "SUBDATASET_14_NAME")
		out=$(echo ${SDS14#*=} | sed 's/[\/\"\:\.\ ]/\_/g' | sed 's/\(.*\)_h..v.._\(.*\)/\1\_\2/g')
		echo "OUT=" $out
		rm -f $outRoot/$out.tif
		gdalwarp -of GTiff -q -cutline $cutlineV -crop_to_cutline -dstnodata $NODATA -s_srs '+proj=sinu +R=6371007.181 +nadgrids=@null +wktext' -t_srs $LAEA -te $xmin $ymin $xmax $ymax "${SDS14#*=}" $outRoot/$out.tif &
		file=$(ls MOD09GA.A$year$doy.h19v04*.hdf)
		SDS19=$(gdalinfo $file | grep "SUBDATASET_19_NAME")
		out=$(echo ${SDS19#*=} | sed 's/[\/\"\:\.\ ]/\_/g' | sed 's/\(.*\)_h..v.._\(.*\)/\1\_\2/g')
		echo "OUT=" $out
		#Define number of gdalwarp running
		npid=$(echo "$(ps aux | grep gdalwarp | wc -l) - 1" | bc)
		while [ $npid -ge $ncores ]
		do
			sleep 1
			#Update number of gdalwarp running
			npid=$(echo "$(ps aux | grep gdalwarp | wc -l) - 1" | bc)
			#Update number of (virtual) cores
			ncores=`grep -c 'processor' /proc/cpuinfo`
		done
		#Process gdalwarp with two SDS tiles
		rm -f $outRoot/$out.tif
		gdalwarp -of GTiff -q -cutline $cutlineV -crop_to_cutline -dstnodata $NODATAQC -s_srs '+proj=sinu +R=6371007.181 +nadgrids=@null +wktext' -t_srs $LAEA -te $xmin $ymin $xmax $ymax "${SDS19#*=}" $outRoot/$out.tif 
	done
done
