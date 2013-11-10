#include<stdio.h>
int lulc_code2libcrop_code(double lulc_code);
double Tsoil(double lst_c,int libcropcode);
double Tair(double lst_c, int libcropcode);
double z0m(double crop_age, int libcropcode);
double lai(double ndvi, int libcropcode);
double g_modis2field(double g_modis,double rn_s,double ndvi,double ndvimax,double ndvimin, int libcropcode);
double leafarea(int libcropcode);
double leafperimeter(int libcropcode);