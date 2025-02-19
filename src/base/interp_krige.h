/*
  -------------------------------------------------------------------
  
  Copyright (C) 2017-2019, Andrew W. Steiner
  
  This file is part of O2scl.
  
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
#ifndef O2SCL_INTERP_KRIGE_H
#define O2SCL_INTERP_KRIGE_H

/** \file interp_krige.h
    \brief One-dimensional interpolation by Kriging
*/

#include <iostream>
#include <string>

#include <gsl/gsl_sf_erf.h>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/operation.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>

#include <o2scl/interp.h>
#include <o2scl/cholesky.h>
#include <o2scl/lu.h>
#include <o2scl/columnify.h>
#include <o2scl/vector.h>
#include <o2scl/vec_stats.h>
#include <o2scl/min_brent_gsl.h>
#include <o2scl/constants.h>

#ifndef DOXYGEN_NO_O2NS
namespace o2scl {
#endif
  
  /** \brief Interpolation by Kriging with a user-specified 
      covariance function
      
      See also the \ref intp_section section of the \o2 User's guide. 

      \note The function \ref set() stores a pointer to the covariance
      function and its derivatives and integrals so they cannot go out
      of scope before any of the interpolation functions are called.

      \note This class is experimental.
  */
  template<class vec_t, class vec2_t=vec_t,
    class covar_func_t=std::function<double(double,double)>,
    class covar_integ_t=std::function<double(double,double,double)> >
    class interp_krige : public interp_base<vec_t,vec2_t> {
    
#ifdef O2SCL_NEVER_DEFINED
  }{
#endif

  public:

    typedef boost::numeric::ublas::vector<double> ubvector;
    typedef boost::numeric::ublas::matrix<double> ubmatrix;
    typedef boost::numeric::ublas::matrix_column<ubmatrix> ubmatrix_column;
    
  protected:

    /** \brief Inverse covariance matrix times function vector
     */
    ubvector Kinvf;

    /** \brief Pointer to user-specified covariance function
     */
    covar_func_t *f;
    
    /** \brief Pointer to user-specified derivative
     */
    covar_func_t *df;
    
    /** \brief Pointer to user-specified second derivative
     */
    covar_func_t *df2;
    
    /** \brief Pointer to user-specified integral
     */
    covar_integ_t *intp;
    
  public:
    
    interp_krige() {
      this->min_size=2;
      matrix_mode=matrix_cholesky;
    }
    
    virtual ~interp_krige() {}
    
    /// \name Select matrix inversion method
    //@{
    /** \brief Method for matrix inversion 
     */
    size_t matrix_mode;
    /// Use Cholesky decomposition
    static const size_t matrix_cholesky=0;
    /// Use LU decomposition
    static const size_t matrix_LU=1;
    //@}
    
    /// Initialize interpolation routine
    virtual void set(size_t size, const vec_t &x, const vec2_t &y) {
      O2SCL_ERR2("Function set(size_t,vec_t,vec_t) unimplemented ",
		 "in interp_krige.",o2scl::exc_eunimpl);
      return;
    }

    /** \brief Initialize interpolation routine, specifying derivatives
	and integrals [not yet implemented]
    */
    virtual int set_covar_di_noise(size_t n_dim, const vec_t &x,
				   const vec_t &y, covar_func_t &fcovar,
				   covar_func_t &fderiv,
				   covar_func_t &fderiv2,
				   covar_func_t &finteg,
				   double noise_var, bool err_on_fail=true) {
      O2SCL_ERR("Function set_covar_di_noise not yet implemented.",
		o2scl::exc_eunimpl);
      return 0;
    }
    
    /// Initialize interpolation routine
    virtual int set_covar_noise(size_t n_dim, const vec_t &x, const vec_t &y,
				covar_func_t &fcovar, double noise_var,
				bool err_on_fail=true) {
      
      if (n_dim<this->min_size) {
	O2SCL_ERR((((std::string)"Vector size, ")+szttos(n_dim)+", is less"+
		   " than "+szttos(this->min_size)+" in interp_krige::"+
		   "set().").c_str(),exc_einval);
      }

      // Store pointer to covariance function
      f=&fcovar;
      
      // Construct the KXX matrix
      ubmatrix KXX(n_dim,n_dim);
      for(size_t irow=0;irow<n_dim;irow++) {
	for(size_t icol=0;icol<n_dim;icol++) {
	  if (irow>icol) {
	    KXX(irow,icol)=KXX(icol,irow);
	  } else if (irow==icol) {
	    KXX(irow,icol)=fcovar(x[irow],x[icol])+noise_var;
	  } else {
	    KXX(irow,icol)=fcovar(x[irow],x[icol]);
	  }
	}
      }
      
      if (matrix_mode==matrix_LU) {
	
	// Construct the inverse of KXX
	ubmatrix inv_KXX(n_dim,n_dim);
	o2scl::permutation p(n_dim);
	int signum;
	o2scl_linalg::LU_decomp(n_dim,KXX,p,signum);
	if (o2scl_linalg::diagonal_has_zero(n_dim,KXX)) {
	  if (err_on_fail) {
	    O2SCL_ERR2("Matrix singular (LU method) ",
		       "in interp_krige::set_covar_noise().",
		       o2scl::exc_esing);
	  }
	  return 1;
	}
	o2scl_linalg::LU_invert<ubmatrix,ubmatrix,ubmatrix_column>
	  (n_dim,KXX,p,inv_KXX);
	
	// Inverse covariance matrix times function vector
	this->Kinvf.resize(n_dim);
	boost::numeric::ublas::axpy_prod(inv_KXX,y,this->Kinvf,true);
	
      } else {
	
	// Construct the inverse of KXX
	int cret=o2scl_linalg::cholesky_decomp(n_dim,KXX,false);
	if (cret!=0) {
	  if (err_on_fail) {
	    O2SCL_ERR2("Matrix singular (Cholesky method) ",
		       "in interp_krige::set_covar_noise().",
		       o2scl::exc_esing);
	  }
	  return 2;
	}
	ubmatrix &inv_KXX=KXX;
	o2scl_linalg::cholesky_invert<ubmatrix>(n_dim,inv_KXX);
	
	// Inverse covariance matrix times function vector
	Kinvf.resize(n_dim);
	boost::numeric::ublas::axpy_prod(inv_KXX,y,Kinvf,true);
	
      }
      
      // Set parent data members
      this->px=&x;
      this->py=&y;
      this->sz=n_dim;
      
      return 0;
    }
    
    /// Initialize interpolation routine
    virtual int set_covar(size_t n_dim, const vec_t &x, const vec_t &y,
			  covar_func_t &fcovar, bool err_on_fail=true) {
      return set_covar_noise(n_dim,x,y,fcovar,0.0,err_on_fail);
    }

    /// Give the value of the function \f$ y(x=x_0) \f$ .
    virtual double eval(double x0) const {

      // Initialize to zero to prevent uninit'ed var. warnings
      double ret=0.0;
      for(size_t i=0;i<this->sz;i++) {
	ret+=(*f)(x0,(*this->px)[i])*Kinvf[i];
      }

      return ret;
    }
    
    /// Give the value of the derivative \f$ y^{\prime}(x=x_0) \f$ .
    virtual double deriv(double x0) const {
      return 0.0;
    }

    /** \brief Give the value of the second derivative  
	\f$ y^{\prime \prime}(x=x_0) \f$
    */
    virtual double deriv2(double x0) const {
      return 0.0;
    }

    /// Give the value of the integral \f$ \int_a^{b}y(x)~dx \f$ .
    virtual double integ(double a, double b) const {
      return 0.0;
    }

    /// Return the type, \c "interp_krige".
    virtual const char *type() const { return "interp_krige"; }

#ifndef DOXYGEN_INTERNAL

  private:

    interp_krige<vec_t,vec2_t,covar_func_t,covar_integ_t>
      (const interp_krige<vec_t,vec2_t,covar_func_t,covar_integ_t> &);
    interp_krige<vec_t,vec2_t,covar_func_t,covar_integ_t>& operator=
      (const interp_krige<vec_t,vec2_t,covar_func_t,covar_integ_t>&);

#endif

  };


