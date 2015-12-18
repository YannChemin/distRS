cd ~/DATA
for file in lst*.tif
do
 r.in.gdal input=$file output=$(echo $file | sed 's/.tif//') --o --q
 r.null map=$(echo $file | sed 's/.tif//') setnull=-28768
done
#for file in eta_sebal*.tif
#do
# r.in.gdal input=$file output=$(echo $file | sed 's/.tif//') --o --q
# r.null map=$(echo $file | sed 's/.tif//') setnull=-28768
#done

