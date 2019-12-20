#include "TCalibrateDescant.h"

double MaximumRecoilEnergy(double gammaEnergy)
{
	return gammaEnergy - gammaEnergy/(1.+2.*gammaEnergy/510.998928);
}

double SourceEnergy(const TCalibrateDescant::ESourceType& source)
{
	switch(source) {
		case TCalibrateDescant::ESourceType::k22NaLow:
			return MaximumRecoilEnergy(510.99895000);
		case TCalibrateDescant::ESourceType::k22NaHigh:
			return MaximumRecoilEnergy(1274.537);
		case TCalibrateDescant::ESourceType::k24NaLow:
			return MaximumRecoilEnergy(1368.626);
		case TCalibrateDescant::ESourceType::k24NaHigh:
			return MaximumRecoilEnergy(2754.007);
		case TCalibrateDescant::ESourceType::k60Co:
			return (MaximumRecoilEnergy(1173.228)+MaximumRecoilEnergy(1332.492))/2.;
		case TCalibrateDescant::ESourceType::k137Cs:
			return MaximumRecoilEnergy(661.657);
		case TCalibrateDescant::ESourceType::k152Eu121:
			return MaximumRecoilEnergy(121.7817);
		case TCalibrateDescant::ESourceType::k152Eu344:
			return MaximumRecoilEnergy(344.2785);
		case TCalibrateDescant::ESourceType::k152Eu1408:
			return MaximumRecoilEnergy(1408.006);
		case TCalibrateDescant::ESourceType::k241AmEdge:
			return MaximumRecoilEnergy(59.5409);
		case TCalibrateDescant::ESourceType::k241Am: // photopeak, not Compton edge!
			return 59.5409;
	}
	return 0.;
}

double SourceEnergyUncertainty(const TCalibrateDescant::ESourceType& source)
{
	switch(source) {
		case TCalibrateDescant::ESourceType::k22NaLow:
			return (MaximumRecoilEnergy(510.99895000)-MaximumRecoilEnergy(510.99895000))/2.;
		case TCalibrateDescant::ESourceType::k22NaHigh:
			return (MaximumRecoilEnergy(1274.537+0.007)-MaximumRecoilEnergy(1274.537-0.007))/2.;
		case TCalibrateDescant::ESourceType::k24NaLow:
			return (MaximumRecoilEnergy(1368.626+0.005)-MaximumRecoilEnergy(1368.626-0.005))/2.;
		case TCalibrateDescant::ESourceType::k24NaHigh:
			return (MaximumRecoilEnergy(2754.007+0.011)-MaximumRecoilEnergy(2754.007-0.011))/2.;
		case TCalibrateDescant::ESourceType::k60Co: // only taking one peaks uncertainty into account ...
			return (MaximumRecoilEnergy(1332.492+0.004)+MaximumRecoilEnergy(1332.492-0.004))/2.;
		case TCalibrateDescant::ESourceType::k137Cs:
			return (MaximumRecoilEnergy(661.657+0.003)-MaximumRecoilEnergy(661.657-0.003))/2.;
		case TCalibrateDescant::ESourceType::k152Eu121:
			return (MaximumRecoilEnergy(121.7817+0.0003)-MaximumRecoilEnergy(121.7817-0.0003))/2.;
		case TCalibrateDescant::ESourceType::k152Eu344:
			return (MaximumRecoilEnergy(344.2785+0.0012)-MaximumRecoilEnergy(344.2785-0.0012))/2.;
		case TCalibrateDescant::ESourceType::k152Eu1408:
			return (MaximumRecoilEnergy(1408.006+0.003)-MaximumRecoilEnergy(1408.006-0.003))/2.;
		case TCalibrateDescant::ESourceType::k241AmEdge:
			return (MaximumRecoilEnergy(59.5409+0.0001)-MaximumRecoilEnergy(59.5409-0.0001))/2.;
		case TCalibrateDescant::ESourceType::k241Am: // photopeak, not Compton edge!
			return 0.0001;
	}
	return 0.;
}

