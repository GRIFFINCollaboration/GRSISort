#include "TSinglePeak.h"
#include "TClass.h"

/// \cond CLASSIMP
ClassImp(TSinglePeak)
/// \endcond

bool TSinglePeak::IsBackgroundParameter(const Int_t& par) const
{
   try {
      return fListOfBGPars.at(par);
   } catch(const std::out_of_range& oor) {
      std::cerr << "Parameter not in list: " << par << std::endl;
      return true;
   }
   return true;   // never gets here...appeals to some compilers.
}

bool TSinglePeak::IsPeakParameter(const Int_t& par) const
{
   return !IsBackgroundParameter(par);
}

Int_t TSinglePeak::GetNParameters() const
{
   if(fTotalFunction != nullptr) {
      return fTotalFunction->GetNpar();
	}
	return 0;
}

TF1* TSinglePeak::GetBackgroundFunction()
{
   if(fBackgroundFunction == nullptr) {
      fBackgroundFunction = new TF1("peak_bg", this, &TSinglePeak::BackgroundFunction, 0, 1, fTotalFunction->GetNpar(), "TSinglePeak", "BackgroundFunction");
      fBackgroundFunction->SetLineStyle(9);
   }
   return fBackgroundFunction;
}

void TSinglePeak::Print(Option_t*) const
{
   std::cout << IsA()->GetName() << ":" << std::endl;
   std::cout << "Centroid = " << std::fixed << Centroid() << " +/- " << CentroidErr() << std::endl;
   std::cout << "Area = " << Area() << " +/- " << AreaErr() << std::endl;
   std::cout << std::endl;
}

void TSinglePeak::PrintParameters() const
{
   if(fTotalFunction != nullptr) {
      for(int i = 0; i < fTotalFunction->GetNpar(); ++i) {
         std::cout << i << "/" << fTotalFunction->GetParName(i) << " = " << fTotalFunction->GetParameter(i) << " ";
      }
   } else {
      std::cout << "no total function ";
   }
}

Double_t TSinglePeak::TotalFunction(Double_t* dim, Double_t* par)
{
   return PeakFunction(dim, par) + BackgroundFunction(dim, par);
}

void TSinglePeak::UpdatePeakParameters()
{
   fPeakFunction->SetParameters(fTotalFunction->GetParameters());
}

void TSinglePeak::UpdateBackgroundParameters()
{
   fBackgroundFunction->SetParameters(fTotalFunction->GetParameters());
}

void TSinglePeak::DrawComponents(Option_t*)
{
   /// This behaves like the draw function except each daughter class decides how to break the draw into multiple components.
   /// This means that we should delegate this task to the daughter class.
}

Double_t TSinglePeak::FWHM()
{
   /// Return the full width at half-maximum.
   if(fPeakFunction == nullptr) {
      std::cerr << __PRETTY_FUNCTION__ << ": peak function (" << fPeakFunction << ") is null" << std::endl;
      return 0.;
   }
   UpdatePeakParameters();
   double low  = Centroid() - 10. * Sigma();
   double high = Centroid() + 10. * Sigma();
   auto   maxX = fPeakFunction->GetMaximumX(low, high);
   auto   max  = fPeakFunction->Eval(maxX);
   return (fPeakFunction->GetX(max / 2., maxX, maxX + 10. * Sigma()) - fPeakFunction->GetX(max / 2., maxX - 10. * Sigma(), maxX));
}

Double_t TSinglePeak::PeakOnGlobalFunction(Double_t* dim, Double_t* par)
{
   if(fGlobalBackground == nullptr) { return 0.0; }

   return PeakFunction(dim, par) + fGlobalBackground->EvalPar(dim, &par[fTotalFunction->GetNpar()]);
}

void TSinglePeak::Draw(Option_t* opt)
{
   // We need to draw this on top of the global background. Probably easiest to make another temporary TF1?
   if(fGlobalBackground == nullptr) { return; }

   Double_t low = 0.;
	Double_t high = 0.;
   fGlobalBackground->GetRange(low, high);
   if(fPeakOnGlobal != nullptr) { fPeakOnGlobal->Delete(); }
   // Make a copy of the total function, and then tack on the global background parameters.
   fPeakOnGlobal = new TF1("draw_peak", this, &TSinglePeak::PeakOnGlobalFunction, low, high, fTotalFunction->GetNpar() + fGlobalBackground->GetNpar(), "TSinglePeak", "PeakOnGlobalFunction");
   for(int i = 0; i < fTotalFunction->GetNpar(); ++i) {
      fPeakOnGlobal->SetParameter(i, fTotalFunction->GetParameter(i));
   }
   for(int i = 0; i < fGlobalBackground->GetNpar(); ++i) {
      fPeakOnGlobal->SetParameter(i + fTotalFunction->GetNpar(), fGlobalBackground->GetParameter(i));
   }
   // Draw a copy of this function
   fPeakOnGlobal->SetLineColor(fTotalFunction->GetLineColor());
   fPeakOnGlobal->SetLineStyle(fTotalFunction->GetLineStyle());
   fPeakOnGlobal->Draw(opt);
}

bool TSinglePeak::ParameterSetByUser(int par)
{
   /// This function checks if a parameter or its limits have been set to a non-zero value.
   /// In case that the user fixed a parameter to be zero, the limits are non-zero, so this case is covered as well.
   Double_t lowLimit = 0.;
   Double_t highLimit = 0.;

   fTotalFunction->GetParLimits(par, lowLimit, highLimit);
   double value = fTotalFunction->GetParameter(par);

   // if either one of the three has been set, we return true
   return (value != 0 || lowLimit != 0 || highLimit != 0);
}
