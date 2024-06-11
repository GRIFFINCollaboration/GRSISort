#include <utility>

#include "TTimeCal.h"

/// \cond CLASSIMP
ClassImp(TTimeCal)
   /// \endcond

   void TTimeCal::Clear(Option_t* opt)
{
   fParameters.clear();
   TCal::Clear(opt);
}

void TTimeCal::WriteToChannel() const
{
   if(GetChannel() == nullptr) {
      Error("WriteToChannel", "No Channel Set");
      return;
   }
   GetChannel()->DestroyTIMECal();
   std::cout<<std::endl
            <<"Writing to channel "<<GetChannel()->GetNumber()<<std::endl;
   for(int i = 0; i < static_cast<int>(fParameters.size()); i++) {
      std::cout<<"p"<<i<<" = "<<fParameters[i]<<"\t ";
      GetChannel()->AddTIMECoefficient(fParameters[i]);
   }
}

void TTimeCal::AddParameter(Double_t param)
{
   fParameters.push_back(param);
}

void TTimeCal::SetParameters(std::vector<Double_t> paramVec)
{
   fParameters = std::move(paramVec);
}

void TTimeCal::SetParameter(Int_t, Double_t)
{
}

void TTimeCal::ReadFromChannel()
{
   if(GetChannel() == nullptr) {
      Error("ReadFromChannel", "No Channel Set");
      return;
   }
   fParameters = GetChannel()->GetTIMECoeff();
}

void TTimeCal::Print(Option_t*) const
{
   if(GetChannel() != nullptr) {
      std::cout<<"Channel Number: "<<GetChannel()->GetNumber()<<std::endl;
   } else {
      std::cout<<"Channel Number: NOT SET"<<std::endl;
   }

   for(int i = 0; i < static_cast<int>(fParameters.size()); i++) {
      std::cout<<"p"<<i<<" = "<<fParameters[i]<<"\t ";
   }
}

std::vector<Double_t> TTimeCal::GetParameters() const
{
   if(fParameters.empty()) {
      Error("GetParameters", "No Parameters Set");
   }

   return fParameters;
}

Double_t TTimeCal::GetParameter(size_t parameter) const
{
   if(parameter < fParameters.size()) {
      return fParameters[parameter];
   }
   Error("Get Parameter", "Parameter Does not exist");
   return 0;
}