double FullEdge(double* x, double* par) {
  //0 - amplitude, 1 - position, 2 - sigma of the upper part (low x), 3 - dSigma of the lower part (high x)
  //4 - amplitude of gaussian peak, 5 - difference of peak position from edge position (par[2]), 6 - sigma of gaussian
  //7 - amplitude of noise gaussian, 8 - position of noise gaussian, 9 - sigma of noise gaussian, 10 - threshold
  //11 - threshold sigma, 12 - bg constant, 13 - bg amplitude, 14 - bg decay constant


  //all parameters should be positive - except maybe the relative peak position???
  for(int i = 0; i < 15; ++i) {
    par[i] = TMath::Abs(par[i]);
  }

  double thresholdFactor = (1.+TMath::Erf((x[0]-par[10])/par[11]))/2.;

  //create a smooth transition from sigma to sigma + dSigma
  double sigma = par[2] + par[3]/2.*(1.+TMath::Erf((x[0]-par[1])/(par[2]+par[3])));

  double edge = par[0]/2.*TMath::Erfc((x[0]-par[1])/sigma);
  double peak = par[4]*TMath::Exp(-0.5*TMath::Power((x[0]-(par[1]-par[5]))/par[6],2));
  double noise = par[7]*TMath::Exp(-0.5*TMath::Power((x[0]-par[8])/par[9],2));

  //Erfc goes from +2 for -inf to 0 for +inf, divide by 2 to make it go from 1 to 0
  return thresholdFactor*(edge + peak + noise + par[12] + par[13]*TMath::Exp(-par[14]*x[0]));
}

TGHorizontalFrame* TParameterInput::Build(TGVerticalFrame*& frame, const std::string& name, const Int_t& baseId, const Double_t& xmin, const Double_t& xmax)
{
	fBaseId = baseId;
	// create frames for entries
	fFrame = new TGHorizontalFrame(frame, 400, 400);

	// create entries and sliders
	// TGNumberEntry(parent window, value, digit width, id, style, attribute, limits, min, max)
	// style - Integer, Hex, Real, Degree, etc.
	// attribute - AnyNumber, NonNegative, or Positive
	// limits - NoLimits, LimitMin, LimitMax, LimitMinMax
	fLabel     = new TGLabel(fFrame, name.c_str());
	fSlider    = new TGTripleHSlider(fFrame, 200, kDoubleScaleBoth, fBaseId, kHorizontalFrame);
	fEntryLow  = new TGNumberEntry(fFrame, 0, 5, fBaseId,  TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive, TGNumberFormat::kNELLimitMinMax, xmin, xmax);
	fEntry     = new TGNumberEntry(fFrame, 0, 5, fBaseId+2,     TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive, TGNumberFormat::kNELLimitMinMax, xmin, xmax);
	fEntryHigh = new TGNumberEntry(fFrame, 0, 5, fBaseId+1, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive, TGNumberFormat::kNELLimitMinMax, xmin, xmax);

	// hints: hints, pad left, pad right, pad top, pad bottom
	fFrame->AddFrame(fLabel,     new TGLayoutHints(kLHintsTop | kLHintsLeft,                   1, 1, 1, 1));
	fFrame->AddFrame(fSlider,    new TGLayoutHints(kLHintsTop | kLHintsLeft  | kLHintsExpandX, 1, 1, 1, 1));
	// these go in reverse order because they are all right aligned!
	fFrame->AddFrame(fEntryHigh, new TGLayoutHints(kLHintsTop | kLHintsRight                 , 1, 1, 1, 1));
	fFrame->AddFrame(fEntry,     new TGLayoutHints(kLHintsTop | kLHintsRight                 , 1, 1, 1, 1));
	fFrame->AddFrame(fEntryLow,  new TGLayoutHints(kLHintsTop | kLHintsRight                 , 1, 1, 1, 1));

	return fFrame;
}

void TParameterInput::Set(double val, double low, double high)
{
	fEntry->SetNumber(val);
	std::cout<<fEntry->GetNumber()<<std::endl;
	fEntry->SetLimits(TGNumberFormat::kNELLimitMinMax, low, high);
	fEntryLow->SetNumber(0.01*val);
	std::cout<<fEntryLow->GetNumber()<<std::endl;
	fEntryLow->SetLimits(TGNumberFormat::kNELLimitMinMax, 0.1*low, high);
	fEntryHigh->SetNumber(100.*val);
	fEntryHigh->SetLimits(TGNumberFormat::kNELLimitMinMax, low, 10.*high);
	UpdateSlider();
	std::cout<<Name()<<": set val "<<val<<", "<<low<<" - "<<high<<": entry "<<fEntry->GetNumber()<<", low entry "<<fEntryLow->GetNumber()<<", high entry "<<fEntryHigh->GetNumber()<<std::endl;
}

