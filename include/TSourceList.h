#ifndef TSOURCECALLIST_H__
#define TSOURCECALLIST_H__

/** \addtogroup Calibration
 *  @{
 */

#include "TNamed.h"
#include "TCalList.h"
#include "TGraphErrors.h"
#include "TNucleus.h"

#include <string>

class TSourceList : public TCalList {
public:
   TSourceList();
   TSourceList(const TNucleus& nuc, const char* name, const char* title = "");
   explicit TSourceList(const char* name, const char* title = "");
   explicit TSourceList(const TNucleus& nuc);
   ~TSourceList() override = default;

   TSourceList(const TSourceList& copy);

   void Copy(TObject& obj) const override;
   void Clear(Option_t* opt = "") override;
   void Print(Option_t* opt = "") const override;

   Int_t SetNucleus(const TNucleus& nuc);
   Int_t SetNucleus(const char* nuc) { return SetNucleus(TNucleus(nuc)); }

private:
   bool AddTransition(TTransition* tran);

   std::string fNucleusName;

   /// \cond CLASSIMP
   ClassDefOverride(TSourceList, 1) // NOLINT
   /// \endcond
};
/*! @} */
#endif
