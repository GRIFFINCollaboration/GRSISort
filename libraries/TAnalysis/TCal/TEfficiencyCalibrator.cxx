#include "TEfficiencyCalibrator.h"

#if __cplusplus >= 201703L

#include <cstdarg>
#include <cstdio>
#include <fcntl.h>

#include "TGTableLayout.h"
#include "TTimer.h"

#include "TGauss.h"
#include "TABPeak.h"
#include "TAB3Peak.h"
#include "TRWPeak.h"
#include "TRedirect.h"

//////////////////////////////////////// TEfficiencyTab ////////////////////////////////////////
TEfficiencyTab::TEfficiencyTab(TEfficiencyDatatypeTab* parent, TNucleus* nucleus, std::tuple<TH1*, TH2*, TH2*> hists, TGCompositeFrame* frame, const int& verboseLevel)
   : fFrame(frame), fNucleus(nucleus), fParent(parent), fVerboseLevel(verboseLevel)
{
   fSingles    = std::get<0>(hists);
   fSummingOut = std::get<1>(hists);   // name *180Corr
   fSummingIn  = std::get<2>(hists);   // name *Sum180Corr
   if(fVerboseLevel > 2) std::cout << "Assigned singles, summing out, and summing in histograms (" << fSingles->GetName() << ", " << fSummingOut->GetName() << ", " << fSummingIn->GetName() << ")" << std::endl;
   BuildInterface();
   {
      TRedirect redirect("fitOutput.txt");
      std::cout << "======================================== Finding peaks in " << fSingles->GetName() << ", " << fSummingOut->GetName() << ", and " << fSummingIn->GetName() << std::endl;
      FindPeaks();
   }
}

TEfficiencyTab::~TEfficiencyTab()
{
}

void TEfficiencyTab::BuildInterface()
{
   // top frame with one canvas, status bar, and controls
   fTopFrame         = new TGHorizontalFrame(fFrame, TEfficiencyCalibrator::WindowWidth() / 2, 450);
   fProjectionCanvas = new TRootEmbeddedCanvas("ProjectionCanvas", fTopFrame, TEfficiencyCalibrator::WindowWidth() / 2, 400);

   fTopFrame->AddFrame(fProjectionCanvas, new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandY | kLHintsExpandX, 2, 2, 2, 2));

   fFrame->AddFrame(fTopFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));

   fStatusBar  = new TGStatusBar(fFrame, TEfficiencyCalibrator::WindowWidth() / 2, 50);
   int parts[] = {35, 15, 20, 15, 15};
   fStatusBar->SetParts(parts, 5);

   fFrame->AddFrame(fStatusBar, new TGLayoutHints(kLHintsBottom | kLHintsExpandX, 2, 2, 2, 2));
   if(fVerboseLevel > 3) std::cout << "Done with " << __PRETTY_FUNCTION__ << std::endl;
}

