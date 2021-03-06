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

#include "antioch_config.h"

#include <valarray>

#ifdef ANTIOCH_HAVE_EIGEN
#include "Eigen/Dense"
#endif

#ifdef ANTIOCH_HAVE_METAPHYSICL
#include "metaphysicl/numberarray.h"
#endif

#ifdef ANTIOCH_HAVE_VEXCL
#include "vexcl/vexcl.hpp"
#endif

// Antioch

// Declare metaprogramming overloads before they're used
#include "antioch/eigen_utils_decl.h"
#include "antioch/metaphysicl_utils_decl.h"
#include "antioch/valarray_utils_decl.h"
#include "antioch/vector_utils_decl.h"
#include "antioch/vexcl_utils_decl.h"

#include "antioch/antioch_asserts.h"
#include "antioch/chemical_species.h"
#include "antioch/chemical_mixture.h"
#include "antioch/reaction_set.h"
#include "antioch/read_reaction_set_data_xml.h"
#include "antioch/cea_thermo.h"
#include "antioch/kinetics_evaluator.h"

#include "antioch/eigen_utils.h"
#include "antioch/metaphysicl_utils.h"
#include "antioch/valarray_utils.h"
#include "antioch/vector_utils.h"
#include "antioch/vexcl_utils.h"

#ifdef ANTIOCH_HAVE_GRVY
#include "grvy.h"

GRVY::GRVY_Timer_Class gt;
#endif

// C++
#include <limits>
#include <string>
#include <vector>


template <typename PairScalars>
int vectester(const std::string& input_name, 
	      const PairScalars& example,
	      const std::string& testname)
{
  using std::abs;

  typedef typename Antioch::value_type<PairScalars>::type Scalar;

  std::vector<std::string> species_str_list;
  const unsigned int n_species = 5;
  species_str_list.reserve(n_species);
  species_str_list.push_back( "N2" );
  species_str_list.push_back( "O2" );
  species_str_list.push_back( "N" );
  species_str_list.push_back( "O" );
  species_str_list.push_back( "NO" );

  Antioch::ChemicalMixture<Scalar> chem_mixture( species_str_list );
  Antioch::ReactionSet<Scalar> reaction_set( chem_mixture );
  Antioch::CEAThermodynamics<Scalar> thermo( chem_mixture );

  Antioch::read_reaction_set_data_xml<Scalar>( input_name, true, reaction_set );

  Antioch::KineticsEvaluator<Scalar,PairScalars> kinetics( reaction_set, example );
  std::vector<PairScalars> omega_dot(n_species, example);

  PairScalars P = example;

  // Mass fractions
  PairScalars Y_vals = example;

  for (unsigned int tuple=0; tuple != ANTIOCH_N_TUPLES; ++tuple)
    {
      P[2*tuple  ] = 1.0e5;
      P[2*tuple+1] = 1.2e5;

      Y_vals[2*tuple  ] = 0.2;
      Y_vals[2*tuple+1] = 0.25;
    }

  std::vector<PairScalars> Y(n_species,Y_vals);

  for (unsigned int tuple=0; tuple != ANTIOCH_N_TUPLES; ++tuple)
    Y[4][2*tuple+1] = 0;

  const unsigned int n_T_samples = 10;
  const Scalar T0 = 500;
  const Scalar T_inc = 500;

  const PairScalars R_mix = chem_mixture.R(Y);

  std::vector<PairScalars> molar_densities(n_species,example);
  std::vector<PairScalars> h_RT_minus_s_R(n_species, example);

  int return_flag = 0;

  for( unsigned int i = 0; i < n_T_samples; i++ )
    {
      PairScalars T = example;

      for (unsigned int tuple=0; tuple != ANTIOCH_N_TUPLES; ++tuple)
        {
          T[2*tuple  ] = T0 + T_inc*static_cast<Scalar>(i);
          T[2*tuple+1] = T[0]+T_inc/2;
	}

#ifdef ANTIOCH_HAVE_GRVY
  const std::string testnormal = testname + "-normal";
  gt.BeginTimer(testnormal);
#endif

      const PairScalars rho = P/(R_mix*T);
      chem_mixture.molar_densities(rho,Y,molar_densities);

      typedef typename Antioch::CEAThermodynamics<Scalar>::
	template Cache<PairScalars> Cache;

      thermo.h_RT_minus_s_R(Cache(T),h_RT_minus_s_R);

      kinetics.compute_mass_sources( T, molar_densities, h_RT_minus_s_R, omega_dot );

#ifdef ANTIOCH_HAVE_GRVY
  gt.EndTimer(testnormal);
#endif

      // Omega dot had better sum to 0.0
      PairScalars sum = omega_dot[0];
      for( unsigned int s = 1; s < n_species; s++ )
	{
	  sum += omega_dot[s];
	}
      const Scalar sum_tol = std::numeric_limits<Scalar>::epsilon() * 5e6; // 5e-10;
      const PairScalars abs_sum = abs(sum);
      if( Antioch::max(abs_sum) > sum_tol )
	{
	  return_flag = 1;
	  std::cerr << "Error: omega_dot did not sum to 0.0." << std::endl
		    << std::scientific << std::setprecision(16)
		    << "T = " << T << std::endl
		    << "sum = " << sum << ", sum_tol = " << sum_tol << std::endl;
	  for( unsigned int s = 0; s < n_species; s++ )
	    {
	      std::cerr << std::scientific << std::setprecision(16)
			<< "omega_dot(" << chem_mixture.chemical_species()[s]->species() << ") = "
			<< omega_dot[s] << std::endl;
	    }
	  std::cout << std::endl << std::endl;
	}
    }
  
#ifdef ANTIOCH_HAVE_EIGEN
  {
    // To do: tests with Eigen instead of std vectors
  }
#endif // ANTIOCH_HAVE_EIGEN

  return return_flag;
}


