#include<stdio.h>
#include<math.h>
#include "sebs_eta.h"
#define PI 3.1415927

double kb_1(double u_zref,double zref,double h,double lai,double fc,double tempka,double pa)
{
  //After Brutsaert, 1982
  //u_zref = wind speed at reference height
  //zref = reference height, 2h<= zref <=hst=(0.1~0.15)hi, (=za)
  //h = canopy height
  //lai = canopy total leaf area index
  //fc = Fractional canopy cover (-)
  //tempka = ambient air temperature (oC)
  //pa = ambient air pressure (Pa)
  //nec = within-canopy wind speed profile extinction coefficient (-)
  
  double c_d = 0.2; //foliage drag coefficient
  double c_t = 0.05; //heat transfer coefficient
  double k = 0.4; //von Karman's constant
  double Pr = 0.7; //Prandtl's number
  double hs = 0.009; //0.009~0.024 Height of soil roughness obstacles
  
  double fs = 1.0-fc; // soil fraction
  if (fc == 0.0) h = hs; // use smooth bare soil roughness
  double z0 = 0.136*h; // z02h(h)*h
  double u_h0 = u_zref*log(2.446)/log((zref-0.667*h)/z0); //wind speed at canopy height
  double ust2u_h = 0.32-0.264/exp(15.1*c_d*lai); // Ustar2u_h(h)
  double ustarh = ust2u_h*u_h0;
  double nu0 = 1.327 *pow(10,-5)*(101325.0/pa)*pow((tempka/273.15),1.81);//kinematic viscosity of the air (m2/s) Massman, 1999b)
  double nec = c_d*lai/(2.0*pow(ust2u_h,2));
  double re_star=hs*ustarh/nu0;//roughness Reynolds Number
  //first term
  double kb1;
  if (nec != 0.0) kb1=(fc*fc)*k*c_d/(4.0*c_t*ust2u_h*(1.0-exp(-nec/2.0)));
  else kb1 = 0.0;
  //second term
  double c_t_star=pow(Pr,-2/3)*pow(re_star,-1/2);
  kb1=kb1+2*fc*fs*(k*ust2u_h*(z0/h))/c_t_star;
  //third term
  re_star=hs*(u_zref*k/log(zref/hs))/nu0;//soil roughness Reynolds Number star
  double kb1soil=pow(2.46*(re_star),0.25);//soil heat transfer number kB-1 (Brutsaert, 1982)
  kb1=kb1+(kb1soil-log(7.4))*fs*fs;
  return(kb1);
}

double psim(double y)
{
 //integrated stability function
 //Stability correction function for momentum, eq.(16)
 //y=-z/L, z is the height, L is the Monin-Obukov Length
 y=fabs(y);
 double a = 0.33;
 double b = 0.41;
//  double m = 1.0; // constants p 443
 double x = pow((y/a),1/3);
 double psi0 = -log(a)+pow(3.0,1/2)*b*pow(a,1/3)*PI/6; //maybe missing something (!PI/6.0)
 double b3 = 1.0/(b*b*b); // b^-3
 if (y <= b3){
  return (log(a+y)-3*b*pow(y,1/3)+b*pow(a,1/3)/2*log((1+x)*(1+x)/(1-x+x*x)) + pow(3,1/2)*b*pow(a,1/3)*atan((2*x-1)/pow(3,1/2)) + psi0);
 }else{
  //if y>b^-3 then y=b^-3
  x = pow(b3/a,1/3);
  return (log(a+b3)-3*b*pow(b3,1/3)+b*pow(a,1/3)/2*log((1+x)*(1+x)/(1-x+x*x)) + pow(3,1/2)*b*pow(a,1/3)*atan((2*x-1)/pow(3,1/2)) + psi0); 
 }
}


