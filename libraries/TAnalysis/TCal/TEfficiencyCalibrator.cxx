#include "TEfficiencyCalibrator.h"

#include <cstdarg>

#include "TGTableLayout.h"
#include "TTimer.h"

#include "TGauss.h"
#include "TABPeak.h"
#include "TAB3Peak.h"
#include "TRWPeak.h"

//////////////////////////////////////// TEfficiencyTab ////////////////////////////////////////
TEfficiencyTab::TEfficiencyTab(TEfficiencySourceTab* parent, TNucleus* nucleus, std::tuple<TH1*, TH2*, TH2*> hists, TGCompositeFrame* frame, const double& range, const double& threshold, const int& bgParam)
   : fFrame(frame), fNucleus(nucleus), fParent(parent), fRange(range), fThreshold(threshold), fBgParam(bgParam)
{
	fSingles    = std::get<0>(hists);
	fSummingOut = std::get<1>(hists);
	fSummingIn  = std::get<2>(hists);
	BuildInterface();
	FindPeaks();
}

TEfficiencyTab::~TEfficiencyTab()
{
}

void TEfficiencyTab::BuildInterface()
{
   // top frame with one canvas, status bar, and controls
   fTopFrame = new TGHorizontalFrame(fFrame, 1200, 450);
   fProjectionCanvas = new TRootEmbeddedCanvas("ProjectionCanvas", fTopFrame, 600, 400);

   fTopFrame->AddFrame(fProjectionCanvas, new TGLayoutHints(kLHintsLeft   | kLHintsTop | kLHintsExpandY | kLHintsExpandX, 2, 2, 2, 2));

   fFrame->AddFrame(fTopFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));

   fStatusBar = new TGStatusBar(fFrame, 1200, 50);
   int parts[] = {35, 15, 20, 15, 15 };
   fStatusBar->SetParts(parts, 5);

	fFrame->AddFrame(fStatusBar, new TGLayoutHints(kLHintsBottom | kLHintsExpandX, 2, 2, 2, 2));
}

