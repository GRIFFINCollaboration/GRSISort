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

void TEnergyCal::SetNucleus(TNucleus* nuc,Option_t *opt){
   TString optstr = opt;
   optstr.ToUpper();
   if(!GetNucleus() || optstr.Contains("F")){ 
      Graph()->Clear();
      TCal::SetNucleus(nuc);
      for(int i = 0; i< GetNucleus()->NTransitions();i++){
         Graph()->SetPoint(i,GetNucleus()->GetTransition(i)->GetEnergy(),0.0);
         Graph()->SetPointError(i,GetNucleus()->GetTransition(i)->GetEnergyUncertainty(),0.0);
      }
   }
   else if(GetNucleus())
      printf("Nucleus already exists. Use \"F\" option to overwrite\n");

   Graph()->Sort();
}

void TEnergyCal::AddPoint(Double_t measured, Double_t accepted, Double_t measured_uncertainty, Double_t accepted_uncertainty){
   Int_t point = Graph()->GetN();
   Graph()->SetPoint(point,accepted,measured);
   Graph()->SetPointError(point,accepted_uncertainty,measured_uncertainty);
   Graph()->Sort();
}

Bool_t TEnergyCal::SetPoint(Int_t idx, Double_t measured){
   if(!GetNucleus()){
      printf("No nucleus set yet...\n");
      return false;
   }

   Double_t x,y;
   Graph()->GetPoint(idx,x,y);
   Graph()->SetPoint(idx,x,measured);
   Graph()->Sort();

   return true;
}

Bool_t TEnergyCal::SetPointError(Int_t idx, Double_t measured_uncertainty){
   if(!GetNucleus()){
      printf("No nucleus set yet...\n");
      return false;
   }

   Graph()->SetPointError(idx,Graph()->GetErrorX(idx),measured_uncertainty);
   Graph()->Sort();

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
   Graph()->Print();
}

void TEnergyCal::Clear(Option_t *opt) {
   TCal::Clear();
}

