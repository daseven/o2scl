/*
  -------------------------------------------------------------------
  
  Copyright (C) 2008-2019, Andrew W. Steiner
  
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
#ifndef O2SCL_CONVERT_UNITS_H
#define O2SCL_CONVERT_UNITS_H

/** \file convert_units.h
    \brief File defining \ref o2scl::convert_units
*/

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>

#include <o2scl/misc.h>
#include <o2scl/constants.h>
#include <o2scl/string_conv.h>

#ifndef DOXYGEN_NO_O2NS
namespace o2scl {
#endif

  /** \brief Convert units

      Allow the user to convert between two different units after
      specifying a conversion factor. This class will also
      automatically combine two conversion factors to create a new
      unit conversion (but it cannot combine more than two).

      Conversions are performed by the \ref convert() function. The
      run-time unit cache is initially filled with hard-coded
      conversions, and \ref convert() searches this cache is searched
      for the requested conversion first. If the conversion is not
      found and if \ref use_gnu_units is true, then \ref convert()
      tries to open a pipe to open a shell to get the conversion
      factor from <tt>'units'</tt>. If this is successful, then the
      conversion factor is returned and the conversion is added to the
      cache.

      If the GNU units command is not in the local path, the user may
      modify \ref units_cmd_string to specify the full pathname. One
      can also modify \ref units_cmd_string to specify a different
      <tt>units.dat</tt> file.

      Example:
      \code
      convert_units cu;
      cout << "A solar mass is " << cu.convert("Msun","g",1.0) 
           << " g. " << endl;
      \endcode

      An object of this type is created by \ref o2scl_settings
      (of type \ref lib_settings_class) for several unit 
      conversions used internally in \o2 .

      \note Combining two conversions allows for some surprising
      apparent contradictions from numerical precision errors. If
      there are two matching unit conversion pairs which give the same
      requested conversion factor, then one can arrange a situation
      where the same conversion factor is reported with slightly
      different values after adding a related conversion to the table.
      One way to fix this is to force the class not to combine two
      conversions by setting \ref combine_two_conv to false.
      Alternatively, one can ensure that no combination is necessary
      by manually adding the desired combination conversion to the
      cache after it is first computed.

      \note \o2 uses some unit aliases which are not used the the GNU
      or OSX units commands, like "Msun" for the solar mass and adds
      some units not present like "Rschwarz" for the Schwarzchild
      radius of a 1 solar mass black hole.
      
      \note Only the const versions, \ref convert_const and
      \ref convert_ret_const are guaranteed to be thread-safe,
      since they are not allowed to update the unit cache.

      \future Add G=1. 

      \future An in_cache() function to test
      to see if a conversion is currently in the cache. 
      
      \future Ideally, a real C++ API for the GNU units command
      would be better.
  */
  class convert_units {

#ifndef DOXYGEN_INTERNAL

  protected:

    /// The type for caching unit conversions
    typedef struct {
      /// The input unit
      std::string f;
      /// The output unit
      std::string t;
      /// The conversion factor
      double c;
    } unit_t;

    /// The cache where unit conversions are stored
    std::map<std::string,unit_t,std::greater<std::string> > mcache;
    
    /// The iterator type
    typedef std::map<std::string,unit_t,
      std::greater<std::string> >::iterator miter;
      
    /// The const iterator type
    typedef std::map<std::string,unit_t,
      std::greater<std::string> >::const_iterator mciter;

    /** \brief The internal conversion function which tries the
	cache first and, if that failed, tries GNU units.

	This function returns 0 if the conversion was successful. If
	the conversion fails and \ref err_on_fail is \c true, then the
	error handler is called. If the conversion fails and \ref
	err_on_fail is \c false, then the value \ref
	o2scl::exc_enotfound is returned.

	The public conversion functions in this class are
	basically just wrappers around this internal function.
    */
    int convert_internal(std::string from, std::string to,
			 double val, double &converted,
			 double &factor, bool &new_conv) const;

    /** \brief Attempt to use GNU units to perform a conversion

	This function attempts to open a pipe to GNU units independent
	of the value of \ref use_gnu_units. However, it will always
	return a non-zero valeu if \c HAVE_POPEN is not defined
	signaling that the <tt>popen()</tt> function is not available
	(but does not call the error handler in this case). The
	function returns 0 if the conversion was successful. If
	HAVE_POPEN is defined but the call to GNU units fails
	for some reason, then the error handler is called if
	\ref err_on_fail is true.
     */
    int convert_gnu_units(std::string from, std::string to,
			 double val, double &converted,
			 double &factor, bool &new_conv) const;

    /** \brief Attempt to construct a conversion from the internal
	unit cache

	This function returns 0 if the conversion was successful and
	\ref o2scl::exc_efailed otherwise. This function
	does not call the error handler.
     */
    int convert_cache(std::string from, std::string to,
			 double val, double &converted,
			 double &factor) const;

#endif

  public:

    /// Create a unit-conversion object
    convert_units();

    virtual ~convert_units() {}

    /// \name Basic usage
    //@{
    /** \brief Return the value \c val after converting using units \c
	from and \c to
    */
    virtual double convert(std::string from, std::string to, double val);

    /** \brief Return the value \c val after converting using units \c
	from and \c to (const version)
    */
    virtual double convert_const(std::string from, std::string to,
				 double val) const;
    //@}

    /// \name User settings
    //@{
    /// Verbosity (default 0)
    int verbose;

    /** \brief If true, use a system call to units to derive new
	conversions (default true)

	This also requires <tt>popen()</tt>.
     */
    bool use_gnu_units;

    /// If true, throw an exception when a conversion fails (default true)
    bool err_on_fail;

    /// If true, allow combinations of two conversions (default true)
    bool combine_two_conv;

    /// Command string to call units (default "units")
    std::string units_cmd_string;
    //@}

    /// \name Conversions which don't throw exceptions
    //@{
    /** \brief Return the value \c val after converting using units \c
	from and \c to, returning a non-zero value on failure
    */
    virtual int convert_ret(std::string from, std::string to, double val,
			    double &converted);

    /** \brief Return the value \c val after converting using units \c
	from and \c to, returning a non-zero value on failure
	(const version)
    */
    virtual int convert_ret_const(std::string from, std::string to,
				  double val, double &converted) const;
    //@}

    /// \name Manipulate cache and create units.dat files
    //@{
    /// Manually insert a unit conversion into the cache
    void insert_cache(std::string from, std::string to, double conv);

    /// Manually remove a unit conversion into the cache
    void remove_cache(std::string from, std::string to);
    
    /// Print the present unit cache to std::cout
    void print_cache() const;

    /** \brief Make a GNU \c units.dat file from the GSL constants

	If \c c_1 is true, then the second is defined in terms of
	meters so that the speed of light is unitless. If \c hbar_1 is
	true, then the kilogram is defined in terms of <tt>s/m^2</tt>
	so that \f$ \hbar \f$ is unitless.

	\note While convert() generally works with the OSX version
	of 'units', the OSX version can't read units.dat files 
	created by this function.

	\note Not all of the GSL constants or the canonical GNU units 
	conversions are given here.
    */
    void make_units_dat(std::string fname, bool c_1=false, 
			bool hbar_1=false, bool K_1=false) const;

    /** \brief Exhaustive test the cache against GNU units
     */
    int test_cache();
    //@}
    
  };

#ifndef DOXYGEN_NO_O2NS
}
#endif

#endif