double psih(double y)
{
 //integrated stability function 
 //Stability correction function for heat, eq.(17)
 //y=-z/L, z is the height, L is the Monin-Obukov Length
 y=fabs(y);
 double c = 0.33;
 double d = 0.057;
 double n = 0.78; //constants p443
 return (((1-d)/n)*log((c+pow(y,n))/c));
}
//Bulk atmospheric Boundary Layer Similarity (Brutsaert, 1999)
//Used in unstable conditions
//Bulk similarity function
double bw(double hi, double molength, double z0)
{
 //hi = height of the Atmospheric Boundary Layer
 double bw;
 double alpha=0.12;
 double beta=125; //These are mid-values as given by Brutsaert (1999)
 double z0_test = (alpha/beta)*hi;//test boundary value for moderate to very rough terrain classification 
 double b1 = -z0/molength;
 if(z0<z0_test) bw=-log(alpha)+psim(-alpha*hi/molength)-psim(-z0/molength);
 else bw=log(hi/(beta*z0))+psim(beta*b1)+psim(-z0/molength);
 if(bw<0.0) bw=0;
 return (bw);
}
//Bulk atmospheric Boundary Layer Similarity (Brutsaert, 1999)
//Used in unstable conditions 
double cw(double hi, double molength, double z0, double z0h)
{
 //hi = height of the Atmospheric Boundary Layer
 double cw;
 double alpha=0.12;
 double beta=125; //These are mid-values as given by Brutsaert (1999)
 double z0_test = (alpha/beta)*hi;//test boundary value for moderate to very rough terrain classification 
 double c1 = -z0/molength;//Stability/unstability value
 if(z0<z0_test) cw=-log(alpha)+psih(-alpha*hi/molength)-psih(-z0/molength);
 else cw=log(hi/(beta*z0))+psih(beta*c1)+psih(-z0/molength);
 if(cw<0.0) cw=0;
 return (cw);
}

double e_sat(double lst, double dem)
{
 //Saturated vapour pressure (Pa)
 double t0 = lst / pow((1.0-dem/44331.0),1.5029);
 if(t0>100) t0-=273.15; //Mean PBL temperature converted to oC
 //Tetens Formula (Buck, 1981)
 double esat = 611.0*exp(17.502*t0/(t0+240.97)); //Pa, (3.8), p.41
//  double hr_pbl = hr_s;
//  double eact = hr_pbl*MEAN(esat); // Actual Vapour Pressure (THIS IS A RASTER FUNCTION!)
 return(esat);
}

double eact(double esat_avg, double hr_s)
{
 //hr_pbl = PBL relative humidity (-)
 double hr_pbl = hr_s; // hr_s is considered hr_pbl in SEBS
 return(hr_pbl * esat_avg); // Actual Vapour Pressure ESAT_AVG from raster average
}

double z0_m(double ndvi, double ndvi_max)
{
  return(0.005+0.5*pow((ndvi/ndvi_max),2.5));
}

double leafai(double ndvi)
{
  //Here we use a simple formula for LAI=f(NDVI), cf Su (1996)
  double lai=pow((ndvi*(1+ndvi)/(1-ndvi+pow(10,-6))),0.5);
  if(lai>6.0) lai=6.0;
  return(lai);
}

double f_c(double ndvi, double ndvi_min, double ndvi_max)
{
  if((ndvi_max-ndvi_min)==0.0) return(pow(ndvi-ndvi_min,2));
  else return(pow(((ndvi-ndvi_min)/(ndvi_max-ndvi_min)),2));
}

double k_exo(double doy)
{
	double kexo_mean,r_prime_es,phi,ds;
	// #mean Earth-Sun distance in (m)
	r_prime_es=1.495978706916*pow(10,11);
	// #Total flux from the solar surface (W)
	phi=3.9*pow(10,26);
	// #mean Earth-Sun distance Kexo
	kexo_mean=phi/(4*PI*pow(r_prime_es,2));
	// #inverse relative distance Earth-Sun (A.U.)
	ds = 1.0 + 0.01672 * sin(2*PI*(doy-93.5)/365.0);
	// #kexo on doy date
	return(kexo_mean*(pow(r_prime_es,2))/(pow(r_prime_es/ds,2)));
}

double k_in( double dem, double doy, double sunza)
{
	/* tsw =  atmospheric transmissivity single-way (~0.7 -) */
	/* doy = Day of Year */
	double ds; /*relative sun-earth distance in astronomical units*/
	double tsw;/* tsw =  atmospheric transmissivity single-way (~0.7 -) */
	double kexo;/* Kexo = exoatmospheric solar irradiance*/
	ds = 1-0.01672*cos(2*PI*(doy-4)/365.0);
	tsw = 0.75 + 2*pow(10,-5)*dem;
	kexo = k_exo(doy);
	return(tsw*kexo*cos(sunza*PI/180.0)/(ds*ds));
}