void TParameterInput::UpdateSlider()
{
	fSlider->SetPointerPosition(Value());
	fSlider->SetPosition(LowLimit(), HighLimit());
	fSlider->SetRange(0.1*LowLimit(), 2.*HighLimit());
	fSlider->PositionChanged();
}

void TParameterInput::UpdateEntries()
{
	fEntry->SetNumber(fSlider->GetPointerPosition());
	fEntryLow->SetNumber(fSlider->GetMinPosition());
	fEntryHigh->SetNumber(fSlider->GetMaxPosition());
}

void TParameterInput::Connect(TCalibrateDescant* parent)
{
	fSlider->Connect("PointerPositionChanged()", "TParameterInput", this, "UpdateEntries()");
	fSlider->Connect("PositionChanged()", "TParameterInput", this, "UpdateEntries()");

	fEntry->Connect("ValueSet(Long_t)", "TParameterInput", this, "UpdateSlider()");
	fEntry->Connect("ValueSet(Long_t)", "TCalibrateDescant", parent, "UpdateInitial()");
	fEntryLow->Connect("ValueSet(Long_t)", "TParameterInput", this, "UpdateSlider()");
	fEntryHigh->Connect("ValueSet(Long_t)", "TParameterInput", this, "UpdateSlider()");
}

TCalibrateDescant::TCalibrateDescant(TH2* hist, const ESourceType& source)
	: TGMainFrame(nullptr, 100, 100, kMainFrame | kHorizontalFrame), fMatrix(hist), fSource(source) {
	BuildInterface();
	MakeConnections();
	CreateGraphicMembers();
	UpdateInterface();
}

