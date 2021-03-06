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

// C++
#include <cmath>
#include <iomanip>
#include <limits>

// Antioch

// Declare metaprogramming overloads before they're used
#include "antioch/eigen_utils_decl.h"
#include "antioch/metaphysicl_utils_decl.h"
#include "antioch/valarray_utils_decl.h"
#include "antioch/vexcl_utils_decl.h"
#include "antioch/vector_utils_decl.h"

#include "antioch/chemical_mixture.h"
#include "antioch/chemical_species.h"
#include "antioch/physical_constants.h"

#include "antioch/eigen_utils.h"
#include "antioch/metaphysicl_utils.h"
#include "antioch/valarray_utils.h"
#include "antioch/vexcl_utils.h"
#include "antioch/vector_utils.h"

#ifdef ANTIOCH_HAVE_GRVY
#include "grvy.h"

GRVY::GRVY_Timer_Class gt;
#endif

// C++
#include <cmath>
#include <iomanip>
#include <limits>

template <typename Scalar>
int test_species( const unsigned int species,
		  const std::vector<Antioch::ChemicalSpecies<Scalar>*>& chemical_species,
		  const std::string& species_name,
		  Scalar molar_mass, Scalar gas_constant, Scalar formation_enthalpy, 
		  Scalar n_tr_dofs, int charge )
{

  int return_flag = 0;

  const Antioch::ChemicalSpecies<Scalar>& chem_species = *(chemical_species[species]);

  if( chem_species.species() != species_name )
    {
      std::cerr << "Error: Name mismatch for "<< species_name << std::endl
		<< "name = " << chem_species.species() << std::endl;
      return_flag = 1;
    }

  if( chem_species.molar_mass() != molar_mass )
    {
      std::cerr << "Error: Molar mass mismatch for "<< species_name << std::endl
		<< "molar mass = " << chem_species.molar_mass() << std::endl;
      return_flag = 1;
    }

  if( chem_species.gas_constant() != gas_constant )
    {
      std::cerr << "Error: Gas constant mismatch for "<< species_name << std::endl
		<< "gas constant = " << chem_species.gas_constant() << std::endl;
      return_flag = 1;
    }

  if( chem_species.formation_enthalpy() != formation_enthalpy )
    {
      std::cerr << "Error: Formation enthalpy mismatch for "<< species_name << std::endl
		<< "formation enthalpy = " << chem_species.formation_enthalpy() << std::endl;
      return_flag = 1;
    }

  if( chem_species.n_tr_dofs() != n_tr_dofs )
    {
      std::cerr << "Error: Number translational DoFs mismatch for "<< species_name << std::endl
		<< "n_tr_dofs = " << chem_species.n_tr_dofs() << std::endl;
      return_flag = 1;
    }

  if( chem_species.charge() != charge )
    {
      std::cerr << "Error: Charge mismatch for "<< species_name << std::endl
		<< "charge = " << chem_species.charge() << std::endl;
      return_flag = 1;
    }

  return return_flag;
}


