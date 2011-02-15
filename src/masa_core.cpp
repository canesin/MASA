// -*-c++-*-
//
//-----------------------------------------------------------------------bl-
//--------------------------------------------------------------------------
//
// MASA - Manufactured Analytical Solutions Abstraction Library
//
// Copyright (C) 2010 The PECOS Development Team
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the Version 2.1 GNU Lesser General
// Public License as published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc. 51 Franklin Street, Fifth Floor,
// Boston, MA  02110-1301  USA
//
//-----------------------------------------------------------------------el-
// $Author$
// $Id$
//
// masa_core.cpp: this is the core set of routines -- the only functions that 
//                should be called by users -- all other files are internal
//
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

#include <config.h>        // for MASA_EXCEPTIONS conditional
#include <masa_internal.h>
#include <map>

using namespace MASA;
using namespace std;

// Anonymous namespace for local helper class/functions
namespace {

using namespace MASA;

template <typename Scalar>
class MasterMS
{
public:
  MasterMS () : _master_pointer(NULL), _master_map() {}

  ~MasterMS () {
    for(typename map<std::string,manufactured_solution<Scalar>*>::iterator iter = this->_master_map.begin(); iter != this->_master_map.end(); iter++)
      delete iter->second;
  }

  const manufactured_solution<Scalar>& get_ms() const {
    verify_pointer_sanity();
    return *_master_pointer;
  }

  manufactured_solution<Scalar>& get_ms() {
    verify_pointer_sanity();
    return *_master_pointer;
  }

  void select_mms(const std::string& my_name);

  void init_mms(const std::string& my_name, const std::string& masa_name);

  void list_mms() const;

  unsigned int size() const { return _master_map.size(); }

private:
  //
  //  this function checks the user has an active mms
  //
  void verify_pointer_sanity() const;

  manufactured_solution<Scalar>*             _master_pointer; // pointer to currently selected manufactured solution
  std::map<std::string, manufactured_solution<Scalar> *> _master_map; // global map b/t unique name and manuf class
};


template <typename Scalar>
int get_list_mms(std::vector<manufactured_solution<Scalar>*>& anim)
{
  // Build a temporary vector of MMS objects, then sort them into our map by name
  anim.push_back(new masa_test_function<Scalar>());   // test function
  anim.push_back(new masa_uninit<Scalar>()); // another test function
  
  // register solutions here
  anim.push_back(new heateq_1d_steady_const<Scalar>());
  anim.push_back(new heateq_2d_steady_const<Scalar>());
  anim.push_back(new heateq_3d_steady_const<Scalar>());

  anim.push_back(new heateq_1d_unsteady_const<Scalar>());
  anim.push_back(new heateq_2d_unsteady_const<Scalar>());
  anim.push_back(new heateq_3d_unsteady_const<Scalar>());

  anim.push_back(new heateq_1d_unsteady_var<Scalar>());
  anim.push_back(new heateq_2d_unsteady_var<Scalar>());
  anim.push_back(new heateq_3d_unsteady_var<Scalar>());

  anim.push_back(new heateq_1d_steady_var<Scalar>());
  anim.push_back(new heateq_2d_steady_var<Scalar>());
  anim.push_back(new heateq_3d_steady_var<Scalar>());

  anim.push_back(new euler_1d<Scalar>());
  anim.push_back(new euler_2d<Scalar>());
  anim.push_back(new euler_3d<Scalar>());

  anim.push_back(new euler_chem_1d<Scalar>());

  anim.push_back(new sod_1d<Scalar>());

  anim.push_back(new navierstokes_2d_compressible<Scalar>());
  anim.push_back(new navierstokes_3d_compressible<Scalar>());

  anim.push_back(new axi_euler<Scalar>());
  anim.push_back(new axi_cns<Scalar>());

  anim.push_back(new rans_sa<Scalar>());

  return 0;
}

// Instantiations for every precision

MasterMS<double>      masa_master_double;
MasterMS<long double> masa_master_longdouble;

// Function to return a MasterMS by precision
template <typename Scalar>
MasterMS<Scalar>&      masa_master() { return masa_master_double; }
template <>
MasterMS<long double>& masa_master() { return masa_master_longdouble; }

}