// Average Solar Diurnal Radiation after Bastiaanssen (1995)
double k_in24(double latitude, double doy, double dem )
{
	double tsw, ws, cosun, latrad, deltarad, ds;
	double	Kexo;
	/* tsw =  atmospheric transmissivity single-way (~0.7 -) */
	tsw = 0.75 + 2*pow(10,-5)*dem;
// 	printf("tsw=%f\n", tsw);
	/* Kexo = exoatmospheric solar irradiance*/
	Kexo = k_exo(doy);
// 	printf("Kexo=%f\n", Kexo);
	//Sun-Earth Distance (ds; A.U.)
	ds = 1.0 + 0.01672 * sin(2*PI*(doy-93.5)/365.0);
// 	printf("ds=%f\n", ds);
	//Convert latitude in radians
// 	printf("lat=%f\n", lat);
	latrad =  latitude * PI / 180.0;
// 	printf("latrad=%f\n", latrad);
	//Solar declination (delta; radians)
// 	deltarad = (0.4093*sin((2*PI*doy/365)-1.39))*(lat/fabs(lat));
	deltarad = (0.4093*sin((2*PI*doy/365)-1.39));
// 	printf("deltarad=%f\t", deltarad*180.0/PI);
	//Solar angle hour
// 	ws = (acos(-tan(latrad)*tan(deltarad)))+PI/2;
	ws = (acos(-tan(latrad)*tan(deltarad)));
// 	printf("ws=%f\t", ws);
	cosun = sin(deltarad)*sin(latrad)+cos(deltarad)*cos(latrad)*sin(ws);
// 	printf("cosun=%f\n", cosun);
// 	printf("solard=%f\n", ( cosun * Kexo * tsw ) / ( PI * ds * ds ));
// 	return ( ws*fabs(cosun) * Kexo * tsw ) / ( PI * ds * ds );
	return ( ws * cosun * Kexo * tsw ) / ( PI * ds * ds );
}

double tsw(double visi, double dem)
{
 //Atmospheric components
 double br0 = 0.0116;
 double ba0 = ( log ( 50.0 ) / visi - br0 );
 double ba55 = 0.0030765;
 double h1 = 5.5 / log( ba0 / ba55 );
 double hba0 = h1 * ba0;
 double hba55 = 0.05 - h1 * ba55;
  //Aerosol Optical Depth
 double tau=0.42*(hba0/exp(dem/(1000.0*h1))+hba55);//dem should be in m
 double tau_t=exp(-tau*2.0); //Supposing sun zenith is 60 as an average value
 return(tau_t);
}

