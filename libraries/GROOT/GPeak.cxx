
#include <GPeak.h>
#include <TGraph.h>
#include <TVirtualFitter.h>
#include <TFitResult.h>
#include <TFitResultPtr.h>


#include "Globals.h"
#include "GRootFunctions.h"
#include "GCanvas.h"

ClassImp(GPeak)

GPeak::GPeak(Double_t cent,Double_t xlow,Double_t xhigh,Option_t *opt)
      : TF1("photopeakbg",GRootFunctions::PhotoPeakBG,xlow,xhigh,7),
        fBGFit("background",GRootFunctions::StepBG,xlow,xhigh,6)  {
  Clear("");
  if(cent>xhigh || cent<xlow) {
    //out of range...
    if(xlow>cent)
      std::swap(xlow,cent);
    if(xlow>xhigh)
      std::swap(xlow,xhigh);
    if(cent>xhigh)
      std::swap(cent,xhigh);
  }

  TF1::SetRange(xlow,xhigh);


  fBGFit.SetNpx(1000);
  fBGFit.SetLineStyle(2);
  fBGFit.SetLineColor(kBlack);

  SetName(Form("Chan%d_%d_to_%d",(Int_t)(cent),(Int_t)(xlow),(Int_t)(xhigh)));
  InitNames();
  TF1::SetParameter("centroid",cent);
  
  SetParent(0);
  //TF1::SetDirectory(0);
  fBGFit.SetParent(0);
  fBGFit.SetBit(TObject::kCanDelete,false);
  //fBGFit.SetDirectory(0);

}

GPeak::GPeak(Double_t cent,Double_t xlow,Double_t xhigh,TF1 *bg,Option_t *opt)
      : TF1("photopeakbg",GRootFunctions::PhotoPeakBG,xlow,xhigh,7) {
  Clear("");
  if(cent>xhigh || cent<xlow) {
    //out of range...
    if(xlow>cent)
      std::swap(xlow,cent);
    if(xlow>xhigh)
      std::swap(xlow,xhigh);
    if(cent>xhigh)
      std::swap(cent,xhigh);
  }
  TF1::SetRange(xlow,xhigh);
  SetName(Form("Chan%d_%d_to_%d",(Int_t)(cent),(Int_t)(xlow),(Int_t)(xhigh)));
  InitNames();
  TF1::SetParameter("centroid",cent);

  if(bg) {
    fBGFit.Clear();
    fBGFit.Copy(*bg);
  } else {
    fBGFit = TF1("BGFit",GRootFunctions::StepBG,xlow,xhigh,10);
  }

  fBGFit.SetNpx(1000);
  fBGFit.SetLineStyle(2);
  fBGFit.SetLineColor(kBlack);
  
  SetParent(0);
  //SetDirectory(0);
  fBGFit.SetParent(0);
  //fBGFit.SetDirectory(0);
}


GPeak::GPeak()
      : TF1("photopeakbg",GRootFunctions::PhotoPeakBG,0,1000,10),
        fBGFit("background",GRootFunctions::StepBG,0,1000,10) {

  Clear();
  InitNames();
  fBGFit.SetNpx(1000);
  fBGFit.SetLineStyle(2);
  fBGFit.SetLineColor(kBlack);
  
  SetParent(0);
  //SetDirectory(0);
  fBGFit.SetParent(0);
  //fBGFit.SetDirectory(0);
}

GPeak::GPeak(const GPeak &peak)
  : TF1(peak) {
  
  SetParent(0);
  //SetDirectory(0);
  fBGFit.SetParent(0);
  //fBGFit.SetDirectory(0);
  peak.Copy(*this);
}

GPeak::~GPeak() {
  //gROOT->RecursiveRemove(&fBGFit);
  //gROOT->RecursiveRemove(this);
  //if(background)
  //  delete background;
}

//void GPeak::Fcn(Int_t &npar,Double_t *gin,Double_T &f,Double_t *par,Int_t iflag) {
  //chisquared calculator
  //
//  int i=0;
//  double chisq = 0;
//  double delta = 0;
//  for(i=0;i<nbins;i++) {
//    delta = (data[i] - GRootFunctions::PhotoPeakBG((x+i),par))/error[i];
//    chisq += delta*delta;
//  }
//  f=chisq;
//}

