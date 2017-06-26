#include <utility>

#include "TCFDCal.h"

/// \cond CLASSIMP
ClassImp(TCFDCal)
   /// \endcond

   void TCFDCal::Clear(Option_t* opt)
{
   fParameters.clear();
   TCal::Clear(opt);
}

void TCFDCal::WriteToChannel() const
{
   if(!GetChannel()) {
      Error("WriteToChannel", "No Channel Set");
      return;
   }
   GetChannel()->DestroyCFDCal();
   printf("\nWriting to channel %d\n", GetChannel()->GetNumber());
   for(int i = 0; i < static_cast<int>(fParameters.size()); i++) {
      printf("p%i = %lf \t", i, fParameters[i]);
      GetChannel()->AddCFDCoefficient(fParameters[i]);
   }
}

void TCFDCal::AddParameter(Double_t param)
{
   fParameters.push_back(param);
}

void TCFDCal::SetParameters(std::vector<Double_t> paramvec)
{
   fParameters = std::move(paramvec);
}

void TCFDCal::SetParameter(Int_t, Double_t)
{
}

void TCFDCal::ReadFromChannel()
{
   if(!GetChannel()) {
      Error("ReadFromChannel", "No Channel Set");
      return;
   }
   fParameters = GetChannel()->GetCFDCoeff();
}

void TCFDCal::Print(Option_t*) const
{
   if(GetChannel()) {
      printf("Channel Number: %u\n", GetChannel()->GetNumber());
   } else {
      printf("Channel Number: NOT SET\n");
   }

   for(int i = 0; i < static_cast<int>(fParameters.size()); i++) {
      printf("p%i = %lf \t", i, fParameters[i]);
   }
}

std::vector<Double_t> TCFDCal::GetParameters() const
{
   if(fParameters.size() == 0) {
      Error("GetParameters", "No Parameters Set");
   }

   return fParameters;
}

Double_t TCFDCal::GetParameter(size_t parameter) const
{
   if(parameter < fParameters.size()) {
      return fParameters[parameter];
   } else {
      Error("Get Parameter", "Parameter Does not exist");
      return 0.;
   }
}
