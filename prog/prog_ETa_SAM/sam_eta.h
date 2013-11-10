#include<stdio.h>

double tc(lst,fc,tsoil,e0);
double ts(lst,fc,tcanopy,e0);
double rns(rn,c,lai,sunza);
double rnc(rn,rns);
double g_0(kg,rns,sunza);
double fc(k,ndvi,ndvimin,ndvimax);
double rohair(dem, lst, delta_T);
double rs(wind_speed,wind_height,tsoil,z0_m,disp,canopy_height,leaf_area,leaf_perimeter,lai,vonK,g,rohair,hc);
double rah(wind_speed, wind_height, tair, temperature_height, canopy_height, crop_age, vonK, g, kb1, rohair, hc);
double ustar(wind_speed,vonK,wind_height,z0_m,blending_height,canopy_height,crop_age,psim);
double rah1(wind_height,z0_m,kb1,psih,u_star,vonK);
double kb1(wind_speed,wind_height,canopy_height,lai,fc,tair,atmopressure,z0);
double deltaT_SEBS(blending_height,wind_height,canopy_height,z0_m,disp,kb1,fc,lst_c,dem);
double hc(tair, tcanopy, rohair, cp, rah,rn_c);
double hs(rohair, cp, tsoil, tair, rah, rs, rn_s, g_s);
double psim(h,rohair,wind_speed,wind_height,temperature,vonK,g);
double psih(h,rohair,wind_speed,wind_height,temperature,vonK,g);
double evap_fr(rn_c, rn_s, g_s, h);
double tcanop(h_c,ra_h,roh_air,cp,tair);
double hsoil(roh_air,cp,tsoil,tair,ra_h,r_s);
double tsoi(ra_h,r_s,roh_air,cp,h_s,tair);
double g0_sam(rn,c,la_i,sunza,kg,ndvi,ndvimin,ndvimax);
double sens_h(rn,c,la_i,sunza,k,ndvi,ndvimin,ndvimax,lst_c,e0,wind_speed,wind_height,canopy_height,atmo_pressure,z0_m,disp,tair,temperature_height,leaf_area,leaf_perimeter,tsoil);
