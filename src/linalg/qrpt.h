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
/** \file qrpt.h
    \brief Header wrapper for \ref qrpt_base.h
*/
#ifndef O2SCL_QRPT_H
#define O2SCL_QRPT_H

#include <o2scl/err_hnd.h>
#include <o2scl/permutation.h>
#include <o2scl/cblas.h>
#include <o2scl/householder.h>
#include <o2scl/givens.h>
#include <o2scl/vector.h>

namespace o2scl_linalg_bracket {
  
#define O2SCL_IX(V,i) V[i]
#define O2SCL_IX2(M,i,j) M[i][j]
#define O2SCL_CBLAS_NAMESPACE o2scl_cblas_bracket
#include <o2scl/qrpt_base.h>  
#undef O2SCL_CBLAS_NAMESPACE
#undef O2SCL_IX
#undef O2SCL_IX2

}

namespace o2scl_linalg {
  
#define O2SCL_IX(V,i) V[i]
#define O2SCL_IX2(M,i,j) M(i,j)
#define O2SCL_CBLAS_NAMESPACE o2scl_cblas
#include <o2scl/qrpt_base.h>  
#undef O2SCL_CBLAS_NAMESPACE
#undef O2SCL_IX
#undef O2SCL_IX2
  
}

#endif
