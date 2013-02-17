//-----------------------------------------------------------------------bl-
//--------------------------------------------------------------------------
// 
// Antioch - A Gas Dynamics Thermochemistry Library
//
// Copyright (C) 2013 The PECOS Development Team
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
//
// $Id$
//
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

// C++
#include <iostream>
#include <cmath>

// Antioch
#include "antioch/blottner_viscosity.h"

int test_viscosity( const double mu, const double mu_exact, const double tol );

int main()
{
  const double a = 3.14e-3;
  const double b = 2.71e-2;
  const double c = 42.0e-5;

  Antioch::BlottnerViscosity<double> mu(a,b,c);

  std::cout << mu << std::endl;

  const double T = 1521.2;

  // octave gives
  const double mu_exact = 0.144422234167703;

  int return_flag = 0;

  const double tol = 1.0e-14;

  return_flag = test_viscosity( mu(T), mu_exact, tol );
  
  const double a2 = 1e-3;
  const double b2 = 2e-2;
  const double c2 = 3e-5;

  mu.reset_coeffs( a2, b2, c2 );

  // octave gives
  const double mu_exact2 = 0.122172495548880;

  return_flag = test_viscosity( mu(T), mu_exact2, tol );

  return return_flag;
}

int test_viscosity( const double mu, const double mu_exact, const double tol )
{
  int return_flag = 0;

  const double rel_error = std::fabs( (mu - mu_exact)/mu_exact);

  if( rel_error  > tol )
    {
      std::cerr << "Error: Mismatch in viscosity" << std::endl
		<< "mu(T)    = " << mu << std::endl
		<< "mu_exact = " << mu_exact << std::endl
		<< "rel_error = " << rel_error << std::endl
		<< "tol = " << tol << std::endl;
      return_flag = 1;
    }

  return return_flag;
}