// Author: Ryan Dunlop    10/16

/** \addtogroup GRSIProof
 *  @{
 */

#ifndef TOBJECTWRAPPER_H
#define TOBJECTWRAPPER_H

#include "Globals.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <ctime>
#include <fstream>
#include <streambuf>

#include "TFragment.h"

/////////////////////////////////////////////////////////////////
///
/// \class TObjectWrapper
///
/// This Class allows proof to work with GRSI
///
///
/////////////////////////////////////////////////////////////////

template <typename T>
class TObjectWrapper : public TObject {
public:
   explicit TObjectWrapper(T* ptr) : pT(ptr) {}
   T* operator->() { return pT; }

private:
   T* pT;

   /// \cond CLASSIMP
   ClassDefOverride(TObjectWrapper, 1)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
