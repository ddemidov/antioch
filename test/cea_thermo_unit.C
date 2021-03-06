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
#include <cmath>
#include <limits>

// Antioch
#include "antioch/physical_constants.h"
#include "antioch/chemical_mixture.h"
#include "antioch/cea_thermo.h"

template <typename Scalar>
int test_cp( const std::string& species_name, unsigned int species, Scalar cp_exact, Scalar T,
	     const Antioch::CEAThermodynamics<Scalar>& thermo )
{
  using std::abs;

  int return_flag = 0;

  const Scalar tol = std::numeric_limits<Scalar>::epsilon() * 5;

  typedef typename Antioch::CEAThermodynamics<Scalar>::template Cache<Scalar> Cache;

  const Scalar cp = thermo.cp(Cache(T), species);

  if( abs( (cp_exact - cp)/cp_exact ) > tol )
    {
      std::cerr << "Error: Mismatch in species specific heat."
		<< "\nspecies    = " << species_name
		<< "\ncp         = " << cp
		<< "\ncp_exact   = " << cp_exact
		<< "\ndifference = " << (cp_exact - cp)
		<< "\ntolerance  = " << tol
		<< "\nT = " << T << std::endl;
      return_flag = 1;
    }

  return return_flag;
}


template <typename Scalar>
Scalar cp( Scalar T, Scalar a0, Scalar a1, Scalar a2, 
	   Scalar a3, Scalar a4, Scalar a5, Scalar a6 )
{
  if( T < 200.1)
    T = 200.1;

  return a0/(T*T) + a1/T + a2 + a3*T + a4*(T*T) + a5*(T*T*T) + a6*(T*T*T*T);
}


