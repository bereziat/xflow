/*
 * Calcul de diverses statistiques pour les champs de vecteurs
 * Sortie latex possible (ligne de tableau).
 *
 * 1.5: - ajout tag_float (pas fini)
 * 1.4: - ajout stats sur le champ référence (nrmin,nrmax,nrstd)
 * 1.3: - meilleur calcul de l'orientation d'un champ (avec atan2)
 *      - un mauvais calcul de l'écart-type de l'orientation a été fixé
 *      - ajout d'un seuil sur la norme (absolue) du champs estimé (voir pour champs référent)
 */
#include <inrimage/image.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define VERSION "1.5"

#define PIX(I,i,j,k,v) (I)[(v)+NDIMV*((i)+NDIMX*((j)+NDIMY*(k)))]
#define DEG(a) (180*(a)/M_PI)

float *readi( char *fname, Fort_int lfmt[], int iz, int z, int verif);

double barron_angular_error(double ur, double vr, double ue, double ve, double delta) {
  return acos( (ur * ue + vr * ve + delta*delta) / 
	    sqrt( (ur * ur + vr * vr + delta * delta) * (ue * ue + ve * ve + delta * delta)));
}

double my_angular_error( double ur, double vr, double ue, double ve, double delta) {
  return acos( (ur * ue + vr * ve) / 
	       (sqrt( (ur * ur + vr * vr) * (ue * ue + ve * ve))+delta*delta));
  // c'est pas top, faut faire du modulo  2pi
  //  return atan(-ve/(ue+alpha )) - atan(-vr/(ur+alpha ));
}

double (*angular_error)(double,double,double,double,double);

float trn = 1e-4;
float tag_float = 202021.25;

