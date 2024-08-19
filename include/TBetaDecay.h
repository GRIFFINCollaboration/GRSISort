#ifndef TBETADECAY_H
#define TBETADECAY_H

/** \addtogroup Fitting Fitting & Analysis
 *  @{
 */

#include <iostream>
#include <fstream>
#include <string>

#include "TNamed.h"
#include "TNucleus.h"

//////////////////////////////////////////////////////////////////
///
/// \class TBetaDecay
///
/// This class contains information about beta decays to be used
/// in analyses.
///
///////////////////////////////////////////////////////////////////

class TBetaDecay : public TNamed {
public:
   TBetaDecay() = default;
   explicit TBetaDecay(TNucleus* parent);
   explicit TBetaDecay(char* name);
   TBetaDecay(Int_t Z, Int_t N);
   ~TBetaDecay();

   TBetaDecay(const TBetaDecay&)            = delete;
   TBetaDecay(TBetaDecay&&)                 = delete;
   TBetaDecay& operator=(const TBetaDecay&) = delete;
   TBetaDecay& operator=(TBetaDecay&&)      = delete;

   TNucleus* GetParent() const { return fParent; }

private:
   Bool_t    fParentAllocated{false};   ///< true if TNucleus was instantiated in TBetaDecay
   TNucleus* fParent{nullptr};          ///< The parent nucleus beta decaying

   /// \cond CLASSIMP
   ClassDefOverride(TBetaDecay, 1)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
