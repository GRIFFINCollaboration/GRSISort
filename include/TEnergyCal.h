#ifndef TENERGYCAL_H__
#define TENERGYCAL_H__

#include "TCal.h"
#include "TPeak.h"
#include "TSpectrum.h"

class TEnergyCal : public TCal {
 public: 
   TEnergyCal();
   TEnergyCal(const char* name, const char* title) : TCal(name,title) {}
   virtual ~TEnergyCal(); 

 public:
   std::vector<Double_t> GetParameters() const;
   Double_t GetParameter(size_t parameter) const;
   void WriteToChannel() const;

   void AddPoint(Double_t measured, Double_t accepted,Double_t measuredUncertainty = 0.0, Double_t acceptedUncertainty = 0.0);
   using TGraphErrors::SetPoint;
   using TGraphErrors::SetPointError;
   Bool_t SetPoint(Int_t idx, Double_t measured);
   Bool_t SetPoint(Int_t idx, TPeak* peak);
   Bool_t SetPointError(Int_t idx, Double_t measuredUncertainty);

   void SetNucleus(TNucleus* nuc, Option_t* opt = "");

   void Clear(Option_t* opt = "");
   void Print(Option_t* opt = "") const;
   void SetDefaultTitles();

   Bool_t IsGroupable() const {return true;}

 private:
   
/// \cond CLASSIMP
   ClassDef(TEnergyCal,1); //Class used for Energy Calibrations
/// \endcond
};

#endif