template <typename Scalar>
int tester()
{
  std::vector<std::string> species_str_list;
  const unsigned int n_species = 5;
  species_str_list.reserve(n_species);
  species_str_list.push_back( "N2" );
  species_str_list.push_back( "O2" );
  species_str_list.push_back( "N" );
  species_str_list.push_back( "O" );
  species_str_list.push_back( "NO" );

  const Scalar Mm_N = 14.008e-3L;
  const Scalar Mm_O = 16.000e-3L;
  const Scalar Mm_N2 = 2.L * Mm_N;
  const Scalar Mm_O2 = 2.L * Mm_O;
  const Scalar Mm_NO = Mm_N + Mm_O;

  Antioch::ChemicalMixture<Scalar> chem_mixture( species_str_list );

  Antioch::CEAThermodynamics<Scalar> thermo( chem_mixture );

  //const Scalar P = 100000.0;
  const Scalar T1 = 190.0;
  const Scalar T2 = 1500.0;
  const Scalar T3 = 10000.0;

  const Scalar R_N2 = Antioch::Constants::R_universal<Scalar>()/Mm_N2;
  const Scalar R_O2 = Antioch::Constants::R_universal<Scalar>()/Mm_O2;
  const Scalar R_N = Antioch::Constants::R_universal<Scalar>()/Mm_N;
  const Scalar R_O = Antioch::Constants::R_universal<Scalar>()/Mm_O;
  const Scalar R_NO = Antioch::Constants::R_universal<Scalar>()/Mm_NO;

  int return_flag = 0;

  // Test N2 cp
  {
    unsigned int index = 0;
    const Scalar cp_N2_1 = R_N2*cp( T1, Scalar(2.21037122e+04), Scalar(-3.81846145e+02), Scalar(6.08273815e+00), 
				    Scalar(-8.53091381e-03),  Scalar(1.38464610e-05), Scalar(-9.62579293e-09),  Scalar(2.51970560e-12));

    const Scalar cp_N2_2 = R_N2*cp( T2, Scalar(5.87709908e+05), Scalar(-2.23924255e+03),  Scalar(6.06694267e+00),
				    Scalar(-6.13965296e-04), Scalar(1.49179819e-07), Scalar(-1.92309442e-11), Scalar(1.06194871e-15) );

    const Scalar cp_N2_3 = R_N2*cp( T3, Scalar(8.30971200e+08), Scalar(-6.42048187e+05),  Scalar(2.02020507e+02), Scalar(-3.06501961e-02),
				    Scalar(2.48685558e-06), Scalar(-9.70579208e-11), Scalar(1.43751673e-15));
    
    const Antioch::Species species = chem_mixture.species_list()[index];
    const std::string species_name = chem_mixture.species_inverse_name_map().find(species)->second;

    int return_flag_temp = 0;
    return_flag_temp = test_cp( species_name, index, cp_N2_1, T1, thermo );
    if( return_flag_temp != 0 ) return_flag = 1;

    return_flag = test_cp( species_name, index, cp_N2_2, T2, thermo );
    if( return_flag_temp != 0 ) return_flag = 1;

    return_flag = test_cp( species_name, index, cp_N2_3, T3, thermo );
    if( return_flag_temp != 0 ) return_flag = 1;
  }

  // Test O2 cp
  {
    unsigned int index = 1;
    const Scalar cp_1 = R_O2*cp( T1, Scalar(-3.42556269e+04), Scalar(4.84699986e+02), Scalar(1.11901159e+00), 
				 Scalar(4.29388743e-03), Scalar(-6.83627313e-07), Scalar(-2.02337478e-09) , Scalar(1.03904064e-12) );

    const Scalar cp_2 = R_O2*cp( T2, Scalar(-1.03793994e+06), Scalar(2.34483275e+03), Scalar(1.81972949e+00), Scalar(1.26784887e-03), 
				 Scalar(-2.18807142e-07), Scalar(2.05372411e-11), Scalar(-8.19349062e-16) );

    const Scalar cp_3 = R_O2*cp( T3, Scalar(4.97515261e+08), Scalar(-2.86602339e+05), Scalar(6.69015464e+01), Scalar(-6.16971869e-03),  
				 Scalar(3.01623757e-07), Scalar(-7.42087888e-12), Scalar(7.27744063e-17));
    
    const Antioch::Species species = chem_mixture.species_list()[index];
    const std::string species_name = chem_mixture.species_inverse_name_map().find(species)->second;

    int return_flag_temp = 0;
    return_flag_temp = test_cp( species_name, index, cp_1, T1, thermo );
    if( return_flag_temp != 0 ) return_flag = 1;

    return_flag = test_cp( species_name, index, cp_2, T2, thermo );
    if( return_flag_temp != 0 ) return_flag = 1;

    return_flag = test_cp( species_name, index, cp_3, T3, thermo );
    if( return_flag_temp != 0 ) return_flag = 1;
  }

  // Test N cp
  {
    unsigned int index = 2;
    const Scalar cp_1 = R_N*cp( T1, Scalar(0.00000000e+00), Scalar(0.00000000e+00), Scalar(2.50000000e+00), Scalar(0.00000000e+00),
				Scalar(0.00000000e+00), Scalar(0.00000000e+00), Scalar(0.00000000e+00));

    const Scalar cp_2 = R_N*cp( T2, Scalar(8.87650138e+04), Scalar(-1.07123150e+02), Scalar(2.36218829e+00), Scalar(2.91672008e-04),
				Scalar(-1.72951510e-07), Scalar(4.01265788e-11), Scalar(-2.67722757e-15) );

    const Scalar cp_3 = R_N*cp( T3, Scalar(5.47518105e+08), Scalar(-3.10757498e+05), Scalar(6.91678274e+01), Scalar(-6.84798813e-03),
				Scalar(3.82757240e-07), Scalar(-1.09836771e-11), Scalar(1.27798602e-16));
    
    const Antioch::Species species = chem_mixture.species_list()[index];
    const std::string species_name = chem_mixture.species_inverse_name_map().find(species)->second;

    int return_flag_temp = 0;
    return_flag_temp = test_cp( species_name, index, cp_1, T1, thermo );
    if( return_flag_temp != 0 ) return_flag = 1;

    return_flag = test_cp( species_name, index, cp_2, T2, thermo );
    if( return_flag_temp != 0 ) return_flag = 1;

    return_flag = test_cp( species_name, index, cp_3, T3, thermo );
    if( return_flag_temp != 0 ) return_flag = 1;
  }


  // Test O cp
  {
    unsigned int index = 3;
    const Scalar cp_1 = R_O*cp( T1, Scalar(-7.95361130e+03) , Scalar(1.60717779e+02), Scalar(1.96622644e+00), Scalar(1.01367031e-03),
				Scalar(-1.11041542e-06), Scalar(6.51750750e-10), Scalar(-1.58477925e-13) );

    const Scalar cp_2 = R_O*cp( T2, Scalar(2.61902026e+05), Scalar(-7.29872203e+02), Scalar(3.31717727e+00), Scalar(-4.28133436e-04), 
				Scalar(1.03610459e-07), Scalar(-9.43830433e-12), Scalar(2.72503830e-16) );

    const Scalar cp_3 = R_O*cp( T3, Scalar(1.77900426e+08), Scalar(-1.08232826e+05), Scalar(2.81077837e+01), Scalar(-2.97523226e-03),
				Scalar(1.85499753e-07), Scalar(-5.79623154e-12), Scalar(7.19172016e-17) );
    
    const Antioch::Species species = chem_mixture.species_list()[index];
    const std::string species_name = chem_mixture.species_inverse_name_map().find(species)->second;

    int return_flag_temp = 0;
    return_flag_temp = test_cp( species_name, index, cp_1, T1, thermo );
    if( return_flag_temp != 0 ) return_flag = 1;

    return_flag = test_cp( species_name, index, cp_2, T2, thermo );
    if( return_flag_temp != 0 ) return_flag = 1;

    return_flag = test_cp( species_name, index, cp_3, T3, thermo );
    if( return_flag_temp != 0 ) return_flag = 1;
  }


  // Test NO cp
  {
    unsigned int index = 4;
    const Scalar cp_1 = R_NO*cp( T1, Scalar(-1.14391658e+04), Scalar(1.53646774e+02), Scalar(3.43146865e+00), Scalar(-2.66859213e-03),
				 Scalar(8.48139877e-06), Scalar(-7.68511079e-09), Scalar(2.38679758e-12) );

    const Scalar cp_2 = R_NO*cp( T2, Scalar(2.23903708e+05), Scalar(-1.28965624e+03), Scalar(5.43394039e+00), Scalar(-3.65605546e-04), 
				 Scalar(9.88101763e-08), Scalar(-1.41608327e-11), Scalar(9.38021642e-16) );

    const Scalar cp_3 = R_NO*cp( T3, Scalar(-9.57530764e+08), Scalar(5.91243671e+05), Scalar(-1.38456733e+02), Scalar(1.69433998e-02), 
				 Scalar(-1.00735146e-06), Scalar(2.91258526e-11), Scalar(-3.29511091e-16) );
    
    const Antioch::Species species = chem_mixture.species_list()[index];
    const std::string species_name = chem_mixture.species_inverse_name_map().find(species)->second;

    int return_flag_temp = 0;
    return_flag_temp = test_cp( species_name, index, cp_1, T1, thermo );
    if( return_flag_temp != 0 ) return_flag = 1;

    return_flag = test_cp( species_name, index, cp_2, T2, thermo );
    if( return_flag_temp != 0 ) return_flag = 1;

    return_flag = test_cp( species_name, index, cp_3, T3, thermo );
    if( return_flag_temp != 0 ) return_flag = 1;
  }

  return return_flag;
}


int main()
{
// We're not getting the full long double precision yet?
  return (tester<double>() ||
//          tester<long double>() ||
          tester<float>());
}