void GPeak::InitNames(){
  TF1::SetParName(0,"Height");
  TF1::SetParName(1,"centroid");
  TF1::SetParName(2,"sigma");
  TF1::SetParName(3,"R");
  TF1::SetParName(4,"beta");
  //TF1::SetParName(5,"step");
  //TF1::SetParName(6,"A");
  //TF1::SetParName(7,"B");
  //TF1::SetParName(8,"C");
  TF1::SetParName(5,"step");
  TF1::SetParName(6,"bg_offset");
  //TF1::SetParName(7,"bg_slope");
}

void GPeak::Copy(TObject &obj) const {
  //printf("0x%08x\n",&obj);
  //fflush(stdout);
  //printf("%s\n",obj.GetName());
  //fflush(stdout);

  TF1::Copy(obj);
  ((GPeak&)obj).init_flag = init_flag;
  ((GPeak&)obj).fArea     = fArea;
  ((GPeak&)obj).fDArea    = fDArea;
  ((GPeak&)obj).fSum      = fSum;
  ((GPeak&)obj).fDSum     = fDSum;
  ((GPeak&)obj).fChi2     = fChi2;
  ((GPeak&)obj).fNdf      = fNdf;

  fBGFit.Copy((((GPeak&)obj).fBGFit));
}

bool GPeak::InitParams(TH1 *fithist){
  if(!fithist){
    printf("No histogram is associated yet, no initial guesses made\n");
    return false;
  }
  //printf("%s called.\n",__PRETTY_FUNCTION__); fflush(stdout);
  //Makes initial guesses at parameters for the fit. Uses the histogram to
  Double_t xlow,xhigh;
  GetRange(xlow,xhigh);

  //Int_t bin = fithist->GetXaxis()->FindBin(GetParameter("centroid"));
  Int_t binlow = fithist->GetXaxis()->FindBin(xlow);
  Int_t binhigh = fithist->GetXaxis()->FindBin(xhigh);

  Double_t highy  = fithist->GetBinContent(binlow);
  Double_t lowy   = fithist->GetBinContent(binhigh);
  for(int x=1;x<5;x++) {
    highy += fithist->GetBinContent(binlow-x);
    lowy  += fithist->GetBinContent(binhigh+x);
  }
  highy = highy/5.0;
  lowy = lowy/5.0;

//  Double_t yhigh  = fithist->GetBinContent(binhigh);
//  Double_t ylow   = fithist->GetBinContent(binlow);
  if(lowy>highy)
    std::swap(lowy,highy);

  double largestx=0.0;
  double largesty=0.0;
  int i = binlow;
  for(;i<=binhigh;i++) {
    if(fithist->GetBinContent(i) > largesty) {
      largesty = fithist->GetBinContent(i);
      largestx = fithist->GetXaxis()->GetBinCenter(i);
    }
  }

  // - par[0]: height of peak
  // - par[1]: cent of peak
  // - par[2]: sigma
  // - par[3]: R:    relative height of skewed gaus to gaus
  // - par[4]: beta: "skewedness" of the skewed gaussin
  // - par[5]: step: size of stepfunction step.

  // - par[6]: base bg height.

  //limits.
  TF1::SetParLimits(0,0,largesty*2);
  TF1::SetParLimits(1,xlow,xhigh);
  TF1::SetParLimits(2,0.1,xhigh-xlow);
  TF1::SetParLimits(3,0.0,40);
  TF1::SetParLimits(4,0.01,5);
  double step = ((highy-lowy)/largesty)*50;

  //TF1::SetParLimits(5,step-step*.1,step+.1*step);
  TF1::SetParLimits(5,0.0,step+step);

  //double slope  = (yhigh-ylow)/(xhigh-xlow);
  //double offset = yhigh-slope*xhigh;
  double offset = lowy;
  TF1::SetParLimits(6,offset-0.5*offset,offset+offset);
  //TF1::SetParLimits(7,-2*slope,2*slope);

  //Make initial guesses
  TF1::SetParameter(0,largesty);         //fithist->GetBinContent(bin));
  TF1::SetParameter(1,largestx);         //GetParameter("centroid"));
  TF1::SetParameter(2,(largestx*.01)/2.35);                    //2,(xhigh-xlow));     //2.0/binWidth); //
  TF1::SetParameter(3,5.);
  TF1::SetParameter(4,1.);
  TF1::SetParameter(5,step);
  TF1::SetParameter(6,offset);
  //TF1::SetParameter(7,slope);

  TF1::SetParError(0,0.10 * largesty);
  TF1::SetParError(1,0.25);
  TF1::SetParError(2,0.10 *((largestx*.01)/2.35));
  TF1::SetParError(3,5);
  TF1::SetParError(4,0.5);
  TF1::SetParError(5,0.10 * step);
  TF1::SetParError(6,0.10 * offset);





  //TF1::Print();

  SetInitialized();
  return true;
}