double sebs_evapfr(double z_pbl, double t_s, double p_s, double u_s, double hr_s, double alt_ms, double eact, double kin, double albedo, double ndvi, double ndvi_min, double ndvi_max, double ndvi_median, double e0, double lst, double dem)
{
 //z_pbl = PBL depth (m)
 //p_pbl = PBL pressure (Pa)
 //t_pbl = PBL potential temperature (K)
 //q_pbl = PBL specific humidity (kg/kg)
 //u_pbl = PBL wind speed (m/s)
 //t_s = Surface Temperature (oC) --- Met Station?
 //p_s = Surface pressure (Pa) --- Met Station?
 //kin = Surface Short Wave clear sky radiation (W/m2)
 //		Also called Global Radiation from image
 //rnetd = Diurnal surface Net Radiation from image (W/m2)
 //albedo = Surface Albedo from image (-)
 //ndvi = NDVI from image (-)
 //e0 = Emissivity from image (-)
 //lst = Land Surface Temperature from image (K)

 double gamma = 67;//psychrometric constant (Pa K-1), (c_p*P/lambda*Mv/Ma)
 double rv = 461.05;//specific gas constant water vapour (J kg-1 K-1)
 double rd = 287.04;//specific gas constant dry air (J kg-1 K-1)
 double cp = 1005;//specific heat (J kg-1 K-1)
//  double p0 = 100000;//sea-level pressure (Pa)
 double l_e = 2.430*pow(10,6);//latent heat of vaporization (J kg-1 K-1) from Brutsaert, 1982, p41
 double g = 9.81;//acceleration due to gravity (m.s-2)
 double sigma = 5.678*pow(10,-8);//Stefan-Boltzmann constant
 double k = 0.41;//von Karman's constant
//  double pr = 0.7;//Prandtl's number
 
//  double c_d = 0.2; //foliage drag coefficient for vegetation
//  double c_t = 0.01; //leaf heat transfer coefficient for vegetation
 
 //Here we use a simple formula for LAI=f(NDVI), cf Su (1996)
 double z0m = z0_m(ndvi_median, ndvi_max);
 double lai = leafai(ndvi_median);
 double fc = f_c(ndvi_median,ndvi_min,ndvi_max);
 double d = z0m * 4.9; // zero plane displacement height
 double h = z0m / 0.136; // the total height of vegetation (m)
 double z_ms = 10.0; //the height of measuring wind (m)
 double u_c = log((z_pbl-d)/z0m)/log((z_ms-d)/z0m);
 double u_pbl = u_s*u_c;
 
 //Calculate intermediate results , some formulas are from Campbell and Norman (1998)
 //Here we use the average PBL and Surface Temperature
 p_s = p_s * pow(((44331.0 - dem)/(44331.0 - alt_ms)),(1/0.1903)); //Surface Pressure
 double p_pbl = p_s * pow(((44331.0 - (dem + z_pbl))/(44331.0 - alt_ms)),(1/0.1903));
 
 double kb1 = kb_1(u_pbl, z_pbl, h, lai, fc, t_s, p_s);
 double z0h = z0m/exp(kb1);
 double t_c = log((z_pbl-d)/z0h)/log((z_ms-d)/z0h);
 //Auto-check/conversion from Celsius to Kelvin
 if(t_s<100) t_s = t_s + 273.15;
 double t_pbl = (t_s*(1.0-fc)+t_c*fc)/pow((1.0-dem/44331.0),1.5029);
 double t0 = lst / pow((1.0-dem/44331.0),1.5029);
 double t0_pbl = 0.5 * (t0 + t_pbl); //Mean potential temperature
 double tcn;
 if(t0>100) tcn = t0 - 273.15; //Mean PBL temperature converted to oC
 else tcn = t0;
 double esat = 611.0*exp(17.502*tcn/(tcn+240.97)); //Pa, (3.8), p.41
//  double hr_pbl = hr_s;
//  double eact = hr_pbl*esat_avg; // Actual Vapour Pressure ESAT_AVG from raster average 
 double q_pbl = 5.0/8.0*eact/p_pbl;
 
 //***************SEBS START A LOOP ON PIXELS HERE****
 double ta;
 if(t0_pbl>100) ta=t0_pbl-273.15; // Used for the calculation of nu (ta, pa)
 else ta=t0_pbl;
 lai = leafai(ndvi);
 fc = f_c(ndvi,ndvi_min,ndvi_max);
 z0m = z0_m(ndvi,ndvi_max);
 d=z0m*4.9;//zero plane displacement height
 h=z0m/0.136;//the total height of vegetation (m)
 kb1 = kb_1(u_pbl, z_pbl, h, lai, fc, ta, p_pbl);
 z0h=z0m/exp(kb1);

 //Calculate intermediate results 
 //lst = potential surface temperature
 double theta_v=lst*(1+0.61*q_pbl); //surface virtual temperature
 double dtair=t0-t_pbl;//tpbl = pot. air temperature at reference height (K)
 
 eact=p_pbl*q_pbl*(rv/rd); //actual vapour pressure
 double rohair=p_s/(rd*theta_v);//Surface air density (kg/m3)
 double rohairm=(p_s/(rd*lst))*(1-0.378*eact/p_s);//moist air density (kg/m3)
 double alpha=0.12;
 double beta=125.0; //These are the mid-values given by Brutsaert,1999
 double hst;//Height of ASL
 if (alpha*z_pbl>=beta*z0m) hst=alpha*z_pbl;
 else hst=beta*z0m;//Height of stabilization
 double ustar=k*u_pbl/(log((z_pbl-d)/z0m));
 h=(dtair*k*rohair*cp)*ustar/(log((z_pbl-d)/z0h));
 double h0=h;
 double reps=10.0;
 int lsteps=0;
 double molength;
 if(z_pbl>=hst)
 {
  while(reps>0.01&&lsteps<100)
  {
    molength=(-rohairm*cp*theta_v/(k*g))*pow(ustar,3)/h;
    ustar=k*u_pbl/((log((z_pbl-d)/z0m))-bw(z_pbl,molength,z0m));
    h=(dtair*k*rohair*cp)*ustar/((log((z_pbl-d)/z0h))-cw(z_pbl,molength,z0m,z0h));
    reps=fabs(h0-h);
    h0=h;
    lsteps=lsteps+1;
  }
 }else{
  while(reps>0.01&&lsteps<100)
  {
    molength=(-rohairm*cp*theta_v/(k*g))*pow(ustar,3)/h;
    ustar=k*u_pbl/((log((z_pbl-d)/z0m))-psim((z_pbl-d)/molength)+psim(z0m/molength));
    h=(dtair*k*rohair*cp)*ustar/((log((z_pbl-d)/z0h))-psih((z_pbl-d)/molength)+psih(z0h/molength));
    reps=fabs(h0-h);
    h0=h;
    lsteps=lsteps+1;
  }
 }
 double e_atm=9.2*pow((t_pbl/1000.0),2);
 double rn=(1.0-albedo)*kin+sigma*(e_atm*pow(t_pbl,4)-e0*pow(lst,4));
 double g0=rn*(0.05+(1-fc)*(0.315-0.05));
 double h_dry=rn-g0;
 //For complete wet areas, we get
 double l_w=-pow(ustar,3)*rohairm/(0.61*k*g*(rn-g0)/l_e); //Equation 19 in SEBS paper
 double cwet;
 if (z_pbl >= hst) cwet = cw(z_pbl,l_w,z0m,z0h);
 else cwet = psih(-z_pbl/l_w);
 double rew=((log((z_pbl-d)/z0h))-cwet)/(k*ustar);
 if(rew<=0.0) rew=(log((z_pbl-d)/z0h))/(k*ustar); //Equation 18 in SEBS paper
 double slope=17.502*240.97*esat/pow((ta+240.97),2); // [Pa/oC] (3.9)
 double h_wet=((rn-g0)-(rohair*cp/rew)*(esat-eact)/gamma)/(1.0+slope/gamma); //Equation 16 in SEBS paper
 double ci;
 if(z_pbl>=hst) ci = cw(z_pbl,molength,z0m,z0h);
 else ci = psih(-z_pbl/molength);
 double rah=((log((z_pbl-d)/z0h))-ci)/(k*ustar);
 double h_pixel=rohair*cp*dtair/rah;
 if(h_pixel > h_dry) h_pixel = h_dry;
 double ev_r=1-(h_pixel-h_wet)/(h_dry-h_wet); //SSEBI Menenti Model
 double evapfr=ev_r*(1-h_wet/h_dry); //Evaporative Fraction
 return(evapfr);
}

