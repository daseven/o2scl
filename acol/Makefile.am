help:
	@echo "This the O2scl makefile for the acol subdirectory."
	@echo
	@echo "The traditional GNU-style targets apply."
	@echo 

# ------------------------------------------------------------      
# Include statement
# ------------------------------------------------------------      

if O2SCL_PARTLIB
O2SCL_PART_MVAR = -DO2SCL_PART
else
O2SCL_PART_MVAR =
endif

if O2SCL_EOSLIB
O2SCL_EOS_MVAR = -DO2SCL_EOS
else
O2SCL_EOS_MVAR =
endif

if O2SCL_HDF
O2SCL_HDF_MVAR = -DO2SCL_HDF
else
O2SCL_HDF_MVAR =
endif

if O2SCL_OPENMP
O2SCL_OPENMP_MVAR = -DO2SCL_OPENMP -fopenmp
else
O2SCL_OPENMP_MVAR =
endif

if O2SCL_EIGEN
O2SCL_EIGEN_MVAR = -DO2SCL_EIGEN
else
O2SCL_EIGEN_MVAR =
endif

if O2SCL_ARMA
O2SCL_ARMA_MVAR = -DO2SCL_ARMA
else
O2SCL_ARMA_MVAR =
endif

if O2SCL_LINUX
O2SCL_LINUX_MVAR = -DO2SCL_LINUX
else 
O2SCL_LINUX_MVAR = 
endif

if O2SCL_OSX
O2SCL_OSX_MVAR = -DO2SCL_OSX
else 
O2SCL_OSX_MVAR = 
endif

if O2SCL_SNAP
AM_CPPFLAGS = -I@top_srcdir@/include/ \
	-DO2SCL_DATA_DIR=\"/snap/o2scl/current/share/o2scl/\" \
	$(O2SCL_PART_MVAR) $(O2SCL_EOS_MVAR) $(O2SCL_HDF_MVAR) \
	$(O2SCL_OPENMP_MVAR) $(O2SCL_EIGEN_MVAR) $(O2SCL_ARMA_MVAR) \
	$(O2SCL_GSL2_MVAR) -DO2SCL_COND_FLAG \
	$(O2SCL_OSX_MVAR) $(O2SCL_LINUX_MVAR)
else
AM_CPPFLAGS = -I@top_srcdir@/include/ -DO2SCL_DATA_DIR=\"${datadir}/o2scl/\" \
	$(O2SCL_PART_MVAR) $(O2SCL_EOS_MVAR) $(O2SCL_HDF_MVAR) \
	$(O2SCL_OPENMP_MVAR) $(O2SCL_EIGEN_MVAR) $(O2SCL_ARMA_MVAR) \
	$(O2SCL_GSL2_MVAR) -DO2SCL_COND_FLAG \
	$(O2SCL_OSX_MVAR) $(O2SCL_LINUX_MVAR)
endif

# ------------------------------------------------------------      
# Program acol 
# ------------------------------------------------------------      

if O2SCL_ENABLE_ACOL

if O2SCL_HDF

bin_PROGRAMS = acol 

acol_SOURCES = acol.cpp 

if O2SCL_READLINE
RLIB = -lreadline -lncurses
else 
RLIB = 
endif

# We look first for the libraries in the distribution directory.
# However, homebrew's superenv appears to move the libraries around,
# so we look for them in pkglibdir
if O2SCL_SNAP
acol_LDFLAGS = -L../src -L../src/hdf -L${pkglibdir} -Wl,-rpath \
	-Wl,/snap/o2scl/current/lib
else
acol_LDFLAGS = -L../src -L../src/hdf -L${pkglibdir}
endif

acol_LDADD = -lo2scl_hdf -lo2scl -lgsl -lgslcblas $(RLIB)

endif

endif