template <typename PairScalars>
int vectester(const PairScalars& example, const std::string& testname)
{
  using std::abs;

  typedef typename Antioch::value_type<PairScalars>::type Scalar;

  const Scalar Mm_N = 14.008e-3L;
  const Scalar Mm_O = 16.000e-3L;
  const Scalar Mm_N2 = 2.L * Mm_N;
  const Scalar Mm_O2 = 2.L * Mm_O;
  const Scalar Mm_NO = Mm_N + Mm_O;

  std::vector<std::string> species_str_list;
  const unsigned int n_species = 5;
  species_str_list.reserve(n_species);
  species_str_list.push_back( "N2" );
  species_str_list.push_back( "O2" );
  species_str_list.push_back( "N" );
  species_str_list.push_back( "O" );
  species_str_list.push_back( "NO" );

  Antioch::ChemicalMixture<Scalar> chem_mixture( species_str_list );

  const std::map<std::string,Antioch::Species>& species_name_map = chem_mixture.species_name_map();
  const std::map<Antioch::Species,std::string>& species_inverse_name_map = chem_mixture.species_inverse_name_map();
  const std::vector<Antioch::ChemicalSpecies<Scalar>*>& chemical_species = chem_mixture.chemical_species();
  const std::vector<Antioch::Species> species_list = chem_mixture.species_list();

  int return_flag = 0;

  // Check name map consistency
  for( unsigned int i = 0; i < n_species; i++ )
    {
      if( species_name_map.find( species_str_list[i] )->second != species_list[i] )
	{
	  std::cerr << "Error: species name map and species list ordering mismatch" << std::endl
		    << "species_name_map = " << species_name_map.find( species_str_list[i] )->second
		    << ", species_list = " << species_list[i] << std::endl;
	  return_flag = 1;
	}
    }

  // Check inverse name map consistency
  for( unsigned int i = 0; i < n_species; i++ )
    {
      if( species_inverse_name_map.find( species_list[i] )->second != species_str_list[i] )
	{
	  std::cerr << "Error: species inverse name map and species list ordering mismatch" << std::endl
		    << "species_inverse_name_map = " << species_inverse_name_map.find( species_list[i] )->second
		    << ", species_str_list = " << species_str_list[i] << std::endl;
	  return_flag = 1;
	}
    }

  // Check N2 properties
  {
    unsigned int index = 0;
    Scalar molar_mass = Mm_N2;
    if( molar_mass != chem_mixture.M(index) )
      {
	std::cerr << "Error: Molar mass inconsistency in mixture" << std::endl
		  << "molar mass = " << chem_mixture.M(index) << std::endl;
	return_flag = 1;
      }
    return_flag = test_species( index, chemical_species, "N2", molar_mass, 
                                Scalar(Antioch::Constants::R_universal<Scalar>()/molar_mass), 
                                Scalar(0.0), Scalar(2.5), Scalar(0));
  }
  
  // Check O2 properties
  {
    unsigned int index = 1;
    Scalar molar_mass = Mm_O2;
    if( molar_mass != chem_mixture.M(index) )
      {
	std::cerr << "Error: Molar mass inconsistency in mixture" << std::endl
		  << "molar mass = " << chem_mixture.M(index) << std::endl;
	return_flag = 1;
      }
    return_flag = test_species( index, chemical_species, "O2", molar_mass, 
                                Scalar(Antioch::Constants::R_universal<Scalar>()/molar_mass),
                                Scalar(0.0), Scalar(2.5), Scalar(0));
  }

  // Check N properties
  {
    unsigned int index = 2;
    Scalar molar_mass = Mm_N;
    if( molar_mass != chem_mixture.M(index) )
      {
	std::cerr << "Error: Molar mass inconsistency in mixture" << std::endl
		  << "molar mass = " << chem_mixture.M(index) << std::endl;
	return_flag = 1;
      }
    return_flag = test_species( index, chemical_species, "N", molar_mass,
                                Scalar(Antioch::Constants::R_universal<Scalar>()/molar_mass), 
                                Scalar(3.3621610000e7), Scalar(1.5), Scalar(0));
  }

  // Check O properties
  {
    unsigned int index = 3;
    Scalar molar_mass = Mm_O;
    if( molar_mass != chem_mixture.M(index) )
      {
	std::cerr << "Error: Molar mass inconsistency in mixture" << std::endl
		  << "molar mass = " << chem_mixture.M(index) << std::endl;
	return_flag = 1;
      }
    return_flag = test_species( index, chemical_species, "O", molar_mass,
                                Scalar(Antioch::Constants::R_universal<Scalar>()/molar_mass), 
                                Scalar(1.5420000000e7), Scalar(1.5), Scalar(0));
  }

  // Check NO properties
  {
    unsigned int index = 4;
    Scalar molar_mass = Mm_NO;
    if( molar_mass != chem_mixture.M(index) )
      {
	std::cerr << "Error: Molar mass inconsistency in mixture" << std::endl
		  << "molar mass = " << chem_mixture.M(index) << std::endl;
	return_flag = 1;
      }
    return_flag = test_species( index, chemical_species, "NO", molar_mass,
                                Scalar(Antioch::Constants::R_universal<Scalar>()/molar_mass), 
                                Scalar(2.9961230000e6), Scalar(2.5), Scalar(0));
  }

  std::vector<PairScalars> mass_fractions( n_species, example );
  PairScalars R_exact = example;
  PairScalars M_exact = example;
  std::vector<PairScalars> X_exact(n_species, example);

  for (unsigned int tuple=0; tuple != ANTIOCH_N_TUPLES; ++tuple)
    {
      mass_fractions[0][2*tuple  ] = 0.25L;
      mass_fractions[1][2*tuple  ] = 0.25L;
      mass_fractions[2][2*tuple  ] = 0.25L;
      mass_fractions[3][2*tuple  ] = 0.25L;
      mass_fractions[4][2*tuple  ] = 0L;
      mass_fractions[0][2*tuple+1] = 0.2L;
      mass_fractions[1][2*tuple+1] = 0.2L;
      mass_fractions[2][2*tuple+1] = 0.2L;
      mass_fractions[3][2*tuple+1] = 0.2L;
      mass_fractions[4][2*tuple+1] = 0.2L;

      R_exact[2*tuple  ] = Antioch::Constants::R_universal<Scalar>()*( 0.25L/Mm_N2 + 0.25L/Mm_O2 + 0.25L/Mm_N + 0.25L/Mm_O);
      R_exact[2*tuple+1] = Antioch::Constants::R_universal<Scalar>()*( 0.2L/Mm_N2 + 0.2L/Mm_O2 + 0.2L/Mm_N + 0.2L/Mm_O + 0.2L/Mm_NO );

      M_exact[2*tuple  ] = 1.0L/( 0.25L*( 1.0L/Mm_N2 + 1.0L/Mm_O2 + 1.0L/Mm_N + 1.0L/Mm_O) );
      M_exact[2*tuple+1] = 1.0L/( 0.2L*( 1.0L/Mm_N2 + 1.0L/Mm_O2 + 1.0L/Mm_N + 1.0L/Mm_O + 1.0L/Mm_NO) );
  
      X_exact[0][2*tuple  ] = 0.25L*M_exact[0]/Mm_N2;
      X_exact[1][2*tuple  ] = 0.25L*M_exact[0]/Mm_O2;
      X_exact[2][2*tuple  ] = 0.25L*M_exact[0]/Mm_N;
      X_exact[3][2*tuple  ] = 0.25L*M_exact[0]/Mm_O;
      X_exact[4][2*tuple  ] = 0L;
      X_exact[0][2*tuple+1] = 0.2L*M_exact[1]/Mm_N2;
      X_exact[1][2*tuple+1] = 0.2L*M_exact[1]/Mm_O2;
      X_exact[2][2*tuple+1] = 0.2L*M_exact[1]/Mm_N;
      X_exact[3][2*tuple+1] = 0.2L*M_exact[1]/Mm_O;
      X_exact[4][2*tuple+1] = 0.2L*M_exact[1]/Mm_NO;
    }

#ifdef ANTIOCH_HAVE_GRVY
  const std::string testnormal = testname + "-normal";
  gt.BeginTimer(testnormal);
#endif

  std::vector<PairScalars> X(n_species, example);

  const PairScalars R = chem_mixture.R(mass_fractions);
  const PairScalars M = chem_mixture.M(mass_fractions);
  chem_mixture.X( M, mass_fractions, X );

#ifdef ANTIOCH_HAVE_GRVY
  gt.EndTimer(testnormal);
#endif

  Scalar tol = std::numeric_limits<Scalar>::epsilon() * 10;

  const PairScalars rel_R_error = 
    abs( (R - R_exact)/R_exact);
  if( Antioch::max(rel_R_error) > tol )
    {
      std::cerr << "Error: Mismatch in mixture gas constant." << std::endl
		<< std::setprecision(16) << std::scientific
		<< "R       = " << R << std::endl
		<< "R_exact = " << R_exact <<  std::endl;
      return_flag = 1;
    }

  const PairScalars rel_M_error = 
    abs( (M - M_exact)/M_exact);
  if( Antioch::max(rel_M_error) > tol )
    {
      std::cerr << "Error: Mismatch in mixture molar mass." << std::endl
		<< std::setprecision(16) << std::scientific
		<< "M       = " << M << std::endl
		<< "M_exact = " << M_exact << std::endl;
      return_flag = 1;
    }
  
  for( unsigned int s = 0; s < n_species; s++ )
    {
      const PairScalars rel_X_error = 
        abs( (X[s] - X_exact[s])/X_exact[s]);
      if( Antioch::max(rel_X_error) > tol )
	{
	  std::cerr << "Error: Mismatch in mole fraction for species " << s << std::endl
		    << std::setprecision(16) << std::scientific
		    << "X       = " << X[s] << std::endl
		    << "X_exact = " << X_exact[s] << std::endl;
	  return_flag = 1;
	}
    }
  

#ifdef ANTIOCH_HAVE_EIGEN
  {
    typedef Eigen::Array<PairScalars,n_species,1> SpeciesVecEigenType;

    SpeciesVecEigenType eigen_mass_fractions;
    Antioch::init_constant(eigen_mass_fractions, mass_fractions[0]);

    SpeciesVecEigenType eigen_X_exact;
    Antioch::init_constant(eigen_X_exact, example);
    for (unsigned int tuple=0; tuple != ANTIOCH_N_TUPLES; ++tuple)
      {
        eigen_mass_fractions[4][2*tuple] = 0L;

        eigen_X_exact[0][2*tuple  ] = 0.25L*M_exact[0]/28.016L;
        eigen_X_exact[1][2*tuple  ] = 0.25L*M_exact[0]/32.0L;
        eigen_X_exact[2][2*tuple  ] = 0.25L*M_exact[0]/14.008L;
        eigen_X_exact[3][2*tuple  ] = 0.25L*M_exact[0]/16.0L;
        eigen_X_exact[4][2*tuple  ] = 0L;
        eigen_X_exact[0][2*tuple+1] = 0.2L*M_exact[1]/28.016L;
        eigen_X_exact[1][2*tuple+1] = 0.2L*M_exact[1]/32.0L;
        eigen_X_exact[2][2*tuple+1] = 0.2L*M_exact[1]/14.008L;
        eigen_X_exact[3][2*tuple+1] = 0.2L*M_exact[1]/16.0L;
        eigen_X_exact[4][2*tuple+1] = 0.2L*M_exact[1]/30.008L;
      }

#ifdef ANTIOCH_HAVE_GRVY
    const std::string testeigenA = testname + "-eigenA";
    gt.BeginTimer(testeigenA);
#endif

    const PairScalars R_eigen = chem_mixture.R(eigen_mass_fractions);
    const PairScalars M_eigen = chem_mixture.M(eigen_mass_fractions);
    SpeciesVecEigenType eigen_X;
    Antioch::init_constant(eigen_X, example);
    chem_mixture.X( M_eigen, eigen_mass_fractions, eigen_X );

#ifdef ANTIOCH_HAVE_GRVY
    gt.EndTimer(testeigenA);
#endif

    Scalar tol = std::numeric_limits<Scalar>::epsilon() * 10;

    const PairScalars eigen_rel_R_error = 
      abs( (R_eigen - R_exact)/R_exact);
    if( Antioch::max(eigen_rel_R_error) > tol )
      {
        std::cerr << "Error: Mismatch in mixture gas constant." << std::endl
		  << std::setprecision(16) << std::scientific
		  << "R_eigen = " << R_eigen << std::endl
		  << "R_exact = " << R_exact <<  std::endl;
        return_flag = 1;
      }

    const PairScalars eigen_rel_M_error = 
      abs( (M_eigen - M_exact)/M_exact);
    if( Antioch::max(eigen_rel_M_error) > tol )
      {
        std::cerr << "Error: Mismatch in mixture molar mass." << std::endl
		  << std::setprecision(16) << std::scientific
		  << "M_eigen = " << M_eigen << std::endl
		  << "M_exact = " << M_exact << std::endl;
        return_flag = 1;
      }
  
    for( unsigned int s = 0; s < n_species; s++ )
      {
        const PairScalars eigen_rel_X_error = 
          abs( (eigen_X[s] - X_exact[s])/X_exact[s]);
        if( Antioch::max(eigen_rel_X_error) > tol )
	  {
	    std::cerr << "Error: Mismatch in mole fraction for species " << s << std::endl
		      << std::setprecision(16) << std::scientific
		      << "X_eigen = " << eigen_X[s] << std::endl
		      << "X_exact = " << X_exact[s] << std::endl;
	    return_flag = 1;
	  }
      }
  }

  {
    typedef Eigen::Matrix<PairScalars,Eigen::Dynamic,1> SpeciesVecEigenType;

    SpeciesVecEigenType eigen_mass_fractions(n_species, 1);
    Antioch::init_constant(eigen_mass_fractions, mass_fractions[0]);

    SpeciesVecEigenType eigen_X_exact(n_species, 1);
    Antioch::init_constant(eigen_X_exact, example);

    for (unsigned int tuple=0; tuple != ANTIOCH_N_TUPLES; ++tuple)
      {
        eigen_mass_fractions[4][2*tuple] = 0L;

        eigen_X_exact[0][2*tuple  ] = 0.25L*M_exact[0]/28.016L;
        eigen_X_exact[1][2*tuple  ] = 0.25L*M_exact[0]/32.0L;
        eigen_X_exact[2][2*tuple  ] = 0.25L*M_exact[0]/14.008L;
        eigen_X_exact[3][2*tuple  ] = 0.25L*M_exact[0]/16.0L;
        eigen_X_exact[4][2*tuple  ] = 0L;
        eigen_X_exact[0][2*tuple+1] = 0.2L*M_exact[1]/28.016L;
        eigen_X_exact[1][2*tuple+1] = 0.2L*M_exact[1]/32.0L;
        eigen_X_exact[2][2*tuple+1] = 0.2L*M_exact[1]/14.008L;
        eigen_X_exact[3][2*tuple+1] = 0.2L*M_exact[1]/16.0L;
        eigen_X_exact[4][2*tuple+1] = 0.2L*M_exact[1]/30.008L;
      }

#ifdef ANTIOCH_HAVE_GRVY
    const std::string testeigenV = testname + "-eigenV";
    gt.BeginTimer(testeigenV);
#endif

    const PairScalars R_eigen = chem_mixture.R(eigen_mass_fractions);
    const PairScalars M_eigen = chem_mixture.M(eigen_mass_fractions);
    SpeciesVecEigenType eigen_X(n_species, 1);
    Antioch::init_constant(eigen_X, example);
    chem_mixture.X( M_eigen, eigen_mass_fractions, eigen_X );

#ifdef ANTIOCH_HAVE_GRVY
    gt.EndTimer(testeigenV);
#endif

    Scalar tol = std::numeric_limits<Scalar>::epsilon() * 10;

    const PairScalars eigen_rel_R_error = 
      abs( (R_eigen - R_exact)/R_exact);
    if( Antioch::max(eigen_rel_R_error) > tol )
      {
        std::cerr << "Error: Mismatch in mixture gas constant." << std::endl
		  << std::setprecision(16) << std::scientific
		  << "R_eigen = " << R_eigen << std::endl
		  << "R_exact = " << R_exact <<  std::endl;
        return_flag = 1;
      }

    const PairScalars eigen_rel_M_error = 
      abs( (M_eigen - M_exact)/M_exact);
    if( Antioch::max(eigen_rel_M_error) > tol )
      {
        std::cerr << "Error: Mismatch in mixture molar mass." << std::endl
		  << std::setprecision(16) << std::scientific
		  << "M_eigen = " << M_eigen << std::endl
		  << "M_exact = " << M_exact << std::endl;
        return_flag = 1;
      }
  
    for( unsigned int s = 0; s < n_species; s++ )
      {
        const PairScalars eigen_rel_X_error = 
          abs( (eigen_X[s] - X_exact[s])/X_exact[s]);
        if( Antioch::max(eigen_rel_X_error) > tol )
	  {
	    std::cerr << "Error: Mismatch in mole fraction for species " << s << std::endl
		      << std::setprecision(16) << std::scientific
		      << "X_eigen = " << eigen_X[s] << std::endl
		      << "X_exact = " << X_exact[s] << std::endl;
	    return_flag = 1;
	  }
      }
  }
#endif // ANTIOCH_HAVE_EIGEN

  return return_flag;
}


