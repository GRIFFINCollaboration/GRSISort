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

void TEnergyCal::AddPoint(Double_t measured, Double_t accepted){



}

void TEnergyCal::WriteToChannel() const {
   if(!GetChannel()){
      Error("WriteToChannel","No Channel Set");
      return;
   }
   GetChannel()->DestroyENGCal();
   printf("Writing to channel %d\n",GetChannel()->GetNumber());
   printf("p0 = %lf \t p1 = %lf\n",this->GetParameter(0),this->GetParameter(1));
   //Set the energy parameters based on the fitted calibration.
   GetChannel()->AddENGCoefficient(this->GetParameter(0));
   GetChannel()->AddENGCoefficient(this->GetParameter(1));
}

void TEnergyCal::Print(Option_t *opt) const {
   TCal::Print();
}

void TEnergyCal::Clear(Option_t *opt) {
   TCal::Clear();
}