void TEfficiencyTab::FindPeaks()
{
	/// Find and fit the peaks in the singles histogram, then checks for each peak how much summing in and summing out happens.
	
	fSummingOutTotalProj = fSummingOut->ProjectionY();
	fSummingOutTotalProjBg = fSummingOutTotalProj->ShowBackground(fBgParam);
	// loop through all gamma-rays of the source
	for(TObject* obj : *(fNucleus->GetTransitionList())) {
		auto transition = static_cast<TTransition*>(obj);
		auto energy = transition->GetEnergy();
		fPeakFitter.RemoveAllPeaks();
		fPeakFitter.ResetInitFlag();
		fPeakFitter.SetRange(energy - fRange, energy + fRange);
		TSinglePeak* peak;
		switch(fPeakType) {
			case kRWPeak:
				peak = new TRWPeak(energy);
				break;
			case kABPeak:
				peak = new TABPeak(energy);
				break;
			case kAB3Peak:
				peak = new TAB3Peak(energy);
				break;
			case kGauss:
				peak = new TGauss(energy);
				break;
			default:
				std::cerr<<"Unknow peak type "<<fPeakType<<", defaulting to TRWPeak!"<<std::endl;
				peak = new TRWPeak(energy);
		}
		fPeakFitter.AddPeak(peak);
		fPeakFitter.Fit(fSingles, "qretryfit");
		if(peak->Area() < fThreshold) {
			if(fVerboseLevel > 2) {
				std::cout<<"Skipping peak at "<<energy<<" keV with centroid "<<peak->Centroid()<<", FWHM "<<peak->FWHM()<<", and area "<<peak->Area()<<std::endl;
			}
			continue;
		}
		// increase number of points of fit function
		fPeakFitter.GetFitFunction()->SetNpx(1000);
		double fwhm = peak->FWHM();
		double centroid = peak->Centroid();
		// for summing in we need to estimate the background and subtract that from the integral
		fSummingInProj.push_back(fSummingIn->ProjectionY(Form("%s_%.0f-%.0f", fSummingIn->GetName(), centroid-fwhm/2., centroid+fwhm/2.), fSummingIn->GetXaxis()->FindBin(centroid-fwhm/2.), fSummingIn->GetXaxis()->FindBin(centroid+fwhm/2.)));
		fSummingInProjBg.push_back(fSummingInProj.back()->ShowBackground(fBgParam));
		double summingIn = fSummingInProj.back()->Integral() - fSummingInProjBg.back()->Integral();
		// for summing out we need to do a background subtraction - to make this easier we just use a total projection and scale it according to the bg integral?
		fSummingOutProj.push_back(fSummingOut->ProjectionY(Form("%s_%.0f-%.0f", fSummingOut->GetName(), centroid-fwhm/2., centroid+fwhm/2.), fSummingOut->GetXaxis()->FindBin(centroid-fwhm/2.), fSummingOut->GetXaxis()->FindBin(centroid+fwhm/2.)));
		double ratio = fSummingOutTotalProjBg->Integral(fSummingOutTotalProjBg->GetXaxis()->FindBin(centroid-fwhm/2.), fSummingOutTotalProjBg->GetXaxis()->FindBin(centroid+fwhm/2.))/fSummingOutTotalProjBg->Integral();
		double summingOut = fSummingOutProj.back()->Integral() - fSummingOutTotalProj->Integral()*ratio;

		double correctedArea = peak->Area() - summingIn + summingOut;
		// uncertainties for summing in and summing out is sqrt(N) (?)
		double correctedAreaErr = TMath::Sqrt(TMath::Power(peak->AreaErr(), 2) + summingIn + summingOut);
		fPeaks.push_back(std::make_tuple(energy, centroid, correctedArea, correctedAreaErr, transition->GetIntensity(), transition->GetIntensityUncertainty(), peak->Area(), peak->AreaErr(), summingIn, summingOut));
	}
}

void TEfficiencyTab::MakeConnections()
{
}

void TEfficiencyTab::Disconnect()
{
}


//////////////////////////////////////// TEfficiencySourceTab ////////////////////////////////////////
TEfficiencySourceTab::TEfficiencySourceTab(TEfficiencyCalibrator* parent, TNucleus* nucleus, std::vector<std::tuple<TH1*, TH2*, TH2*>> hists, TGCompositeFrame* frame, const double& range, const double& threshold, const int& bgParam, TGHProgressBar* progressBar)
   : fFrame(frame), fProgressBar(progressBar), fNucleus(nucleus), fParent(parent), fRange(range), fThreshold(threshold), fBgParam(bgParam)
{
	fDataTab = new TGTab(fFrame, 1200, 500);

	fEfficiencyTab.resize(hists.size(), nullptr);
	for(size_t i = 0; i < hists.size(); ++i) {
		fEfficiencyTab[i] = new TEfficiencyTab(this, fNucleus, hists[i], fDataTab->AddTab(Form("%s_%d", fNucleus->GetName(), (int)i)), fRange, fThreshold, fBgParam);
		fProgressBar->Increment(1);
	}

	fFrame->AddFrame(fDataTab, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));

	fParameterFrame = new TGGroupFrame(fFrame, "Parameters", kHorizontalFrame);
	fParameterFrame->SetLayoutManager(new TGMatrixLayout(fParameterFrame, 0, 2, 5));
	fRangeLabel = new TGLabel(fParameterFrame, "Range");
	fRangeEntry = new TGNumberEntry(fParameterFrame, fRange, 5, kRangeEntry, TGNumberFormat::EStyle::kNESRealTwo, TGNumberFormat::EAttribute::kNEAPositive);
	fBgParamLabel = new TGLabel(fParameterFrame, "BG Parameter");
	fBgParamEntry = new TGNumberEntry(fParameterFrame, fBgParam, 5, kBgParamEntry, TGNumberFormat::EStyle::kNESRealTwo, TGNumberFormat::EAttribute::kNEAPositive);
	fThresholdLabel = new TGLabel(fParameterFrame, "Threshold");
	fThresholdEntry = new TGNumberEntry(fParameterFrame, fThreshold, 5, kThresholdEntry, TGNumberFormat::EStyle::kNESRealThree, TGNumberFormat::EAttribute::kNEAPositive);
	fPeakTypeBox = new TGComboBox(fParameterFrame, kPeakTypeBox);
	fPeakTypeBox->AddEntry("Radware", TEfficiencyTab::EPeakType::kRWPeak); 
	fPeakTypeBox->AddEntry("Addback", TEfficiencyTab::EPeakType::kABPeak); 
	fPeakTypeBox->AddEntry("Addback3", TEfficiencyTab::EPeakType::kAB3Peak); 
	fPeakTypeBox->AddEntry("Gaussian", TEfficiencyTab::EPeakType::kGauss); 

	fParameterFrame->AddFrame(fRangeLabel);
	fParameterFrame->AddFrame(fRangeEntry);
	fParameterFrame->AddFrame(fBgParamLabel);
	fParameterFrame->AddFrame(fBgParamEntry);
	fParameterFrame->AddFrame(fThresholdLabel);
	fParameterFrame->AddFrame(fThresholdEntry);
	fParameterFrame->AddFrame(fPeakTypeBox);

	fFrame->AddFrame(fParameterFrame, new TGLayoutHints(kLHintsBottom | kLHintsExpandX, 2, 2, 2, 2));
}