  /** \brief One-dimensional interpolation using an 
      optimized covariance function

      See also the \ref intp_section section of the \o2 User's guide. 

      \note This class is experimental.
  */
  template<class vec_t, class vec2_t=vec_t>
    class interp_krige_optim : public interp_krige<vec_t,vec2_t> {

  public:

  typedef boost::numeric::ublas::vector<double> ubvector;
  typedef boost::numeric::ublas::matrix<double> ubmatrix;
  typedef boost::numeric::ublas::matrix_column<ubmatrix> ubmatrix_column;

  protected:

  /// Function object for the covariance
  std::function<double(double,double)> ff;
  
  /// The covariance function length scale
  double len;

  /// The quality factor of the optimization
  double qual;

  /// The covariance function
  double covar(double x1, double x2) {
    return exp(-(x1-x2)*(x1-x2)/len/len/2.0);
  }

  /// The derivative of the covariance function
  double deriv(double x1, double x2) {
    return -exp(-(x1-x2)*(x1-x2)/len/len/2.0)/len/len*(x1-x2);
  }

  /// The second derivative of the covariance function
  double deriv2(double x1, double x2) {
    return ((x1-x2)*(x1-x2)-len*len)*
    exp(-(x1-x2)*(x1-x2)/len/len/2.0)/len/len/len/len;
  }

