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
   TBetaDecay();
   TBetaDecay(TNucleus* parent);
   TBetaDecay(char* name);
   TBetaDecay(Int_t Z, Int_t N);
   ~TBetaDecay() override;

public:
   TNucleus* GetParent() const { return fParent; }

private:
   Bool_t    fParentAllocated;   ///< true if TNucleus was instantiated in TBetaDecay
   TNucleus* fParent;            ///< The parent nucleus beta decaying

   /// \cond CLASSIMP
   ClassDefOverride(TBetaDecay, 1);   // Information about beta decays
   /// \endcond
};
/*! @} */
#endif