TEfficiencySourceTab::~TEfficiencySourceTab()
{
}

void TEfficiencySourceTab::MakeConnections()
{
}

void TEfficiencySourceTab::Disconnect()
{
}

//////////////////////////////////////// TEfficiencyCalibrator ////////////////////////////////////////
TEfficiencyCalibrator::TEfficiencyCalibrator(double range, double threshold, int n...)
	: TGMainFrame(nullptr, 1200, 600)
{
	fRange = range;
	fThreshold = threshold;

	va_list args;
	va_start(args, n);
	for(int i = 0; i < n; ++i) {
		const char* name = va_arg(args, const char*);
		fFiles.push_back(new TFile(name));
		if(!fFiles.back()->IsOpen()) {
			std::cerr<<"Failed to open "<<i+1<<". file \""<<name<<"\""<<std::endl;
			fFiles.pop_back();
			continue;
		}
		bool goodFile = false;
		if(fFiles.back()->FindKey("griffinE") != nullptr) {
			// unsuppressed singles data
			fHistograms.push_back(std::vector<std::tuple<TH1*, TH2*, TH2*>>());
			fHistograms.back().push_back(std::make_tuple(
						static_cast<TH1*>(fFiles.back()->Get("griffinE")), 
						static_cast<TH2*>(fFiles.back()->Get("griffinGriffinE180Corr")), 
						static_cast<TH2*>(fFiles.back()->Get("griffinGriffinESum180Corr"))));
			goodFile = true;
		}
		if(fFiles.back()->FindKey("griffinESupp") != nullptr) {
			// suppressed singles data
			fHistograms.push_back(std::vector<std::tuple<TH1*, TH2*, TH2*>>());
			fHistograms.back().push_back(std::make_tuple(
						static_cast<TH1*>(fFiles.back()->Get("griffinESupp")), 
						static_cast<TH2*>(fFiles.back()->Get("griffinGriffinEMixed180Corr")), 
						static_cast<TH2*>(fFiles.back()->Get("griffinGriffinESuppSum180Corr"))));
			goodFile = true;
		}
		if(fFiles.back()->FindKey("griffinEAddback") != nullptr) {
			// unsuppressed addback data
			fHistograms.push_back(std::vector<std::tuple<TH1*, TH2*, TH2*>>());
			fHistograms.back().push_back(std::make_tuple(
						static_cast<TH1*>(fFiles.back()->Get("griffinEAddback")), 
						static_cast<TH2*>(fFiles.back()->Get("griffinGriffinEAddback180Corr")), 
						static_cast<TH2*>(fFiles.back()->Get("griffinGriffinEAddbackSum180Corr"))));
			goodFile = true;
		}
		if(fFiles.back()->FindKey("griffinESuppAddback") != nullptr) {
			// suppressed addback data
			fHistograms.push_back(std::vector<std::tuple<TH1*, TH2*, TH2*>>());
			fHistograms.back().push_back(std::make_tuple(
						static_cast<TH1*>(fFiles.back()->Get("griffinESuppAddback")), 
						static_cast<TH2*>(fFiles.back()->Get("griffinGriffinEMixedAddback180Corr")), 
						static_cast<TH2*>(fFiles.back()->Get("griffinGriffinESuppAddbackSum180Corr"))));
			goodFile = true;
		}
		if(!goodFile) {
			std::cerr<<"Failed to find any histogram in "<<i+1<<". file \""<<name<<"\""<<std::endl;
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
		}
	}
	va_end(args);

	if(fHistograms.empty()) {
      throw std::runtime_error("Unable to find any suitable histograms in the provided file(s)!");
	}

	// quick sanity check, should have at least one vector the size of that vector should equal the source size and file size
	if(fSources.size() != fFiles.size() || fHistograms[0].size() != fFiles.size()) {
		std::stringstream str;
		str<<"Wrong sizes, from "<<fFiles.size()<<" file(s) we got "<<fSources.size()<<" source(s), and "<<fHistograms[0].size()<<" histogram(s)!"<<std::endl;
		throw std::runtime_error(str.str());
	}

	fOutput = new TFile("TEfficiencyCalibrator.root", "recreate");
   if(!fOutput->IsOpen()) {
      throw std::runtime_error("Unable to open output file \"TEfficiencyCalibrator.root\"!");
   }

	// build the first screen
   BuildFirstInterface();
   MakeFirstConnections();

   // Set a name to the main frame
   SetWindowName("EffiencyCalibrator");

   // Map all subwindows of main frame
   MapSubwindows();

   // Initialize the layout algorithm
   Resize(GetDefaultSize());

   // Map main frame
   MapWindow();
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
	//delete element;
	//element = nullptr;
}