void TCalibrateDescant::BuildInterface()
{
   // create the left frame and right frame
   fLeftFrame = new TGVerticalFrame(this, 400, 400);
   fRightFrame = new TGVerticalFrame(this, 400, 400);

   // create canvas widgets
   fFitCanvas = new TRootEmbeddedCanvas("FitCanvas", fLeftFrame, 200, 200);

   fCalibrationCanvas = new TRootEmbeddedCanvas("CalibrationCanvas", fLeftFrame, 200, 200);
	
	// maybe add splitter between canvases?

	// create status bar
	fStatusBar = new TGStatusBar(fLeftFrame, 400, 10, kHorizontalFrame);
	Int_t parts[] = { 25, 25, 50 };
	fStatusBar->SetParts(parts, 3);

	// build parameter entries
	fAmplitudeFrame = fAmplitude.Build(fRightFrame, "Amplitude: ", kAmplitude, 0., 1.);
	fPositionFrame = fPosition.Build(fRightFrame, "Position: ", kPosition, 0., 1.);
	fSigmaFrame = fSigma.Build(fRightFrame, "Sigma: ", kSigma, 0., 1.);
	fDSigmaFrame = fDSigma.Build(fRightFrame, "#DeltaSigma: ", kDSigma, 0., 1.);
	fPeakAmpFrame = fPeakAmp.Build(fRightFrame, "Peak Amplitude: ", kPeakAmp, 0., 1.);
	fPeakPosFrame = fPeakPos.Build(fRightFrame, "Peak Position: ", kPeakPos, 0., 1.);
	fPeakSigmaFrame = fPeakSigma.Build(fRightFrame, "Peak Sigma: ", kPeakSigma, 0., 1.);
	fNoiseAmpFrame = fNoiseAmp.Build(fRightFrame, "Noise Amplitude: ", kNoiseAmp, 0., 1.);
	fNoisePosFrame = fNoisePos.Build(fRightFrame, "Noise Position: ", kNoisePos, 0., 1.);
	fNoiseSigmaFrame = fNoiseSigma.Build(fRightFrame, "Noise Sigma: ", kNoiseSigma, 0., 1.);
	fThresholdFrame = fThreshold.Build(fRightFrame, "Threshold: ", kThreshold, 0., 1.);
	fThresholdSigmaFrame = fThresholdSigma.Build(fRightFrame, "Threshold Sigma: ", kThresholdSigma, 0., 1.);
	fBgConstFrame = fBgConst.Build(fRightFrame, "Background Constant: ", kBgConst, 0., 1.);
	fBgAmpFrame = fBgAmp.Build(fRightFrame, "Background Amplitude: ", kBgAmp, 0., 1.);
	fBgDecayConstFrame = fBgDecayConst.Build(fRightFrame, "Background Decay Constant: ", kBgDecayConst, 0., 1.);

	// build button frame
	fButtonFrame = new TGHorizontalFrame(fRightFrame, 400, 400);

	// buttons
	fPreviousButton = new TGTextButton(fButtonFrame, "&Previous");
	fFitButton = new TGTextButton(fButtonFrame, "&Fit");
	fNextButton = new TGTextButton(fButtonFrame, "&Next");
	fSaveButton = new TGTextButton(fButtonFrame, "&Save");
	
	fButtonFrame->AddFrame(fPreviousButton, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 1, 1, 1, 1));
	fButtonFrame->AddFrame(fFitButton, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 1, 1, 1, 1));
	fButtonFrame->AddFrame(fNextButton, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 1, 1, 1, 1));
	fButtonFrame->AddFrame(fSaveButton, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 1, 1, 1, 1));

	// build left and right frame
	fLeftFrame->AddFrame(fFitCanvas, new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX | kLHintsExpandY, 1, 1, 1, 2));
	fLeftFrame->AddFrame(fStatusBar, new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 1, 1, 1, 2));
	fLeftFrame->AddFrame(fCalibrationCanvas, new TGLayoutHints(kLHintsBottom | kLHintsCenterX | kLHintsExpandX | kLHintsExpandY, 1, 1, 2, 1));

	fRightFrame->AddFrame(fAmplitudeFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 1, 1, 1, 1));
	fRightFrame->AddFrame(fPositionFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 1, 1, 1, 1));
	fRightFrame->AddFrame(fSigmaFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 1, 1, 1, 1));
	fRightFrame->AddFrame(fDSigmaFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 1, 1, 1, 1));
	fRightFrame->AddFrame(fPeakAmpFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 1, 1, 1, 1));
	fRightFrame->AddFrame(fPeakPosFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 1, 1, 1, 1));
	fRightFrame->AddFrame(fPeakSigmaFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 1, 1, 1, 1));
	fRightFrame->AddFrame(fNoiseAmpFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 1, 1, 1, 1));
	fRightFrame->AddFrame(fNoisePosFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 1, 1, 1, 1));
	fRightFrame->AddFrame(fNoiseSigmaFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 1, 1, 1, 1));
	fRightFrame->AddFrame(fThresholdFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 1, 1, 1, 1));
	fRightFrame->AddFrame(fThresholdSigmaFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 1, 1, 1, 1));
	fRightFrame->AddFrame(fBgConstFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 1, 1, 1, 1));
	fRightFrame->AddFrame(fBgAmpFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 1, 1, 1, 1));
	fRightFrame->AddFrame(fBgDecayConstFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 1, 1, 1, 1));

	fRightFrame->AddFrame(fButtonFrame, new TGLayoutHints(kLHintsBottom | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 1, 1, 1, 1));

	AddFrame(fLeftFrame,  new TGLayoutHints(kLHintsTop | kLHintsLeft  | kLHintsExpandX | kLHintsExpandY, 1, 1, 1, 1));
	AddFrame(fRightFrame, new TGLayoutHints(kLHintsTop | kLHintsRight | kLHintsExpandX | kLHintsExpandY, 1, 1, 1, 1));

	SetWindowName("Descant Calibration");

	MapSubwindows();
	Resize(GetDefaultSize());
	MapWindow();
}

void TCalibrateDescant::MakeConnections()
{
	// connect canvases zoom function
	fFitCanvas->GetCanvas()->Connect("RangeChanged()", "TCalibrateDescant", this, "FitCanvasZoomed()");
	fCalibrationCanvas->GetCanvas()->Connect("RangeChanged()", "TCalibrateDescant", this, "CalibrationCanvasZoomed()");

	// connect status bar info to canvases
	fFitCanvas->GetCanvas()->Connect("ProcessedEvent(Int_t, Int_t, Int_t, TObject*)", "TCalibrateDescant", this, "Status(Int_t, Int_t, Int_t, TObject*)");
	fCalibrationCanvas->GetCanvas()->Connect("ProcessedEvent(Int_t, Int_t, Int_t, TObject*)", "TCalibrateDescant", this, "Status(Int_t, Int_t, Int_t, TObject*)");

	// connect buttons
	fPreviousButton->Connect("Clicked()", "TCalibrateDescant", this, "Previous()");
	fFitButton->Connect("Clicked()", "TCalibrateDescant", this, "Fit()");
	fNextButton->Connect("Clicked()", "TCalibrateDescant", this, "Next()");
	fSaveButton->Connect("Clicked()", "TCalibrateDescant", this, "Save()");

	// connect input parameters
	fAmplitude.Connect(this);
	fPosition.Connect(this);
	fSigma.Connect(this);
	fDSigma.Connect(this);
	fPeakAmp.Connect(this);
	fPeakPos.Connect(this);
	fPeakSigma.Connect(this);
	fNoiseAmp.Connect(this);
	fNoisePos.Connect(this);
	fNoiseSigma.Connect(this);
	fThreshold.Connect(this);
	fThresholdSigma.Connect(this);
	fBgConst.Connect(this);
	fBgAmp.Connect(this);
	fBgDecayConst.Connect(this);
}