void TEfficiencyTab::FindPeaks()
{
   /// Find and fit the peaks in the singles histogram, then checks for each peak how much summing in and summing out happens.

   if(fVerboseLevel > 3) {
      std::cout << __PRETTY_FUNCTION__ << std::endl;
      std::cout << "Using parent " << fParent << " and transition list " << fNucleus->GetTransitionListByEnergy() << std::endl;
      fNucleus->GetTransitionListByEnergy()->Print();
      std::cout << "Using parent " << fParent << std::flush << ", trying to get peak type " << TEfficiencyCalibrator::PeakType() << std::endl;
   }
   fProjectionCanvas->GetCanvas()->cd();
   if(fVerboseLevel > 3) {
      std::cout << "Got peak type " << TEfficiencyCalibrator::PeakType() << ", getting projection from " << fSummingOut->GetName() << std::endl;
   }
   fSummingOutTotalProj = fSummingOut->ProjectionY();
   if(fVerboseLevel > 2) {
      std::cout << "Writing '" << fSummingOutTotalProj->GetName() << "' to '" << gDirectory->GetName() << "'" << std::endl;
   }
   fSummingOutTotalProj->Write(nullptr, TObject::kOverwrite);
   if(fVerboseLevel > 3) {
      std::cout << "Got projection " << fSummingOutTotalProj << ", getting background from " << fSummingOutTotalProj->GetName() << std::endl;
   }
   fSummingOutTotalProjBg = fSummingOutTotalProj->ShowBackground(TEfficiencyCalibrator::BgParam());
   if(fVerboseLevel > 2) {
      std::cout << "Writing '" << fSummingOutTotalProjBg->GetName() << "' to '" << gDirectory->GetName() << "'" << std::endl;
   }
   fSummingOutTotalProjBg->Write(nullptr, TObject::kOverwrite);
   // loop through all gamma-rays of the source
   // TODO: combine ranges if peak ranges overlap
   // probably need to: change loop to using size and ->At(index), plus extra loop that finds all peaks within the range
   // maybe also change the creation of the peak to either a function or using TClass::New()
   for(int t = 0; t < fNucleus->GetTransitionListByEnergy()->GetSize(); ++t) {
      auto* transition = static_cast<TTransition*>(fNucleus->GetTransitionListByEnergy()->At(t));
      auto energy     = transition->GetEnergy();
      if(energy > fSingles->GetXaxis()->GetXmax()) {
         // need to check if we also want to reject peaks whose fit range is partially outside the range of the histogram
         std::cout << "Skipping peak at " << energy << " keV of " << fNucleus->GetA() << fNucleus->GetSymbol() << ", maximum range of histogram is " << fSingles->GetXaxis()->GetXmax() << std::endl;
         continue;
      }
      if(fVerboseLevel > 3) {
         std::cout << "Fitting peak at " << energy << " keV, using range " << energy - TEfficiencyCalibrator::Range() << ", " << energy + TEfficiencyCalibrator::Range() << " peak type " << TEfficiencyCalibrator::PeakType() << " singles histogram " << fSingles->GetName() << std::endl;
      }
      fPeakFitter.RemoveAllPeaks();
      fPeakFitter.ResetInitFlag();

      std::vector<TSinglePeak*> peaks;
      peaks.push_back(NewPeak(energy));
      double lastEnergy = energy;
      for(int t2 = t + 1; t2 < fNucleus->GetTransitionListByEnergy()->GetSize(); ++t2) {
         auto* transition2 = static_cast<TTransition*>(fNucleus->GetTransitionListByEnergy()->At(t2));
         auto energy2     = transition2->GetEnergy();
         if(lastEnergy + TEfficiencyCalibrator::Range() > energy2 - TEfficiencyCalibrator::Range()) {
            if(fVerboseLevel > 2) {
               std::cout << t << ". peak: Found " << peaks.size() << ". overlapping peak for " << lastEnergy << ", and " << energy2 << " with range " << TEfficiencyCalibrator::Range() << std::endl;
            }
            // peak ranges overlap, so we add this peak to the fit, skip it in the main loop, and update lastEnergy to refer to it instead
            peaks.push_back(NewPeak(energy2));
            ++t;
            lastEnergy = energy2;
         } else {
            // list of transitions is sorted by energy, so all others will be outside the range as well
            if(fVerboseLevel > 2) {
               std::cout << t << ". peak: Found " << peaks.size() << ". overlapping peaks, stopping here with " << lastEnergy << ", and " << energy2 << " with range " << TEfficiencyCalibrator::Range() << std::endl;
            }
            break;
         }
      }

      fPeakFitter.SetRange(energy - TEfficiencyCalibrator::Range(), lastEnergy + TEfficiencyCalibrator::Range());
      for(auto& peak : peaks) {
         fPeakFitter.AddPeak(peak);
      }
      std::cout << "---------------------------------------- Fitting peak(s) at " << energy << "/" << lastEnergy << " from " << energy - TEfficiencyCalibrator::Range() << " to " << energy + TEfficiencyCalibrator::Range() << std::endl;
      fPeakFitter.Print();
      fPeakFitter.PrintParameters();
      fPeakFitter.Fit(fSingles, "retryfit");
      for(auto& peak : peaks) {
         if(fVerboseLevel > 2) {
            std::cout << "================================================================================" << std::endl;
            std::cout << "Got centroid " << peak->Centroid() << " +- " << peak->CentroidErr() << " (" << 100 * peak->CentroidErr() / peak->Centroid() << " %), area " << peak->Area() << " +- " << peak->AreaErr() << " (" << 100 * peak->AreaErr() / peak->Area() << " %), FWHM " << peak->FWHM() << ", red. chi sq. " << peak->GetReducedChi2() << std::endl;
         }
         if(peak->Area() < TEfficiencyCalibrator::Threshold() || peak->CentroidErr() > peak->Centroid() || peak->AreaErr() > peak->Area() || std::isnan(peak->Centroid()) || std::isnan(peak->Area()) || std::isnan(peak->CentroidErr()) || std::isnan(peak->AreaErr())) {
            if(fVerboseLevel > 2) {
               std::cout << "Skipping peak at " << energy << " keV with centroid " << peak->Centroid() << " +- " << peak->CentroidErr() << " (" << 100 * peak->CentroidErr() / peak->Centroid() << " %), FWHM " << peak->FWHM() << ", and area " << peak->Area() << " +- " << peak->AreaErr() << " (" << 100 * peak->AreaErr() / peak->Area() << " %)" << std::endl;
               std::cout << "================================================================================" << std::endl;
            }
            fRemovedFitFunctions.push_back(fPeakFitter.GetFitFunction()->Clone(Form("removed_fit_%.1f", peak->Centroid())));
            continue;
         }
         if(fVerboseLevel > 2) {
            std::cout << "================================================================================" << std::endl;
         }
         // increase number of points of fit function
         fPeakFitter.GetFitFunction()->SetNpx(1000);
         peak->GetPeakFunction()->SetNpx(1000);
         double fwhm        = peak->FWHM();
         double centroid    = peak->Centroid();
         double centroidErr = peak->CentroidErr();
         if(fVerboseLevel > 3) {
            std::cout << "Got centroid " << centroid << " keV, FWHM " << fwhm << ":" << std::endl;
         }
         if(fVerboseLevel > 2) {
            std::cout << "Writing '" << Form("%s_%.0fkeV", fSingles->GetName(), centroid) << "' to '" << gDirectory->GetName() << "'" << std::endl;
         }
         fSingles->Write(Form("%s_%.0fkeV", fSingles->GetName(), centroid), TObject::kOverwrite);
         // for summing in we need to estimate the background and subtract that from the integral
         fSummingInProj.push_back(fSummingIn->ProjectionY(Form("%s_%.0f_to_%.0f", fSummingIn->GetName(), centroid - fwhm / 2., centroid + fwhm / 2.), fSummingIn->GetXaxis()->FindBin(centroid - fwhm / 2.), fSummingIn->GetXaxis()->FindBin(centroid + fwhm / 2.)));
         fSummingInProjBg.push_back(fSummingInProj.back()->ShowBackground(TEfficiencyCalibrator::BgParam()));
         if(fVerboseLevel > 2) {
            std::cout << "Writing '" << Form("%s_%.0fkeV", fSummingInProj.back()->GetName(), centroid) << "' and '" << Form("%s_%.0fkeV", fSummingInProjBg.back()->GetName(), centroid) << "' to '" << gDirectory->GetName() << "'" << std::endl;
         }
         double summingIn = fSummingInProj.back()->Integral() - fSummingInProjBg.back()->Integral();
         fSummingInProj.back()->Write(Form("%s_%.0fkeV", fSummingInProj.back()->GetName(), centroid), TObject::kOverwrite);
         fSummingInProjBg.back()->Write(Form("%s_%.0fkeV", fSummingInProjBg.back()->GetName(), centroid), TObject::kOverwrite);
         // for summing out we need to do a background subtraction - to make this easier we just use a total projection and scale it according to the bg integral?
         fSummingOutProj.push_back(fSummingOut->ProjectionY(Form("%s_%.0f_to_%.0f", fSummingOut->GetName(), centroid - fwhm / 2., centroid + fwhm / 2.), fSummingOut->GetXaxis()->FindBin(centroid - fwhm / 2.), fSummingOut->GetXaxis()->FindBin(centroid + fwhm / 2.)));
         double ratio      = fSummingOutTotalProjBg->Integral(fSummingOutTotalProjBg->GetXaxis()->FindBin(centroid - fwhm / 2.), fSummingOutTotalProjBg->GetXaxis()->FindBin(centroid + fwhm / 2.)) / fSummingOutTotalProjBg->Integral();
         double summingOut = fSummingOutProj.back()->Integral() - fSummingOutTotalProj->Integral() * ratio;
         if(fVerboseLevel > 2) {
            std::cout << "Writing '" << Form("%s_%.0fkeV", fSummingOutProj.back()->GetName(), centroid) << "' to '" << gDirectory->GetName() << "'" << std::endl;
         }
         fSummingOutProj.back()->Write(Form("%s_%.0fkeV", fSummingOutProj.back()->GetName(), centroid), TObject::kOverwrite);
         auto* hist = static_cast<TH1*>(fSummingOutProj.back()->Clone(Form("%s_subtracted_%.0f", fSummingOutProj.back()->GetName(), 1000000 * ratio)));
         hist->Add(fSummingOutTotalProj, -ratio);
         if(fVerboseLevel > 2) {
            std::cout << "Writing '" << hist->GetName() << "' to '" << gDirectory->GetName() << "'" << std::endl;
         }
         hist->Write(nullptr, TObject::kOverwrite);

         double correctedArea = peak->Area() - summingIn + summingOut;
         // uncertainties for summing in and summing out is sqrt(N) (?)
         double correctedAreaErr = TMath::Sqrt(TMath::Power(peak->AreaErr(), 2) + summingIn + summingOut);
         if(fVerboseLevel > 3) {
            std::cout << "Got summingIn " << summingIn << ", ratio " << ratio << ", summingOut " << summingOut << ", correctedArea " << correctedArea << " +- " << correctedAreaErr << std::endl;
         }
         fPeaks.push_back(std::make_tuple(transition, centroid, centroidErr, correctedArea, correctedAreaErr, peak->Area(), peak->AreaErr(), summingIn, summingOut));
         fFitFunctions.push_back(fPeakFitter.GetFitFunction()->Clone(Form("fit_%.1f", centroid)));
      }
   }

   Redraw();
   if(fVerboseLevel > 3) {
      std::cout << "Unsorted peak vector:";
      for(const auto& peak : fPeaks) std::cout << " " << std::get<0>(peak)->GetEnergy();
      std::cout << std::endl;
   }
   std::sort(fPeaks.begin(), fPeaks.end(), [](const std::tuple<TTransition*, double, double, double, double, double, double, double, double>& lhs, const std::tuple<TTransition*, double, double, double, double, double, double, double, double>& rhs) -> bool { return std::get<0>(lhs)->GetEnergy() < std::get<0>(rhs)->GetEnergy(); });
   if(fVerboseLevel > 3) {
      std::cout << "Sorted peak vector:";
      for(auto& peak : fPeaks) std::cout << " " << std::get<0>(peak)->GetEnergy();
      std::cout << std::endl;
   }
}

TSinglePeak* TEfficiencyTab::NewPeak(const double& energy)
{
   switch(TEfficiencyCalibrator::PeakType()) {
   case kRWPeak:
      return new TRWPeak(energy);
      break;
   case kABPeak:
      return new TABPeak(energy);
      break;
   case kAB3Peak:
      return new TAB3Peak(energy);
      break;
   case kGauss:
      return new TGauss(energy);
      break;
   default:
      std::cerr << "Unknow peak type " << TEfficiencyCalibrator::PeakType() << ", defaulting to TRWPeak!" << std::endl;
      return new TRWPeak(energy);
   }
}

void TEfficiencyTab::Redraw()
{
   fProjectionCanvas->GetCanvas()->cd();
   fSingles->GetListOfFunctions()->Clear();
   for(auto& fit : fFitFunctions) {
      static_cast<TF1*>(fit)->SetLineColor(2);   // red line color for fits
      fSingles->GetListOfFunctions()->Add(fit);
   }
   if(TEfficiencyCalibrator::ShowRemovedFits()) {
      for(auto& fit : fRemovedFitFunctions) {
         static_cast<TF1*>(fit)->SetLineColor(15);   // grey line color for removed fits
         fSingles->GetListOfFunctions()->Add(fit);
      }
   }
   fSingles->Draw();
   fProjectionCanvas->GetCanvas()->Modified();
}

void TEfficiencyTab::MakeConnections()
{
   fProjectionCanvas->GetCanvas()->Connect("ProcessedEvent(Int_t, Int_t, Int_t, TObject*)", "TEfficiencyTab", this, "Status(Int_t, Int_t, Int_t, TObject*)");
}