void TEfficiencyCalibrator::BuildFirstInterface()
{
	/// Build initial interface with histogram <-> source assignment

   auto layoutManager = new TGTableLayout(this, fFiles.size()+1, 2, true, 5);
   if(fVerboseLevel > 1) std::cout<<"created table layout manager with 2 columns, "<<fFiles.size()+1<<" rows"<<std::endl;
   SetLayoutManager(layoutManager);

   // The matrices and source selection boxes
   size_t i = 0;
   for(i = 0; i < fFiles.size(); ++i) {
      fSourceLabel.push_back(new TGLabel(this, fFiles[i]->GetName()));
      if(fVerboseLevel > 2) std::cout<<"Text height "<<fSourceLabel.back()->GetFont()->TextHeight()<<std::endl;
      fSourceBox.push_back(new TGComboBox(this, "Select source", kSourceBox + fSourceBox.size()));
		if(fSources[i] != nullptr) {
			fSourceBox.back()->AddEntry(fSources[i]->GetName(), i);
			fSourceBox.back()->Select(0);
		} else {
			fSourceBox.back()->AddEntry("22Na", k22Na);
			fSourceBox.back()->AddEntry("56Co", k56Co);
			fSourceBox.back()->AddEntry("60Co", k60Co);
			fSourceBox.back()->AddEntry("133Ba", k133Ba);
			fSourceBox.back()->AddEntry("152Eu", k152Eu);
			fSourceBox.back()->AddEntry("241Am", k241Am);
		}
		fSourceBox.back()->SetMinHeight(200);
		fSourceBox.back()->Resize(100, fLineHeight);
		if(fVerboseLevel > 2) std::cout<<"Attaching "<<i<<". label to 0, 1, "<<i<<", "<<i+1<<", and box to 1, 2, "<<i<<", "<<i+1<<std::endl;
		AddFrame(fSourceLabel.back(), new TGTableLayoutHints(0, 1, i, i+1, kLHintsRight | kLHintsCenterY));
		AddFrame(fSourceBox.back(),   new TGTableLayoutHints(1, 2, i, i+1, kLHintsLeft  | kLHintsCenterY));
	}

   // The buttons
   if(fVerboseLevel > 1) std::cout<<"Attaching start button to 0, 2, "<<i<<", "<<i+1<<std::endl;
   fStartButton = new TGTextButton(this, "Accept && Continue", kStartButton);
   if(fVerboseLevel > 1) std::cout<<"start button "<<fStartButton<<std::endl;
   AddFrame(fStartButton, new TGTableLayoutHints(0, 2, i, i+1, kLHintsCenterX | kLHintsCenterY));
   Layout();
}