void TCalibrateDescant::CreateGraphicMembers()
{
	fProjections.resize(fMatrix->GetXaxis()->GetNbins());
	fCalibrations.resize(fMatrix->GetXaxis()->GetNbins());
	for(size_t i = 0; i < fProjections.size(); ++i) {
		fProjections[i] = fMatrix->ProjectionY(Form("%s_py%ld", fMatrix->GetName(), i+1), i+1, i+1);
		fCalibrations[i] = new TGraphErrors;
	}
	double xmin = fMatrix->GetYaxis()->GetBinLowEdge(1);
	double xmax = fMatrix->GetYaxis()->GetBinLowEdge(fMatrix->GetYaxis()->GetNbins()+1);
	fInitial = new TF1("initial", FullEdge, xmin, xmax, 15);
	fInitial->SetNpx(10000);
	fInitial->SetLineColor(2);
	fInitial->SetLineStyle(2);
	fFit = new TF1("fit", FullEdge, xmin, xmax, 15);
	fFit->SetNpx(10000);
	fFit->SetLineColor(2);
	fEdge = new TF1("edge", FullEdge, xmin, xmax, 15);
	fEdge->SetNpx(10000);
	fEdge->SetLineColor(3);
	fPeak = new TF1("peak", FullEdge, xmin, xmax, 15);
	fPeak->SetNpx(10000);
	fPeak->SetLineColor(4);
	fNoise = new TF1("noise", FullEdge, xmin, xmax, 15);
	fNoise->SetNpx(10000);
	fNoise->SetLineColor(6);
	fBg = new TF1("bg", FullEdge, xmin, xmax, 15);
	fBg->SetNpx(10000);
	fBg->SetLineColor(7);
}

void TCalibrateDescant::UpdateInterface()
{
	std::cout<<"Updating inferface for current projection "<<fCurrentProjection<<std::endl;
	InitializeParameters();
	fProjections[fCurrentProjection]->GetListOfFunctions()->Clear();
	fProjections[fCurrentProjection]->GetListOfFunctions()->Add(fInitial);
	// draw projection and calibration graph
	fFitCanvas->GetCanvas()->cd();
	fProjections[fCurrentProjection]->Draw();
	fCalibrationCanvas->GetCanvas()->cd();
	fCalibrations[fCurrentProjection]->Draw("ap");
	// force redrawing?
	MapWindow();
}

void TCalibrateDescant::UpdateInitial()
{
	fInitial->FixParameter(0, fAmplitude.Value());
	fInitial->FixParameter(1, fPosition.Value());
	fInitial->FixParameter(2, fSigma.Value());
	fInitial->FixParameter(3, fDSigma.Value());
	fInitial->FixParameter(4, fPeakAmp.Value());
	fInitial->FixParameter(5, fPeakPos.Value());
	fInitial->FixParameter(6, fPeakSigma.Value());
	fInitial->FixParameter(7, fNoiseAmp.Value());
	fInitial->FixParameter(8, fNoisePos.Value());
	fInitial->FixParameter(9, fNoiseSigma.Value());
	fInitial->FixParameter(10, fThreshold.Value());
	fInitial->FixParameter(11, fThresholdSigma.Value());
	fInitial->FixParameter(12, fBgConst.Value());
	fInitial->FixParameter(13, fBgAmp.Value());
	fInitial->FixParameter(14, fBgDecayConst.Value());
	fInitial->Copy(*fFit);
	for(int i = 0; i < 15; ++i) {
		std::cout<<fInitial->GetParName(i)<<": "<<fInitial->GetParameter(i)<<std::endl;
	}
	fFitCanvas->GetCanvas()->cd();
	fProjections[fCurrentProjection]->Draw();
}

