#include "TEfficiencyCal.h"

ClassImp(TEfficiencyCal)

TEfficiencyCal::TEfficiencyCal(){
}

TEfficiencyCal::~TEfficiencyCal(){}

TEfficiencyCal::TEfficiencyCal(const TEfficiencyCal &copy) : TCal(copy){
   ((TEfficiencyCal&)copy).Copy(*this);
}

void TEfficiencyCal::Copy(TObject &obj) const{
   ((TEfficiencyCal&)obj).fscale_factor = fscale_factor; 
   TCal::Copy(obj);
}

void TEfficiencyCal::ScaleGraph(Double_t scale_factor){
   if(!Graph()){
      Error("ScaleGraph","Graph does not exist");
      return;
   }
   for(int i=0;i<Graph()->GetN();i++){ 
      Graph()->GetY()[i] *= scale_factor;
      Graph()->GetEY()[i] *=scale_factor;
   }

   fscale_factor = scale_factor;
}

void TEfficiencyCal::AddPoint(TPeak* peak){
   if(!peak){
      Error("AddPoint","Peak is empty");
      return;
   }
   AddPoint(peak->GetCentroid(),peak->GetArea(),peak->GetCentroidErr(),peak->GetAreaErr());
}

void TEfficiencyCal::AddPoint(Double_t energy, Double_t area, Double_t d_energy, Double_t d_area){
   if(!GetNucleus()){
      Error("AddPoint", "No nucleus set");
      return;
   }
//Will eventually write a method that doesn't need a nucleus

   Double_t efficiency,d_efficiency;
   Double_t intensity = 1.0;//nuc;
   Double_t d_intensity = 0.1;

   efficiency = area/intensity;
   d_efficiency = efficiency*TMath::Sqrt(TMath::Power(d_efficiency/efficiency,2.0) + TMath::Power(d_area/area,2.0));

   Graph()->SetPoint(Graph()->GetN(), energy, efficiency);
   Graph()->SetPointError(Graph()->GetN()-1,d_energy,d_efficiency);

   Graph()->Sort(); //This keeps the points in order of energy;

}


void TEfficiencyCal::Print(Option_t *opt) const {
   TCal::Print();
}

void TEfficiencyCal::Clear(Option_t *opt) {
   fscale_factor = 1.0;
   TCal::Clear();
}