Bool_t GPeak::Fit(TH1 *fithist,Option_t *opt) {
  if(!fithist)
    return false;
  TString options = opt;
  if(!IsInitialized())
    InitParams(fithist);
  TVirtualFitter::SetMaxIterations(100000);

  bool verbose = !options.Contains("Q");
  bool noprint =  options.Contains("no-print");
  if(noprint) {
    options.ReplaceAll("no-print","");
  }

  if(fithist->GetSumw2()->fN!=fithist->GetNbinsX()+2)
    fithist->Sumw2();

  TFitResultPtr fitres = fithist->Fit(this,Form("%sLRSME",options.Data()));

  //fitres.Get()->Print();
  printf("chi^2/NDF = %.02f\n",this->GetChisquare()/(double)this->GetNDF());

  
  
  if(!fitres.Get()->IsValid()) {
    printf(RED  "fit has failed, trying refit... " RESET_COLOR);
    //SetParameter(3,0.1);
    //SetParameter(4,0.01);
    //SetParameter(5,0.0);
    fithist->GetListOfFunctions()->Last()->Delete();
    fitres = fithist->Fit(this,Form("%sLRSME",options.Data())); //,Form("%sRSM",options.Data()))
    if( fitres.Get()->IsValid() ) {
      printf(DGREEN " refit passed!" RESET_COLOR "\n");
    } else {
      printf(DRED " refit also failed :( " RESET_COLOR "\n");
    }
  }

  //if(fitres->ParError(2) != fitres->ParError(2)) { // checks if nan.
  //  if(fitres->Parameter(3)<1) {
  //    FixParameter(4,0);
  //    FixParameter(3,1);
      //printf("Beta may have broken the fit, retrying with R=0);
  //    fithist->GetListOfFunctions()->Last()->Delete();
  //    fitres = fithist->Fit(this,Form("%sRSM",options.Data()));
  //  }
  //}

  //TF1::Print();


  //Double_t binwidth = fithist->GetBinWidth(GetParameter("centroid"));
  //Double_t width    = TF1::GetParameter("sigma");
  Double_t xlow,xhigh;
  //Double_t int_low,int_high;
  TF1::GetRange(xlow,xhigh);
  //int_low  = xlow - 5.*width;
  //int_high = xhigh +5.*width;

  //Make a function that does not include the background
  //Intgrate the background.
  //GPeak *tmppeak = new GPeak;
  //this->Copy(*tmppeak);

  //tmppeak->SetParameter("bg_offset",0.0);
  //tmppeak->SetRange(int_low,int_high);//This will help get the true area of the gaussian 200 ~ infinity in a gaus
  //tmppeak->SetName("tmppeak");

  //fArea = (tmppeak->Integral(int_low,int_high))/binwidth;
//  TMatrixDSym CovMat = fitres->GetCovarianceMatrix();
//  CovMat(6,6) = 0.0;
//  CovMat(7,7) = 0.0;
//  CovMat(8,8) = 0.0;
//  CovMat(9,9) = 0.0;

//  fDArea = (tmppeak->IntegralError(int_low,int_high,tmppeak->GetParameters(),CovMat.GetMatrixArray()))/binwidth;


  double bgpars[5];
  bgpars[0] = TF1::GetParameters()[0];
  bgpars[1] = TF1::GetParameters()[1];
  bgpars[2] = TF1::GetParameters()[2];
  bgpars[3] = TF1::GetParameters()[5];
  bgpars[4] = TF1::GetParameters()[6];
  //bgpars[5] = TF1::GetParameters()[7];

  fBGFit.SetParameters(bgpars);
  //fithist->GetListOfFunctions()->Print();


  fArea = this->Integral(xlow,xhigh) / fithist->GetBinWidth(1);
  double bgArea = fBGFit.Integral(xlow,xhigh) / fithist->GetBinWidth(1);
  fArea -= bgArea;


  if(xlow>xhigh)
    std::swap(xlow,xhigh);
  fSum = fithist->Integral(fithist->GetXaxis()->FindBin(xlow),
                           fithist->GetXaxis()->FindBin(xhigh)); //* fithist->GetBinWidth(1);
  printf("sum between markers: %02f\n",fSum);
  fDSum = TMath::Sqrt(fSum);
  fSum -= bgArea;
  printf("sum after subtraction: %02f\n",fSum);


  if(!verbose) {
    Print();/*
    printf("BG Area:         %.02f\n",bgArea);
    printf("GetChisquared(): %.4f\n", TF1::GetChisquare());
    printf("GetNDF():        %i\n",   TF1::GetNDF());
    printf("GetProb():       %.4f\n", TF1::GetProb());*/
    //TF1::Print();
  }

  //printf("fithist->GetListOfFunctions()->FindObject(this) = 0x%08x\n",fithist->GetListOfFunctions()->FindObject(GetName()));
  //fflush(stdout);
  Copy(*fithist->GetListOfFunctions()->FindObject(GetName()));
  //  fithist->GetListOfFunctions()->Add(&fBGFit); //use to be a clone.
  fithist->GetListOfFunctions()->Add(fBGFit.Clone()); //use to be a clone.

  SetParent(0); //fithist);

  //delete tmppeak;
  return true;
}