void TCalibrateDescant::InitializeParameters()
{
	/// initialize parameters base on our current histogram
	// only use zoomed in region
	int firstBin = fProjections[fCurrentProjection]->GetXaxis()->GetFirst();
	int nBins = fProjections[fCurrentProjection]->GetXaxis()->GetLast();

	double xmin = fProjections[fCurrentProjection]->GetXaxis()->GetBinLowEdge(firstBin);
	double xmax = fProjections[fCurrentProjection]->GetXaxis()->GetBinLowEdge(nBins+1);

	// first find the threshold (aka first bin with more than 2 counts)
	int threshold;
	for(threshold = firstBin; threshold < nBins; ++threshold) {
		if(fProjections[fCurrentProjection]->GetBinContent(threshold) > 2) {
			break;
		}
	}
	// find the rough position of the edge (first bin from the top above the average)
	double average = fProjections[fCurrentProjection]->Integral()/nBins;
	int roughBin;
	for(roughBin = nBins; roughBin >= firstBin; --roughBin) {
		if(fProjections[fCurrentProjection]->GetBinContent(roughBin) > average) {
			break;
		}
	}
	double roughPos = fProjections[fCurrentProjection]->GetBinCenter(roughBin);

	std::cout<<"Initializing parameters for current projection "<<fCurrentProjection<<" based on threshold bin "<<threshold<<", average "<<average<<", rough bin "<<roughBin<<", and rough position "<<roughPos<<std::endl;

	fInitial->FixParameter(0, 0.9*fProjections[fCurrentProjection]->GetBinContent((roughBin-threshold)/2.)); // amplitude
	fInitial->FixParameter(1, roughPos); // position
	fInitial->FixParameter(2, 0.1*roughPos); // sigma
	fInitial->FixParameter(3, 0.2*roughPos); // d sigma
	fInitial->FixParameter(4, 0.3*fProjections[fCurrentProjection]->GetBinContent((roughBin-threshold)/2.)); // peak amp
	fInitial->FixParameter(5, 0.3*roughPos); // peak pos
	fInitial->FixParameter(6, 0.2*roughPos); // peak sigma
	fInitial->FixParameter(7, 5.*fProjections[fCurrentProjection]->Integral(threshold, threshold+10)/TMath::Exp(-0.001*threshold)); // noise amp
	fInitial->FixParameter(8, 1.); // noise pos
	fInitial->FixParameter(9, 1000.); // noise sigma
	fInitial->FixParameter(10, fProjections[fCurrentProjection]->GetBinCenter(threshold)); // threshold
	fInitial->FixParameter(11, 1.); // thres sigma
	fInitial->FixParameter(12, 10.); // bg const
	fInitial->FixParameter(13, 1.); // bg amp
	fInitial->FixParameter(14, 0.0001); // bg decay const

	fInitial->SetParName(0,  fAmplitude.Name());
	fInitial->SetParName(1,  fPosition.Name());
	fInitial->SetParName(2,  fSigma.Name());
	fInitial->SetParName(3,  fDSigma.Name());
	fInitial->SetParName(4,  fPeakAmp.Name());
	fInitial->SetParName(5,  fPeakPos.Name());
	fInitial->SetParName(6,  fPeakSigma.Name());
	fInitial->SetParName(7,  fNoiseAmp.Name());
	fInitial->SetParName(8,  fNoisePos.Name());
	fInitial->SetParName(9,  fNoiseSigma.Name());
	fInitial->SetParName(10, fThreshold.Name());
	fInitial->SetParName(11, fThresholdSigma.Name());
	fInitial->SetParName(12, fBgConst.Name());
	fInitial->SetParName(13, fBgAmp.Name());
	fInitial->SetParName(14, fBgDecayConst.Name());

	fAmplitude.Set(fInitial->GetParameter(0), xmin, xmax);
	fPosition.Set(fInitial->GetParameter(1), xmin, xmax);
	fSigma.Set(fInitial->GetParameter(2), xmin, xmax);
	fDSigma.Set(fInitial->GetParameter(3), xmin, xmax);
	fPeakAmp.Set(fInitial->GetParameter(4), xmin, xmax);
	fPeakPos.Set(fInitial->GetParameter(5), xmin, xmax);
	fPeakSigma.Set(fInitial->GetParameter(6), xmin, xmax);
	fNoiseAmp.Set(fInitial->GetParameter(7), xmin, xmax);
	fNoisePos.Set(fInitial->GetParameter(8), xmin, xmax);
	fNoiseSigma.Set(fInitial->GetParameter(9), xmin, xmax);
	fThreshold.Set(fInitial->GetParameter(10), xmin, xmax);
	fThresholdSigma.Set(fInitial->GetParameter(11), xmin, xmax);
	fBgConst.Set(fInitial->GetParameter(12), xmin, xmax);
	fBgAmp.Set(fInitial->GetParameter(13), xmin, xmax);
	fBgDecayConst.Set(fInitial->GetParameter(14), xmin, xmax);

	fInitial->Copy(*fFit);
}