int main( int argc, char **argv) {
  char fname[256];
  float *w_r, *w_e;
  int z=0, iz=1;
  Fort_int lfmt[9];
  double delta = 1e-6;
  double alpha = 1e-12;
  double thresh = 0;
  char format[10] = "%f";

  inr_init( argc, argv, VERSION, "ref est [options]", 
	    "Compute and display statistics between two vector fields. Options are:\n"
	    "  -z : number of frame to process (all by default)\n"
	    "  -iz : start at this frame (first is the default)\n"
	    "  -f= : format of float number in latex output (%f is the default)\n" 
	    "  -latex= : output is in LaTeX format, this parameter gives a list of\n"
	    "            statistics to display, separated by a comma, possible \n"
	    "            values are: \n"
	    "      aemin,aemean,eastd: angular error min, mean, max and std dev.\n"
	    "      rnemin,rnemean,rnemax, nestd: relative norm error min,mean,...\n"
	    "      anemin,anemean,anemax, anestd: absolute norm error min,mean,...\n"
	    "      epemean,epestd: end point error, mean, std-dev (MiddleBurry)\n"
	    "      nrmin,nrmean,nrmax, nrstd: norm of <ref> min,mean,...\n"
	    "      nemin,nemean,nemax, nestd: norm of <est> min,mean,...\n"
	    "      oemin,oemean,oemax, oestd: orientation of <est> min,mean,...\n"
	    "      corru, corrv: correlation between <ref> and <est> for component u, v\n"
	    "  -trn: threshold for computation of relative norms\n"
	    "  -barron: use the Barron formulae to compute angular error\n"
	    "  -thresh: only consider vector whose norm is greater or equal to\n" 
	    "           thresh (0 by default).\n"
	    "  -tag_float: FIX ME\n"
	    );
  igetopt1("-trn","%f",&trn);
  igetopt1("-z","%d",&z);
  igetopt1("-iz","%d",&iz);
  igetopt0x("-f=","%s",format);
  if( igetopt0("-barron")) {
    angular_error = barron_angular_error;
    delta = 1;
  } else
    angular_error = my_angular_error;

  igetopt1("-delta","%lf",&delta);
  igetopt1("-alpha","%lf",&alpha);
  if( igetopt1("-thresh","%lf",&thresh))
    thresh *= thresh;

  infileopt( fname);
  w_r = readi( fname, lfmt, iz, z, 0);

  infileopt( fname);
  w_e = readi( fname, lfmt, iz, z, 1);  

  /* FIXME: plein de variables inutiles */

  /* auxiliaires */
  double u_e, v_e, u_r, v_r;
  int i, j, k, n, n_rel = 0;
  double norm_r, norm_e, rne_rel, ang_err, ang;

  /* norme des vecteurs */
  double sum_norm_e = 0, sum_norm_r = 0;
  double mean_norm_r, mean_norm_e;
  double min_norm_r, max_norm_r, stdv_norm_r = 0;
  double min_norm_e, max_norm_e, stdv_norm_e = 0;

  /* ane */
  double sum_ane = 0;
  double ane, min_ane = 1e11 , mean_ane = 0, max_ane = -1e11, stdv_ane = 0;

  /* rne */
  double sum_rne_rel = 0;
  double min_rne_rel = 1e11, max_rne_rel = -1e11, mean_rne_rel = 0, stdv_rne_rel = 0;

  /* erreur angulaire */
  double sum_ang_err = 0;
  double min_ang_err = 1e11, max_ang_err = -1e11, mean_ang_err, stdv_ang_err;

  /* end point error */
  double sum_epe = 0;
  double min_epe = 1e11, max_epe = 0, mean_epe = 0, stdv_epe = 0;

  /* angle */
  double sum_ang = 0;
  double min_ang = 1e11, mean_ang, max_ang = -1e11, stdv_ang;

  /* corr sur u */
  double sum_ur = 0, sum_ue = 0, sum_ur_ue = 0;
  double mean_ur, mean_ue;
  double corr_u;

  /* corr sur v */
  double sum_vr = 0, sum_ve = 0, sum_vr_ve = 0;
  double mean_vr, mean_ve;
  double corr_v;

  n = 0;
  /* Premiere passe */  
  for( k = 0; k < NDIMZ; k++)
    for( j = 0; j < NDIMY; j++)
      for( i = 0; i < NDIMX; i++) {
	
	u_r = PIX(w_r, i, j, k, 0);
	v_r = PIX(w_r, i, j, k, 1);
	u_e = PIX(w_e, i, j, k, 0);
	v_e = PIX(w_e, i, j, k, 1);
	
	norm_e = hypot( u_e, v_e);

	if( norm_e >= thresh &&
	    u_r != tag_float &&
	    v_r != tag_float
	    ) {

	/* norme des vecteurs */
	norm_r = hypot( u_r, v_r);
	sum_norm_r += norm_r;
	if( norm_r > max_norm_r) max_norm_r = norm_r;
	if( norm_r < min_norm_r) min_norm_r = norm_r;

	sum_norm_e += norm_e;
	if( norm_e > max_norm_e) max_norm_e = norm_e;
	if( norm_e < min_norm_e) min_norm_e = norm_e;

	/* absolute norme error */
	ane = fabs(hypot( u_r, v_r)-hypot( u_e, v_e));
	sum_ane += ane;
	if( ane < min_ane) min_ane = ane;
	if( ane > max_ane) max_ane = ane;


	/* relative norme error */
	if( norm_r > trn) {
	  // Ce seuil devrait être de l'ordre du 10ieme ou
	  // du 100ieme de la norme moyenne du champ référence ?
	  // Deux passes ? 
	  rne_rel = fabs(hypot( u_r, v_r)-hypot( u_e, v_e))/norm_r;
	  sum_rne_rel += rne_rel;
	  if( rne_rel > max_rne_rel) max_rne_rel = rne_rel;
	  if( rne_rel < min_rne_rel) min_rne_rel = rne_rel;
	  n_rel ++;
	}	

	/* end point error */
	ane = hypot( u_r-u_e, v_r-v_e);
	sum_epe += ane;
	if( ane < min_epe) min_epe = ane;
	if( ane > max_epe) max_epe = ane;

	/* Erreur angulaire */
	ang_err = fabs(angular_error(u_r, v_r, u_e, v_e, delta));
	sum_ang_err += ang_err;
	if( ang_err > max_ang_err) max_ang_err = ang_err; 
	if( ang_err < min_ang_err) min_ang_err = ang_err; 
	
	/* Orientation estime */
	// ang = -atan(v_e/(u_e+alpha));
	ang = -atan2(v_e,u_e);
	sum_ang += ang;
	if( ang>max_ang) max_ang = ang;
	if( ang<min_ang) min_ang = ang;

	/* correlation sur u */
	sum_ur += u_r;
	sum_ue += u_e;

	/* correlation sur v */
	sum_vr += v_r;
	sum_ve += v_e;
	
	n ++;

	} /* if norm_e */
      } /* for */
  //  n = NDIMX*NDIMY*NDIMZ;

  mean_ane = sum_ane / n;

  mean_norm_r = sum_norm_r / n;
  mean_norm_e = sum_norm_e / n;
  mean_rne_rel = sum_rne_rel / n_rel;

  mean_ang_err = sum_ang_err / n;
  mean_ang = sum_ang / n;
  
  mean_ur = sum_ur/n;
  mean_ue = sum_ue/n;

  mean_vr = sum_vr/n;
  mean_ve = sum_ve/n;

  mean_epe = sum_epe/n;
  
  /* Seconde passe */
  sum_ane = 0;
  sum_norm_e = 0;
  sum_rne_rel = 0;
  sum_ang_err = 0;
  sum_ang = 0;
  sum_ur = sum_ue = 0;
  sum_vr = sum_ve = 0;
  sum_epe = 0;

  for( k = 0; k < NDIMZ; k++)
    for( j = 0; j < NDIMY; j++)
      for( i = 0; i < NDIMX; i++) {
	
	u_r = PIX(w_r, i, j, k, 0);
	v_r = PIX(w_r, i, j, k, 1);
	u_e = PIX(w_e, i, j, k, 0);
	v_e = PIX(w_e, i, j, k, 1);

	if( hypot(u_e, v_e) >= thresh &&
	    u_r != tag_float &&
	    v_r != tag_float
	    ) {

	/* stdev sur norm_e */
	norm_e = hypot( u_e, v_e) - mean_norm_e;
	sum_norm_e += norm_e * norm_e;

	/* stdev sur norm_r */
	norm_r = hypot( u_r, v_r) - mean_norm_r;
	sum_norm_r += norm_r * norm_r;

	/* stdev de l'erreur en norme relative */
	norm_r = hypot( u_r, v_r);

	if( norm_r > trn) {
	  rne_rel = fabs(hypot( u_r, v_r)-hypot( u_e, v_e))/norm_r -  mean_rne_rel;
	  sum_rne_rel += rne_rel * rne_rel;

	}

	ane = fabs(hypot( u_r, v_r)-hypot( u_e, v_e)) - mean_ane;
	sum_ane += ane*ane;
	
	ane = hypot( u_r-u_e, v_r-v_e) - mean_epe;
	sum_epe += ane*ane;

	/* stdev de l'erreur angulaire */
	ang_err = angular_error(u_r, v_r, u_e, v_e, delta) - mean_ang_err;
	sum_ang_err += ang_err * ang_err;

	/* stdev de l'orientation */
	// ang = -atan(v_e/(u_e+alpha));
	ang = -atan2(v_e,u_e) - mean_ang;
	sum_ang += ang * ang;

	/* corr sur u */
	sum_ur += (u_r - mean_ur) * (u_r - mean_ur);
	sum_ue += (u_e - mean_ue) * (u_e - mean_ue);
	sum_ur_ue += (u_r - mean_ur)*(u_e - mean_ue);

	/* corr sur u */
	sum_vr += (v_r - mean_vr) * (v_r - mean_vr);
	sum_ve += (v_e - mean_ve) * (v_e - mean_ve);
	sum_vr_ve += (v_r - mean_vr)*(v_e - mean_ve);
	} /* if norm_e */
      } /* for */

  stdv_norm_e = sqrt( sum_norm_e / n);
  stdv_norm_r = sqrt( sum_norm_r / n);
  stdv_rne_rel = sqrt( sum_rne_rel / n_rel);
  stdv_ane = sqrt( sum_ane / n);
  stdv_epe = sqrt( sum_epe / n);
  stdv_ang_err = sqrt( sum_ang_err / n);
  stdv_ang = sqrt( sum_ang/n);
  corr_u = sum_ur_ue / (n * sqrt(sum_ue/n) * sqrt(sum_ur/n));
  corr_v = sum_vr_ve / (n * sqrt(sum_ve/n) * sqrt(sum_vr/n));

  /* Impression */
  if( !igetopt0x("-latex=", "%s", fname)) {
    printf( "*** r=reference e=estimated\n");
     
    printf( "* Norms\n");
    printf( "  Min  ||r|| : %g\n", min_norm_r);
    printf( "  Mean ||r|| : %g\n", mean_norm_r);
    printf( "  Max  ||r|| : %g\n", max_norm_r);
    printf( "  StDv ||r|| : %g\n", stdv_norm_r);

    printf( "  Min  ||e|| : %g\n", min_norm_e);
    printf( "  Mean ||e|| : %g\n", mean_norm_e);
    printf( "  Max  ||e|| : %g\n", max_norm_e);
    printf( "  StDv ||e|| : %g\n", stdv_norm_e);
    
    printf( "* Orientation\n");
    printf( "  Min Theta_e  : %g\n", DEG(min_ang));
    printf( "  Max Theta_e  : %g\n", DEG(max_ang));
    printf( "  Mean Theta_e : %g\n", DEG(mean_ang));
    printf( "  StDv Theta_e : %g\n", DEG(stdv_ang));
    
    printf( "* Relative norm errors\n");
    printf( "  Min  abs(||r|| - ||e||) / ||r|| : %g\n", min_rne_rel);
    printf( "  Max  abs(||r|| - ||e||) / ||r|| : %g\n", max_rne_rel);
    printf( "  Mean abs(||r|| - ||e||) / ||r|| : %g\n", mean_rne_rel);
    printf( "  StDv abs(||r|| - ||e||) / ||r|| : %g\n", stdv_rne_rel);

    printf( "* Absolute norm errors\n");
    printf( "  Min  abs(||r|| - ||e||) : %g\n", min_ane);
    printf( "  Max  abs(||r|| - ||e||) : %g\n", max_ane);
    printf( "  Mean abs(||r|| - ||e||) : %g\n", mean_ane);
    printf( "  StDv abs(||r|| - ||e||) : %g\n", stdv_ane);
    
    printf( "* End points errors\n");
    printf( "  Min  abs(||r - e||) : %g\n", min_epe);
    printf( "  Max  abs(||r - e||) : %g\n", max_epe);
    printf( "  Mean abs(||r - e||) : %g\n", mean_epe);
    printf( "  StDv abs(||r - e||) : %g\n", stdv_epe);

    printf( "* Angular errors\n");
    printf( "  Min  (|Theta_r - Theta_e|) : %g\n", DEG(min_ang_err));
    printf( "  Max  (|Theta_r - Theta_e|) : %g\n", DEG(max_ang_err));
    printf( "  Mean (|Theta_r - Theta_e|) : %g\n", DEG(mean_ang_err));
    printf( "  StDv (|Theta_r - Theta_e|) : %g\n", DEG(stdv_ang_err));
    
    printf( "* Correlation sur u: %g\n", corr_u);
    printf( "* Correlation sur v: %g\n", corr_v);
  } else {
    char *parse;
    
    printf( " & ");
    for( parse=fname; parse; ) {
      if( strstr( parse, "aemin") == parse)       printf( format, DEG(min_ang_err));
      else if( strstr( parse, "aemax") == parse)  printf( format, DEG(max_ang_err));
      else if( strstr( parse, "aemean") == parse) printf( format, DEG(mean_ang_err));
      else if( strstr( parse, "aestd") == parse)  printf( format, DEG(stdv_ang_err));
      
      else if( strstr( parse, "rnemin") == parse) printf( format, min_rne_rel);
      else if( strstr( parse, "rnemax") == parse) printf( format, max_rne_rel);
      else if( strstr( parse, "rnemean") == parse) printf( format, mean_rne_rel);
      else if( strstr( parse, "rnestd") == parse)  printf( format, stdv_rne_rel);
      
      else if( strstr( parse, "anemin") == parse) printf( format, min_ane);
      else if( strstr( parse, "anemax") == parse) printf( format, max_ane);
      else if( strstr( parse, "anemean") == parse) printf( format, mean_ane);
      else if( strstr( parse, "anestd") == parse)  printf( format, stdv_ane);
      
      else if( strstr( parse, "nrmean") == parse) printf( format, mean_norm_r);
      else if( strstr( parse, "nrmin") == parse)  printf( format, min_norm_r);
      else if( strstr( parse, "nrmax") == parse)  printf( format, max_norm_r);
      else if( strstr( parse, "nrstd") == parse)  printf( format, stdv_norm_r);

      else if( strstr( parse, "nemean") == parse) printf( format, mean_norm_e);
      else if( strstr( parse, "nemin") == parse)  printf( format, min_norm_e);
      else if( strstr( parse, "nemax") == parse)  printf( format, max_norm_e);
      else if( strstr( parse, "nestd") == parse)  printf( format, stdv_norm_e);


      else if( strstr( parse, "epemin") == parse)  printf( format, min_epe);
      else if( strstr( parse, "epemax") == parse)  printf( format, max_epe);
      else if( strstr( parse, "epemean") == parse)  printf( format, mean_epe);
      else if( strstr( parse, "epestd") == parse)  printf( format, stdv_epe);
      
      else if( strstr( parse, "oemin") == parse)  printf( format, DEG(min_ang));
      else if( strstr( parse, "oemax") == parse)  printf( format, DEG(max_ang));
      else if( strstr( parse, "oemean") == parse) printf( format, DEG(mean_ang));
      else if( strstr( parse, "oestd") == parse)  printf( format, DEG(stdv_ang));

      else if( strstr( parse, "corru") == parse) printf( format, corr_u);
      else if( strstr( parse, "corrv") == parse) printf( format, corr_v);
      else 
	fprintf( stderr, "erreur: %s n'existe pas\n", parse);

      parse=strchr(parse,',');	
      if(parse) {
	printf(" & ");
	parse++;
      } else
	printf(" \\\\\n");
    }
  }
  
  return 0;
}


float *readi( char *fname, Fort_int lfmt[], int iz, int z, int verif) {
  float *buf;
  Fort_int gfmt[9];
  struct image *nf;

  nf = image_(fname, "e", "", verif?gfmt:lfmt);

  if ( !(TYPE==REELLE && BSIZE==4 && NDIMV==2))
    imerror( 6, "%s: format non recevable (codage reelle a deux composantes requit!\n", fname);
  if( verif && !(NDIMX==gfmt[I_NDIMX] && NDIMY==gfmt[I_NDIMY]))
    imerror( 7, "les deux images doivent avoir le même nombre de lignes et de colonnes!\n");
    
  if(z) NDIMZ = z;
  buf  = (float *)malloc(DIMX*NDIMY*NDIMZ*sizeof(float));
  c_lptset( nf, NDIMY*(iz-1)+1);
  c_lect( nf, NDIMY*NDIMZ, buf);
  fermnf_( &nf);
  return buf;
}

