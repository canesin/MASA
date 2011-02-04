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
// c_euler1d.c :program that tests masa against known source term
//
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

#include <config.h> // for MASA_STRICT_REGRESSION
#include <masa.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const double threshold = 1.0e-15; // should be small enough to catch any obvious problems

double fsol_(double x)
{
  return 2*x;
}

double anQ_p (double x,double p_0,double p_x,double a_px,double L)
{
  const double pi = acos(-1);
  double p_exact = p_0 + p_x * cos(a_px * pi * x / L);
  return p_exact;
}
  
double anQ_u (double x,double u_0,double u_x,double a_ux,double L)
{
  const double pi = acos(-1); 
  double u_exact = u_0 + u_x * sin(a_ux * pi * x / L);
  return u_exact;
} 
 
double anQ_rho (double x,double rho_0,double rho_x,double a_rhox,double L)
{ 
  const double pi = acos(-1);  
  double rho_exact = rho_0 + rho_x * sin(a_rhox * pi * x / L);
  return rho_exact;
}

double SourceQ_rho (
  double x,
  double u_0,
  double u_x,
  double rho_0,
  double rho_x,
  double p_0,
  double p_x,
  double a_px,
  double a_rhox,
  double a_ux,
  double L)
{
  const double pi = acos(-1);  
  double Q_rho;
  double RHO;
  double U;

  RHO = rho_0 + rho_x * sin(a_rhox * pi * x / L);
  U = u_0 + u_x * sin(a_ux * pi * x / L);

  Q_rho = cos(a_ux * pi * x / L) * RHO * a_ux * pi * u_x / L + cos(a_rhox * pi * x / L) * U * a_rhox * pi * rho_x / L;

  return(Q_rho);
}

double SourceQ_u (
  double x,
  double u_0,
  double u_x,
  double rho_0,
  double rho_x,
  double p_0,
  double p_x,
  double a_px,
  double a_rhox,
  double a_ux,
  double L)
{
  const double pi = acos(-1); 
  double Q_u;
  double RHO;
  double U;

  RHO = rho_0 + rho_x * sin(a_rhox * pi * x / L);
  U = u_0 + u_x * sin(a_ux * pi * x / L);

  Q_u = 0.2e1 * cos(a_ux * pi * x / L) * RHO * U * a_ux * pi * u_x / L + cos(a_rhox * pi * x / L) * U * U * a_rhox * pi * rho_x / L - sin(a_px * pi * x / L) * a_px * pi * p_x / L;
  
  return(Q_u);
}

double SourceQ_e (
  double x,
  double u_0,
  double u_x,
  double rho_0,
  double rho_x,
  double p_0,
  double p_x,
  double a_px,
  double a_rhox,
  double a_ux,
  double Gamma,
  double mu,
  double L)
{
  const double pi = acos(-1);  
  double Q_e;
  double RHO;
  double U;
  double P;

  RHO = rho_0 + rho_x * sin(a_rhox * pi * x / L);
  P = p_0 + p_x * cos(a_px * pi * x / L);
  U = u_0 + u_x * sin(a_ux * pi * x / L);

  Q_e = cos(a_rhox * pi * x / L) * pow(U, 0.3e1) * a_rhox * pi * rho_x / L / 0.2e1 + cos(a_ux * pi * x / L) * P * a_ux * pi * u_x * Gamma / L / (Gamma - 0.1e1) + 0.3e1 / 0.2e1 * cos(a_ux * pi * x / L) * RHO * U * U * a_ux * pi * u_x / L - sin(a_px * pi * x / L) * U * a_px * pi * p_x * Gamma / L / (Gamma - 0.1e1);
  return(Q_e);
}