void TCalibrateDescant::Previous()
{
	if(fCurrentProjection > 0) --fCurrentProjection;
	UpdateInterface();
}

void TCalibrateDescant::Next()
{
	if(fCurrentProjection+1 < static_cast<int>(fProjections.size())) ++fCurrentProjection;
	UpdateInterface();
}

void TCalibrateDescant::Fit()
{
	fFitCanvas->GetCanvas()->cd();

	// set parameters from TParameterInput
	fFit->SetParameter(0, fAmplitude.Value());
	fFit->SetParLimits(0, fAmplitude.LowLimit(), fAmplitude.HighLimit());
	fFit->SetParameter(1, fPosition.Value());
	fFit->SetParLimits(1, fPosition.LowLimit(), fPosition.HighLimit());
	fFit->SetParameter(2, fSigma.Value());
	fFit->SetParLimits(2, fSigma.LowLimit(), fSigma.HighLimit());
	fFit->SetParameter(3, fDSigma.Value());
	fFit->SetParLimits(3, fDSigma.LowLimit(), fDSigma.HighLimit());
	fFit->SetParameter(4, fPeakAmp.Value());
	fFit->SetParLimits(4, fPeakAmp.LowLimit(), fPeakAmp.HighLimit());
	fFit->SetParameter(5, fPeakPos.Value());
	fFit->SetParLimits(5, fPeakPos.LowLimit(), fPeakPos.HighLimit());
	fFit->SetParameter(6, fPeakSigma.Value());
	fFit->SetParLimits(6, fPeakSigma.LowLimit(), fPeakSigma.HighLimit());
	fFit->SetParameter(7, fNoiseAmp.Value());
	fFit->SetParLimits(7, fNoiseAmp.LowLimit(), fNoiseAmp.HighLimit());
	fFit->SetParameter(8, fNoisePos.Value());
	fFit->SetParLimits(8, fNoisePos.LowLimit(), fNoisePos.HighLimit());
	fFit->SetParameter(9, fNoiseSigma.Value());
	fFit->SetParLimits(9, fNoiseSigma.LowLimit(), fNoiseSigma.HighLimit());
	fFit->SetParameter(10, fThreshold.Value());
	fFit->SetParLimits(10, fThreshold.LowLimit(), fThreshold.HighLimit());
	fFit->SetParameter(11, fThresholdSigma.Value());
	fFit->SetParLimits(11, fThresholdSigma.LowLimit(), fThresholdSigma.HighLimit());
	fFit->SetParameter(12, fBgConst.Value());
	fFit->SetParLimits(12, fBgConst.LowLimit(), fBgConst.HighLimit());
	fFit->SetParameter(13, fBgAmp.Value());
	fFit->SetParLimits(13, fBgAmp.LowLimit(), fBgAmp.HighLimit());
	fFit->SetParameter(14, fBgDecayConst.Value());
	fFit->SetParLimits(14, fBgDecayConst.LowLimit(), fBgDecayConst.HighLimit());

	std::cout<<"done setting parameters"<<std::endl;

	// perform the fit
	fProjections[fCurrentProjection]->Fit(fFit, "RQ");

	std::cout<<"done fitting"<<std::endl;

	// get parameters and parameter errors

	// update the component functions
	for(int i = 0; i < 15; ++i) {
		std::cout<<"parameter "<<i<<std::endl;
		switch(i) {
			case 0:
				fEdge->SetParameter(i, fFit->GetParameter(i));
				fPeak->SetParameter(i, 0.);
				fNoise->SetParameter(i, 0.);
				fBg->SetParameter(i, 0.);
				break;
			case 4:
				fEdge->SetParameter(i, 0.);
				fPeak->SetParameter(i, fFit->GetParameter(i));
				fNoise->SetParameter(i, 0.);
				fBg->SetParameter(i, 0.);
				break;
			case 7:
				fEdge->SetParameter(i, 0.);
				fPeak->SetParameter(i, 0.);
				fNoise->SetParameter(i, fFit->GetParameter(i));
				fBg->SetParameter(i, 0.);
				break;
			case 12:
				fEdge->SetParameter(i, 0.);
				fPeak->SetParameter(i, 0.);
				fNoise->SetParameter(i, 0.);
				fBg->SetParameter(i, fFit->GetParameter(i));
				break;
			case 13:
				fEdge->SetParameter(i, 0.);
				fPeak->SetParameter(i, 0.);
				fNoise->SetParameter(i, 0.);
				fBg->SetParameter(i, fFit->GetParameter(i));
				break;
			default:
				fEdge->SetParameter(i, fFit->GetParameter(i));
				fPeak->SetParameter(i, fFit->GetParameter(i));
				fNoise->SetParameter(i, fFit->GetParameter(i));
				fBg->SetParameter(i, fFit->GetParameter(i));
				break;
		};
	}

	std::cout<<"done updating components"<<std::endl;

	// add all functions to histogram (after clearing all functions from it)
	fProjections[fCurrentProjection]->GetListOfFunctions()->Clear();
	fProjections[fCurrentProjection]->GetListOfFunctions()->Add(fInitial);
	fProjections[fCurrentProjection]->GetListOfFunctions()->Add(fBg);
	fProjections[fCurrentProjection]->GetListOfFunctions()->Add(fNoise);
	fProjections[fCurrentProjection]->GetListOfFunctions()->Add(fPeak);
	fProjections[fCurrentProjection]->GetListOfFunctions()->Add(fEdge);
	fProjections[fCurrentProjection]->GetListOfFunctions()->Add(fFit);

	std::cout<<"done adding functions"<<std::endl;

	// add new point to calibration graph
	AddCalibrationPoint(fFit->GetParameter(1), fFit->GetParError(1));

	std::cout<<"done"<<std::endl;
}

