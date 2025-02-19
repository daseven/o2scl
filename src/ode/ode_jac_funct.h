/*
  -------------------------------------------------------------------
  
  Copyright (C) 2006-2019, Andrew W. Steiner
  
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
#ifndef O2SCL_ODE_JAC_FUNCT_H
#define O2SCL_ODE_JAC_FUNCT_H

/** \file ode_jac_funct.h
    \brief File defining ODE Jacobian function objects
*/

#include <string>

#ifndef DOXYGEN_NO_O2NS
namespace o2scl {
#endif

  /// Functor for ODE Jacobians in src/ode/ode_jac_funct.h
  typedef std::function<
    int(double,size_t,const boost::numeric::ublas::vector<double> &,
	boost::numeric::ublas::matrix<double> &,
	boost::numeric::ublas::vector<double> &) > ode_jac_funct;

#ifndef DOXYGEN_NO_O2NS
}
#endif

#endif
