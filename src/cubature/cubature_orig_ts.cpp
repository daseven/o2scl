/*
  -------------------------------------------------------------------
  
  Copyright (C) 2015-2019, Andrew W. Steiner
  
  This file is part of O2scl. It has been adapted from cubature 
  written by Steven G. Johnson. 
  
  O2scl is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.
  
  O2scl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with O2scl. If not, see <http://www.gnu.org/licenses/>.

  -------------------------------------------------------------------
*/
/* 
 * Copyright (c) 2005-2013 Steven G. Johnson
 *
 * Portions (see comments) based on HIntLib (also distributed under
 * the GNU GPL, v2 or later), copyright (c) 2002-2005 Rudolf Schuerer.
 *     (http://www.cosy.sbg.ac.at/~rschuer/hintlib/)
 *
 * Portions (see comments) based on GNU GSL (also distributed under
 * the GNU GPL, v2 or later), copyright (c) 1996-2000 Brian Gough.
 *     (http://www.gnu.org/software/gsl/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <iostream>
#include <o2scl/cubature_orig.h>
#include <o2scl/test_mgr.h>

static const bool debug=false;

using namespace std;
using namespace o2scl;

int cub_count = 0;
int which_integrand;
const double radius = 0.50124145262344534123412; /* random */

/* Simple constant function */
double fconst (double x[], size_t dim, void *params) {
  return 1;
}

/*** f0, f1, f2, and f3 are test functions from the Monte-Carlo
     integration routines in GSL 1.6 (monte/test.c).  Copyright (c)
     1996-2000 Michael Booth, GNU GPL. ****/

/* Simple product function */
double f0 (unsigned dim, const double *x, void *params) {
  double prod = 1.0;
  unsigned int i;
  for (i = 0; i < dim; ++i)
    prod *= 2.0 * x[i];
  return prod;
}

#define K_2_SQRTPI 1.12837916709551257390

/* Gaussian centered at 1/2. */
double f1 (unsigned dim, const double *x, void *params) {
  double a = *(double *)params;
  double sum = 0.;
  unsigned int i;
  for (i = 0; i < dim; i++) {
    double dx = x[i] - 0.5;
    sum += dx * dx;
  }
  return (pow (K_2_SQRTPI / (2. * a), (double) dim) *
	  exp (-sum / (a * a)));
}

/* double gaussian */
double f2 (unsigned dim, const double *x, void *params) {
  double a = *(double *)params;
  double sum1 = 0.;
  double sum2 = 0.;
  unsigned int i;
  for (i = 0; i < dim; i++) {
    double dx1 = x[i] - 1. / 3.;
    double dx2 = x[i] - 2. / 3.;
    sum1 += dx1 * dx1;
    sum2 += dx2 * dx2;
  }
  return 0.5 * pow (K_2_SQRTPI / (2. * a), dim) 
    * (exp (-sum1 / (a * a)) + exp (-sum2 / (a * a)));
}

/* Tsuda's example */
double f3 (unsigned dim, const double *x, void *params) {
  double c = *(double *)params;
  double prod = 1.;
  unsigned int i;
  for (i = 0; i < dim; i++)
    prod *= c / (c + 1) * pow((c + 1) / (c + x[i]), 2.0);
  return prod;
}

/* test integrand from W. J. Morokoff and R. E. Caflisch, "Quasi=
   Monte Carlo integration," J. Comput. Phys 122, 218-230 (1995).
   Designed for integration on [0,1]^dim, integral = 1. */
static double morokoff(unsigned dim, const double *x, void *params) {
  double p = 1.0 / dim;
  double prod = pow(1 + p, dim);
  unsigned int i;
  for (i = 0; i < dim; i++)
    prod *= pow(x[i], p);
  return prod;
}

/*** end of GSL test functions ***/

int f_test(unsigned dim, const double *x, void *data_,
	   unsigned fdim, double *retval) {
  
  double val;
  unsigned i, j;
  ++cub_count;
  (void) data_; /* not used */
  for (j = 0; j < 1; ++j) {
    double fdata = which_integrand == 6 ? (1.0+sqrt (10.0))/9.0 : 0.1;
    switch (which_integrand) {
    case 0: /* simple smooth (separable) objective: prod. cos(x[i]). */
      val = 1;
      for (i = 0; i < dim; ++i)
	val *= cos(x[i]);
      break;
    case 1: { /* integral of exp(-x^2), rescaled to (0,infinity) limits */
      double scale = 1.0;
      val = 0;
      for (i = 0; i < dim; ++i) {
	if (x[i] > 0) {
	  double z = (1 - x[i]) / x[i];
	  val += z * z;
	  scale *= K_2_SQRTPI / (x[i] * x[i]);
	}
	else {
	  scale = 0;
	  break;
	}
      }
      val = exp(-val) * scale;
      break;
    }
    case 2: /* discontinuous objective: volume of hypersphere */
      val = 0;
      for (i = 0; i < dim; ++i)
	val += x[i] * x[i];
      val = val < radius * radius;
      break;
    case 3:
      val = f0(dim, x, &fdata);
      break;
    case 4:
      val = f1(dim, x, &fdata);
      break;
    case 5:
      val = f2(dim, x, &fdata);
      break;
    case 6:
      val = f3(dim, x, &fdata);
      break;
    case 7:
      val = morokoff(dim, x, &fdata);
      break;
    default:
      cout << "Unknown integrand." << endl;
      exit(-1);
    }
    retval[j] = val;
  }
  return 0;
}