int main()
{
  int returnval = 0;

  returnval = returnval ||
    vectester (std::valarray<float>(2*ANTIOCH_N_TUPLES), "valarray<float>");
  returnval = returnval ||
    vectester (std::valarray<double>(2*ANTIOCH_N_TUPLES), "valarray<double>");
  returnval = returnval ||
    vectester (std::valarray<long double>(2*ANTIOCH_N_TUPLES), "valarray<ld>");
#ifdef ANTIOCH_HAVE_EIGEN
  returnval = returnval ||
    vectester (Eigen::Array<float, 2*ANTIOCH_N_TUPLES, 1>(), "Eigen::ArrayXf");
  returnval = returnval ||
    vectester (Eigen::Array<double, 2*ANTIOCH_N_TUPLES, 1>(), "Eigen::ArrayXd");
  returnval = returnval ||
    vectester (Eigen::Array<long double, 2*ANTIOCH_N_TUPLES, 1>(), "Eigen::ArrayXld");
#endif
#ifdef ANTIOCH_HAVE_METAPHYSICL
  returnval = returnval ||
    vectester (MetaPhysicL::NumberArray<2*ANTIOCH_N_TUPLES, float> (0), "NumberArray<float>");
  returnval = returnval ||
    vectester (MetaPhysicL::NumberArray<2*ANTIOCH_N_TUPLES, double> (0), "NumberArray<double>");
  returnval = returnval ||
    vectester (MetaPhysicL::NumberArray<2*ANTIOCH_N_TUPLES, long double> (0), "NumberArray<ld>");
#endif
#ifdef ANTIOCH_HAVE_VEXCL
  vex::Context ctx_f (vex::Filter::All);
  if (!ctx_f.empty())
    returnval = returnval ||
      vectester (vex::vector<float> (ctx_f, 2*ANTIOCH_N_TUPLES), "vex::vector<float>");

  vex::Context ctx_d (vex::Filter::DoublePrecision);
  if (!ctx_d.empty())
    returnval = returnval ||
      vectester (vex::vector<double> (ctx_d, 2*ANTIOCH_N_TUPLES), "vex::vector<double>");
#endif

#ifdef ANTIOCH_HAVE_GRVY
  gt.Finalize();
  gt.Summarize();
#endif

  return returnval;
}
