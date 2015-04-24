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

void TEnergyCal::SetNucleus(TNucleus* nuc){
   TCal::SetNucleus(nuc);
   for(int i = 0; i< GetNucleus()->NTransitions();i++){
      Graph()->SetPoint(i,GetNucleus()->GetTransition(i)->GetEnergy(),0.0);
      Graph()->SetPointError(i,GetNucleus()->GetTransition(i)->GetEnergyUncertainty(),0.0);
   }
}

void TEnergyCal::AddPoint(Double_t measured, Double_t accepted){



}

Bool_t TEnergyCal::AddPoint(Int_t idx, Double_t measured){
   if(!GetNucleus()){
      printf("No nucleus set yet...\n");
      return false;
   }

   return true;
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

