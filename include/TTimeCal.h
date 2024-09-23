#ifndef TTIMECAL_H
#define TTIMECAL_H

/** \addtogroup Calibration
 *  @{
 */

#include "TCal.h"

class TTimeCal : public TCal {
public:
   TTimeCal() = default;
   TTimeCal(const char* name, const char* title) : TCal(name, title){};
   TTimeCal(const TTimeCal&)                = default;
   TTimeCal(TTimeCal&&) noexcept            = default;
   TTimeCal& operator=(const TTimeCal&)     = default;
   TTimeCal& operator=(TTimeCal&&) noexcept = default;
   ~TTimeCal()                              = default;

   // pure virtual functions
   Bool_t IsGroupable() const override { return false; }

   void                  WriteToChannel() const override;
   virtual void          ReadFromChannel();
   std::vector<Double_t> GetParameters() const override;
   Double_t              GetParameter(size_t parameter) const override;

   void AddParameter(Double_t param);
   void SetParameters(std::vector<Double_t> paramVec);
   void SetParameter(Int_t idx, Double_t param);

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

private:
   std::vector<Double_t> fParameters;

   /// \cond CLASSIMP
   ClassDefOverride(TTimeCal, 1)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