void TEfficiencyCalibrator::MakeFirstConnections()
{
	/// Create connections for the file <-> source assignment interface
	
	// Connect the selection of the source
   for(auto box : fSourceBox) {
      box->Connect("Selected(Int_t, Int_t)", "TEfficiencyCalibrator", this, "SetSource(Int_t, Int_t)");
   }

   //Connect the clicking of buttons
   fStartButton->Connect("Clicked()", "TEfficiencyCalibrator", this, "Start()");
}

void TEfficiencyCalibrator::DisconnectFirst()
{
	/// Disconnect all signals from the file <-> source assignment interface
   for(auto box : fSourceBox) {
      box->Disconnect("Selected(Int_t, Int_t)", this, "SetSource(Int_t, Int_t)");
   }
   fStartButton->Disconnect("Clicked()", this, "Start()");
}

void TEfficiencyCalibrator::DeleteFirst()
{

   fSourceBox.clear();
   DeleteElement(fStartButton);
   if(fVerboseLevel > 2) std::cout<<"Deleted start button "<<fStartButton<<std::endl;
}

void TEfficiencyCalibrator::SetSource(Int_t windowId, Int_t entryId)
{
	int index = windowId-kSourceBox;
   if(fVerboseLevel > 1) std::cout<<__PRETTY_FUNCTION__<<": windowId "<<windowId<<", entryId "<<entryId<<" => "<<index<<std::endl;
   TNucleus* nucleus = fSources[index];
   if(nucleus != nullptr) delete nucleus;
   nucleus = new TNucleus(fSourceBox[index]->GetListBox()->GetEntry(entryId)->GetTitle());
   fSources[index] = nucleus;
}

void TEfficiencyCalibrator::Start()
{
	if(fVerboseLevel > 1) std::cout<<__PRETTY_FUNCTION__<<": fEmitter "<<fEmitter<<", fStartButton "<<fStartButton<<std::endl;
   if(fEmitter == nullptr) { // we only want to do this once at the beginning (after fEmitter was initialized to nullptr)
      fEmitter = fStartButton;
      TTimer::SingleShot(100, "TEfficiencyCalibrator", this, "HandleTimer()");
   }
}

void TEfficiencyCalibrator::HandleTimer()
{
	if(fVerboseLevel > 1) std::cout<<__PRETTY_FUNCTION__<<": fEmitter "<<fEmitter<<", fStartButton "<<fStartButton<<std::endl;
   if(fEmitter == fStartButton) {
      SecondWindow();
   }
}