int main()
{
  //variables 
  double u_0;
  double u_x;
  double rho_0;
  double rho_x;
  double p_0;
  double p_x;
  double a_px;
  double a_rhox;
  double a_ux;
  double Gamma;
  double mu;
  double L;

  // parameters
  double x;
  int i;

  //problem size
  int nx    = 200;  // number of points
  int lx    = 10;     // length
  double dx = (double)(lx)/(double)(nx);

  // solutions
  double ufield,ufield2,ufield3;
  double vfield,vfield2,vfield3;
  double efield,efield2,efield3;
  double rho,rho2,rho3;

  double u_exact,u_exact2,u_exact3;
  double v_exact,v_exact2,v_exact3;
  double p_exact,p_exact2,p_exact3;
  double rho_exact,rho_exact2,rho_exact3;

  // initalize
  cmasa_init("euler-test","euler_1d");

  // initialize the default parameters
  cmasa_init_param();

  // get defaults for comparison to source terms
  // get vars
  u_0 = cmasa_get_param("u_0");
  u_x = cmasa_get_param("u_x");

  rho_0 = cmasa_get_param("rho_0");
  rho_x = cmasa_get_param("rho_x");

  p_0 = cmasa_get_param("p_0");
  p_x = cmasa_get_param("p_x");

  a_px = cmasa_get_param("a_px");

  a_rhox = cmasa_get_param("a_rhox");

  a_ux = cmasa_get_param("a_ux");

  Gamma = cmasa_get_param("Gamma");
  mu    = cmasa_get_param("mu");
  L     = cmasa_get_param("L");

  // check that all terms have been initialized
  cmasa_sanity_check();

  // evaluate source terms (1D)
  for(i=0;i<nx;i++)
    {
      x=i*dx;

      //evalulate source terms
      ufield = cmasa_eval_1d_u_source  (x);
      efield = cmasa_eval_1d_e_source  (x);
      rho    = cmasa_eval_1d_rho_source(x);
	
      //evaluate analytical terms
      u_exact   = cmasa_eval_1d_u_exact      (x);
      p_exact   = cmasa_eval_1d_p_exact      (x);
      rho_exact = cmasa_eval_1d_rho_exact     (x);
	
      // get fundamental source term solution
      ufield2   = SourceQ_u  (x,u_0,u_x,rho_0,rho_x,p_0,p_x,a_px,a_rhox,a_ux,L);
      rho2      = SourceQ_rho(x,u_0,u_x,rho_0,rho_x,p_0,p_x,a_px,a_rhox,a_ux,L);
      efield2   = SourceQ_e  (x,u_0,u_x,rho_0,rho_x,p_0,p_x,a_px,a_rhox,a_ux,Gamma,mu,L);
  
      u_exact2   = anQ_u   (x,u_0,u_x,a_ux,L);
      rho_exact2 = anQ_rho (x,rho_0,rho_x,a_rhox,L);
      p_exact2   = anQ_p   (x,p_0,p_x,a_px,L);

      // test the result is roughly zero
      // choose between abs and rel error
#ifdef MASA_STRICT_REGRESSION

      ufield3 = fabs(ufield-ufield2);
      efield3 = fabs(efield-efield2);
      rho3    = fabs(rho-rho2);
      
      u_exact3   = fabs(u_exact-u_exact2);
      rho_exact3 = fabs(rho_exact-rho_exact2);
      p_exact3   = fabs(p_exact-p_exact2);

#else

      ufield3 = fabs(ufield-ufield2)/fabs(ufield2);
      efield3 = fabs(efield-efield2)/fabs(efield2);
      rho3    = fabs(rho-rho2)/fabs(rho2);
      
      u_exact3   = fabs(u_exact-u_exact2)/fabs(u_exact2);
      rho_exact3 = fabs(rho_exact-rho_exact2)/fabs(rho_exact2);
      p_exact3   = fabs(p_exact-p_exact2)/fabs(p_exact2);

#endif

	if(ufield3 > threshold)
	  {
	    printf("\nMASA REGRESSION TEST FAILED: C-binding Euler-1d\n");
	    printf("U Field Source Term\n");
	    printf("Threshold Exceeded: %g\n",ufield3);
	    printf("CMASA:              %5.16f\n",ufield);
	    printf("Maple:              %5.16f\n",ufield2);
	    exit(1);
	  }

	if(u_exact3 > threshold)
	  {
	    printf("\nMASA REGRESSION TEST FAILED: C-binding Euler-1d\n");
	    printf("U Field Analytical Term\n");
	    printf("Threshold Exceeded: %g\n",u_exact3);
	    printf("CMASA:              %5.16f\n",u_exact);
	    printf("Maple:              %5.16f\n",u_exact2);
	    exit(1);
	  }

	if(efield3 > threshold)
	  {

	    printf("\nMASA REGRESSION TEST FAILED: C-binding Euler-1d\n");
	    printf("Energy Source Term\n");
	    printf("Threshold Exceeded: %g\n",efield3);
	    printf("CMASA:              %5.16f\n",efield);
	    printf("Maple:              %5.16f\n",efield2);
	    exit(1);
	  }

	if(p_exact3 > threshold)
	  {
	    
	    printf("\nMASA REGRESSION TEST FAILED: C-binding Euler-1d\n");
	    printf("P Field Analytical Term\n");
	    exit(1);
	  }

	if(rho3 > threshold)
	  {

	    printf("\nMASA REGRESSION TEST FAILED: C-binding Euler-1d\n");
	    printf("RHO Source Term\n");
	    exit(1);
	  }

	if(rho_exact3 > threshold)
	  {	    
	    printf("\nMASA REGRESSION TEST FAILED: C-binding Euler-1d\n");
	    printf("RHO Analytical Term\n");
	    exit(1);
	  }

    } // done interating 

  // tests passed
  return 0;
}