  /// The integral of the covariance function
  double integ(double x, double x1, double x2) {
    exit(-1);
    // This is probably wrong
    //return 0.5*len*sqrt(o2scl_const::pi)*
    //(gsl_sf_erf((x2-x)/len)+gsl_sf_erf((x-x1)/len));
  }

  /// Pointer to the user-specified minimizer
  min_base<> *mp;
  
  /** \brief Function to optimize the covariance parameters
   */
  double qual_fun(double x, double noise_var, int &success) {

    len=x;
    success=0;

    size_t size=this->sz;

    if (mode==mode_loo_cv) {
      
      qual=0.0;
      for(size_t k=0;k<size;k++) {
	
	// Leave one observation out
	ubvector x2(size-1);
	o2scl::vector_copy_jackknife((*this->px),k,x2);
	ubvector y2(size-1);
	o2scl::vector_copy_jackknife((*this->py),k,y2);
	
	// Construct the KXX matrix
	ubmatrix KXX(size-1,size-1);
	for(size_t irow=0;irow<size-1;irow++) {
	  for(size_t icol=0;icol<size-1;icol++) {
	    if (irow>icol) {
	      KXX(irow,icol)=KXX(icol,irow);
	    } else {
	      KXX(irow,icol)=exp(-pow((x2[irow]-x2[icol])/len,2.0)/2.0);
	      if (irow==icol) KXX(irow,icol)+=noise_var;
	    }
	  }
	}
	
	if (this->matrix_mode==this->matrix_LU) {
	  
	  // Construct the inverse of KXX
	  ubmatrix inv_KXX(size-1,size-1);
	  o2scl::permutation p(size-1);
	  int signum;
	  o2scl_linalg::LU_decomp(size-1,KXX,p,signum);
	  if (o2scl_linalg::diagonal_has_zero(size-1,KXX)) {
	    success=1;
	    return 1.0e99;
	  }
	  o2scl_linalg::LU_invert<ubmatrix,ubmatrix,ubmatrix_column>
	    (size-1,KXX,p,inv_KXX);
	  
	  // Inverse covariance matrix times function vector
	  this->Kinvf.resize(size-1);
	  boost::numeric::ublas::axpy_prod(inv_KXX,y2,this->Kinvf,true);
	  
	} else {
	  
	  // Construct the inverse of KXX
	  int cret=o2scl_linalg::cholesky_decomp(size-1,KXX,false);
	  if (cret!=0) {
	    success=2;
	    return 1.0e99;
	  }
	  ubmatrix &inv_KXX=KXX;
	  o2scl_linalg::cholesky_invert<ubmatrix>(size-1,inv_KXX);
	  
	  // Inverse covariance matrix times function vector
	  this->Kinvf.resize(size-1);
	  boost::numeric::ublas::axpy_prod(inv_KXX,y2,this->Kinvf,true);
	  
	}
	
	double ypred=0.0;
	double yact=(*this->py)[k];
	for(size_t i=0;i<size-1;i++) {
	  ypred+=exp(-pow(((*this->px)[k]-x2[i])/len,2.0)/2.0)*this->Kinvf[i];
	}
	
	// Measure the quality with a chi-squared like function
	qual+=pow(yact-ypred,2.0);
	
      }
      
    } else if (mode==mode_max_lml) {

      // Construct the KXX matrix
      ubmatrix KXX(size,size);
      for(size_t irow=0;irow<size;irow++) {
	for(size_t icol=0;icol<size;icol++) {
	  if (irow>icol) {
	    KXX(irow,icol)=KXX(icol,irow);
	  } else {
	    KXX(irow,icol)=exp(-pow(((*this->px)[irow]-
				     (*this->px)[icol])/len,2.0)/2.0);
	    if (irow==icol) KXX(irow,icol)+=noise_var;
	  }
	}
      }

      // Note: We have to use LU here because O2scl doesn't yet
      // have a lndet() function for Cholesky decomp
      
      // Construct the inverse of KXX
      ubmatrix inv_KXX(size,size);
      o2scl::permutation p(size);
      int signum;
      o2scl_linalg::LU_decomp(size,KXX,p,signum);
      if (o2scl_linalg::diagonal_has_zero(size,KXX)) {
	success=1;
	return 1.0e99;
      }
      o2scl_linalg::LU_invert<ubmatrix,ubmatrix,ubmatrix_column>
	(size,KXX,p,inv_KXX);
      
      double lndet=o2scl_linalg::LU_lndet<ubmatrix>(size,KXX);
      
      // Inverse covariance matrix times function vector
      this->Kinvf.resize(size);
      boost::numeric::ublas::axpy_prod(inv_KXX,*this->py,this->Kinvf,true);

      // Compute the log of the marginal likelihood, without
      // the constant term
      for(size_t i=0;i<size;i++) {
	qual+=0.5*(*this->py)[i]*this->Kinvf[i];
      }
      qual+=0.5*lndet;
    }

    return qual;
  }
  