void TEfficiencyTab::Disconnect()
{
   fProjectionCanvas->GetCanvas()->Disconnect("ProcessedEvent(Int_t, Int_t, Int_t, TObject*)", this, "Status(Int_t, Int_t, Int_t, TObject*)");
}

void TEfficiencyTab::Status(Int_t, Int_t px, Int_t py, TObject* selected)
{
   fStatusBar->SetText(selected->GetName(), 0);
   fStatusBar->SetText(selected->GetObjectInfo(px, py), 1);
}

//////////////////////////////////////// TEfficiencyDatatypeTab ////////////////////////////////////////
TEfficiencyDatatypeTab::TEfficiencyDatatypeTab(TEfficiencyCalibrator* parent, std::vector<TNucleus*> nucleus, std::vector<std::tuple<TH1*, TH2*, TH2*>> hists, TGCompositeFrame* frame, const std::string& dataType, TGHProgressBar* progressBar, const int& verboseLevel)
   : fFrame(frame), fNucleus(nucleus), fParent(parent), fDataType(dataType), fVerboseLevel(verboseLevel)
{
   if(fVerboseLevel > 1) {
      std::cout << "======================================== creating tab for data type " << dataType << std::endl;
   }
   // create new subdirectory for this tab (and its channel tabs), but store the old directory
   fMainDirectory = gDirectory;
   fSubDirectory  = gDirectory->mkdir(dataType.c_str());
   if(fVerboseLevel > 2) {
      std::cout << "switching from gDirectory " << gDirectory->GetName() << " to sub directory " << fSubDirectory;
   }
   fSubDirectory->cd();
   if(fVerboseLevel > 2) {
      std::cout << " = " << fSubDirectory->GetName() << ", main directory is " << fMainDirectory << " = " << (fMainDirectory == nullptr ? "" : fMainDirectory->GetName()) << std::endl;
   }

   // left frame with histograms tabs
   fLeftFrame = new TGVerticalFrame(fFrame, TEfficiencyCalibrator::WindowWidth() / 2, TEfficiencyCalibrator::WindowWidth() / 2);

   fDataTab = new TGTab(fLeftFrame, TEfficiencyCalibrator::WindowWidth() / 2, 500);
   fEfficiencyTab.resize(hists.size(), nullptr);
   for(size_t i = 0; i < hists.size(); ++i) {
      if(fVerboseLevel > 2) std::cout << i << ": Creating efficiency tab using " << fNucleus[i] << " = " << fNucleus[i]->GetName() << ", " << std::get<0>(hists[i])->GetName() << ", " << std::get<1>(hists[i])->GetName() << ", " << std::get<2>(hists[i])->GetName() << ", " << TEfficiencyCalibrator::Range() << ", " << TEfficiencyCalibrator::Threshold() << ", " << TEfficiencyCalibrator::BgParam() << std::endl;
      fEfficiencyTab[i] = new TEfficiencyTab(this, fNucleus[i], hists[i], fDataTab->AddTab(fNucleus[i]->GetName()), fVerboseLevel);
      progressBar->Increment(1);
   }

   fFittingParameterFrame = new TGGroupFrame(fLeftFrame, "Fitting Parameters", kHorizontalFrame);
   fFittingParameterFrame->SetLayoutManager(new TGTableLayout(fFittingParameterFrame, 2, 4, false, 5));   // rows, columns, not homogenous cell sizes, 5 = interval between frames in pixel
   fRangeLabel     = new TGLabel(fFittingParameterFrame, "Range");
   fRangeEntry     = new TGNumberEntry(fFittingParameterFrame, TEfficiencyCalibrator::Range(), 5, kRangeEntry, TGNumberFormat::EStyle::kNESRealTwo, TGNumberFormat::EAttribute::kNEAPositive);
   fBgParamLabel   = new TGLabel(fFittingParameterFrame, "BG Parameter");
   fBgParamEntry   = new TGNumberEntry(fFittingParameterFrame, TEfficiencyCalibrator::BgParam(), 5, kBgParamEntry, TGNumberFormat::EStyle::kNESRealTwo, TGNumberFormat::EAttribute::kNEAPositive);
   fThresholdLabel = new TGLabel(fFittingParameterFrame, "Threshold");
   fThresholdEntry = new TGNumberEntry(fFittingParameterFrame, TEfficiencyCalibrator::Threshold(), 5, kThresholdEntry, TGNumberFormat::EStyle::kNESRealThree, TGNumberFormat::EAttribute::kNEAPositive);
   fPeakTypeBox    = new TGComboBox(fFittingParameterFrame, kPeakTypeBox);
   fPeakTypeBox->AddEntry("Radware", TEfficiencyTab::EPeakType::kRWPeak);
   fPeakTypeBox->AddEntry("Addback", TEfficiencyTab::EPeakType::kABPeak);
   fPeakTypeBox->AddEntry("Addback3", TEfficiencyTab::EPeakType::kAB3Peak);
   fPeakTypeBox->AddEntry("Gaussian", TEfficiencyTab::EPeakType::kGauss);
   fPeakTypeBox->SetMinHeight(200);
   fPeakTypeBox->Resize(100, TEfficiencyCalibrator::LineHeight());
   fPeakTypeBox->Select(TEfficiencyCalibrator::PeakType());
   fFittingControlGroup = new TGHButtonGroup(fFittingParameterFrame, "");
   fRefitButton         = new TGTextButton(fFittingControlGroup, "Refit this source");
   fRefitAllButton      = new TGTextButton(fFittingControlGroup, "Refit all sources");

   fFittingParameterFrame->AddFrame(fRangeLabel, new TGTableLayoutHints(0, 1, 0, 1));
   fFittingParameterFrame->AddFrame(fRangeEntry, new TGTableLayoutHints(1, 2, 0, 1));
   fFittingParameterFrame->AddFrame(fBgParamLabel, new TGTableLayoutHints(2, 3, 0, 1));
   fFittingParameterFrame->AddFrame(fBgParamEntry, new TGTableLayoutHints(3, 4, 0, 1));
   fFittingParameterFrame->AddFrame(fThresholdLabel, new TGTableLayoutHints(0, 1, 1, 2));
   fFittingParameterFrame->AddFrame(fThresholdEntry, new TGTableLayoutHints(1, 2, 1, 2));
   fFittingParameterFrame->AddFrame(fPeakTypeBox, new TGTableLayoutHints(2, 3, 1, 2));
   fFittingParameterFrame->AddFrame(fFittingControlGroup, new TGTableLayoutHints(3, 4, 1, 2));

   fLeftFrame->AddFrame(fDataTab, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));
   fLeftFrame->AddFrame(fFittingParameterFrame, new TGLayoutHints(kLHintsBottom | kLHintsExpandX, 2, 2, 2, 2));

   // right frame with canvas, status bar, and the degree entry
   fRightFrame       = new TGVerticalFrame(fFrame, TEfficiencyCalibrator::WindowWidth() / 2, TEfficiencyCalibrator::WindowWidth() / 2);
   fEfficiencyCanvas = new TRootEmbeddedCanvas("EfficiencyCanvas", fRightFrame, TEfficiencyCalibrator::WindowWidth() / 2, 450);
   fStatusBar        = new TGStatusBar(fRightFrame, TEfficiencyCalibrator::WindowWidth() / 2, 50);
   int parts[]       = {35, 65};
   fStatusBar->SetParts(parts, 2);
   fGraphParameterFrame = new TGGroupFrame(fRightFrame, "Graph Parameters", kHorizontalFrame);
   fGraphParameterFrame->SetLayoutManager(new TGTableLayout(fGraphParameterFrame, 3, 4, false, 5));   // rows, columns, not homogenous cell sizes, 5 = interval between frames in pixel
   fDegreeEntry                 = new TGNumberEntry(fGraphParameterFrame, TEfficiencyCalibrator::Degree(), 2, kDegreeEntry, TGNumberFormat::EStyle::kNESInteger);
   fDegreeLabel                 = new TGLabel(fGraphParameterFrame, "Type of efficiency curve");
   fCalibrationUncertaintyLabel = new TGLabel(fGraphParameterFrame, "Calibration Uncertainty");
   fCalibrationUncertaintyEntry = new TGNumberEntry(fGraphParameterFrame, TEfficiencyCalibrator::CalibrationUncertainty(), 5, kCalibrationUncertaintyEntry, TGNumberFormat::EStyle::kNESRealOne, TGNumberFormat::EAttribute::kNEAPositive);
   fPlotOptionFrame             = new TGGroupFrame(fGraphParameterFrame, "Plot Selection", kHorizontalFrame);
   fPlotOptionFrame->SetLayoutManager(new TGTableLayout(fPlotOptionFrame, 2, 3, false, 5));   // rows, columns, not homogenous cell sizes, 5 = interval between frames in pixel
   fPlotEfficiencyCheck = new TGCheckButton(fPlotOptionFrame, "Efficiency", kPlotEfficiencyCheck);
   fPlotEfficiencyCheck->SetState(kButtonDown);
   fPlotUncorrEfficiencyCheck = new TGCheckButton(fPlotOptionFrame, "Uncorrected efficiency", kPlotUncorrEfficiencyCheck);
   fPlotUncorrEfficiencyCheck->SetState(kButtonUp);
   fPlotPeakAreaCheck = new TGCheckButton(fPlotOptionFrame, "Peak area", kPlotPeakAreaCheck);
   fPlotPeakAreaCheck->SetState(kButtonUp);
   fPlotSummingInCheck = new TGCheckButton(fPlotOptionFrame, "Summing in", kPlotSummingInCheck);
   fPlotSummingInCheck->SetState(kButtonUp);
   fPlotSummingOutCheck = new TGCheckButton(fPlotOptionFrame, "Summing out", kPlotSummingOutCheck);
   fPlotSummingOutCheck->SetState(kButtonUp);
   fPlotOptionFrame->AddFrame(fPlotEfficiencyCheck, new TGTableLayoutHints(0, 1, 0, 1));
   fPlotOptionFrame->AddFrame(fPlotUncorrEfficiencyCheck, new TGTableLayoutHints(1, 3, 0, 1));
   fPlotOptionFrame->AddFrame(fPlotPeakAreaCheck, new TGTableLayoutHints(0, 1, 1, 2));
   fPlotOptionFrame->AddFrame(fPlotSummingInCheck, new TGTableLayoutHints(1, 2, 1, 2));
   fPlotOptionFrame->AddFrame(fPlotSummingOutCheck, new TGTableLayoutHints(2, 3, 1, 2));
   fRecalculateButton = new TGTextButton(fGraphParameterFrame, "Recalculate graphs");
   fGraphParameterFrame->AddFrame(fDegreeLabel, new TGTableLayoutHints(0, 1, 0, 1));
   fGraphParameterFrame->AddFrame(fDegreeEntry, new TGTableLayoutHints(1, 2, 0, 1));
   fGraphParameterFrame->AddFrame(fCalibrationUncertaintyLabel, new TGTableLayoutHints(0, 1, 1, 2));
   fGraphParameterFrame->AddFrame(fCalibrationUncertaintyEntry, new TGTableLayoutHints(1, 2, 1, 2));
   fGraphParameterFrame->AddFrame(fPlotOptionFrame, new TGTableLayoutHints(2, 4, 0, 2));
   fGraphParameterFrame->AddFrame(fRecalculateButton, new TGTableLayoutHints(0, 4, 2, 3));
   fRightFrame->AddFrame(fEfficiencyCanvas, new TGLayoutHints(kLHintsExpandX));
   fRightFrame->AddFrame(fStatusBar, new TGLayoutHints(kLHintsExpandX));
   fRightFrame->AddFrame(fGraphParameterFrame, new TGLayoutHints(kLHintsExpandX));

   fFrame->SetLayoutManager(new TGHorizontalLayout(fFrame));
   fFrame->AddFrame(fLeftFrame, new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandY, 2, 2, 2, 2));
   fFrame->AddFrame(fRightFrame, new TGLayoutHints(kLHintsRight | kLHintsTop | kLHintsExpandY, 2, 2, 2, 2));

   DrawGraph();
   if(fVerboseLevel > 2) {
      std::cout << "switching from sub directory " << gDirectory->GetName() << " to main directory " << fMainDirectory;
   }
   fMainDirectory->cd();
   if(fVerboseLevel > 2) {
      std::cout << " = " << fMainDirectory->GetName() << ", sub directory is " << fSubDirectory << " = " << (fSubDirectory == nullptr ? "" : fSubDirectory->GetName()) << std::endl;
   }
   if(fVerboseLevel > 1) {
      std::cout << "======================================== done creating tab for data type " << dataType << std::endl;
   }
}