void GPeak::Clear(Option_t *opt){
  TString options = opt;
  //Clear the GPeak including functions and histogram
  if(options.Contains("all"))
    TF1::Clear();
  init_flag = false;
  fArea  = 0.0;
  fDArea = 0.0;
  fSum   = 0.0;
  fDSum  = 0.0;
  fChi2  = 0.0;
  fNdf   = 0.0;
}

void GPeak::Print(Option_t *opt) const {
  TString options = opt;
  printf(GREEN );
  printf("Name: %s \n", this->GetName());
  printf("Centroid:  %1f +/- %1f \n", this->GetParameter("centroid"),this->GetParError(GetParNumber("centroid")));
  printf("Area:      %1f +/- %1f \n", fArea, fDArea);
  printf("Sum:       %1f +/- %1f \n", fSum, fDSum);
  printf("FWHM:      %1f +/- %1f \n",this->GetFWHM(),this->GetFWHMErr());
  printf("Reso:      %1f%%  \n",this->GetFWHM()/this->GetParameter("centroid")*100.);
  printf("Chi^2/NDF: %1f\n",fChi2/fNdf);
  if(options.Contains("all")){
    TF1::Print(opt);
  }
  printf(RESET_COLOR);
  printf("\n");
}


void GPeak::DrawResiduals(TH1 *hist) const{
  if(hist){
    return;
  }
  if(fChi2<0.000000001){
    printf("No fit performed\n");
    return;
  }
  Double_t xlow,xhigh;
  GetRange(xlow,xhigh);
  Int_t nbins = hist->GetXaxis()->GetNbins();
  Double_t *res = new Double_t[nbins];
  Double_t *bin = new Double_t[nbins];
  Int_t points = 0;
  for(int i =1;i<=nbins;i++) {
    if(hist->GetBinCenter(i) <= xlow || hist->GetBinCenter(i) >= xhigh)
      continue;
    res[points] = (hist->GetBinContent(i) - this->Eval(hist->GetBinCenter(i)))+ this->GetParameter("Height")/2;
    bin[points] = hist->GetBinCenter(i);
    points++;
  }
  new GCanvas();
  TGraph *residuals = new TGraph(points,bin,res);
  residuals->Draw("*AC");
  delete[] res;
  delete[] bin;
}