#define K_PI 3.14159265358979323846

/* surface area of n-dimensional unit hypersphere */
static double S(unsigned n) {
  double val;
  int fact = 1;
  if (n % 2 == 0) { /* n even */
    val = 2 * pow(K_PI, n * 0.5);
    n = n / 2;
    while (n > 1) fact *= (n -= 1);
    val /= fact;
  }
  else { /* n odd */
    val = (1 << (n/2 + 1)) * pow(K_PI, n/2);
    while (n > 2) fact *= (n -= 2);
    val /= fact;
  }
  return val;
}

static double exact_integral(int which, unsigned dim, const double *xmax) {
  unsigned i;
  double val;
  switch(which) {
  case 0:
    val = 1;
    for (i = 0; i < dim; ++i)
      val *= sin(xmax[i]);
    break;
  case 2:
    val = dim == 0 ? 1 : S(dim) * pow(radius * 0.5, dim) / dim;
    break;
  default:
    val = 1.0;
  }
  return val;
}

int fv(unsigned ndim, size_t npt, const double *x, unsigned fdim,
       double *fval) {
  for (unsigned i = 0; i < npt; ++i) {
    if (f_test(ndim, x + i*ndim, 0, fdim, fval + i*fdim)) {
      return o2scl::gsl_failure;
    }
  }
  if (debug) {
    cout << ndim << " " << fdim << " " << npt << endl;
    for(unsigned i=0;i<npt;i++) {
      for(size_t k=0;k<ndim;k++) {
	cout << (x+i*ndim)[k] << " ";
      }
      for(size_t k=0;k<fdim;k++) {
	cout << (fval+i*fdim)[k] << " ";
      }
      cout << endl;
    }
    char ch;
    cin >> ch;
  }
  return o2scl::success;
}

/** Test integrating a few functions at once
 */
int fv2(unsigned ndim, size_t npt, const double *x, unsigned fdim,
	  double *fval) {
  for (size_t i=0;i<npt;i++) {
    const double *x2=x+i*ndim;
    double *f2=fval+i*fdim;
    f2[0]=exp(-((x2[0]-0.2)*(x2[0]-0.2)+
		(x2[1]-0.5)*(x2[1]-0.5)));
    f2[1]=exp(-((x2[0]-0.2)*(x2[0]-0.2)+
		(x2[1]-0.5)*(x2[1]-0.5)))*x2[0]*x2[0];
    f2[2]=exp(-((x2[0]-0.2)*(x2[0]-0.2)+
		(x2[1]-0.5)*(x2[1]-0.5)))*x2[0]*x2[0]*x2[1]*x2[1];
  }
  return 0;
}

