#ifndef TCFDCAL_H
#define TCFDCAL_H

/** \addtogroup Calibration
 *  @{
 */

#include "TCal.h"

class TCFDCal : public TCal {
public:
   TCFDCal() = default;
   TCFDCal(const char* name, const char* title) : TCal(name, title){};
	TCFDCal(const TCFDCal&) = default;
	TCFDCal(TCFDCal&&) noexcept = default;
	TCFDCal& operator=(const TCFDCal&) = default;
	TCFDCal& operator=(TCFDCal&&) noexcept = default;
   ~TCFDCal() = default;

   // pure virtual functions
   Bool_t IsGroupable() const override { return false; }

   void                  WriteToChannel() const override;
   virtual void          ReadFromChannel();
   std::vector<Double_t> GetParameters() const override;
   Double_t              GetParameter(size_t parameter) const override;

   void AddParameter(Double_t param);
   void SetParameters(std::vector<Double_t> paramvec);
   void SetParameter(Int_t idx, Double_t param);

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

private:
   std::vector<Double_t> fParameters;

   /// \cond CLASSIMP
   ClassDefOverride(TCFDCal, 1) // NOLINT
   /// \endcond
};
/*! @} */
#endif