TEfficiencyDatatypeTab::~TEfficiencyDatatypeTab()
{
   fMainDirectory->cd();
}

void TEfficiencyDatatypeTab::MakeConnections()
{
   fFittingControlGroup->Connect("Clicked(Int_t)", "TEfficiencyDatatypeTab", this, "FittingControl(Int_t)");
   fPlotEfficiencyCheck->Connect("Clicked()", "TEfficiencyDatatypeTab", this, "DrawGraph()");
   fPlotUncorrEfficiencyCheck->Connect("Clicked()", "TEfficiencyDatatypeTab", this, "DrawGraph()");
   fPlotPeakAreaCheck->Connect("Clicked()", "TEfficiencyDatatypeTab", this, "DrawGraph()");
   fPlotSummingInCheck->Connect("Clicked()", "TEfficiencyDatatypeTab", this, "DrawGraph()");
   fPlotSummingOutCheck->Connect("Clicked()", "TEfficiencyDatatypeTab", this, "DrawGraph()");
   fRecalculateButton->Connect("Clicked()", "TEfficiencyDatatypeTab", this, "DrawGraph()");
   fEfficiencyCanvas->GetCanvas()->Connect("ProcessedEvent(Int_t, Int_t, Int_t, TObject*)", "TEfficiencyDatatypeTab", this, "Status(Int_t, Int_t, Int_t, TObject*)");
   for(auto& tab : fEfficiencyTab) {
      tab->MakeConnections();
   }
   fPeakTypeBox->Connect("Selected(Int_t)", "TEfficiencyDatatypeTab", this, "UpdatePeakType()");
}

void TEfficiencyDatatypeTab::Disconnect()
{
   fFittingControlGroup->Disconnect("Clicked(Int_t)", this, "FittingControl(Int_t)");
   fPlotEfficiencyCheck->Disconnect("Clicked()", this, "DrawGraph()");
   fPlotUncorrEfficiencyCheck->Disconnect("Clicked()", this, "DrawGraph()");
   fPlotPeakAreaCheck->Disconnect("Clicked()", this, "DrawGraph()");
   fPlotSummingInCheck->Disconnect("Clicked()", this, "DrawGraph()");
   fPlotSummingOutCheck->Disconnect("Clicked()", this, "DrawGraph()");
   fRecalculateButton->Disconnect("Clicked()", this, "DrawGraph()");
   fEfficiencyCanvas->GetCanvas()->Disconnect("ProcessedEvent(Int_t, Int_t, Int_t, TObject*)", this, "Status(Int_t, Int_t, Int_t, TObject*)");
   for(auto& tab : fEfficiencyTab) {
      tab->Disconnect();
   }
}

void TEfficiencyDatatypeTab::Status(Int_t, Int_t px, Int_t py, TObject* selected)
{
   fStatusBar->SetText(selected->GetName(), 0);
   fStatusBar->SetText(selected->GetObjectInfo(px, py), 1);
}