template <typename Scalar>
void MasterMS<Scalar>::verify_pointer_sanity() const
{
  if(_master_pointer == 0)
    {    
      std::cout << "MASA FATAL ERROR:: No initialized Manufactured Solution!" << std::endl;
      std::cout << "Have you called masa_init?" << std::endl;
      masa_exit(1);
    }  
}


//
//  limited masa exception handling
//
void MASA::masa_exit(int ex)
{

#ifdef MASA_EXCEPTIONS
  std::cout << "MASA:: caught exception " << ex << std::endl;
  throw(ex);
#else
  std::cout << "MASA:: ABORTING\n";
  exit(ex);
#endif
  
}

template <typename Scalar>
int MASA::masa_purge_default_param()
{
  return masa_master<Scalar>().get_ms().purge_var();
}

template <typename Scalar>
int MASA::pass_func(Scalar (*in_func)(Scalar),Scalar a)
{
  return masa_master<Scalar>().get_ms().pass_function(in_func,a);
}

//
//  this function selects an already initialized manufactured class
//
template <typename Scalar>
void MasterMS<Scalar>::select_mms(const std::string& my_name)
{
  // check that the class does exist
  typename std::map<std::string, manufactured_solution<Scalar> *>::iterator it=_master_map.find(my_name);
  if(it != _master_map.end()) // found a name
    { 
      std::cout << "MASA :: selected " << my_name << std::endl;
      _master_pointer=it->second; // set pointer to currently selected solution      
    }      
  else 
    {
      std::cout << "\nMASA FATAL ERROR:: No such manufactured solution (" << my_name << ") has been initialized.\n";
      this->list_mms();
      masa_exit(1);
    } 
}


template <typename Scalar>
int MASA::masa_select_mms(std::string name)
{
  masa_master<Scalar>().select_mms(name);
  return 0;
}

//
//  this function will initiate a masa manufactured class
//
template <typename Scalar>
int MASA::masa_init(std::string unique_name, std::string str)
{
  masa_master<Scalar>().init_mms(unique_name, str);
  
  return 0; // steady as she goes
}


template <typename Scalar>
void MasterMS<Scalar>::init_mms(const std::string& my_name,
                                const std::string& masa_name)
{
  std::vector<manufactured_solution<Scalar>*> anim;
  get_list_mms<Scalar>(anim); //construct maps of MMS objects

  std::string mapped_name = masa_name;
  MASA::masa_map(&mapped_name);

  for (unsigned int i=0; i != anim.size(); ++i)
    {
      std::string name;
      anim[i]->return_name(&name);
      if (name.empty())
        {
          std::cout << "MASA FATAL ERROR:: manufactured solution has no name!\n";
          masa_exit(1);
        }
      if (name == mapped_name)
        {
          _master_map[my_name] = _master_pointer = anim[i];
          return;
        }
      else
        delete anim[i];
    }

  std::cout << "MASA FATAL ERROR:: no manufactured solution named " << masa_name << " found!\n";
  masa_exit(1);
}


template <typename Scalar>
void MasterMS<Scalar>::list_mms() const
{
  std::string str;

  // output the size of the map
  std::cout << "Number of initialized solutions: " << this->size() << std::endl;
  for(typename map<std::string,manufactured_solution<Scalar>*>::const_iterator iter = this->_master_map.begin(); iter != this->_master_map.end(); iter++)
    {
      (iter->second)->return_name(&str);
      std::cout << iter->first << " : " << str << std::endl;
    }
}



template <typename Scalar>
int MASA::masa_list_mms()
{
  masa_master<Scalar>().list_mms();
  return 0;
}

