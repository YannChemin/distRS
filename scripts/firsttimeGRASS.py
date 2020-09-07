
def firsttimeGRASS(infiles,adminfile,maskfile):
    """
    Run a maxlikelihood unsupervised classification on the data
    nclasses: number of expected classes
    infiles: list of raster files to import and process
    firstime: if firsttime, it will import all files in GRASS
    """
    from grass_session import Session
    from grass.script import core as gcore
    from grass.pygrass.modules.shortcuts import raster as r
    from grass.pygrass.modules.shortcuts import vector as v
    from grass.pygrass.modules.shortcuts import general as g
    from grass.pygrass.modules.shortcuts import imagery as i
    # create a new location from EPSG code (can also be a GeoTIFF or SHP or ... file)
    with Session(gisdb="/tmp", location="loc", create_opts="EPSG:4326"):
        # First run, needs to import the files and create a mask
        # Import admin boundary
        #v.import_(input=adminfile,output="admin",quiet=True,superquiet=True)  
        gcore.parse_command("v.import",input=adminfile,output="admin",quiet=True)  
        # Set computational region to admin boundary
        g.region(flags="s",vector="admin",quiet=True)
        # Keep only file name for output
        outmf=maskfile.split("/")[-1]
        # Import Mask file
        r.in_gdal(input=maskfile,output=outmf,quiet=True)
        # Apply Mask
        r.mask(raster=outmf,maskcats="0",quiet=True)
        # Set computational resolution to mask pixel size
        g.region(flags="s",raster=outmf,quiet=True)
        # Import files
        for f in infiles:
            # Keep only file name for output
            outf=f.split("/")[-1]
            # Landsat files not in Geo lat long needs reproj on import
            #r.import_(input=f,output=outf,quiet=True)
            gcore.parse_command("r.import",input=f,output=outf,quiet=True)
            # Create group
            i.group(group="l8",subgroup="l8",input=outf,quiet=True)