void TEfficiencyDatatypeTab::DrawGraph()
{
   ReadValues();
   UpdateEfficiencyGraph();
   FitEfficiency();
   if(fLegend == nullptr) {
      fLegend = new TLegend(0.8, 0.95 - fNucleus.size() * 0.05, 0.95, 0.95);
   } else {
      fLegend->Clear();
   }

   fEfficiencyCanvas->GetCanvas()->cd();
   bool firstPlot = true;
   int  color     = 1;
   if(fPlotEfficiencyCheck->IsDown()) {
      if(fVerboseLevel > 2) {
         std::cout << "Drawing efficiency graph " << fEfficiencyGraph << ":" << std::endl;
         fEfficiencyGraph->Print("e");
      }
      for(size_t g = 0; g < fEfficiencyGraph->NumberOfGraphs(); ++g) {
         fEfficiencyGraph->SetColorStyle(g, color++);
      }
      fEfficiencyGraph->DrawCalibration("*", fLegend);
      firstPlot = false;
   }
   if(fPlotUncorrEfficiencyCheck->IsDown()) {
      if(fVerboseLevel > 2) {
         std::cout << "Drawing uncorrected efficiency graph " << fUncorrEfficiencyGraph << ":" << std::endl;
         fUncorrEfficiencyGraph->Print("e");
      }
      for(size_t g = 0; g < fUncorrEfficiencyGraph->NumberOfGraphs(); ++g) {
         fUncorrEfficiencyGraph->SetColorStyle(g, color++);
      }
      if(firstPlot) fUncorrEfficiencyGraph->DrawCalibration("*", fLegend);
      else fUncorrEfficiencyGraph->DrawCalibration("same*", fLegend);
      firstPlot = false;
   }
   if(fPlotPeakAreaCheck->IsDown()) {
      if(fVerboseLevel > 2) {
         fPeakAreaGraph->Print("");
      }
      for(size_t g = 0; g < fPeakAreaGraph->NumberOfGraphs(); ++g) {
         fPeakAreaGraph->SetColorStyle(g, color++);
      }
      if(firstPlot) fPeakAreaGraph->DrawCalibration("*", fLegend);
      else fPeakAreaGraph->DrawCalibration("same*", fLegend);
      firstPlot = false;
   }
   if(fPlotSummingInCheck->IsDown()) {
      if(fVerboseLevel > 2) {
         fSummingInGraph->Print("");
      }
      for(size_t g = 0; g < fSummingInGraph->NumberOfGraphs(); ++g) {
         fSummingInGraph->SetColorStyle(g, color++);
      }
      if(firstPlot) fSummingInGraph->DrawCalibration("*", fLegend);
      else fSummingInGraph->DrawCalibration("same*", fLegend);
      firstPlot = false;
   }
   if(fPlotSummingInCheck->IsDown()) {
      if(fVerboseLevel > 2) {
         fSummingOutGraph->Print("");
      }
      for(size_t g = 0; g < fSummingOutGraph->NumberOfGraphs(); ++g) {
         fSummingOutGraph->SetColorStyle(g, color++);
      }
      if(firstPlot) fSummingOutGraph->DrawCalibration("*", fLegend);
      else fSummingOutGraph->DrawCalibration("same*", fLegend);
      firstPlot = false;
   }
   fLegend->Draw();
   fEfficiencyCanvas->GetCanvas()->Modified();
}

void TEfficiencyDatatypeTab::UpdateEfficiencyGraph()
{
   if(fMainDirectory == nullptr) {
      std::cout << "No main directory open (" << fMainDirectory << "), sub directory is " << fSubDirectory << ", gDirectory is " << gDirectory->GetName() << std::endl;
      exit(1);
   }
   if(fVerboseLevel > 2) {
      std::cout << __PRETTY_FUNCTION__ << ": switching from gDirectory " << gDirectory->GetName() << " to main directory " << fMainDirectory << " = " << fMainDirectory->GetName() << std::endl;
   }
   fMainDirectory->cd();
   if(fSubDirectory == nullptr) {
      std::cout << "No subdirectory open (" << fSubDirectory << "), main directory is " << fMainDirectory << ", gDirectory is " << gDirectory->GetName() << std::endl;
   } else {
      if(fVerboseLevel > 2) {
         std::cout << __PRETTY_FUNCTION__ << ": switching from gDirectory " << gDirectory->GetName() << " to sub directory " << fSubDirectory;
      }
      fSubDirectory->cd();
      if(fVerboseLevel > 2) {
         std::cout << " = " << fSubDirectory->GetName() << ", main directory is " << fMainDirectory << " = " << (fMainDirectory == nullptr ? "" : fMainDirectory->GetName()) << std::endl;
      }
   }
   if(fEfficiencyGraph != nullptr) {
      delete fEfficiencyGraph;
   }
   fEfficiencyGraph = new TCalibrationGraphSet();
   fEfficiencyGraph->VerboseLevel(fVerboseLevel);
   if(fUncorrEfficiencyGraph != nullptr) {
      delete fUncorrEfficiencyGraph;
   }
   fUncorrEfficiencyGraph = new TCalibrationGraphSet();
   fUncorrEfficiencyGraph->VerboseLevel(fVerboseLevel);
   if(fPeakAreaGraph != nullptr) {
      delete fPeakAreaGraph;
   }
   fPeakAreaGraph = new TCalibrationGraphSet();
   fPeakAreaGraph->VerboseLevel(fVerboseLevel);
   if(fSummingInGraph != nullptr) {
      delete fSummingInGraph;
   }
   fSummingInGraph = new TCalibrationGraphSet();
   fSummingInGraph->VerboseLevel(fVerboseLevel);
   if(fSummingOutGraph != nullptr) {
      delete fSummingOutGraph;
   }
   fSummingOutGraph = new TCalibrationGraphSet();
   fSummingOutGraph->VerboseLevel(fVerboseLevel);

   for(auto& tab : fEfficiencyTab) {
      // vector of tuple with transition and 8 doubles:
      // centroid, centroidErr, correctedArea, correctedAreaErr, peak->Area(), peak->AreaErr(), summingIn, summingOut
      auto                peaks = tab->Peaks();
      TTransition*        transition;
      double              centroid         = 0.;
      double              centroidErr      = 0.;
      double              correctedArea    = 0.;
      double              correctedAreaErr = 0.;
      double              peakArea         = 0.;
      double              peakAreaErr      = 0.;
      double              summingIn        = 0.;
      double              summingOut       = 0.;
      std::vector<double> energy;
      std::vector<double> energyErr;
      std::vector<double> efficiency;
      std::vector<double> efficiencyErr;
      std::vector<double> uncorrEfficiency;
      std::vector<double> uncorrEfficiencyErr;
      std::vector<double> peakAreaVec;
      std::vector<double> peakAreaErrVec;
      std::vector<double> summingInVec;
      std::vector<double> summingOutVec;
      int                 goodPeaks = 0;
      for(auto& peak : peaks) {
         std::tie(transition, centroid, centroidErr, correctedArea, correctedAreaErr, peakArea, peakAreaErr, summingIn, summingOut) = peak;
         if(std::abs(transition->GetEnergy() - centroid) < transition->GetEnergyUncertainty() + centroidErr + TEfficiencyCalibrator::CalibrationUncertainty()) {
            energy.push_back(transition->GetEnergy());
            energyErr.push_back(transition->GetEnergyUncertainty());
            efficiency.push_back(correctedArea / transition->GetIntensity());
            efficiencyErr.push_back(TMath::Sqrt(TMath::Power(correctedAreaErr / transition->GetIntensity(), 2) + TMath::Power(correctedArea / transition->GetIntensity() * transition->GetIntensityUncertainty() / transition->GetIntensity(), 2)));
            uncorrEfficiency.push_back(peakArea / transition->GetIntensity());
            uncorrEfficiencyErr.push_back(TMath::Sqrt(TMath::Power(peakAreaErr / transition->GetIntensity(), 2) + TMath::Power(peakArea / transition->GetIntensity() * transition->GetIntensityUncertainty() / transition->GetIntensity(), 2)));
            peakAreaVec.push_back(peakArea);
            peakAreaErrVec.push_back(peakAreaErr);
            summingInVec.push_back(summingIn);
            summingOutVec.push_back(summingOut);
            ++goodPeaks;
         } else if(fVerboseLevel > 2) {
            std::cout << "Rejecting centroid " << centroid << " +- " << centroidErr << " with area " << correctedArea << " +- " << correctedAreaErr << " for transition at " << transition->GetEnergy() << " +- " << transition->GetEnergyUncertainty() << " using calibration uncertainty " << TEfficiencyCalibrator::CalibrationUncertainty() << " (" << std::abs(transition->GetEnergy() - centroid) << " >= " << transition->GetEnergyUncertainty() + centroidErr + TEfficiencyCalibrator::CalibrationUncertainty() << ")" << std::endl;
         }
      }
      auto* effGraph = new TGraphErrors(goodPeaks, energy.data(), efficiency.data(), energyErr.data(), efficiencyErr.data());
      fEfficiencyGraph->Add(effGraph, tab->GetName());
      auto* uncorrEffGraph = new TGraphErrors(goodPeaks, energy.data(), uncorrEfficiency.data());
      fUncorrEfficiencyGraph->Add(uncorrEffGraph, Form("%s uncorr. Eff.", tab->GetName()));
      auto* peakAreaGraph = new TGraphErrors(goodPeaks, energy.data(), peakAreaVec.data(), energyErr.data(), peakAreaErrVec.data());
      fPeakAreaGraph->Add(peakAreaGraph, tab->GetName());
      auto* summingInGraph = new TGraphErrors(goodPeaks, energy.data(), summingInVec.data());
      fSummingInGraph->Add(summingInGraph, Form("%s summing in", tab->GetName()));
      auto* summingOutGraph = new TGraphErrors(goodPeaks, energy.data(), summingOutVec.data());
      fSummingOutGraph->Add(summingOutGraph, Form("%s summing out", tab->GetName()));
   }
   fEfficiencyGraph->Scale();
   fUncorrEfficiencyGraph->Scale();
   fEfficiencyGraph->SetAxisTitle("energy [keV];corrected efficiency [a.u.]");
   fUncorrEfficiencyGraph->SetAxisTitle("energy [keV];uncorrected efficiency [a.u.]");
   fPeakAreaGraph->SetAxisTitle("energy [keV];peak areas [a.u.]");
   fSummingInGraph->SetAxisTitle("energy [keV];summing in corrections [a.u.]");
   fSummingOutGraph->SetAxisTitle("energy [keV];summing out corrections [a.u.]");
   fEfficiencyGraph->Write(Form("%s_efficiency", fDataType.c_str()), TObject::kOverwrite);
   fUncorrEfficiencyGraph->Write(Form("%s_uncorr_efficiency", fDataType.c_str()), TObject::kOverwrite);
   fPeakAreaGraph->Write(Form("%s_peak_area", fDataType.c_str()), TObject::kOverwrite);
   fSummingInGraph->Write(Form("%s_summing_in", fDataType.c_str()), TObject::kOverwrite);
   fSummingOutGraph->Write(Form("%s_summing_out", fDataType.c_str()), TObject::kOverwrite);
   fMainDirectory->cd();
}

