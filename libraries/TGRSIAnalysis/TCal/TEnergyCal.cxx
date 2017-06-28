#include "TEnergyCal.h"

/// \cond CLASSIMP
ClassImp(TEnergyCal)
   /// \endcond
   ////////////////////////////////////////////////////////////////
   //                                                            //
   // TEnergyCal                                                 //
   //
   // Class for performing energy calibrations using a single
   // nucleus.
   //                                                            //
   ////////////////////////////////////////////////////////////////

   TEnergyCal::TEnergyCal()
{
   // Default Constructor
   SetDefaultTitles();
}

TEnergyCal::~TEnergyCal() = default;

void TEnergyCal::SetDefaultTitles()
{
   // Sets the default titles of the TGraph in the TEnergyCal
   SetTitle("Energy Calibration");
   GetYaxis()->SetTitle("Accepted Energy (keV)");
   GetXaxis()->SetTitle("Measured Centroid");
   GetYaxis()->CenterTitle();
   GetXaxis()->CenterTitle();
}

std::vector<Double_t> TEnergyCal::GetParameters() const
{
   // WILL NEED TO CHANGE THIS APPROPRIATELY
   std::vector<Double_t> paramList;
   Int_t                 nParams = GetFunction("energy")->GetNpar();

   for(int i = 0; i < nParams; i++) {
      paramList.push_back(GetParameter(i));
   }

   return paramList;
}

Double_t TEnergyCal::GetParameter(size_t parameter) const
{
   // WILL NEED TO CHANGE THIS APPROPRIATELY
   return GetFunction("gain")->GetParameter(parameter); // Root does all of the checking for us.
}

void TEnergyCal::SetNucleus(TNucleus* nuc, Option_t* opt)
{
   // Sets the nucleus of the TEnergyCal. This function sets the data points
   // of the TEnergyCal automatically with the provided nucleus
   TString optstr = opt;
   optstr.ToUpper();
   if(!GetNucleus() || optstr.Contains("F")) {
      TGraphErrors::Clear();
      TCal::SetNucleus(nuc);
      for(int i = 0; i < GetNucleus()->NTransitions(); i++) {
         TGraphErrors::SetPoint(i, 0.0, GetNucleus()->GetTransition(i)->GetEnergy());
         TGraphErrors::SetPointError(i, 0.0, GetNucleus()->GetTransition(i)->GetEnergyUncertainty());
      }
   } else if(GetNucleus()) {
      printf("Nucleus already exists. Use \"F\" option to overwrite\n");
   }

   SetDefaultTitles();
   //  Sort();
}

void TEnergyCal::AddPoint(Double_t measured, Double_t accepted, Double_t measuredUncertainty,
                          Double_t acceptedUncertainty)
{
   // Add a point to the TEnergyCal. The points are sorted by increasing measured centroid.
   Int_t point = GetN();
   TGraphErrors::SetPoint(point, measured, accepted);
   TGraphErrors::SetPointError(point, measuredUncertainty, acceptedUncertainty);
   //  Sort();
}

Bool_t TEnergyCal::SetPoint(Int_t idx, Double_t measured)
{
   // Sets the data point at index idx.
   if(!GetNucleus()) {
      printf("No nucleus set yet...\n");
      return false;
   }

   Double_t x, y;
   Double_t dx, dy;
   GetPoint(idx, x, y);
   dx = GetErrorX(idx);
   dy = GetErrorY(idx);
   TGraphErrors::SetPoint(idx, measured, y);
   TGraphErrors::SetPointError(idx, dx, dy);
   //  Sort();

   return true;
}

Bool_t TEnergyCal::SetPoint(Int_t idx, TPeak* peak)
{
   // Sets the data point at index idx using the centroid, and sigma of a fitted TPeak.
   if(!peak) {
      printf("No Peak, pointer is null\n");
      return false;
   }
   Double_t centroid  = peak->GetCentroid();
   Double_t dCentroid = peak->GetCentroidErr();

   SetPoint(idx, centroid);
   return SetPointError(idx, dCentroid);
}

Bool_t TEnergyCal::SetPointError(Int_t idx, Double_t measuredUncertainty)
{
   // Sets the measured Error of the data point at index idx.
   if(!GetNucleus()) {
      printf("No nucleus set yet...\n");
      return false;
   }

   TGraphErrors::SetPointError(idx, measuredUncertainty, GetErrorX(idx));
   // Sort();

   return true;
}

void TEnergyCal::WriteToChannel() const
{
   // Write the energy calibration information to the current TChannel.
   if(!GetChannel()) {
      Error("WriteToChannel", "No Channel Set");
      return;
   }
   GetChannel()->DestroyENGCal();
   printf("Writing to channel %d\n", GetChannel()->GetNumber());
   printf("p0 = %lf \t p1 = %lf\n", GetParameter(0), GetParameter(1));
   // Set the energy parameters based on the fitted calibration.
   GetChannel()->AddENGCoefficient(GetParameter(0));
   GetChannel()->AddENGCoefficient(GetParameter(1));
}

void TEnergyCal::Print(Option_t*) const
{
   // Print the TEnergyCal information
   TCal::Print();
   TGraphErrors::Print();
}

void TEnergyCal::Clear(Option_t*)
{
   // Clear the TEnergyCal and reset the default titles.
   TCal::Clear();
   SetDefaultTitles();
}
