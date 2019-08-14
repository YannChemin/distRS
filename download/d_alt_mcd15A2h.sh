APP_KEY="AD2B5536-97D7-11E9-8A83-DB883D88392C"

export http_proxy="http://10.168.209.72:8012/"
export https_proxy="http://10.168.209.72:8012/"

PROD="MCD15A2H"
#PROD="MOD09GQ"
for YEAR in 2003 2004 2005 2006 2007 2008 2009 2010 2011 2012 2013 2014 2015 2016 2017 2018 2019 2002
do
	for DOY in 001 009 017 025 033 041 049 057 065 073 081 089 097 105 113 121 129 137 145 153 161 169 177 185 193 201 209 217 225 233 241 249 257 265 273 281 289 297 305 313 321 329 337 345 353 361
	do
		for TILE in h19v04
		do
			echo "wget -e robots=off -t 1 -m -np -R .html,.tmp -nH --cut-dirs=3 -A \"*$TILE*.hdf\" "https://ladsweb.modaps.eosdis.nasa.gov/archive/allData/6/$PROD/$YEAR/$DOY/" --header \"Authorization: Bearer $APP_KEY\" -P /data/mcd15a2h/"
			wget -e robots=off -t 1 -m -np -nH --cut-dirs=6 -R .html,.tmp -A "*$TILE*.hdf" "https://ladsweb.modaps.eosdis.nasa.gov/archive/allData/6/$PROD/$YEAR/$DOY/" --header "Authorization: Bearer $APP_KEY" -P /data/mcd15a2h/ 
		done
	done
done