void TEfficiencyDatatypeTab::UpdatePeakType()
{
   if(fPeakTypeBox == nullptr) {
      throw std::runtime_error("Failed to find peak type box, but got a signal it was selected?");
   }
   if(fVerboseLevel > 3) {
      std::cout << "peak type box " << fPeakTypeBox << std::flush << ", getting selected " << fPeakTypeBox->GetSelected() << std::endl;
   }

   TEfficiencyCalibrator::PeakType(static_cast<TEfficiencyTab::EPeakType>(fPeakTypeBox->GetSelected()));
}

int TEfficiencyDatatypeTab::Degree()
{
   if(fDegreeEntry != nullptr) {
      TEfficiencyCalibrator::Degree(fDegreeEntry->GetNumber());
   }
   return TEfficiencyCalibrator::Degree();
}

/// The efficiency curves can be:
/// e(E) = ln E + (ln E)/E + (ln E)^2/E + (ln E)^3/E + (ln E)^4/E,
/// or the radware form
/// ln(e(E)) = ((a1 + a2 x + a3 x^2)^-a7 + (a4 + a5 y + a6 y^2)^-a7)^-1/a7
/// with x = ln(E/100), y = ln(E/1000)
/// or a polynomial ln(e(E)) = sum i 0->8 a_i (ln(E))^i (Ryan's & Andrew's PhD theses)
double TEfficiencyDatatypeTab::EfficiencyDebertin(double* x, double* par) // NOLINT(readability-non-const-parameter)
{
   double eff = par[0] * TMath::Log(x[0]) + par[1] * TMath::Log(x[0]) / x[0] + par[2] * TMath::Power(TMath::Log(x[0]) / x[0], 2) + par[3] * TMath::Power(TMath::Log(x[0]) / x[0], 4) + par[4] * TMath::Power(TMath::Log(x[0]) / x[0], 5);
   return eff;
}

double TEfficiencyDatatypeTab::EfficiencyRadware(double* val, double* par) // NOLINT(readability-non-const-parameter)
{
   double x      = TMath::Log(val[0] / 100.);
   double y      = TMath::Log(val[0] / 1000.);
   double logEff = TMath::Power(par[0] + par[1] * x + par[2] * x * x, -par[6]) + TMath::Power(par[3] + par[4] * y + par[5] * y * y, -1. / par[6]);
   return TMath::Exp(logEff);
}

double TEfficiencyDatatypeTab::EfficiencyPolynomial(double* x, double* par) // NOLINT(readability-non-const-parameter)
{
   // first parameter: degree of polynomial
   double logEff = 0;
   for(int i = 0; i < par[0]; ++i) {
      logEff += par[i + 1] * TMath::Power(TMath::Log(x[0]), i);
   }
   return TMath::Exp(logEff);
}

void TEfficiencyDatatypeTab::FitEfficiency()
{
   ReadValues();
   if(fEfficiency != nullptr) delete fEfficiency;
   // degree of fit function: 0 = ln(E)/E form, 1 = radware form, everything else polynomial ln(e(E)) = sum i 0->8 a_i (ln(E))^i (Ryan's & Andrew's PhD theses)
   switch(TEfficiencyCalibrator::Degree()) {
   case 0:
      fEfficiency = new TF1("EfficiencyDebertin", TEfficiencyDatatypeTab::EfficiencyDebertin, 0., 10000., 5);
      break;
   case 1:
      fEfficiency = new TF1("EfficiencyRadware", TEfficiencyDatatypeTab::EfficiencyRadware, 0., 10000., 7);
      break;
   default:
      fEfficiency = new TF1("EfficiencyPolynomial", TEfficiencyDatatypeTab::EfficiencyPolynomial, 0., 10000., TEfficiencyCalibrator::Degree() + 1);
      fEfficiency->FixParameter(0, TEfficiencyCalibrator::Degree());
      break;
   }

   fEfficiencyGraph->Fit(fEfficiency);
}

void TEfficiencyDatatypeTab::FittingControl(Int_t id)
{
   int currentTabId = fDataTab->GetCurrent();
   if(fVerboseLevel > 1) std::cout << __PRETTY_FUNCTION__ << ": id " << id << ", current tab id " << currentTabId << std::endl;
   ReadValues();
   switch(id) {
   case 1:   // re-fit
      fEfficiencyTab[currentTabId]->FindPeaks();
      UpdateEfficiencyGraph();
      break;
   case 2:   // re-fit all
      for(auto& tab : fEfficiencyTab) {
         tab->FindPeaks();
      }
      UpdateEfficiencyGraph();
      break;
   default:
      break;
   }
}

void TEfficiencyDatatypeTab::ReadValues()
{
   TEfficiencyCalibrator::Range(fRangeEntry->GetNumber());
   TEfficiencyCalibrator::Threshold(fThresholdEntry->GetNumber());
   TEfficiencyCalibrator::BgParam(fBgParamEntry->GetNumberEntry()->GetIntNumber());
   TEfficiencyCalibrator::Degree(fDegreeEntry->GetNumberEntry()->GetIntNumber());
   TEfficiencyCalibrator::CalibrationUncertainty(fCalibrationUncertaintyEntry->GetNumber());
}

//////////////////////////////////////// TEfficiencyCalibrator ////////////////////////////////////////
double                    TEfficiencyCalibrator::fRange                  = 20.;
double                    TEfficiencyCalibrator::fThreshold              = 100.;
int                       TEfficiencyCalibrator::fBgParam                = 20;
TEfficiencyTab::EPeakType TEfficiencyCalibrator::fPeakType               = TEfficiencyTab::EPeakType::kRWPeak;
int                       TEfficiencyCalibrator::fDegree                 = 0;
double                    TEfficiencyCalibrator::fCalibrationUncertainty = 1.;
bool                      TEfficiencyCalibrator::fShowRemovedFits        = false;

unsigned int TEfficiencyCalibrator::fLineHeight  = 20;
unsigned int TEfficiencyCalibrator::fWindowWidth = 1200;

