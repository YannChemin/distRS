for dir in $(ls -d */)
do
	cd $dir
	make clean
	cd ..
done
