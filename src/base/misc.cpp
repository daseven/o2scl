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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// For gsl_finite()
#include <gsl/gsl_sys.h>

// For glob()
#include <glob.h>

// For wordexp()
#include <wordexp.h>

#include <o2scl/misc.h>

using namespace std;
using namespace o2scl;

int o2scl_python_test(int x) {
  return x*x;
}

int o2scl::pipe_cmd_string(std::string cmd, std::string &result,
			   bool err_on_fail, int nmax) {
  
#ifdef HAVE_POPEN
  
  FILE *ps_pipe=popen(cmd.c_str(),"r");
  if (!ps_pipe) {
    if (err_on_fail) {
      O2SCL_ERR("Pipe could not be opened in o2scl::pipe_cmd_string().",
		o2scl::exc_efailed);
    }
    return 1;
  }
  
  char char_arr[nmax];
  
  // Variable 'cret' is unused, but put here to avoid
  // unused return value errors
  char *cret=fgets(char_arr,nmax,ps_pipe);
  if (cret==0) {
    if (err_on_fail) {
      O2SCL_ERR("Null pointer returned by fgets in o2scl::pipe_cmd_string().",
		o2scl::exc_efailed);
    }
    return 2;
  }
  
  result=char_arr;
  
  int pret=pclose(ps_pipe);
  if (pret!=0) {
    if (err_on_fail) {
      O2SCL_ERR("Close pipe returned non-zero in o2scl::pipe_cmd_string().",
		o2scl::exc_efailed);
    }
    return 4;
  }
  
#else
  
  if (err_on_fail) {
    O2SCL_ERR("Compiled without popen support in o2scl::pipe_cmd_string().",
	      o2scl::exc_efailed);
  }
  return 3;
  
#endif
  
  return 0;
}

std::string o2scl::pipe_cmd_string(std::string cmd, int nmax) {
  std::string result;
  pipe_cmd_string(cmd,result,true,nmax);
  return result;
}

std::string o2scl::binary_to_hex(std::string s) {
  std::string t="";
  char nums[16]={'0','1','2','3','4','5','6','7',
		 '8','9','A','B','C','D','E','F'};
  for(size_t i=0;i<s.length();i++) {
    bool found=false;
    if (s[i]=='1' || s[i]=='0') {
      if (i+1<s.length() && (s[i+1]=='1' || s[i+1]=='0')) {
	if (i+2<s.length() && (s[i+2]=='1' || s[i+2]=='0')) {
	  if (i+3<s.length() && (s[i+3]=='1' || s[i+3]=='0')) {
	    found=true;
	    int cnt=0;
	    if (s[i]=='1') cnt+=8;
	    if (s[i+1]=='1') cnt+=4;
	    if (s[i+2]=='1') cnt+=2;
	    if (s[i+3]=='1') cnt++;
	    t+=nums[cnt];
	    i+=3;
	  }
	}
      }
    }
    if (found==false) {
      t+=s[i];
    }
  }
  return t;
}

double o2scl::fermi_function(double E, double mu, double T, double limit) {
  double ret, x=(E-mu)/T;
  
  if (x>limit) {
    ret=0.0;
  } else if (x<-limit) {
    ret=1.0;
  } else {
    ret=1.0/(1.0+exp(x));
  }
  return ret;
}

double o2scl::bose_function(double E, double mu, double T, double limit) {
  double ret, x=(E-mu)/T;
  
  if (x>limit) {
    ret=0.0;
  } else if (x<-limit) {
    ret=-1.0;
  } else if (fabs(x)<1.0e-3) {
    double x2=x*x;
    double x3=x2*x;
    double x5=x3*x2;
    double x7=x5*x2;
    ret=1.0/x-0.5+x/12.0-x3/720.0+x5/30240.0-x7/1209600.0;
  } else {
    ret=1.0/(exp(x)-1.0);
  }
  return ret;
}

size_t o2scl::count_words(string str) {
  string st;
  istringstream *is=new istringstream(str.c_str());
  size_t ctr=0;
  while((*is) >> st) ctr++;
  delete is;

  return ctr;
}