  public:

  interp_krige_optim() {
    nlen=20;
    full_min=false;
    mp=&def_min;
    verbose=0;
    mode=mode_loo_cv;
  }

  /// \name Function to minimize and various option
  //@{
  /// Leave-one-out cross validation
  static const size_t mode_loo_cv=1;
  /// Minus Log-marginal-likelihood
  static const size_t mode_max_lml=2;
  /// Function to minimize (default \ref mode_loo_cv)
  size_t mode;
  ///@}
    
  /// Verbosity parameter
  int verbose;
  
  /** \brief Number of length scale points to try when full minimizer 
      is not used (default 20)
  */
  size_t nlen;

  /// Default minimizer
  min_brent_gsl<> def_min;

  /// If true, use the full minimizer
  bool full_min;

  /// Initialize interpolation routine
  virtual int set_noise(size_t size, const vec_t &x, const vec2_t &y,
			double noise_var, bool err_on_fail=true) {

    // Set parent data members
    this->px=&x;
    this->py=&y;
    this->sz=size;

    int success=0;
      
    if (full_min) {

      if (verbose>1) {
	std::cout << "interp_krige_optim: full minimization"
		  << std::endl;
      }
      
      // Choose first interval as initial guess
      double len_opt=x[1]-x[0];

      funct mf=std::bind
      (std::mem_fn<double(double,double,int &)>
       (&interp_krige_optim<vec_t,vec2_t>::qual_fun),this,
       std::placeholders::_1,noise_var,std::ref(success));
      
      mp->min(len_opt,qual,mf);
      len=len_opt;

      if (success!=0) {
	if (err_on_fail) {
	  O2SCL_ERR2("Minimization failed in ",
		     "interp_krige_optim::set_noise().",
		     o2scl::exc_efailed);
	}
      }

    } else {

      if (verbose>1) {
	std::cout << "interp_krige_optim: simple minimization"
		  << std::endl;
      }

      // Compute a finite-difference array
      std::vector<double> diff(size-1);
      for(size_t i=0;i<size-1;i++) {
	diff[i]=fabs(x[i+1]-x[i]);
      }
      
      // Range of the length parameter
      double len_min=o2scl::vector_min_value
      <std::vector<double>,double>(size-1,diff)/3.0;
      double len_max=fabs(x[size-1]-x[0])*3.0;
      double len_ratio=len_max/len_min;

      if (verbose>1) {
	std::cout << "len (min,max,ratio): " << len_min << " "
		  << len_max << " "
		  << pow(len_ratio,((double)1)/((double)nlen-1))
		  << std::endl;
      }
	
      // Initialize to zero to prevent uninit'ed var. warnings
      double min_qual=0.0, len_opt=0.0;
      
      if (verbose>1) {
	std::cout << "ilen len qual fail min_qual" << std::endl;
      }

      // Loop over the full range, finding the optimum
      bool min_set=false;
      for(size_t j=0;j<nlen;j++) {
	len=len_min*pow(len_ratio,((double)j)/((double)nlen-1));

	int success=0;
	qual=qual_fun(len,noise_var,success);
	
	if (success==0 && (min_set==false || qual<min_qual)) {
	  len_opt=len;
	  min_qual=qual;
	  min_set=true;
	}
	
	if (verbose>1) {
	  std::cout << "interp_krige_optim: ";
	  std::cout.width(2);
	  std::cout << j << " " << len << " " << qual << " "
		    << success << " " << min_qual << " "
		    << len_opt << std::endl;
	}
	  
      }
      
      // Now that we've optimized the covariance function,
      // just use the parent class to interpolate
      len=len_opt;

    }

    ff=std::bind(std::mem_fn<double(double,double)>
	      (&interp_krige_optim<vec_t,vec2_t>::covar),this,
	      std::placeholders::_1,std::placeholders::_2);
    
    this->set_covar_noise(size,x,y,ff,noise_var);
      
    return 0;
  }

  /// Initialize interpolation routine
  virtual void set(size_t size, const vec_t &x, const vec2_t &y) {

    // Use the mean absolute value to determine noise
    double mean_abs=0.0;
    for(size_t j=0;j<size;j++) {
      mean_abs+=fabs(y[j]);
    }
    mean_abs/=size;

    set_noise(size,x,y,mean_abs/1.0e8,true);
    
    return;
  }
  
  
  };
  
#ifndef DOXYGEN_NO_O2NS
}
#endif

#endif