int main(void) {

  cout.setf(ios::scientific);
  
  test_mgr tmgr;
  tmgr.set_output_level(1);

  size_t dim=3;
  double xmin[3], xmax[3];
  std::vector<double> xmin2(3), xmax2(3);
  for (size_t i=0;i<dim;++i) {
    xmin[i]=0.0;
    xmax[i]=1.0;
    xmin2[i]=0.0;
    xmax2[i]=1.0;
  }

  typedef std::function<
    int(unsigned,size_t,const double *,unsigned,double *)> cub_funct_arr;
  inte_hcubature_orig<cub_funct_arr> hc;
  inte_pcubature_orig<cub_funct_arr,std::vector<double> > pc;

  inte_cubature_orig_base::error_norm en=
    inte_cubature_orig_base::ERROR_INDIVIDUAL;
  
  cub_funct_arr cfa=fv;

  // Test both hcubature and pcubature with several integrands
  // and compare with original cubature testing results
  
  int test_n[14]={33,125,693,4913,70785,33,3861,35937,3465,35937,297,
		  729,33,729};
  
  double test_vals[14][3]={{5.958229e-01,3.519922e-06,3.523658e-07},
			   {5.958236e-01,2.130785e-04,3.832854e-07},
			   {1.002290e+00,9.980917e-03,2.290472e-03},
			    {9.999119e-01,1.113448e-03,8.812269e-05},
			    {6.514615e-02,6.405123e-04,7.924271e-04},
			    {1.000000e+00,2.220446e-16,2.220446e-16},
			    {1.000753e+00,9.612568e-03,7.526466e-04},
			    {1.000000e+00,2.155111e-04,1.324296e-08},
			    {9.852783e-01,9.774575e-03,1.472168e-02},
			   {9.999963e-01,7.175992e-05,3.650226e-06},
			   {9.998328e-01,7.738486e-03,1.671812e-04},
			   {9.999948e-01,1.425689e-03,5.187945e-06},
			   {1.001055e+00,4.808302e-03,1.055387e-03},
			   {9.967782e-01,6.471054e-03,3.221771e-03}};
  
  int tcnt=0;
  for(size_t test_iand=0;test_iand<8;test_iand++) {

    double tol, val, err;
    unsigned maxEval;

    tol=1.0e-2;
    maxEval=0;
    
    which_integrand = test_iand; 
    
    if (test_iand!=2) {

      cub_count=0;
      hc.integ(1,cfa,dim,xmin,xmax,maxEval,0,tol,en,&val,&err);
	       
      cout << "# " << which_integrand << " " 
	   << "integral " << val << " " << "est. error " << err << " " 
	   << "true error " 
	   << fabs(val-exact_integral(which_integrand,dim,xmax)) << endl;
      cout << "evals " << cub_count << endl;

      tmgr.test_gen(fabs(val-exact_integral(which_integrand,dim,xmax))<
		    err*2.0,"hcub 2");
      tmgr.test_gen(test_n[tcnt]==cub_count,"cub_count");
      tmgr.test_rel(val,test_vals[tcnt][0],5.0e-6,"val");
      tmgr.test_rel(err,test_vals[tcnt][1],5.0e-6,"err");
      tmgr.test_rel(fabs(val-exact_integral(which_integrand,dim,xmax)),
		    test_vals[tcnt][2],5.0e-6,"diff w/ exact");
      tcnt++;
    }

    if (test_iand!=3) {

      cub_count=0;
      pc.integ(1,cfa,dim,xmin2,xmax2,maxEval,0,tol,en,&val,&err);
	       
      cout << "# " << which_integrand << " " 
	   << "integral " << val << " " << "est. error " << err << " " 
	   << "true error " 
	   << fabs(val-exact_integral(which_integrand,dim,xmax)) << endl;
      cout << "evals " << cub_count << endl;

      tmgr.test_gen(fabs(val-exact_integral(which_integrand,dim,xmax))<
		    err*2.0,"pcub 2");
      tmgr.test_gen(test_n[tcnt]==cub_count,"cub_count");
      tmgr.test_rel(val,test_vals[tcnt][0],5.0e-6,"val");
      tmgr.test_rel(err,test_vals[tcnt][1],5.0e-6,"err");
      tmgr.test_rel(fabs(val-exact_integral(which_integrand,dim,xmax)),
		    test_vals[tcnt][2],5.0e-6,"diff w/ exact");
      tcnt++;
    }
    
  }

  // With parallelism for hcubature
  hc.use_parallel=1;
  
  tcnt=0;
  for(size_t test_iand=0;test_iand<8;test_iand++) {

    double tol, val=0.0, err=0.0;
    unsigned maxEval;

    tol=1.0e-2;
    maxEval=0;
    
    which_integrand = test_iand; 
    
    if (test_iand!=2) {

      cub_count=0;
      hc.integ(1,cfa,dim,xmin,xmax,maxEval,0,tol,en,&val,&err);
	       
      cout << "# " << which_integrand << " " 
	   << "integral " << val << " " << "est. error " << err << " " 
	   << "true error " 
	   << fabs(val-exact_integral(which_integrand,dim,xmax)) << endl;
      cout << "evals " << cub_count << endl;

      tmgr.test_gen(fabs(val-exact_integral(which_integrand,dim,xmax))<
		    err*2.0,"hcub 2");
      tmgr.test_gen(test_n[tcnt]==cub_count,"cub_count");
      tmgr.test_rel(val,test_vals[tcnt][0],5.0e-6,"val");
      tmgr.test_rel(err,test_vals[tcnt][1],5.0e-6,"err");
      tmgr.test_rel(fabs(val-exact_integral(which_integrand,dim,xmax)),
		    test_vals[tcnt][2],5.0e-6,"diff w/ exact");
      tcnt++;
    }

    if (test_iand!=3) {
      tcnt++;
    }
    
  }

  // Now run with dim=1 (without parallelism)

  hc.use_parallel=0;

  int test_n2[14]={15,5,75,17,257,15,45,33,105,33,15,9,15,3};
  
  double test_vals2[14][3]={{8.414710e-01,9.342205e-15,0.000000e+00},
			    {8.414712e-01,3.009270e-04,1.804358e-07},
			    {1.000000e+00,2.435248e-06,6.890044e-13},
			    {9.999706e-01,1.113513e-03,2.937509e-05},
			    {5.030680e-01,3.067965e-03,1.826509e-03},
			    {1.000000e+00,1.110223e-14,0.000000e+00},
			    {1.000000e+00,1.225522e-04,4.218679e-10},
			    {1.000000e+00,2.155111e-04,4.414323e-09},
			    {9.999988e-01,2.820725e-07,1.214234e-06},
			    {9.999988e-01,7.176009e-05,1.216743e-06},
			    {1.000000e+00,8.919302e-07,8.393286e-14},
			    {9.999983e-01,1.425694e-03,1.729318e-06},
			    {1.000000e+00,1.110223e-14,0.000000e+00},
			    {1.000000e+00,0.000000e+00,0.000000e+00}};
  
  dim=1;
  tcnt=0;

  for(size_t test_iand=0;test_iand<8;test_iand++) {

    double tol, val, err;
    unsigned maxEval;

    tol=1.0e-2;
    maxEval=0;
    
    which_integrand = test_iand; 
    
    if (test_iand!=2) {

      cub_count=0;
      hc.integ(1,cfa,dim,xmin,xmax,maxEval,0,tol,en,&val,&err);
	       
      cout << "# " << which_integrand << " " 
	   << "integral " << val << " " << "est. error " << err << " " 
	   << "true error " 
	   << fabs(val-exact_integral(which_integrand,dim,xmax)) << endl;
      cout << "evals " << cub_count << endl;

      if (test_iand!=5) {
	tmgr.test_gen(fabs(val-exact_integral(which_integrand,dim,xmax))<
		      err*2.0,"hcub 2");
      }
      tmgr.test_gen(test_n2[tcnt]==cub_count,"cub_count");
      tmgr.test_rel(val,test_vals2[tcnt][0],5.0e-6,"val");
      tmgr.test_rel(err,test_vals2[tcnt][1],5.0e-6,"err");
      tmgr.test_rel(fabs(val-exact_integral(which_integrand,dim,xmax)),
		    test_vals2[tcnt][2],5.0e-6,"diff w/ exact");
      tcnt++;
    }

    if (test_iand!=3) {

      cub_count=0;
      pc.integ(1,cfa,dim,xmin2,xmax2,maxEval,0,tol,en,&val,&err);
	       
      cout << "# " << which_integrand << " " 
	   << "integral " << val << " " << "est. error " << err << " " 
	   << "true error " 
	   << fabs(val-exact_integral(which_integrand,dim,xmax)) << endl;
      cout << "evals " << cub_count << endl;

      if (test_iand!=7) {
	tmgr.test_gen(fabs(val-exact_integral(which_integrand,dim,xmax))<
		      err*2.0,"pcub 2");
      }
      tmgr.test_gen(test_n2[tcnt]==cub_count,"cub_count");
      tmgr.test_rel(val,test_vals2[tcnt][0],5.0e-6,"val");
      tmgr.test_rel(err,test_vals2[tcnt][1],5.0e-6,"err");
      tmgr.test_rel(fabs(val-exact_integral(which_integrand,dim,xmax)),
		    test_vals2[tcnt][2],5.0e-6,"diff w/ exact");
      tcnt++;
    }
    
  }

  // Run another test which tests integrating more than one
  // function at a time
  
  {
    double dlow[2]={-2.0,-2.0};
    double dhigh[2]={2.0,2.0};
    vector<double> vlow(2), vhigh(2);
    vlow[0]=-2.0;
    vlow[1]=-2.0;
    vhigh[0]=2.0;
    vhigh[1]=2.0;
    double dres[3], derr[3];
    cub_funct_arr cfa2=fv2;
    int ret=hc.integ(3,cfa2,2,dlow,dhigh,10000,0.0,1.0e-4,en,dres,derr);
    tmgr.test_gen(ret==0,"hc mdim ret");
    tmgr.test_rel(3.067993,dres[0],1.0e-6,"hc mdim val 0");
    tmgr.test_rel(1.569270,dres[1],1.0e-6,"hc mdim val 1");
    tmgr.test_rel(1.056968,dres[2],1.0e-6,"hc mdim val 2");
    ret=pc.integ(3,cfa2,2,vlow,vhigh,10000,0.0,1.0e-4,en,dres,derr);
    tmgr.test_gen(ret==0,"pc mdim ret");
    tmgr.test_rel(3.067993,dres[0],1.0e-6,"pc mdim val 0");
    tmgr.test_rel(1.569270,dres[1],1.0e-6,"pc mdim val 1");
    tmgr.test_rel(1.056968,dres[2],1.0e-6,"pc mdim val 2");
  }
    
  tmgr.report();
  return 0;
}