double sebs_eta(double doy, double ndvi, double ndvi_max, double ndvi_min, double ndvi_median, double u_s, double z_pbl, double t_s, double p_s, double alt_ms, double e_act, double latitude, double kin24, double albedo, double dem, double lst, double e0, double evapfr)
{
 //doy = day of year (-)
 //visi = atmospheric visibility (km)
 //latitude = Latitude (dd.dd)
  
 //****************Calculating dail evaporation***********
 double d, h, kb1, z0h,t_c,t_pbl,t0,t0_pbl,ta;
 double z_ms = 10.0; //the height of measuring wind (m)
 //Auto-check/conversion from Celsius to Kelvin
 if(t_s<100) t_s = t_s + 273.15;
 double sigma = 5.678*pow(10,-8);//Stefan-Boltzmann constant

 //Here we use a simple formula for LAI=f(NDVI), cf Su (1996)
 double lai = leafai(ndvi_median);
 double fc = f_c(ndvi_median,ndvi_min,ndvi_max);
 double z0m = z0_m(ndvi_median,ndvi_max);
 d 	= z0m * 4.9; // zero plane displacement height
 double u_c = log((z_pbl-d)/z0m)/log((z_ms-d)/z0m);
 double u_pbl = u_s*u_c;
 double p_pbl = p_s * pow(((44331.0 - dem + z_pbl)/(44331.0 - alt_ms)),(1/0.1903));
 int i;
 for(i=0;i<2;i++)
 {
  d 	= z0m * 4.9; // zero plane displacement height
  h 	= z0m / 0.136; // the total height of vegetation (m)
  if(i==0) kb1 = kb_1(u_pbl, z_pbl, h, lai, fc, t_s,p_s);
  else  kb1 = kb_1(u_pbl, z_pbl, h, lai, fc, ta, p_pbl);
  z0h	= z0m/exp(kb1);
  t_c	= log((z_pbl-d)/z0h)/log((z_ms-d)/z0h);
  t_pbl	= (t_s*(1.0-fc)+t_c*fc)/pow((1.0-dem/44331.0),1.5029);
  if(i==0)
  {
   t0 	= lst / pow((1.0-dem/44331.0),1.5029);
   t0_pbl = 0.5 * (t0 + t_pbl); //Mean potential temperature
   if(t0_pbl>100) ta=t0_pbl-273.15; // Used for the calculation of nu (ta, pa)
   lai	= leafai(ndvi);
   fc	= f_c(ndvi,ndvi_min,ndvi_max);
   z0m	= z0_m(ndvi,ndvi_max);
  }
 }
 double em_air = 0.34-0.14*pow((0.001*e_act),0.5); //Atmospheric emissivity
 double tae=pow(em_air,0.5)*pow(t_pbl,2);
 double lambda = 2.501-0.02361*t_pbl;
 double t0e=pow(e0,0.5)*pow(lst,2);
 double l24=sigma*(tae+t0e)*(tae-t0e);

 double rnetd=(1-albedo)*kin24+l24;
 if(fabs(lambda)<pow(10,-5)) lambda=pow(10,-5);
 double eta24=0.0864*evapfr*rnetd/lambda;
 return(eta24);
}
