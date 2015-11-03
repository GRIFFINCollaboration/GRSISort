#include "TEfficiencyCal.h"

ClassImp(TEfficiencyCal)

TEfficiencyCal::TEfficiencyCal(){
}

TEfficiencyCal::~TEfficiencyCal(){}

TEfficiencyCal::TEfficiencyCal(const TEfficiencyCal& copy) : TCal(copy) {
   copy.Copy(*this);
}

void TEfficiencyCal::Copy(TObject& obj) const {
	static_cast<TEfficiencyCal&>(obj).fScaleFactor = fScaleFactor; 
   TCal::Copy(obj);
}

void TEfficiencyCal::ScaleGraph(Double_t scaleFactor) {
   if(!GetN()){
      Error("ScaleGraph","Graph does not exist");
      return;
   }

   for(int i = 0; i < GetN(); i++){ 
      GetY()[i]  *= scaleFactor;
      GetEY()[i] *= scaleFactor;
   }

   fScaleFactor = scaleFactor;
}

void TEfficiencyCal::AddPoint(TPeak* peak){
   if(!peak){
      Error("AddPoint","Peak is empty");
      return;
   }
   AddPoint(peak->GetCentroid(),peak->GetArea(),peak->GetCentroidErr(),peak->GetAreaErr());
}

void TEfficiencyCal::AddPoint(Double_t energy, Double_t area, Double_t dEnergy, Double_t dArea){
   if(!GetNucleus()){
      Error("AddPoint", "No nucleus set");
      return;
   }
	//Will eventually write a method that doesn't need a nucleus

   Double_t efficiency,dEfficiency;
   Double_t intensity = 1.0;//nuc;

   efficiency = area/intensity;
 	dEfficiency = dArea/intensity;
   //dEfficiency = efficiency*TMath::Sqrt(TMath::Power(dEfficiency/efficiency,2.0) + TMath::Power(dArea/area,2.0));

   SetPoint(GetN(), energy, efficiency);
   SetPointError(GetN()-1,dEnergy,dEfficiency);

   Sort(); //This keeps the points in order of energy;
}


void TEfficiencyCal::Print(Option_t *opt) const {
   TCal::Print();
}

void TEfficiencyCal::Clear(Option_t *opt) {
   fScaleFactor = 1.0;
   TCal::Clear();
}

