#include "TEnergyCal.h"

ClassImp(TEnergyCal)

TEnergyCal::TEnergyCal(){
   SetDefaultTitles();
}

TEnergyCal::~TEnergyCal(){}

void TEnergyCal::SetDefaultTitles(){
   this->SetTitle("Energy Calibration");
   this->GetYaxis()->SetTitle("Accepted Energy (keV)");
   this->GetXaxis()->SetTitle("Measured Centroid");
   this->GetYaxis()->CenterTitle();
   this->GetXaxis()->CenterTitle();
}


std::vector<Double_t> TEnergyCal::GetParameters() const{
   //WILL NEED TO CHANGE THIS APPROPRIATELY
   std::vector<Double_t> paramlist;
   Int_t nparams = this->GetFunction("energy")->GetNpar();

   for(int i=0;i<nparams;i++)
      paramlist.push_back(GetParameter(i));

   return paramlist;
}

Double_t TEnergyCal::GetParameter(Int_t parameter) const{
   //WILL NEED TO CHANGE THIS APPROPRIATELY
   return GetFunction("gain")->GetParameter(parameter); //Root does all of the checking for us.
}

void TEnergyCal::SetNucleus(TNucleus* nuc,Option_t *opt){
   TString optstr = opt;
   optstr.ToUpper();
   if(!GetNucleus() || optstr.Contains("F")){ 
      TGraphErrors::Clear();
      TCal::SetNucleus(nuc);
      for(int i = 0; i< GetNucleus()->NTransitions();i++){
         TGraphErrors::SetPoint(i,0.0,GetNucleus()->GetTransition(i)->GetEnergy());
         TGraphErrors::SetPointError(i,0.0,GetNucleus()->GetTransition(i)->GetEnergyUncertainty());
      }
   }
   else if(GetNucleus())
      printf("Nucleus already exists. Use \"F\" option to overwrite\n");

   SetDefaultTitles();
   this->Sort();
}

void TEnergyCal::AddPoint(Double_t measured, Double_t accepted, Double_t measured_uncertainty, Double_t accepted_uncertainty){
   Int_t point = this->GetN();
   TGraphErrors::SetPoint(point,measured,accepted);
   TGraphErrors::SetPointError(point,measured_uncertainty,accepted_uncertainty);
   this->Sort();
}

Bool_t TEnergyCal::SetPoint(Int_t idx, Double_t measured){
   if(!GetNucleus()){
      printf("No nucleus set yet...\n");
      return false;
   }

   Double_t x,y;
   this->GetPoint(idx,x,y);
   TGraphErrors::SetPoint(idx,measured,y);
   this->Sort();

   return true;
}

Bool_t TEnergyCal::SetPoint(Int_t idx, TPeak *peak){
   if(!peak){
      printf("No Peak, pointer is null\n");
      return false;
   }
   Double_t centroid = peak->GetCentroid();
   Double_t d_centroid = peak->GetCentroidErr();

   SetPoint(idx,centroid);
   return SetPointError(idx,d_centroid);


}

Bool_t TEnergyCal::SetPointError(Int_t idx, Double_t measured_uncertainty){
   if(!GetNucleus()){
      printf("No nucleus set yet...\n");
      return false;
   }

   TGraphErrors::SetPointError(idx,measured_uncertainty,GetErrorX(idx));
   Sort();

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
   TGraphErrors::Print();
}

void TEnergyCal::Clear(Option_t *opt) {
   TCal::Clear();
   SetDefaultTitles();
}

