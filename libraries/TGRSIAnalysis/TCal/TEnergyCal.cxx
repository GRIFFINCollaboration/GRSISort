#include "TEnergyCal.h"

ClassImp(TEnergyCal)

TEnergyCal::TEnergyCal(){}

TEnergyCal::~TEnergyCal(){}

std::vector<Double_t> TEnergyCal::GetParameters() const{
   //WILL NEED TO CHANGE THIS APPROPRIATELY
   std::vector<Double_t> paramlist;
   Int_t nparams = this->Graph()->GetFunction("energy")->GetNpar();

   for(int i=0;i<nparams;i++)
      paramlist.push_back(GetParameter(i));

   return paramlist;
}

Double_t TEnergyCal::GetParameter(Int_t parameter) const{
   //WILL NEED TO CHANGE THIS APPROPRIATELY
   return Graph()->GetFunction("gain")->GetParameter(parameter); //Root does all of the checking for us.
}

void TEnergyCal::Print(Option_t *opt) const {
   TCal::Print();
}

void TEnergyCal::Clear() {
   TCal::Clear();
}

