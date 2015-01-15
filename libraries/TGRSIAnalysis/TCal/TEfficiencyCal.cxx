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

void TEfficiencyCal::Print(Option_t *opt) const {
   TCal::Print();
}

void TEfficiencyCal::Clear(Option_t *opt) {
   fscale_factor = 1.0;
   TCal::Clear();
}

