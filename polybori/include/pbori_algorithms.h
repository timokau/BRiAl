// -*- c++ -*-
//*****************************************************************************
/** @file pbori_algorithm.h
 *
 * @author Alexander Dreyer
 * @date 2007-05-11
 *
 * This file contains algorithmic procedures, which can be used together with
 * PolyBoRi classes. 
 * @note This file depends on several PolyBoRi classes, so it should
 * not be included by PolyBoRi headers (excl. polybori.h), because this would
 * lead to curious dependencies. Several templates, which do not depend on
 * PolyBoRi classes can be found in pbori_algo.h.
 *
 * @par Copyright:
 *   (c) 2007 by
 *   Dep. of Mathematics, Kaiserslautern University of Technology and @n
 *   Fraunhofer Institute for Industrial Mathematics (ITWM)
 *   D-67663 Kaiserslautern, Germany
 *
 * @internal 
 * @version \$Id$
 *
 * @par History:
 * @verbatim
 * $Log$
 * Revision 1.3  2007/05/18 11:48:39  dreyer
 * ADD: sophisticated term_accumulate
 *
 * Revision 1.2  2007/05/14 16:09:40  dreyer
 * Fix: removed infinite recursion in specialized case
 *
 * Revision 1.1  2007/05/11 11:38:42  dreyer
 * ADD: started pbori_algorithms.h and term_accumulation()
 *
 * @endverbatim
**/
//*****************************************************************************

// include standard headers
#include <numeric>

// include basic definitions
#include "pbori_defs.h"

// include PolyBoRi algorithm, which do not depend on PolyBoRi classes
#include "pbori_algo.h"

// include PolyBoRi class definitions, which are used here
#include "BoolePolynomial.h"
#include "BooleMonomial.h"
#include "CGenericIter.h"


BEGIN_NAMESPACE_PBORI

/// @func spoly
/// Compute spoly of two polynomials
inline BoolePolynomial 
spoly(const BoolePolynomial& first, const BoolePolynomial& second){

   BooleMonomial lead1(first.lead()), lead2(second.lead());

   BooleMonomial prod = lead1;
   prod *= lead2;

   return ( first * (prod / lead1) ) + ( second * (prod / lead2) );
}

template <class NaviType, class LowerIterator, class ValueType>
ValueType 
lower_term_accumulate(NaviType navi, 
                      LowerIterator lstart, LowerIterator lfinish, 
                      ValueType init) {
  if (lstart == lfinish){
    return BoolePolynomial(false);
  }
  
  if (navi.isConstant())
    return BooleSet(navi);
  
  assert(*lstart >= *navi);

  ValueType resthen, reselse;
  if (*lstart > *navi) {
    resthen = navi.thenBranch();
    reselse = lower_term_accumulate(navi.elseBranch(), lstart, lfinish, init);
  }
  else  {
    assert(*lstart == *navi);
    ++lstart;
    resthen = lower_term_accumulate(navi.thenBranch(), lstart, lfinish, init);
    reselse = BoolePolynomial(false).navigation();
  }

  return  BooleSet(*navi, resthen.navigation(), reselse.navigation());
}


template <class UpperIterator, class NaviType, class ValueType>
ValueType 
upper_term_accumulate(UpperIterator ustart, UpperIterator ufinish,
                        NaviType navi, 
                        ValueType init) {

   if (ustart == ufinish){
     return true;
   }

   while (*navi < *ustart)
     navi.incrementElse();
   ++ustart;

   ValueType resthen = upper_term_accumulate(ustart, ufinish,
                                             navi.thenBranch(), init);

   ValueType reselse = navi.elseBranch();

   return BooleSet(*navi, resthen.navigation(), reselse.navigation());

}

///@note: assuming lstart .. lfinish not marking the term one
template <class UpperIterator, class NaviType, class LowerIterator, 
          class ValueType>
ValueType 
term_accumulate(UpperIterator ustart, UpperIterator ufinish, NaviType navi, 
                LowerIterator lstart, LowerIterator lfinish, ValueType init) {

  if (ustart == ufinish)
    return true;

  if (lstart == lfinish)
    return upper_term_accumulate(ustart, ufinish, navi, init);

  while (*navi < *ustart)
    navi.incrementElse();
  ++ustart;
  

  
  if (navi.isConstant())
    return BooleSet(navi);

  assert(*lstart >= *navi);

  ValueType resthen, reselse;
  if (*lstart > *navi) {
    resthen = upper_term_accumulate(ustart, ufinish, navi.thenBranch(), init);
    reselse = lower_term_accumulate(navi.elseBranch(), lstart, lfinish, init);
  }
  else  {
    assert(*lstart == *navi);
    ++lstart;
    resthen = term_accumulate(ustart, ufinish, navi.thenBranch(),
                              lstart, lfinish, init);
    reselse = BoolePolynomial(false).navigation() ;
  }

  return BooleSet(*navi, resthen.navigation(), reselse.navigation());
}




/// @func term_accumulate
/// Routine for adding all terms given by iterators
template <class InputIterator, class ValueType>
ValueType 
term_accumulate(InputIterator first, InputIterator last, ValueType init) {

  if(last.isOne())
    return upper_term_accumulate(first.begin(), first.end(), 
                                 first.navigation(), init) + ValueType(1);

  ValueType result = term_accumulate(first.begin(), first.end(), 
                                     first.navigation(),
                                     last.begin(), last.end(), init);

  assert(result == std::accumulate(first, last, init) ); 

  return result;
}



END_NAMESPACE_PBORI