#include "TEfficiencyCal.h"

ClassImp(TEfficiencyCal)

TEfficiencyCal::TEfficiencyCal(){}

TEfficiencyCal::~TEfficiencyCal(){}

std::vector<Double_t> TEfficiencyCal::GetParameters() const{
   //WILL NEED TO CHANGE THIS APPROPRIATELY
   std::vector<Double_t> paramlist;
   Int_t nparams = this->Graph()->GetFunction("efficiency")->GetNpar();

   for(int i=0;i<nparams;i++)
      paramlist.push_back(GetParameter(i));

   return paramlist;
}

Double_t TEfficiencyCal::GetParameter(Int_t parameter) const{
   //WILL NEED TO CHANGE THIS APPROPRIATELY
   return Graph()->GetFunction("efficiency")->GetParameter(parameter); //Root does all of the checking for us.
}

void TEfficiencyCal::Print(Option_t *opt) const {
   TCal::Print();
}

void TEfficiencyCal::Clear() {
   TCal::Clear();
}