void TEfficiencyCalibrator::SecondWindow()
{
	if(fVerboseLevel > 1) std::cout<<__PRETTY_FUNCTION__<<std::endl;
   // check that all sources have been set
   for(size_t i = 0; i < fSources.size(); ++i) {
      if(fSources[i] == nullptr) {
         std::cerr<<"Source "<<i<<" not set!"<<std::endl;
         return;
      }
   }
   // now check that we don't have the same source twice (which wouldn't make sense)
   for(size_t i = 0; i < fSources.size(); ++i) {
      for(size_t j = i + 1; j < fSources.size(); ++j) {
         if(*(fSources[i]) == *(fSources[j])) {
            std::cerr<<"Duplicate sources: "<<i<<" - "<<fSources[i]->GetName()<<" and "<<j<<" - "<<fSources[j]->GetName()<<std::endl;
            return;
         }
      }
   }

   // disconnect signals of first screen and remove all elements
   DisconnectFirst();
   RemoveAll();
   DeleteFirst();

   SetLayoutManager(new TGHorizontalLayout(this));

	// create intermediate progress bar
   fProgressBar = new TGHProgressBar(this, TGProgressBar::kFancy, 600);
   fProgressBar->SetRange(0., fFiles.size());
   fProgressBar->Percent(true);
   if(fVerboseLevel > 2) std::cout<<"Set range of progress bar to 0. - "<<fProgressBar->GetMax()<<" = "<<fFiles.size()<<std::endl;
   AddFrame(fProgressBar, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY | kLHintsExpandX | kLHintsExpandY, 0, 0, 0, 0));

   // Map all subwindows of main frame
   MapSubwindows();

   // Initialize the layout algorithm
   Resize(TGDimension(600, fLineHeight));

   // Map main frame
   MapWindow();

   // create second screen and its connections
   BuildSecondInterface();
   MakeSecondConnections();

   // remove progress bar
   DeleteElement(fProgressBar);

   // Map all subwindows of main frame
   MapSubwindows();

   // Initialize the layout algorithm
   Resize(TGDimension(1200, 600));

   // Map main frame
   MapWindow();
   if(fVerboseLevel > 2) std::cout<<__PRETTY_FUNCTION__<<" done"<<std::endl;
}

void TEfficiencyCalibrator::BuildSecondInterface()
{
   SetLayoutManager(new TGHorizontalLayout(this));

	// left frame with tabs for each source
	fLeftFrame = new TGVerticalFrame(this, 600, 600);
	fSourceTab = new TGTab(fLeftFrame, 600, 600);
   fEfficiencySourceTab.resize(fSources.size()+1);
   for(size_t i = 0; i < fSources.size(); ++i) {
      fEfficiencySourceTab[i] = new TEfficiencySourceTab(this, fSources[i], fHistograms[i], fSourceTab->AddTab(fSources[i]->GetName()), fRange, fThreshold, fBgParam, fProgressBar);
   }
	fLeftFrame->AddFrame(fSourceTab, new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 0, 0, 0, 0));
	AddFrame(fLeftFrame, new TGLayoutHints(kLHintsLeft | kLHintsExpandY, 2, 2, 2, 2));

	// right frame with canvas, status bar, and the degree entry
	fRightFrame = new TGVerticalFrame(this, 600, 600);
	fEfficiencyCanvas = new TRootEmbeddedCanvas("EfficiencyCanvas", fRightFrame, 600, 450);
	fStatusBar = new TGStatusBar(fRightFrame, 600, 50);
	fDegreeEntry = new TGNumberEntry(fRightFrame, fDegree, 2, kDegreeEntry, TGNumberFormat::EStyle::kNESInteger);
	fDegreeLabel = new TGLabel(fRightFrame, "Type of efficiency curve");
	fRightFrame->AddFrame(fDegreeEntry);
	fRightFrame->AddFrame(fDegreeLabel);
   AddFrame(fRightFrame, new TGLayoutHints(kLHintsRight | kLHintsExpandY, 2, 2, 2, 2));

   if(fVerboseLevel > 2) std::cout<<"Second interface done"<<std::endl;
}