TEfficiencyCalibrator::TEfficiencyCalibrator(int n...)
   : TGMainFrame(nullptr, TEfficiencyCalibrator::WindowWidth() + 10, TEfficiencyCalibrator::WindowWidth() / 2)   // +10 for padding between frames
{
   // remove old file with redirect from fitting
   std::remove("fitOutput.txt");

   // we want to store the histograms in a vector of types (suppressed/addback), but we don't know yet if we have all types present
   // so we create a vector of 4 now (unsuppressed singles, suppressed singles, unsuppressed addback, and suppressed addback)
   // and then remove the missing ones at the end
   fHistograms.resize(4);
   fDataType = {"Unsuppressed Singles", "Suppressed Singles", "Unsuppressed Addback", "Suppressed Addback"};

   va_list args;
   va_start(args, n);
   bool allSourcesFound = true;
   for(int i = 0; i < n; ++i) {
      const char* name = va_arg(args, const char*);
      fFiles.push_back(new TFile(name));
      if(!fFiles.back()->IsOpen()) {
         std::cerr << "Failed to open " << i + 1 << ". file \"" << name << "\"" << std::endl;
         fFiles.pop_back();
         continue;
      }
      bool goodFile = false;
      if(fVerboseLevel > 0) {
         std::cout << "Checking file \"" << name << "\":";
      }
      if(fFiles.back()->FindKey("griffinE") != nullptr) {
         // unsuppressed singles data
         fHistograms[0].push_back(std::make_tuple(
            static_cast<TH1*>(fFiles.back()->Get("griffinE")),
            static_cast<TH2*>(fFiles.back()->Get("griffinGriffinE180Corr")),
            static_cast<TH2*>(fFiles.back()->Get("griffinGriffinESum180Corr"))));
         goodFile = true;
         if(fVerboseLevel > 0) {
            std::cout << " found unsuppressed singles";
         }
      }
      if(fFiles.back()->FindKey("griffinESupp") != nullptr) {
         // suppressed singles data
         fHistograms[1].push_back(std::make_tuple(
            static_cast<TH1*>(fFiles.back()->Get("griffinESupp")),
            static_cast<TH2*>(fFiles.back()->Get("griffinGriffinEMixed180Corr")),
            static_cast<TH2*>(fFiles.back()->Get("griffinGriffinESuppSum180Corr"))));
         goodFile = true;
         if(fVerboseLevel > 0) {
            std::cout << " found suppressed singles";
         }
      }
      if(fFiles.back()->FindKey("griffinEAddback") != nullptr) {
         // unsuppressed addback data
         fHistograms[2].push_back(std::make_tuple(
            static_cast<TH1*>(fFiles.back()->Get("griffinEAddback")),
            static_cast<TH2*>(fFiles.back()->Get("griffinGriffinEAddback180Corr")),
            static_cast<TH2*>(fFiles.back()->Get("griffinGriffinEAddbackSum180Corr"))));
         goodFile = true;
         if(fVerboseLevel > 0) {
            std::cout << " found unsuppressed addback";
         }
      }
      if(fFiles.back()->FindKey("griffinESuppAddback") != nullptr) {
         // suppressed addback data
         fHistograms[3].push_back(std::make_tuple(
            static_cast<TH1*>(fFiles.back()->Get("griffinESuppAddback")),
            static_cast<TH2*>(fFiles.back()->Get("griffinGriffinEMixedAddback180Corr")),
            static_cast<TH2*>(fFiles.back()->Get("griffinGriffinESuppAddbackSum180Corr"))));
         goodFile = true;
         if(fVerboseLevel > 0) {
            std::cout << " found suppressed addback";
         }
      }
      if(fVerboseLevel > 0) {
         std::cout << ": got " << fFiles.size() << " files, " << fHistograms[0].size() << " unsuppressed singles, " << fHistograms[1].size() << " suppressed singles, " << fHistograms[2].size() << " unsuppressed addback, " << fHistograms[3].size() << " suppressed addback" << std::endl;
      }
      if(!goodFile) {
         std::cerr << "Failed to find any histogram in " << i + 1 << ". file \"" << name << "\"" << std::endl;
         fFiles.pop_back();
         continue;
      }
      fSources.push_back(nullptr);
      // check if the file name matches any source
      if(std::strstr(name, "22Na") != nullptr) {
         fSources.back() = new TNucleus("22Na");
      } else if(std::strstr(name, "56Co")) {
         fSources.back() = new TNucleus("56Co");
      } else if(std::strstr(name, "60Co")) {
         fSources.back() = new TNucleus("60Co");
      } else if(std::strstr(name, "133Ba")) {
         fSources.back() = new TNucleus("133Ba");
      } else if(std::strstr(name, "152Eu")) {
         fSources.back() = new TNucleus("152Eu");
      } else if(std::strstr(name, "241Am")) {
         fSources.back() = new TNucleus("241Am");
      } else {
         allSourcesFound = false;
      }
   }
   va_end(args);

   // now we remove any type from the histogram vector that has no entries
   auto type = fDataType.begin();
   for(auto it = fHistograms.begin(); it != fHistograms.end();) {
      if(it->size() != fFiles.size()) {
         if(fVerboseLevel > 1) {
            std::cout << std::distance(fHistograms.begin(), it) << ": found " << it->size() << " histograms for " << fFiles.size() << " files, removing this type" << std::endl;
         }
         it   = fHistograms.erase(it);
         type = fDataType.erase(type);
      } else {
         ++it;
         ++type;
      }
   }

   if(fVerboseLevel > 0) {
      std::cout << "Read " << fFiles.size() << " files, got";
      for(auto& vec : fHistograms) {
         std::cout << " " << vec.size();
      }
      std::cout << " histograms" << std::endl;
   }

   if(fHistograms.empty()) {
      throw std::runtime_error("Unable to find any suitable histograms in the provided file(s)!");
   }

   // quick sanity check, should have at least one vector the size of that vector should equal the source size and file size
   if(fSources.size() != fFiles.size() || fHistograms[0].size() != fFiles.size() || fHistograms.size() != fDataType.size()) {
      std::stringstream str;
      str << "Wrong sizes, from " << fFiles.size() << " file(s) we got " << fSources.size() << " source(s), and " << fHistograms[0].size() << " histogram(s), " << fDataType.size() << " data types, and " << fHistograms.size() << " data type histogram(s)!" << std::endl;
      throw std::runtime_error(str.str());
   }

   fOutput = new TFile("TEfficiencyCalibrator.root", "recreate");
   if(!fOutput->IsOpen()) {
      throw std::runtime_error("Unable to open output file \"TEfficiencyCalibrator.root\"!");
   }

   // build the first screen
   if(!allSourcesFound) {
      BuildFirstInterface();
      MakeFirstConnections();
   } else {
      SecondWindow();
   }

   // Set a name to the main frame
   SetWindowName("EffiencyCalibrator");

   // Map all subwindows of main frame
   MapSubwindows();

   // Initialize the layout algorithm
   Resize(GetDefaultSize());

   // Map main frame
   MapWindow();

   fOutput->Close();
}

TEfficiencyCalibrator::~TEfficiencyCalibrator()
{
   for(auto& file : fFiles) {
      file->Close();
   }
   if(fOutput != nullptr) fOutput->Close();
}

void TEfficiencyCalibrator::DeleteElement(TGFrame* element)
{
   HideFrame(element);
   RemoveFrame(element);
   // delete element;
   // element = nullptr;
}

void TEfficiencyCalibrator::BuildFirstInterface()
{
   /// Build initial interface with histogram <-> source assignment

   auto* layoutManager = new TGTableLayout(this, fFiles.size() + 1, 2, true, 5);
   if(fVerboseLevel > 1) std::cout << "created table layout manager with 2 columns, " << fFiles.size() + 1 << " rows" << std::endl;
   SetLayoutManager(layoutManager);

   // The matrices and source selection boxes
   size_t i = 0;
   for(i = 0; i < fFiles.size(); ++i) {
      fSourceLabel.push_back(new TGLabel(this, fFiles[i]->GetName()));
      if(fVerboseLevel > 2) std::cout << "Text height " << fSourceLabel.back()->GetFont()->TextHeight() << std::endl;
      fSourceBox.push_back(new TGComboBox(this, "Select source", kSourceBox + fSourceBox.size()));
      if(fSources[i] != nullptr) {
         fSourceBox.back()->AddEntry(fSources[i]->GetName(), i);
         fSourceBox.back()->Select(0);
         if(fVerboseLevel > 2) std::cout << "Found source, setting entry to " << fSources[i]->GetName() << " and selecting " << fSourceBox.back()->GetSelected() << std::endl;
      } else {
         fSourceBox.back()->AddEntry("22Na", k22Na);
         fSourceBox.back()->AddEntry("56Co", k56Co);
         fSourceBox.back()->AddEntry("60Co", k60Co);
         fSourceBox.back()->AddEntry("133Ba", k133Ba);
         fSourceBox.back()->AddEntry("152Eu", k152Eu);
         fSourceBox.back()->AddEntry("241Am", k241Am);
         if(fVerboseLevel > 2) std::cout << "Didn't find source, created source box with " << fSourceBox.back()->GetNumberOfEntries() << std::endl;
      }
      fSourceBox.back()->SetMinHeight(200);
      fSourceBox.back()->Resize(100, LineHeight());
      if(fVerboseLevel > 2) std::cout << "Attaching " << i << ". label to 0, 1, " << i << ", " << i + 1 << ", and box to 1, 2, " << i << ", " << i + 1 << std::endl;
      AddFrame(fSourceLabel.back(), new TGTableLayoutHints(0, 1, i, i + 1, kLHintsRight | kLHintsCenterY));
      AddFrame(fSourceBox.back(), new TGTableLayoutHints(1, 2, i, i + 1, kLHintsLeft | kLHintsCenterY));
   }

   // The buttons
   if(fVerboseLevel > 1) std::cout << "Attaching start button to 0, 2, " << i << ", " << i + 1 << std::endl;
   fStartButton = new TGTextButton(this, "Accept && Continue", kStartButton);
   if(fVerboseLevel > 1) std::cout << "start button " << fStartButton << std::endl;
   AddFrame(fStartButton, new TGTableLayoutHints(0, 2, i, i + 1, kLHintsCenterX | kLHintsCenterY));
   Layout();
}

