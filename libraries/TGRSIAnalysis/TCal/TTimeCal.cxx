#include "TTimeCal.h"

/// \cond CLASSIMP
ClassImp(TTimeCal)
/// \endcond

void TTimeCal::Clear(Option_t *opt) {
   fParameters.clear();
   TCal::Clear(opt);
}

void TTimeCal::WriteToChannel() const {
   if(!GetChannel()) {
      Error("WriteToChannel","No Channel Set");
      return;
   }
   GetChannel()->DestroyTIMECal();
   printf("\nWriting to channel %d\n",GetChannel()->GetNumber());
   for(int i = 0; i < static_cast<int>(fParameters.size()); i++) {
      printf("p%i = %lf \t", i, fParameters[i]);
      GetChannel()->AddTIMECoefficient(fParameters[i]);
   }
}

void TTimeCal::AddParameter(Double_t param) {
   fParameters.push_back(param);
}

void TTimeCal::SetParameters(std::vector<Double_t> paramVec) {
   fParameters = paramVec;
}

void TTimeCal::SetParameter(Int_t idx, Double_t param) {
/*   try {
	   fParameters.at(idx) = param;
   } 
   catch(const std::out_of_range& oor) {
      Error("SetParameter","Parameter %d does not exist yet",idx);
   } */
}

void TTimeCal::ReadFromChannel() {
   if(!GetChannel()) {
      Error("ReadFromChannel","No Channel Set");
      return;
   }
   fParameters = GetChannel()->GetTIMECoeff();
}

void TTimeCal::Print(Option_t *opt) const {
   if(GetChannel())
      printf("Channel Number: %u\n",GetChannel()->GetNumber());
   else
      printf("Channel Number: NOT SET\n");

   for(int i = 0; i < static_cast<int>(fParameters.size()); i++) {
      printf("p%i = %lf \t",i,fParameters[i]);
   }
}

std::vector<Double_t> TTimeCal::GetParameters() const {
   if(!fParameters.size())
      Error("GetParameters","No Parameters Set");

   return fParameters;
}

Double_t TTimeCal::GetParameter(size_t parameter) const {
   if(parameter < fParameters.size()) 
      return fParameters[parameter];
   else {
      Error("Get Parameter","Parameter Does not exist");
      return 0;
   }
}

