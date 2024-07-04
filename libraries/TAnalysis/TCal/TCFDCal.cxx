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
   if(GetChannel() == nullptr) {
      Error("WriteToChannel", "No Channel Set");
      return;
   }
   GetChannel()->DestroyCFDCal();
   std::cout << std::endl
             << "Writing to channel " << GetChannel()->GetNumber() << std::endl;
   for(int i = 0; i < static_cast<int>(fParameters.size()); i++) {
      std::cout << "p" << i << " = " << fParameters[i] << "\t ";
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
   if(GetChannel() == nullptr) {
      Error("ReadFromChannel", "No Channel Set");
      return;
   }
   fParameters = GetChannel()->GetCFDCoeff();
}

void TCFDCal::Print(Option_t*) const
{
   if(GetChannel() != nullptr) {
      std::cout << "Channel Number: " << GetChannel()->GetNumber() << std::endl;
   } else {
      std::cout << "Channel Number: NOT SET" << std::endl;
   }

   for(int i = 0; i < static_cast<int>(fParameters.size()); i++) {
      std::cout << "p" << i << " = " << fParameters[i] << "\t ";
   }
}

std::vector<Double_t> TCFDCal::GetParameters() const
{
   if(fParameters.empty()) {
      Error("GetParameters", "No Parameters Set");
   }

   return fParameters;
}

Double_t TCFDCal::GetParameter(size_t parameter) const
{
   if(parameter < fParameters.size()) {
      return fParameters[parameter];
   }
   Error("Get Parameter", "Parameter Does not exist");
   return 0.;
}
