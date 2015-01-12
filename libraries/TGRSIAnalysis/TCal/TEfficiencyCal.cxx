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

void TEfficiencyCal::SetFitFunction(void* fcn){
//Here is where we put the function pointer to the function used to fit the efficiency TGraph

}

Double_t TEfficiencyCal::GetParameter(Int_t parameter) const{
   //WILL NEED TO CHANGE THIS APPROPRIATELY
   return Graph()->GetFunction("efficiency")->GetParameter(parameter); //Root does all of the checking for us.
}

void TEfficiencyCal::Print(Option_t *opt) const {
   TCal::Print();
   printf("Coefficients:\n");
/*   std::vector<Double_t> parameters = GetParameters();
   for(Int_t i=0; i<parameters.size();i++){
      printf("Coefficient %d: %lf +/- %lf\n",i,fcoeffs[i],fdcoeffs[i]);
   }
*/
}

void TEfficiencyCal::Clear(Option_t *opt) {
   fscale_factor = 1.0;
   TCal::Clear();
}