void TEfficiencyCalibrator::MakeFirstConnections()
{
   /// Create connections for the file <-> source assignment interface

   // Connect the selection of the source
   for(auto* box : fSourceBox) {
      box->Connect("Selected(Int_t, Int_t)", "TEfficiencyCalibrator", this, "SetSource(Int_t, Int_t)");
   }

   // Connect the clicking of buttons
   fStartButton->Connect("Clicked()", "TEfficiencyCalibrator", this, "Start()");
}

void TEfficiencyCalibrator::DisconnectFirst()
{
   /// Disconnect all signals from the file <-> source assignment interface
   for(auto* box : fSourceBox) {
      box->Disconnect("Selected(Int_t, Int_t)", this, "SetSource(Int_t, Int_t)");
   }
   fStartButton->Disconnect("Clicked()", this, "Start()");
}

void TEfficiencyCalibrator::DeleteFirst()
{

   fSourceBox.clear();
   DeleteElement(fStartButton);
   if(fVerboseLevel > 2) std::cout << "Deleted start button " << fStartButton << std::endl;
}

void TEfficiencyCalibrator::SetSource(Int_t windowId, Int_t entryId)
{
   int index = windowId - kSourceBox;
   if(fVerboseLevel > 1) std::cout << __PRETTY_FUNCTION__ << ": windowId " << windowId << ", entryId " << entryId << " => " << index << std::endl;
   TNucleus* nucleus = fSources[index];
   if(nucleus != nullptr) delete nucleus;
   nucleus         = new TNucleus(fSourceBox[index]->GetListBox()->GetEntry(entryId)->GetTitle());
   fSources[index] = nucleus;
}

void TEfficiencyCalibrator::Start()
{
   if(fVerboseLevel > 1) std::cout << __PRETTY_FUNCTION__ << ": fEmitter " << fEmitter << ", fStartButton " << fStartButton << std::endl;
   if(fEmitter == nullptr) {   // we only want to do this once at the beginning (after fEmitter was initialized to nullptr)
      fEmitter = fStartButton;
      TTimer::SingleShot(100, "TEfficiencyCalibrator", this, "HandleTimer()");
   }
}

void TEfficiencyCalibrator::HandleTimer()
{
   if(fVerboseLevel > 1) std::cout << __PRETTY_FUNCTION__ << ": fEmitter " << fEmitter << ", fStartButton " << fStartButton << std::endl;
   if(fEmitter == fStartButton) {
      // disconnect signals of first screen and remove all elements
      DisconnectFirst();
      RemoveAll();
      DeleteFirst();

      SecondWindow();
   }
}

void TEfficiencyCalibrator::SecondWindow()
{
   if(fVerboseLevel > 1) std::cout << __PRETTY_FUNCTION__ << std::endl;
   // check that all sources have been set
   for(size_t i = 0; i < fSources.size(); ++i) {
      if(fSources[i] == nullptr) {
         std::cerr << "Source " << i << " not set!" << std::endl;
         return;
      }
      if(fVerboseLevel > 0) {
         std::cout << i << " - source " << fSources[i] << " = " << fSources[i]->GetName() << std::endl;
      }
   }
   // now check that we don't have the same source twice (which wouldn't make sense)
   for(size_t i = 0; i < fSources.size(); ++i) {
      for(size_t j = i + 1; j < fSources.size(); ++j) {
         if(*(fSources[i]) == *(fSources[j])) {
            std::cerr << "Duplicate sources: " << i << " - " << fSources[i]->GetName() << " and " << j << " - " << fSources[j]->GetName() << std::endl;
            return;
         }
      }
   }

   SetLayoutManager(new TGHorizontalLayout(this));

   // create intermediate progress bar
   fProgressBar      = new TGHProgressBar(this, TGProgressBar::kFancy, WindowWidth() / 2);
   int nofHistograms = 0;
   for(auto& type : fHistograms) {   // loop over all data types and add the number of source to the counter
      nofHistograms += type.size();
   }
   nofHistograms *= 3;   // there are three histograms for each data type/source combo
   fProgressBar->SetRange(0., nofHistograms);
   fProgressBar->Percent(true);
   if(fVerboseLevel > 2) std::cout << "Set range of progress bar to 0. - " << fProgressBar->GetMax() << " = " << fFiles.size() << std::endl;
   AddFrame(fProgressBar, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY | kLHintsExpandX | kLHintsExpandY, 0, 0, 0, 0));

   // Map all subwindows of main frame
   MapSubwindows();

   // Initialize the layout algorithm
   Resize(TGDimension(WindowWidth(), LineHeight()));

   // Map main frame
   MapWindow();

   // create second screen and its connections
   if(fVerboseLevel > 2) std::cout << "Starting to build second interface:" << std::endl;
   BuildSecondInterface();
   MakeSecondConnections();

   // remove progress bar
   DeleteElement(fProgressBar);

   // Map all subwindows of main frame
   MapSubwindows();

   // Initialize the layout algorithm
   Resize(TGDimension(WindowWidth(), WindowWidth() / 2));

   // Map main frame
   MapWindow();
   if(fVerboseLevel > 2) std::cout << __PRETTY_FUNCTION__ << " done" << std::endl;
}

void TEfficiencyCalibrator::BuildSecondInterface()
{
   SetLayoutManager(new TGHorizontalLayout(this));

   // left frame with tabs for each source
   fDatatypeTab = new TGTab(this, WindowWidth(), WindowWidth() / 2);
   fEfficiencyDatatypeTab.resize(fHistograms.size());
   fEfficiencyGraph.resize(fHistograms.size());
   if(fVerboseLevel > 2) std::cout << __PRETTY_FUNCTION__ << " creating " << fHistograms.size() << " tabs" << std::endl;
   for(size_t i = 0; i < fHistograms.size(); ++i) {
      if(fVerboseLevel > 2) std::cout << i << ": Creating efficiency source tab using " << fHistograms[i].size() << " histograms, and " << fSources.size() << " sources, " << fRange << ", " << fThreshold << ", " << fProgressBar << std::endl;
      auto* frame = fDatatypeTab->AddTab(fDataType[i].c_str());
      std::replace(fDataType[i].begin(), fDataType[i].end(), ' ', '_');
      fEfficiencyDatatypeTab[i] = new TEfficiencyDatatypeTab(this, fSources, fHistograms[i], frame, fDataType[i], fProgressBar, fVerboseLevel);
      // fEfficiencyDatatypeTab[i]->UpdateEfficiencyGraph();
      fEfficiencyGraph[i] = fEfficiencyDatatypeTab[i]->EfficiencyGraph();
   }
   AddFrame(fDatatypeTab, new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 0, 0, 0, 0));

   if(fVerboseLevel > 2) std::cout << "Second interface done" << std::endl;
}

void TEfficiencyCalibrator::MakeSecondConnections()
{
   if(fVerboseLevel > 1) std::cout << __PRETTY_FUNCTION__ << std::endl;
   // we don't need to connect the range, threshold, and degree number entries, those are automatically read when we start the calibration
   for(auto* sourceTab : fEfficiencyDatatypeTab) {
      sourceTab->MakeConnections();
   }
}

void TEfficiencyCalibrator::DisconnectSecond()
{
   if(fVerboseLevel > 1) std::cout << __PRETTY_FUNCTION__ << std::endl;
   for(auto* sourceTab : fEfficiencyDatatypeTab) {
      sourceTab->Disconnect();
   }
}

#endif