//
// function that prints all registered masa solutions
//
template <typename Scalar>
int MASA::masa_printid()
{
  std::vector<manufactured_solution<Scalar>*> anim;

  get_list_mms(anim); //construct list 

  std::cout << std::endl;
  std::cout << "\nMASA :: Available Solutions:\n";
  std::cout << "*-------------------------------------*" ;

  for (typename std::vector<manufactured_solution<Scalar>*>::const_iterator it = anim.begin(); it != anim.end(); ++it) 
    {
      std::string name;
      (*it)->return_name(&name); // get name
      std::cout << std::endl << name;
      delete *it;
    } // done with for loop 

  std::cout << "\n*-------------------------------------*\n" ;

  return 0; // steady as she goes
}// done with masa print id

template <typename Scalar>
void MASA::masa_set_param(std::string param,Scalar paramval)
{
  masa_master<Scalar>().get_ms().set_var(param,paramval);
}

//
// Set all parameters to default values
//
template <typename Scalar>
int MASA::masa_init_param()
{
  return masa_master<Scalar>().get_ms().init_var();
}

//
// Function that returns value of parameter selected by string
// 

template <typename Scalar>
Scalar MASA::masa_get_param(std::string param)
{
  return masa_master<Scalar>().get_ms().get_var(param);
}


template <typename Scalar>
int MASA::masa_display_param()
{
  masa_master<Scalar>().get_ms().display_var();

  return 0;
}

/* ------------------------------------------------
 *
 *         1D functions
 *
 * -----------------------------------------------
 */ 


  // --------------------------------
  // source terms
  // --------------------------------

