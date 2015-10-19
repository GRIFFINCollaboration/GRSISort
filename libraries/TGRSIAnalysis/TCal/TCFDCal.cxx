#include "TCFDCal.h"

ClassImp(TCFDCal)

void TCFDCal::Clear(Option_t *opt) {
   fparameters.clear();
   TCal::Clear(opt);
}

void TCFDCal::WriteToChannel() const {
   if(!GetChannel()){
      Error("WriteToChannel","No Channel Set");
      return;
   }
   GetChannel()->DestroyCFDCal();
   printf("\nWriting to channel %d\n",GetChannel()->GetNumber());
   for(int i=0;i<(int)fparameters.size();i++){
      printf("p%i = %lf \t",i,fparameters.at(i));
      GetChannel()->AddCFDCoefficient(fparameters.at(i));
   }
}

void TCFDCal::AddParameter(Double_t param){
   fparameters.push_back(param);
}

void TCFDCal::SetParameters(std::vector<Double_t> paramvec){
   fparameters = paramvec;
}

void TCFDCal::SetParameter(Int_t idx, Double_t param){
/*   try {
	   fparameters.at(idx) = param;
   } 
   catch(const std::out_of_range& oor) {
      Error("SetParameter","Parameter %d does not exist yet",idx);
   } */
}

void TCFDCal::ReadFromChannel() {
   if(!GetChannel()){
      Error("ReadFromChannel","No Channel Set");
      return;
   }
   fparameters = GetChannel()->GetCFDCoeff();
}

void TCFDCal::Print(Option_t *opt) const{
   if(GetChannel())
      printf("Channel Number: %u\n",GetChannel()->GetNumber());
   else
      printf("Channel Number: NOT SET\n");

   for(int i=0;i<(int)fparameters.size();i++){
      printf("p%i = %lf \t",i,fparameters.at(i));
   }
}

std::vector<Double_t> TCFDCal::GetParameters() const{
  
   if(!fparameters.size())
      Error("GetParameters","No Parameters Set");

   return fparameters;
}

Double_t TCFDCal::GetParameter(Int_t parameter) const{
   if((size_t)parameter < fparameters.size())
      return fparameters.at(parameter);
   else{
      Error("Get Parameter","Parameter Does not exist");
      return 0;
   }
}
