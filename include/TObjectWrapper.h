// Author: Ryan Dunlop    10/16

/** \addtogroup GRSIProof
 *  @{
 */

#ifndef TOBJECTWRAPPER_H
#define TOBJECTWRAPPER_H

#include <ctime>

#include "TObject.h"

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
   T* pT{nullptr};

   /// \cond CLASSIMP
   ClassDefOverride(TObjectWrapper, 1)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