int main(int argc, char* argv[])
{
  // Check command line count.
  if( argc < 2 )
    {
      // TODO: Need more consistent error handling.
      std::cerr << "Error: Must specify reaction set XML input file." << std::endl;
      antioch_error();
    }

  int returnval = 0;

  returnval = returnval ||
    vectester (argv[1], std::valarray<float>(2*ANTIOCH_N_TUPLES), "valarray<float>");
  returnval = returnval ||
    vectester (argv[1], std::valarray<double>(2*ANTIOCH_N_TUPLES), "valarray<double>");
  returnval = returnval ||
    vectester (argv[1], std::valarray<long double>(2*ANTIOCH_N_TUPLES), "valarray<ld>");
#ifdef ANTIOCH_HAVE_EIGEN
  returnval = returnval ||
    vectester (argv[1], Eigen::Array<float, 2*ANTIOCH_N_TUPLES, 1>(), "Eigen::ArrayXf");
  returnval = returnval ||
    vectester (argv[1], Eigen::Array<double, 2*ANTIOCH_N_TUPLES, 1>(), "Eigen::ArrayXd");
  returnval = returnval ||
    vectester (argv[1], Eigen::Array<long double, 2*ANTIOCH_N_TUPLES, 1>(), "Eigen::ArrayXld");
#endif
#ifdef ANTIOCH_HAVE_METAPHYSICL
  returnval = returnval ||
    vectester (argv[1], MetaPhysicL::NumberArray<2*ANTIOCH_N_TUPLES, float>(0), "NumberArray<float>");
  returnval = returnval ||
    vectester (argv[1], MetaPhysicL::NumberArray<2*ANTIOCH_N_TUPLES, double>(0), "NumberArray<double>");
  returnval = returnval ||
    vectester (argv[1], MetaPhysicL::NumberArray<2*ANTIOCH_N_TUPLES, long double>(0), "NumberArray<ld>");
#endif
#ifdef ANTIOCH_HAVE_VEXCL
  vex::Context ctx_f (vex::Filter::All);
  if (!ctx_f.empty())
    returnval = returnval ||
      vectester (argv[1], vex::vector<float> (ctx_f, 2*ANTIOCH_N_TUPLES), "vex::vector<float>");

  vex::Context ctx_d (vex::Filter::DoublePrecision);
  if (!ctx_d.empty())
    returnval = returnval ||
      vectester (argv[1], vex::vector<double> (ctx_d, 2*ANTIOCH_N_TUPLES), "vex::vector<double>");
#endif

#ifdef ANTIOCH_HAVE_GRVY
  gt.Finalize();
  gt.Summarize();
#endif

  return returnval;
}