template <typename Scalar>
Scalar MASA::masa_eval_source_t(Scalar x) //x 
{
  return masa_master<Scalar>().get_ms().eval_q_t(x);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_t(Scalar x,Scalar t) //x,t
{
  return masa_master<Scalar>().get_ms().eval_q_t(x,t);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_u(Scalar x)
{
  return masa_master<Scalar>().get_ms().eval_q_u(x);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_v(Scalar x)  // for SA model
{
  return masa_master<Scalar>().get_ms().eval_q_v(x);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_w(Scalar x)
{
  return masa_master<Scalar>().get_ms().eval_q_w(x);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_rho(Scalar x)
{
  return masa_master<Scalar>().get_ms().eval_q_rho(x);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_rho_u(Scalar x)
{
  return masa_master<Scalar>().get_ms().eval_q_rho_u(x);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_rho_e(Scalar x)
{
  return masa_master<Scalar>().get_ms().eval_q_rho_e(x);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_rho_N(Scalar x)
{
  return masa_master<Scalar>().get_ms().eval_q_rho_N(x);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_rho_N2(Scalar x)
{
  return masa_master<Scalar>().get_ms().eval_q_rho_N2(x);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_e(Scalar x)
{
  return masa_master<Scalar>().get_ms().eval_q_e(x);
}

  // --------------------------------
  // analytical terms
  // --------------------------------

template <typename Scalar>
Scalar MASA::masa_eval_exact_t(Scalar x)
{
  return masa_master<Scalar>().get_ms().eval_exact_t(x);
}

template <typename Scalar>
Scalar MASA::masa_eval_exact_u(Scalar x)
{
  return masa_master<Scalar>().get_ms().eval_exact_u(x);
}

template <typename Scalar>
Scalar MASA::masa_eval_exact_v(Scalar x) // for SA model
{
  return masa_master<Scalar>().get_ms().eval_exact_v(x);
}

template <typename Scalar>
Scalar MASA::masa_eval_exact_w(Scalar x)
{
  return masa_master<Scalar>().get_ms().eval_exact_w(x);
}

template <typename Scalar>
Scalar MASA::masa_eval_exact_p(Scalar x)
{
  return masa_master<Scalar>().get_ms().eval_exact_p(x);
}

template <typename Scalar>
Scalar MASA::masa_eval_exact_rho(Scalar x)
{
  return masa_master<Scalar>().get_ms().eval_exact_rho(x);
}

template <typename Scalar>
Scalar MASA::masa_eval_exact_rho_N(Scalar x)
{
  return masa_master<Scalar>().get_ms().eval_exact_rho_N(x);
}

template <typename Scalar>
Scalar MASA::masa_eval_exact_rho_N2(Scalar x)
{
  return masa_master<Scalar>().get_ms().eval_exact_rho_N2(x);
}

template <typename Scalar>
Scalar MASA::masa_eval_grad_1d_u(Scalar x)
{
  return masa_master<Scalar>().get_ms().eval_1d_g_u(x);
}

template <typename Scalar>
Scalar MASA::masa_eval_grad_1d_v(Scalar x)
{
  return masa_master<Scalar>().get_ms().eval_1d_g_v(x);
}

template <typename Scalar>
Scalar MASA::masa_eval_grad_1d_w(Scalar x)
{
  return masa_master<Scalar>().get_ms().eval_1d_g_w(x);
}

template <typename Scalar>
Scalar MASA::masa_eval_grad_1d_p(Scalar x)
{
  return masa_master<Scalar>().get_ms().eval_1d_g_p(x);
}

template <typename Scalar>
Scalar MASA::masa_eval_grad_1d_rho(Scalar x)
{
  return masa_master<Scalar>().get_ms().eval_1d_g_rho(x);
}


/* ------------------------------------------------
 *
 *         2D functions
 *
 * -----------------------------------------------
 */ 

  // --------------------------------
  // source terms
  // --------------------------------

template <typename Scalar>
Scalar MASA::masa_eval_source_t(Scalar x,Scalar y,Scalar t)
{
  return masa_master<Scalar>().get_ms().eval_q_t(x,y,t);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_u(Scalar x,Scalar y)
{
  return masa_master<Scalar>().get_ms().eval_q_u(x,y);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_v(Scalar x,Scalar y)
{  
  return masa_master<Scalar>().get_ms().eval_q_v(x,y);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_w(Scalar x,Scalar y)
{
  return masa_master<Scalar>().get_ms().eval_q_w(x,y);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_rho(Scalar x,Scalar y)
{
  return masa_master<Scalar>().get_ms().eval_q_rho(x,y);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_e(Scalar x,Scalar y)
{
  return masa_master<Scalar>().get_ms().eval_q_e(x,y);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_rho_u(Scalar x,Scalar y)
{
  return masa_master<Scalar>().get_ms().eval_q_rho_u(x,y);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_rho_v(Scalar x,Scalar y)
{
  return masa_master<Scalar>().get_ms().eval_q_rho_v(x,y);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_rho_w(Scalar x,Scalar y)
{
  return masa_master<Scalar>().get_ms().eval_q_rho_w(x,y);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_rho_e(Scalar x,Scalar y)
{
  return masa_master<Scalar>().get_ms().eval_q_rho_e(x,y);
}

  // --------------------------------
  // analytical terms
  // --------------------------------

template <typename Scalar>
Scalar MASA::masa_eval_exact_t(Scalar x,Scalar y)
{
  return masa_master<Scalar>().get_ms().eval_exact_t(x,y);
}

template <typename Scalar>
Scalar MASA::masa_eval_exact_u(Scalar x,Scalar y)
{
  return masa_master<Scalar>().get_ms().eval_exact_u(x,y);
}

template <typename Scalar>
Scalar MASA::masa_eval_exact_v(Scalar x,Scalar y)
{
  return masa_master<Scalar>().get_ms().eval_exact_v(x,y);
}

template <typename Scalar>
Scalar MASA::masa_eval_exact_w(Scalar x,Scalar y)
{
  return masa_master<Scalar>().get_ms().eval_exact_w(x,y);
}

template <typename Scalar>
Scalar MASA::masa_eval_exact_p(Scalar x,Scalar y)
{
  return masa_master<Scalar>().get_ms().eval_exact_p(x,y);
}

template <typename Scalar>
Scalar MASA::masa_eval_exact_rho(Scalar x,Scalar y)
{
  return masa_master<Scalar>().get_ms().eval_exact_rho(x,y);
}

template <typename Scalar>
Scalar MASA::masa_eval_grad_2d_u(Scalar x,Scalar y,int i)
{
  return masa_master<Scalar>().get_ms().eval_2d_g_u(x,y,i);
}

template <typename Scalar>
Scalar MASA::masa_eval_grad_2d_v(Scalar x,Scalar y,int i)
{
  return masa_master<Scalar>().get_ms().eval_2d_g_v(x,y,i);
}

template <typename Scalar>
Scalar MASA::masa_eval_grad_2d_w(Scalar x,Scalar y,int i)
{
  return masa_master<Scalar>().get_ms().eval_2d_g_w(x,y,i);
}

template <typename Scalar>
Scalar MASA::masa_eval_grad_2d_p(Scalar x,Scalar y,int i)
{
  return masa_master<Scalar>().get_ms().eval_2d_g_p(x,y,i);
}

template <typename Scalar>
Scalar MASA::masa_eval_grad_2d_rho(Scalar x,Scalar y,int i)
{
  return masa_master<Scalar>().get_ms().eval_2d_g_rho(x,y,i);
}

/* ------------------------------------------------
 *
 *         3D functions
 *
 * -----------------------------------------------
 */ 

  // --------------------------------
  // source terms
  // --------------------------------

template <typename Scalar>
Scalar MASA::masa_eval_source_t(Scalar x,Scalar y,Scalar z,Scalar t)
{
  return masa_master<Scalar>().get_ms().eval_q_t(x,y,z,t);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_u(Scalar x,Scalar y,Scalar z)
{
  return masa_master<Scalar>().get_ms().eval_q_u(x,y,z);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_v(Scalar x,Scalar y,Scalar z)
{
  return masa_master<Scalar>().get_ms().eval_q_v(x,y,z);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_w(Scalar x,Scalar y,Scalar z)
{
  return masa_master<Scalar>().get_ms().eval_q_w(x,y,z);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_rho(Scalar x,Scalar y, Scalar z)
{
  return masa_master<Scalar>().get_ms().eval_q_rho(x,y,z);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_e(Scalar x,Scalar y,Scalar z)
{
  return masa_master<Scalar>().get_ms().eval_q_e(x,y,z);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_rho_u(Scalar x,Scalar y,Scalar z)
{
  return masa_master<Scalar>().get_ms().eval_q_rho_u(x,y,z);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_rho_v(Scalar x,Scalar y,Scalar z)
{
  return masa_master<Scalar>().get_ms().eval_q_rho_v(x,y,z);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_rho_w(Scalar x,Scalar y,Scalar z)
{
  return masa_master<Scalar>().get_ms().eval_q_rho_w(x,y,z);
}

template <typename Scalar>
Scalar MASA::masa_eval_source_rho_e(Scalar x,Scalar y,Scalar z)
{
  return masa_master<Scalar>().get_ms().eval_q_rho_e(x,y,z);
}

  // --------------------------------
  // analytical terms
  // --------------------------------

template <typename Scalar>
Scalar MASA::masa_eval_exact_t(Scalar x,Scalar y,Scalar z)
{
  return masa_master<Scalar>().get_ms().eval_exact_t(x,y,z);
}

template <typename Scalar>
Scalar MASA::masa_eval_exact_t(Scalar x,Scalar y,Scalar z,Scalar t)
{
  return masa_master<Scalar>().get_ms().eval_exact_t(x,y,z,t);
}

template <typename Scalar>
Scalar MASA::masa_eval_exact_u(Scalar x,Scalar y,Scalar z)
{
  return masa_master<Scalar>().get_ms().eval_exact_u(x,y,z);
}

template <typename Scalar>
Scalar MASA::masa_eval_exact_v(Scalar x,Scalar y,Scalar z)
{
  return masa_master<Scalar>().get_ms().eval_exact_v(x,y,z);
}

template <typename Scalar>
Scalar MASA::masa_eval_exact_w(Scalar x,Scalar y,Scalar z)
{
  return masa_master<Scalar>().get_ms().eval_exact_w(x,y,z);
}

template <typename Scalar>
Scalar MASA::masa_eval_exact_p(Scalar x,Scalar y,Scalar z)
{
  return masa_master<Scalar>().get_ms().eval_exact_p(x,y,z);
}

template <typename Scalar>
Scalar MASA::masa_eval_exact_rho(Scalar x,Scalar y,Scalar z)
{
  return masa_master<Scalar>().get_ms().eval_exact_rho(x,y,z);
}

template <typename Scalar>
Scalar MASA::masa_eval_grad_3d_u(Scalar x,Scalar y,Scalar z,int i)
{
  return masa_master<Scalar>().get_ms().eval_3d_g_u(x,y,z,i);
}

template <typename Scalar>
Scalar MASA::masa_eval_grad_3d_v(Scalar x,Scalar y,Scalar z,int i)
{
  return masa_master<Scalar>().get_ms().eval_3d_g_v(x,y,z,i);
}

template <typename Scalar>
Scalar MASA::masa_eval_grad_3d_w(Scalar x,Scalar y,Scalar z,int i)
{
  return masa_master<Scalar>().get_ms().eval_3d_g_w(x,y,z,i);
}

template <typename Scalar>
Scalar MASA::masa_eval_grad_3d_p(Scalar x,Scalar y,Scalar z,int i)
{
  return masa_master<Scalar>().get_ms().eval_3d_g_p(x,y,z,i);
}

template <typename Scalar>
Scalar MASA::masa_eval_grad_3d_rho(Scalar x,Scalar y,Scalar z,int i)
{
  return masa_master<Scalar>().get_ms().eval_3d_g_rho(x,y,z,i);
}

/* ------------------------------------------------
 *
 *         utility functions
 *
 * -----------------------------------------------
 */ 


template <typename Scalar>
int MASA::masa_get_name(std::string* name)
{
  masa_master<Scalar>().get_ms().return_name(name); // set string to name
  return 0;
}


template <typename Scalar>
int MASA::masa_get_dimension(int* dim)
{
  masa_master<Scalar>().get_ms().return_dim(dim); // set string to name
  return 0;
}


template <typename Scalar>
int MASA::masa_test_poly()
{
  return masa_master<Scalar>().get_ms().poly_test(); // return error condition
}


template <typename Scalar>
int MASA::masa_sanity_check()
{
  return masa_master<Scalar>().get_ms().sanity_check(); // set string to name
}

int MASA::masa_version_stdout()
{
  std::cout << "--------------------------------------------------------" << std::endl;
  std::cout << "MASA Library: Version = " << MASA_LIB_VERSION;
  std::cout << " (" << MASA::masa_get_numeric_version() << ")" << std::endl << std::endl;

  std::cout << MASA_LIB_RELEASE << std::endl << std::endl;

  std::cout << "Build Date   = " << MASA_BUILD_DATE     << std::endl;
  std::cout << "Build Host   = " << MASA_BUILD_HOST     << std::endl;
  std::cout << "Build User   = " << MASA_BUILD_USER     << std::endl;
  std::cout << "Build Arch   = " << MASA_BUILD_ARCH     << std::endl;
  std::cout << "Build Rev    = " << MASA_BUILD_VERSION  << std::endl << std::endl;

  std::cout << "C++ Config   = " << MASA_CXX << " "     << MASA_CXXFLAGS << std::endl;
  std::cout << "F90 Config   = " << MASA_FC MASA_FCFLAGS << std::endl;
  std::cout << "--------------------------------------------------------" << std::endl;
  return 0;
}

int MASA::masa_get_numeric_version()
{
  // Note: return format follows the versioning convention xx.yy.zz where
  //
  // xx = major version number
  // yy = minor version number
  // zz = micro version number
  //
  // For example:
  // v.   0.23  -> 002300 = 2300
  // v   0.23.1 -> 002301 = 2301
  // v. 10.23.2 -> 102302

  int major_version = 0;
  int minor_version = 0;
  int micro_version = 0;

  #ifdef MASA_MAJOR_VERSION
  major_version = MASA_MAJOR_VERSION;
  #endif

  #ifdef MASA_MINOR_VERSION
  minor_version = MASA_MINOR_VERSION;
  #endif

  #ifdef MASA_MICRO_VERSION
  micro_version = MASA_MICRO_VERSION;
  #endif

  return(major_version*10000 + minor_version*100 + micro_version);

}


// Instantiations

#define INSTANTIATE_ALL_FUNCTIONS(Scalar) \
  template int masa_init      <Scalar>(std::string, std::string); \
  template int masa_select_mms<Scalar>(std::string); \
  template int masa_list_mms  <Scalar>(); \
  template int masa_purge_default_param <Scalar>(); \
  template int pass_func                <Scalar>(Scalar (*)(Scalar),Scalar); \
  template int    masa_init_param<Scalar>(); \
  template void   masa_set_param<Scalar>(std::string,Scalar); \
  template Scalar masa_get_param<Scalar>(std::string); \
  template Scalar masa_eval_source_t  <Scalar>(Scalar);         \
  template Scalar masa_eval_source_t  <Scalar>(Scalar,Scalar);  \
  template Scalar masa_eval_source_u  <Scalar>(Scalar); \
  template Scalar masa_eval_source_v  <Scalar>(Scalar);         \
  template Scalar masa_eval_source_w  <Scalar>(Scalar); \
  template Scalar masa_eval_source_e  <Scalar>(Scalar); \
  template Scalar masa_eval_source_rho<Scalar>(Scalar); \
  template Scalar masa_eval_source_rho_u<Scalar>(Scalar);  \
  template Scalar masa_eval_source_rho_e<Scalar>(Scalar);  \
  template Scalar masa_eval_source_rho_N<Scalar>(Scalar);  \
  template Scalar masa_eval_source_rho_N2<Scalar>(Scalar);  \
  template Scalar masa_eval_exact_t      <Scalar>(Scalar);         \
  template Scalar masa_eval_exact_t      <Scalar>(Scalar,Scalar);  \
  template Scalar masa_eval_exact_u      <Scalar>(Scalar); \
  template Scalar masa_eval_exact_v      <Scalar>(Scalar);         \
  template Scalar masa_eval_exact_w      <Scalar>(Scalar); \
  template Scalar masa_eval_exact_p      <Scalar>(Scalar); \
  template Scalar masa_eval_exact_rho    <Scalar>(Scalar); \
  template Scalar masa_eval_exact_rho_N   <Scalar>(Scalar); \
  template Scalar masa_eval_exact_rho_N2  <Scalar>(Scalar); \
  template Scalar masa_eval_source_t  <Scalar>(Scalar,Scalar,Scalar);  \
  template Scalar masa_eval_source_u  <Scalar>(Scalar,Scalar); \
  template Scalar masa_eval_source_v  <Scalar>(Scalar,Scalar); \
  template Scalar masa_eval_source_w  <Scalar>(Scalar,Scalar);  \
  template Scalar masa_eval_source_e  <Scalar>(Scalar,Scalar); \
  template Scalar masa_eval_source_rho_u<Scalar>(Scalar,Scalar);  \
  template Scalar masa_eval_source_rho_v<Scalar>(Scalar,Scalar);   \
  template Scalar masa_eval_source_rho_w<Scalar>(Scalar,Scalar);  \
  template Scalar masa_eval_source_rho_e<Scalar>(Scalar,Scalar);  \
  template Scalar masa_eval_source_rho<Scalar>(Scalar,Scalar); \
  template Scalar masa_eval_exact_t      <Scalar>(Scalar,Scalar,Scalar);  \
  template Scalar masa_eval_exact_u      <Scalar>(Scalar,Scalar); \
  template Scalar masa_eval_exact_v      <Scalar>(Scalar,Scalar); \
  template Scalar masa_eval_exact_w      <Scalar>(Scalar,Scalar);  \
  template Scalar masa_eval_exact_p      <Scalar>(Scalar,Scalar); \
  template Scalar masa_eval_exact_rho    <Scalar>(Scalar,Scalar); \
  template Scalar masa_eval_source_t  <Scalar>(Scalar,Scalar,Scalar,Scalar);  \
  template Scalar masa_eval_source_u  <Scalar>(Scalar,Scalar,Scalar); \
  template Scalar masa_eval_source_v  <Scalar>(Scalar,Scalar,Scalar); \
  template Scalar masa_eval_source_w  <Scalar>(Scalar,Scalar,Scalar); \
  template Scalar masa_eval_source_e  <Scalar>(Scalar,Scalar,Scalar); \
  template Scalar masa_eval_source_rho_u<Scalar>(Scalar,Scalar,Scalar);     \
  template Scalar masa_eval_source_rho_v<Scalar>(Scalar,Scalar,Scalar);     \
  template Scalar masa_eval_source_rho_w<Scalar>(Scalar,Scalar,Scalar);     \
  template Scalar masa_eval_source_rho_e<Scalar>(Scalar,Scalar,Scalar);     \
  template Scalar masa_eval_source_rho<Scalar>(Scalar,Scalar,Scalar); \
  template Scalar masa_eval_exact_t      <Scalar>(Scalar,Scalar,Scalar,Scalar);  \
  template Scalar masa_eval_exact_u      <Scalar>(Scalar,Scalar,Scalar); \
  template Scalar masa_eval_exact_v      <Scalar>(Scalar,Scalar,Scalar); \
  template Scalar masa_eval_exact_w      <Scalar>(Scalar,Scalar,Scalar); \
  template Scalar masa_eval_exact_p      <Scalar>(Scalar,Scalar,Scalar); \
  template Scalar masa_eval_exact_rho    <Scalar>(Scalar,Scalar,Scalar); \
  template Scalar masa_eval_grad_1d_u  <Scalar>(Scalar); \
  template Scalar masa_eval_grad_2d_u  <Scalar>(Scalar,Scalar,int); \
  template Scalar masa_eval_grad_3d_u  <Scalar>(Scalar,Scalar,Scalar,int); \
  template Scalar masa_eval_grad_1d_v  <Scalar>(Scalar); \
  template Scalar masa_eval_grad_2d_v  <Scalar>(Scalar,Scalar,int); \
  template Scalar masa_eval_grad_3d_v  <Scalar>(Scalar,Scalar,Scalar,int); \
  template Scalar masa_eval_grad_1d_w  <Scalar>(Scalar); \
  template Scalar masa_eval_grad_2d_w  <Scalar>(Scalar,Scalar,int); \
  template Scalar masa_eval_grad_3d_w  <Scalar>(Scalar,Scalar,Scalar,int); \
  template Scalar masa_eval_grad_1d_p  <Scalar>(Scalar); \
  template Scalar masa_eval_grad_2d_p  <Scalar>(Scalar,Scalar,int); \
  template Scalar masa_eval_grad_3d_p  <Scalar>(Scalar,Scalar,Scalar,int); \
  template Scalar masa_eval_grad_1d_rho<Scalar>(Scalar); \
  template Scalar masa_eval_grad_2d_rho<Scalar>(Scalar,Scalar,int); \
  template Scalar masa_eval_grad_3d_rho<Scalar>(Scalar,Scalar,Scalar,int); \
  template int masa_test_poly<Scalar>();                             \
  template int masa_printid<Scalar>(); \
  template int masa_display_param<Scalar>(); \
  template int masa_get_name<Scalar>(std::string*); \
  template int masa_get_dimension<Scalar>(int*); \
  template int masa_sanity_check<Scalar>()

namespace MASA {

INSTANTIATE_ALL_FUNCTIONS(double);
INSTANTIATE_ALL_FUNCTIONS(long double);

}