void TEfficiencyCalibrator::MakeSecondConnections()
{
	if(fVerboseLevel > 1) std::cout<<__PRETTY_FUNCTION__<<std::endl;
   //fNavigationGroup->Connect("Clicked(Int_t)", "TEfficiencyCalibrator", this, "Navigate(Int_t)");
   // we don't need to connect the range, threshold, and degree number entries, those are automatically read when we start the calibration
   for(auto sourceTab : fEfficiencySourceTab) {
      sourceTab->MakeConnections();
   }
}

void TEfficiencyCalibrator::DisconnectSecond()
{
   if(fVerboseLevel > 1) std::cout<<__PRETTY_FUNCTION__<<std::endl;
   //fNavigationGroup->Disconnect("Clicked(Int_t)", this, "Navigate(Int_t)");
   for(auto sourceTab : fEfficiencySourceTab) {
      sourceTab->Disconnect();
   }
}

void TEfficiencyCalibrator::Navigate(Int_t id)
{
   // we get the current source tab id and use it to get the channel tab from the right source tab
   // since the current id only refers to the position within the open tabs we need to keep track of the actual id it relates to
   // for this we created a vector with all ids at the beginning and now remove the position correspoding to the current id when we remove the tab
   //int currentSourceId = fTab->GetCurrent();
   //int actualSourceId = fActualSourceId[currentSourceId];
   //auto currentTab = fEfficiencySourceTab[actualSourceId]->ChannelTab();
   //int currentChannelId = currentTab->GetCurrent();
   //int nofTabs = currentTab->GetNumberOfTabs();
   //if(fVerboseLevel > 1) std::cout<<__PRETTY_FUNCTION__<<": id "<<id<<", source tab id "<<currentSourceId<<", actual source tab id "<<actualSourceId<<", channel tab id "<<currentTab->GetCurrent()<<", # of tabs "<<nofTabs<<std::endl;
   //switch(id) {
   //   case 1: // previous
   //      currentTab->SetTab(currentChannelId-1);
   //      SelectedTab(currentChannelId-1);
   //      break;
   //   case 2: // find peaks
   //      FindPeaks();
   //      break;
   //   case 3: // find peaks fast
   //      FindPeaksFast();
   //      break;
   //   case 4: // calibrate
   //      Calibrate();
   //      break;
   //   case 5: // discard
   //      // select the next (or if we are on the last tab, the previous) tab
   //      if(currentChannelId < nofTabs - 1) {
   //         currentTab->SetTab(currentChannelId+1);
   //      } else {
   //         currentTab->SetTab(currentChannelId-1);
   //      }
   //      // remove the original active tab
   //      currentTab->RemoveTab(currentChannelId);
   //      break;
   //   case 6: // accept
   //      AcceptChannel(currentChannelId);
   //      break;
   //   case 7: // accept all (no argument = -1 = all)
   //      AcceptChannel();
   //      break;
   //   case 8: // next
   //      currentTab->SetTab(currentChannelId+1);
   //      SelectedTab(currentChannelId+1);
   //      break;
   //   default:
   //      break;
   //}
}

void TEfficiencyCalibrator::SelectedTab(Int_t id)
{
   /// Simple function that enables and disables the previous and next buttons depending on which tab was selected
   //if(fVerboseLevel > 1) std::cout<<__PRETTY_FUNCTION__<<": id "<<id<<std::endl;
   //if(id == 0) fPreviousButton->SetEnabled(false);
   //else        fPreviousButton->SetEnabled(true);

   //if(id == fEfficiencySourceTab[fTab->GetCurrent()]->ChannelTab()->GetNumberOfTabs() - 1) fNextButton->SetEnabled(false);
   //else                                                                          fNextButton->SetEnabled(true);
}