void TCalibrateDescant::Save()
{
}

void TCalibrateDescant::AddCalibrationPoint(double value, double uncertainty)
{
	// check if point already exists for this energy and update it if so
	int i;
	Double_t* x = fCalibrations[fCurrentProjection]->GetX();
	for(i = 0; i < fCalibrations[fCurrentProjection]->GetN(); ++i) {
		if(x[i] == SourceEnergy(fSource)) {
			fCalibrations[fCurrentProjection]->SetPoint(i, x[i], value);
			fCalibrations[fCurrentProjection]->SetPointError(i, fCalibrations[fCurrentProjection]->GetErrorX(i), uncertainty);
			return;
		}
	}
	// create new point
	fCalibrations[fCurrentProjection]->SetPoint(i, SourceEnergy(fSource), value);
	fCalibrations[fCurrentProjection]->SetPointError(i, SourceEnergyUncertainty(fSource), uncertainty);
}

void TCalibrateDescant::FitCanvasZoomed()
{
	// update range of fit functions
	Double_t xmin, ymin, xmax, ymax;
	fFitCanvas->GetCanvas()->GetRange(xmin, ymin, xmax, ymax);

	fInitial->SetRange(xmin, xmax);
	fFit->SetRange(xmin, xmax);
	fEdge->SetRange(xmin, xmax);
	fPeak->SetRange(xmin, xmax);
	fNoise->SetRange(xmin, xmax);
	fBg->SetRange(xmin, xmax);

	//InitializeParameters();// or UpdateInterface()?
	//fProjections[fCurrentProjection]->GetListOfFunctions()->Clear();
	//fProjections[fCurrentProjection]->GetListOfFunctions()->Add(fInitial);
}

void TCalibrateDescant::CalibrationCanvasZoomed()
{
	// nothing to do for this one?
}

void TCalibrateDescant::Status(Int_t px, Int_t py, Int_t , TObject* selected)
{
	fStatusBar->SetText(selected->GetName(), 0);
	fStatusBar->SetText(selected->GetTitle(), 1);
	fStatusBar->SetText(selected->GetObjectInfo(px, py), 2);
}