void o2scl::remove_whitespace(std::string &s) {
  // The index 'i' must be an integer so we can do 'i--' below
  for(int i=0;i<((int)s.length());i++) {
    if (s[i]==9 || s[i]==10 || s[i]==11 || s[i]==12 || s[i]==13 
	|| s[i]==32) {
      s=s.substr(0,i)+s.substr(i+1,s.length()-i-1);
      i--;
    }
  }
  return;
}

void o2scl::RGBtoHSV(double r, double g, double b, 
		     double &h, double &s, double &v) {
  
  double min, max, delta;

  if (r<g && r<b) min=r;
  else if (g<b) min=g;
  else min=b;
  if (r>g && r>b) max=r;
  else if (g>b) max=g;
  else max=b;
  v=max;

  delta=max-min;

  if (max!=0) {
    s=delta/max;
  } else {
    // r=g=b=0		
    // s=0,v is undefined
    s=0;
    h=-1;
    return;
  }

  if (r==max) {
    // between yellow & magenta
    h=(g-b)/delta;		
  } else if (g==max) {
    // between cyan & yellow
    h=2+(b-r)/delta;	
  } else {
    // between magenta & cyan
    h=4+(r-g)/delta;	
  }
  
  // degrees
  h*=60;				
  if(h<0) {
    h+=360.0;
  }

  return;
}

/** \brief Convert HSV color to RGB
*/
void o2scl::HSVtoRGB(double h, double s, double v, 
		     double &r, double &g, double &b) {

  int i;
  double f,p,q,t;
  
  if (s==0.0) {
    // achromatic (grey)
    r=g=b=v;
    return;
  }

  if (h==360.0) {
    h=0.0;
  }

  // sector 0 to 5
  h/=60.0;			
  i=((int)floor(h));
  // fractional part of h
  f=h-i;			
  p=v*(1-s);
  q=v*(1-s*f);
  t=v*(1-s*(1-f));

  switch(i) {
  case 0:
    r=v;
    g=t;
    b=p;
    break;
  case 1:
    r=q;
    g=v;
    b=p;
    break;
  case 2:
    r=p;
    g=v;
    b=t;
    break;
  case 3:
    r=p;
    g=q;
    b=v;
    break;
  case 4:
    r=t;
    g=p;
    b=v;
    break;
  default:
    r=v;
    g=p;
    b=q;
    break;
  }

  return;
}

int o2scl::glob_wrapper(std::string pattern,
			std::vector<std::string> &matches) {
  glob_t pglob;
  pglob.gl_offs=0;
  pglob.gl_pathc=0;
  matches.clear();
  int ret=glob(pattern.c_str(),GLOB_MARK | GLOB_TILDE,NULL,&pglob);
  if (ret==0) {
    for(size_t i=0;i<pglob.gl_pathc;i++) {
      matches.push_back(pglob.gl_pathv[i]);
    }
  }
  globfree(&pglob);
  return ret;
}

int o2scl::wordexp_wrapper(std::string word,
			   std::vector<std::string> &matches) {
  wordexp_t pwordexp;
  char **w;
  matches.clear();
  int ret=wordexp(word.c_str(),&pwordexp,0);
  if (ret==0) {
    w=pwordexp.we_wordv;
    for(size_t i=0;i<pwordexp.we_wordc;i++) {
      matches.push_back(w[i]);
    }
  }
  wordfree(&pwordexp);
  return ret;
}

void o2scl::wordexp_single_file(std::string &fname) {
  std::vector<std::string> matches;
  int wret=wordexp_wrapper(fname,matches);
  if (wret!=0) {
    O2SCL_ERR2("Function wordexp_wrapper() failed in ",
	       "wordexp_single_file().",o2scl::exc_einval);
  }
  if (matches.size()>1) {
    O2SCL_ERR2("More than one match found for ",
	       "wordexp_single_file().",o2scl::exc_einval);
  }
  if (matches.size()==0) {
    O2SCL_ERR2("Zero matches in ",
	       "wordexp_single_file().",o2scl::exc_einval);
  }
  fname=matches[0];
  return;
}

