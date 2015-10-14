#include "TTimeCal.h"

ClassImp(TTimeCal)

void TTimeCal::Clear(Option_t *opt) {
   fparameters.clear();
   TCal::Clear(opt);
}

void TTimeCal::WriteToChannel() const {
   if(!GetChannel()){
      Error("WriteToChannel","No Channel Set");
      return;
   }
   GetChannel()->DestroyTIMECal();
   printf("\nWriting to channel %d\n",GetChannel()->GetNumber());
   for(int i=0;i<(int)fparameters.size();i++){
      printf("p%i = %lf \t",i,fparameters.at(i));
      GetChannel()->AddTIMECoefficient(fparameters.at(i));
   }
}

void TTimeCal::AddParameter(Double_t param){
   fparameters.push_back(param);
}

void TTimeCal::SetParameters(std::vector<Double_t> paramvec){
   fparameters = paramvec;
}

void TTimeCal::SetParameter(Int_t idx, Double_t param){
/*   try {
	   fparameters.at(idx) = param;
   } 
   catch(const std::out_of_range& oor) {
      Error("SetParameter","Parameter %d does not exist yet",idx);
   } */
}

void TTimeCal::ReadFromChannel() {
   if(!GetChannel()){
      Error("ReadFromChannel","No Channel Set");
      return;
   }
   fparameters = GetChannel()->GetTIMECoeff();
}

void TTimeCal::Print(Option_t *opt) const{
   if(GetChannel())
      printf("Channel Number: %u\n",GetChannel()->GetNumber());
   else
      printf("Channel Number: NOT SET\n");

   for(int i=0;i<(int)fparameters.size();i++){
      printf("p%i = %lf \t",i,fparameters.at(i));
   }
}

std::vector<Double_t> TTimeCal::GetParameters() const{
  
   if(!fparameters.size())
      Error("GetParameters","No Parameters Set");

   return fparameters;
}

Double_t TTimeCal::GetParameter(UInt_t parameter) const{
   if(parameter < fparameters.size() )
      return fparameters.at(parameter);
   else{
      Error("Get Parameter","Parameter Does not exist");
      return 0;
   }
}
