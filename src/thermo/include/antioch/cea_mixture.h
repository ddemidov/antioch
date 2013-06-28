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

#ifndef ANTIOCH_CEA_MIXTURE_H
#define ANTIOCH_CEA_MIXTURE_H

// Antioch
#include "antioch/cea_curve_fit.h"
#include "antioch/chemical_mixture.h"
#include "antioch/chemical_species.h"
#include "antioch/input_utils.h"
#include "antioch/temp_cache.h"
#include "antioch/cea_evaluator.h"

// C++
#include <iomanip>
#include <vector>
#include <cmath>

namespace Antioch
{
  
  template<typename CoeffType> class CEAEvaluator;

  template<typename CoeffType=double>
  class CEAThermoMixture
  {
  public:

    CEAThermoMixture( const ChemicalMixture<CoeffType>& chem_mixture );

    //! Destructor
    /*! virtual so this can be subclassed by the user. */
    virtual ~CEAThermoMixture();

    void add_curve_fit( const std::string& species_name, const std::vector<CoeffType>& coeffs );

    const CEACurveFit<CoeffType>& curve_fit( unsigned int s ) const;

    CoeffType cp_at_200p1( unsigned int s ) const;

    //! Checks that curve fits have been specified for all species in the mixture.
    bool check() const;

    const ChemicalMixture<CoeffType>& chemical_mixture() const;
    
  protected:

    const ChemicalMixture<CoeffType>& _chem_mixture;

    std::vector<CEACurveFit<CoeffType>* > _species_curve_fits;

    std::vector<CoeffType> _cp_at_200p1;

  private:
    
    //! Default constructor
    /*! Private to force to user to supply a ChemicalMixture object.*/
    CEAThermoMixture();

  };

  /* --------------------- Constructor/Destructor -----------------------*/
  template<typename CoeffType>
  CEAThermoMixture<CoeffType>::CEAThermoMixture( const ChemicalMixture<CoeffType>& chem_mixture )
    : _chem_mixture(chem_mixture),
      _species_curve_fits(chem_mixture.n_species(), NULL),
      _cp_at_200p1( _species_curve_fits.size() )
  {
    return;
  }


  template<typename CoeffType>
  CEAThermoMixture<CoeffType>::~CEAThermoMixture()
  {
    // Clean up all the CEACurveFits we created
    for( typename std::vector<CEACurveFit<CoeffType>* >::iterator it = _species_curve_fits.begin();
	 it < _species_curve_fits.end(); ++it )
      {
	delete (*it);
      }

    return;
  }

  /* ------------------------- Inline Functions -------------------------*/
  template<typename CoeffType>
  inline
  void CEAThermoMixture<CoeffType>::add_curve_fit( const std::string& species_name,
                                                   const std::vector<CoeffType>& coeffs )
  {
    antioch_assert( _chem_mixture.active_species_name_map().find(species_name) !=
		    _chem_mixture.active_species_name_map().end() );

    unsigned int s = _chem_mixture.active_species_name_map().find(species_name)->second;

    antioch_assert_less_equal( s, _species_curve_fits.size() );
    antioch_assert( !_species_curve_fits[s] );

    _species_curve_fits[s] = new CEACurveFit<CoeffType>(coeffs);

    CEAEvaluator<CoeffType> evaluator( *this );
    _cp_at_200p1[s] = evaluator.cp( TempCache<CoeffType>(200.1), s );

    return;
  }


  template<typename CoeffType>
  inline
  bool CEAThermoMixture<CoeffType>::check() const
  {
    bool valid = true;

    for( typename std::vector<CEACurveFit<CoeffType>* >::const_iterator it = _species_curve_fits.begin();
	 it != _species_curve_fits.end(); ++ it )
      {
	if( !(*it) )
	  valid = false;
      }

    return valid;
  }

  template<typename CoeffType>
  inline
  const CEACurveFit<CoeffType>& CEAThermoMixture<CoeffType>::curve_fit( unsigned int s ) const
  {
    antioch_assert_less( s, _species_curve_fits.size() );
    return *_species_curve_fits[s];
  }

  template<typename CoeffType>
  inline
  CoeffType CEAThermoMixture<CoeffType>::cp_at_200p1( unsigned int s ) const
  {
    antioch_assert_less( s, _cp_at_200p1.size() );
    return _cp_at_200p1[s];
  }

  template<typename CoeffType>
  inline
  const ChemicalMixture<CoeffType>& CEAThermoMixture<CoeffType>::chemical_mixture() const
  {
    return _chem_mixture;
  }

} // end namespace Antioch

#endif // ANTIOCH_CEA_MIXTURE_H
