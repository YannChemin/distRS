#include<stdio.h>
//----------------------------------------------------------
//CSU Crop functions for Coleambally Irrigation Area
//----------------------------------------------------------

int lulc_code2libcrop_code(double lulc_code){
	if(lulc_code==0){
		printf("Undefined Crop\n");
		return(0);
	}
	if(lulc_code==1){
		printf("CIA Corn 2008-2010 (unpublished)\n");
		return(1);
	}
	if(lulc_code==2){
		printf("CIA Rice 2008-2010 (unpublished)\n");
		return(2);
	}
	if(lulc_code==3){
		printf("CIA Canola 2008-2010 (unpublished)\n");
		return(3);
	}
	if(lulc_code==4){
		printf("CIA Wheat 2008-2010 (unpublished)\n");
		return(4);
	}
}
// Tsoil(C) Temperature for Soil from MODIS 1Km (R2=0.6689)
double Tsoil(double lst_c,int libcropcode){
	if(libcropcode==0){
		printf("unknown crop\n");
		return(lst_c);
	}
	if(libcropcode==1){
		printf("CIA Corn 2008-2010 (unpublished)\n");
		return(0.9456*lst_c-1.77);
	}
}
// Tair(C) Temperature for air from MODIS 1Km (R2=0.6546)
double Tair(double lst_c, int libcropcode){
	if(libcropcode==0){
		printf("unknown crop\n");
		return(-28678);
	}
	if(libcropcode==1){
		printf("CIA Corn 2008-2010 (unpublished)\n");
		return(0.6832*lst_c+2.43);
	}
}

// Roughness length for heat momentum
double z0m(double crop_age, int libcropcode){
	if(libcropcode==0){
		printf("unknown crop\n");
		return(-28768);
	}
	if(libcropcode==1){
		printf("CIA Corn 2008-2010 (unpublished)\n");
		return(log(crop_age-24)/10 - 0.1*pow(crop_age,2)/pow(181,2));
	}
}
// LAI function using MODIS 1Km input
double lai(double ndvi, int libcropcode){
	if(libcropcode==0){
		printf("unknown crop\n");
		return(-28768);
	}
	if(libcropcode==1){
		printf("CIA Corn 2008-2010 (unpublished)\n");
		return(3.2658*log(ndvi)+6.9212);
	}
}
//Rescale g0 from MODIS 1Km to field (R2=0.3195)
double g_modis2field(double g_modis,double rn_s,double ndvi,double ndvimax,double ndvimin, int libcropcode){
	if(libcropcode==0){
		printf("unknown crop\n");
		return(-28768);
	}
	if(libcropcode==1){
		printf("CIA Corn 2008-2010 (unpublished)\n");
		ndviscaled=(ndvi-ndvimin)/(ndvimax-ndvimin);
		g_s=g_modis*ndviscaled;
		g_s=0.3006*g_s+8.6387;
		if( g_s > rn_s ){
			g_s = rn_s;
		}
		return(g_s);
	}
}
// Leaf Area by cropcode
double leafarea(int libcropcode){
	if(libcropcode==0){
		printf("unknown crop\n");
		return(-28768);
	}
	if(libcropcode==1){
		printf("CIA Corn 2008-2010 (unpublished)\n");
		return(560);
	}
}
// Leaf Area by cropcode
double leafperimeter(int libcropcode){
	if(libcropcode==0){
		printf("unknown crop\n");
		return(-28768);
	}
	if(libcropcode==1){
		// The ratio leaf_area/leaf_perimeter was taken as 3.2184 and was created from knowledge scattered in
		// Maddonni et al. (2001), Jordan-Meille et al. (2004) and Garcia (2010).
		printf("CIA Corn 2008-2010 (unpublished)\n");
		return(174);
	}
}
