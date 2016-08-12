
#include "GRootCommands.h"
#include "Globals.h"
#include <cstdio>
//#include <string>
#include <sstream>
#include <fstream>

#include <TRint.h>
#include <TTree.h>
#include <Getline.h>
#include <TAxis.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TPolyMarker.h>
#include <TSpectrum.h>
#include <TText.h>
#include <TExec.h>
#include <TKey.h>
#include <TObject.h>
#include <TObjArray.h>
#include <TH1.h>
#include <TPython.h>
#include <TTimer.h>
#include <TF1.h>

#include <GCanvas.h>
#include <GPeak.h>
#include <GGaus.h>
#include <GH2D.h>
#include <GH1D.h>
//#include <GRootObjectManager.h>
#include <TGRSIOptions.h>
//#include <TGRUTInt.h>
#include <GNotifier.h>

TChain* gFragment = NULL;
TChain* gAnalysis = NULL;



void Help()     { printf("This is helpful information.\n"); }

void Commands() { printf("this is a list of useful commands.\n");}

void Prompt() { Getlinem(EGetLineMode::kInit,((TRint*)gApplication)->GetPrompt()); }

void Version() {
  system(Form("%s/bin/grutinizer-config --version", getenv("GRUTSYS")));
}

bool GetProjection(GH2D *hist,double low, double high, double bg_low,double bg_high){
  if(!hist) return 0;
  GCanvas *C_projections = 0;
  GCanvas *C_gammagamma  = 0;
  if(gROOT->GetListOfCanvases()->FindObject("C_projections"))
    C_projections = (GCanvas*)gROOT->GetListOfCanvases()->FindObject("C_projections");
  else{
    C_projections = new GCanvas("C_projections","Projection Canvas",0,0,1450,600);
    C_projections->Divide(2,1);
  }

  if(gROOT->GetListOfCanvases()->FindObject("C_gammagamma"))
    C_gammagamma = (GCanvas*)gROOT->GetListOfCanvases()->FindObject("C_gammagamma");
  else
    C_gammagamma = new GCanvas("C_gammagamma","Gamma-Gamma Canvas",1700,0,650,650);

  C_gammagamma->cd();
  hist->Draw();

  C_projections->cd(1);
  GH1D *Proj_x = hist->ProjectionX("Gamma_Gamma_xProjection");

  GH1D *Proj_x_Clone = (GH1D*)Proj_x->Clone();
  GH1D *Proj_gated = 0;

  if(bg_high>0 && bg_low>0){
    Proj_x->SetTitle(Form("Projection with Gate From [%.01f,%.01f] and Background [%.01f,%.01f]",low,high,bg_low,bg_high));
  }else{
    Proj_x->SetTitle(Form("Projection with Gate From [%.01f,%.01f] NO background",low,high));
  }
  Proj_x->GetXaxis()->SetTitle("Energy [keV]");
  Proj_x->GetYaxis()->SetTitle("Counts ");



  double Grace = 300;
  double ZoomHigh = high+Grace;
  double ZoomLow  = low-Grace;
  if(bg_high>0 && bg_high>high)
    ZoomHigh = bg_high+Grace;
  if(bg_low>0 && bg_low<low)
    ZoomLow = bg_low-Grace;

  Proj_x->GetXaxis()->SetRangeUser(ZoomLow,ZoomHigh);
  Proj_x->Draw();
  double Projx_Max = Proj_x->GetMaximum();
  double Projx_Min = Proj_x->GetMinimum();

  TLine *CutLow  = new TLine(low,Projx_Min,low,Projx_Max);
  TLine *CutHigh = new TLine(high,Projx_Min,high,Projx_Max);
  TLine *BGLow   = new TLine(bg_low,Projx_Min,bg_low,Projx_Max);
  TLine *BGHigh  = new TLine(bg_high,Projx_Min,bg_high,Projx_Max);
  CutLow->SetLineColor(kRed);
  CutHigh->SetLineColor(kRed);
  CutLow->SetLineWidth(2);
  CutHigh->SetLineWidth(2);
  BGLow->SetLineColor(kBlue);
  BGHigh->SetLineColor(kBlue);
  BGLow->SetLineWidth(2);
  BGHigh->SetLineWidth(2);
  BGLow->SetLineStyle(kDashed);
  BGHigh->SetLineStyle(kDashed);
  CutLow->Draw("same");
  CutHigh->Draw("same");
  if(bg_low>0 && bg_high>0){
    BGHigh->Draw("same");
    BGLow->Draw("same");
    Proj_gated = Proj_x_Clone->Project_Background(low,high,
						  bg_low,bg_high,
						  kRegionBackground);
  }else{
    Proj_gated = Proj_x_Clone->Project(low,high);
  }

  if(bg_high>0 && bg_low>0){
    Proj_gated->SetTitle(Form("Gate From [%.01f,%.01f] with Background [%.01f,%.01f]",low,high,bg_low,bg_high));
  }else{
    Proj_gated->SetTitle(Form("Gate From [%.01f,%.01f] NO Background",low,high));
  }
  Proj_gated->GetXaxis()->SetTitle("Energy [keV]");
  Proj_gated->GetYaxis()->SetTitle("Counts");

  C_projections->cd(2);
  Proj_gated->Draw();
  return 1;
}

int LabelPeaks(TH1 *hist,double sigma,double thresh,Option_t *opt) {
  TSpectrum::StaticSearch(hist,sigma,"Qnodraw",thresh);
  TPolyMarker *pm = (TPolyMarker*)hist->GetListOfFunctions()->FindObject("TPolyMarker");
  if(!pm) {
    //something has gone wrong....
    return 0;
  }
  TObjArray *array = (TObjArray*)hist->GetListOfFunctions()->FindObject("PeakLabels");
  if(array) {
    hist->GetListOfFunctions()->Remove((TObject*)array);
    array->Delete();
  }
  array = new TObjArray();
  array->SetName("PeakLabels");
  int n = pm->GetN();
  if(n==0)
    return n;
  TText *text;
  double *x = pm->GetX();
  //  double *y = pm->GetY();
  for(int i=0;i<n;i++) {
    //y[i] += y[i]*0.15;
    double y = 0;
    for(int i_x = x[i]-3;i_x<x[i]+3;i_x++){
      if((hist->GetBinContent(hist->GetXaxis()->FindBin(i_x)))>y){
	y = hist->GetBinContent(hist->GetXaxis()->FindBin(i_x));
      }
    }
    y+=y*0.1;
    text = new TText(x[i],y,Form("%.1f",x[i]));
    text->SetTextSize(0.025);
    text->SetTextAngle(90);
    text->SetTextAlign(12);
    text->SetTextFont(42);
    text->SetTextColor(hist->GetLineColor());
    array->Add(text);
  }
  hist->GetListOfFunctions()->Remove(pm);
  pm->Delete();
  hist->GetListOfFunctions()->Add(array);
  return n;
}



bool ShowPeaks(TH1 **hists,unsigned int nhists,double sigma,double thresh) {
  int num_found = 0;
  for(unsigned int i=0;i<nhists;i++) {
    if(TObject *obj = hists[i]->GetListOfFunctions()->FindObject("PeakLabels")) {
      hists[i]->GetListOfFunctions()->Remove(obj);
      ((TObjArray*)obj)->Delete();
    }
    num_found += LabelPeaks(hists[i],sigma,thresh,"");
  }
  if(num_found)
    return true;
  return false;
}

bool RemovePeaks(TH1 **hists,unsigned int nhists) {
  bool flag = false;
  for(unsigned int i=0;i<nhists;i++) {
    if(TObject *obj=hists[i]->GetListOfFunctions()->FindObject("PeakLabels")) {
      hists[i]->GetListOfFunctions()->Remove(obj);
      ((TObjArray*)obj)->Delete();
      flag = true;
    }
  }
  return flag;
}

//bool PeakFit(TH1 *hist,Double_t xlow, Double_t xhigh,Option_t *opt) {
//  if(!hist)
//   return;
//  TString option = opt;
//}

GGaus *GausFit(TH1 *hist,double xlow, double xhigh,Option_t *opt) {
  //bool edit = false;
  if(!hist)
    return 0;
  if(xlow>xhigh)
    std::swap(xlow,xhigh);

  //std::cout << "here." << std::endl;

  GGaus *mypeak= new GGaus(xlow,xhigh);
  std::string options = opt;
  options.append("Q+");
  mypeak->Fit(hist,options.c_str());
  //mypeak->Background()->Draw("SAME");
  TF1 *bg = new TF1(*mypeak->Background());
  hist->GetListOfFunctions()->Add(bg);
  //edit = true;

  return mypeak;
}


TF1 *DoubleGausFit(TH1 *hist,double cent1,double cent2,double xlow, double xhigh,Option_t *opt) {
  if(!hist)
    return 0;
  if(xlow>xhigh)
    std::swap(xlow,xhigh);

  //std::cout << "here." << std::endl;

  GGaus *mypeak= new GGaus(xlow,xhigh);
  std::string options = opt;
  options.append("Q+");
  mypeak->Fit(hist,options.c_str());
  //mypeak->Background()->Draw("SAME");
  TF1 *bg = new TF1(*mypeak->Background());
  hist->GetListOfFunctions()->Add(bg);
  //edit = true;

  return mypeak;
}









/*



=======



  bool edit = false;
  if(!hist)
    return edit;
  int binx[2];
  double y[2];
  if(xlow>xhigh)
    std::swap(xlow,xhigh);
  binx[0] = hist->GetXaxis()->FindBin(xlow);
  binx[1] = hist->GetXaxis()->FindBin(xhigh);
  y[0] = hist->GetBinContent(binx[0]);
  y[1] = hist->GetBinContent(binx[1]);

  TF1 *myGaus= new TF1(Form("gaus_%i_%i",int(xlow),int(xhigh)),"gaus(0)+pol1(3)",xlow,xhigh);
  TF1 *bg = new TF1("bg","pol1",xlow,xhigh);

  myGaus->SetParameter(0,hist->GetBinContent((binx[1]+binx[0])/2));
  myGaus->SetParameter(2,(xhigh-xlow)/2.0);
  myGaus->SetParameter(1,(xhigh-xlow)/2.0+xlow);
  myGaus->SetParameter(4,(y[1]-y[0])/(xhigh-xlow));
  myGaus->SetParameter(3,((y[1]-y[0])/(xhigh-xlow))*xlow);

  myGaus->SetParName(0,"height");
  myGaus->SetParName(1,"centroid");
  myGaus->SetParName(2,"sigma");
  myGaus->SetParName(4,"bg_slope");
  myGaus->SetParName(3,"bg_offset");


  hist->Fit(myGaus,"RQ+");

  bg->SetParameters(myGaus->GetParameter(3),myGaus->GetParameter(4));
  bg->SetLineColor(kBlue);
  hist->GetListOfFunctions()->Add(bg);

  double param[5];
  double error[5];

  myGaus->GetParameters(param);
  error[0] = myGaus->GetParError(0);
  error[1] = myGaus->GetParError(1);
  error[2] = myGaus->GetParError(2);
  error[3] = myGaus->GetParError(3);
  error[4] = myGaus->GetParError(4);

  printf(GREEN "Integral from % 4.01f to % 4.01f: %f" RESET_COLOR "\n",
               xlow,xhigh,myGaus->Integral(xlow,xhigh)/hist->GetBinWidth(1)-
                          bg->Integral(xlow,xhigh)/hist->GetBinWidth(1));
  printf(GREEN "Centroid  : % 4.02f +/- %.02f" RESET_COLOR "\n",param[1],error[1]);
  printf(GREEN "FWHM      : % 4.02f +/- %.02f" RESET_COLOR "\n",TMath::Abs(param[2]*2.35),TMath::Abs(error[2]*2.35));
  printf(GREEN "Resolution: %.02f %%" RESET_COLOR "\n",TMath::Abs(param[2]*2.35)/param[1]*100.0);

  edit = true;

  TIter it(hist->GetListOfFunctions());
  while(TObject *obj=it.Next()) {
    if(!hist->InheritsFrom(TF1::Class()))
      continue;
    ((TF1*)obj)->Draw("same");
  }


  return edit;

}
*/


GPeak *PhotoPeakFit(TH1 *hist,double xlow, double xhigh,Option_t *opt) {
  //bool edit = 0;
  if(!hist)
    return 0;
  if(xlow>xhigh)
    std::swap(xlow,xhigh);

  //std::cout << "here." << std::endl;

  GPeak *mypeak= new GPeak((xlow+xhigh)/2.0,xlow,xhigh);
  std::string options = opt;
  options.append("Q+");
  mypeak->Fit(hist,options.c_str());
  //mypeak->Background()->Draw("SAME");
  TF1 *bg = new TF1(*mypeak->Background());
  hist->GetListOfFunctions()->Add(bg);
  //edit = true;

  return mypeak;
}

std::string MergeStrings(const std::vector<std::string>& strings, char split) {
  std::stringstream ss;
  for(auto it = strings.begin(); it != strings.end(); it++) {
    ss << *it;

    auto next = it;
    next++;
    if(next != strings.end()){
      ss << split;
    }
  }
  return ss.str();
}


TH1 *GrabHist(int i)  {
  //return the histogram from the current canvas, pad i.
  TH1 *hist = 0;
  if(!gPad)
    return hist;
  TIter iter(gPad->GetListOfPrimitives());
  int j=0;
  while(TObject *obj = iter.Next()) {
    if(obj->InheritsFrom(TH1::Class())) {
      if(j==i) {
        hist = (TH1*)obj;
        break;
      }
      j++;
    }
  }
  return hist;
}

TF1 *GrabFit(int i)  {
  //return the histogram from the current canvas, pad i.
  TH1 *hist = 0;
  TF1 *fit = 0;
  if(!gPad)
    return fit;
  TIter iter(gPad->GetListOfPrimitives());
  int j=0;
  while(TObject *obj = iter.Next()) {
    if(obj->InheritsFrom(TH1::Class())) {
      hist = (TH1*)obj;
      TIter iter2(hist->GetListOfFunctions());
      while(TObject *obj2 = iter2.Next()){
	if(obj2->InheritsFrom(TF1::Class())){
	  if(j==i) {
	    fit=(TF1*)obj2;
	    return fit;
	  }
	  j++;
	}
      }
    }
  }
  return fit;
}


namespace {
  bool gui_is_running = false;
}

void StartGUI() {
  std::string   script_filename = Form("%s/pygui/grut-view.py",getenv("GRUTSYS"));
  std::ifstream script(script_filename);
  std::string   script_text((std::istreambuf_iterator<char>(script)),
                             std::istreambuf_iterator<char>());
  TPython::Exec(script_text.c_str());

  TTimer* gui_timer = new TTimer("TPython::Exec(\"update()\");", 10, true);
  gui_timer->TurnOn();

  gui_is_running = true;
  for(int i=0;i<gROOT->GetListOfFiles()->GetSize();i++) {
    TPython::Bind((TFile*)gROOT->GetListOfFiles()->At(i),"tdir");
    gROOT->ProcessLine("TPython::Exec(\"window.AddDirectory(tdir)\");");
  }
}

bool GUIIsRunning() {
  return gui_is_running;
}



TH2 *AddOffset(TH2 *mat,double offset,EAxis axis) {
 TH2 *toreturn = 0;
 if(!mat)
    return toreturn;
 //int dim = mat->GetDimension();
 int xmax = mat->GetXaxis()->GetNbins()+1;
 int ymax = mat->GetYaxis()->GetNbins()+1;
 /*
 switch(dim) {
   case 3:
     xmax = mat->GetXaxis()->GetNbins()+1;
     ymax = mat->GetYaxis()->GetNbins()+1;
     zmax = mat->GetZaxis()->GetNbins()+1;
     break;
   case 2:
     if(axis>3) {
       fprintf(stderr,"%s z-axis offest called on %s but has no z-axis",
               __PRETTY_FUNCTION__,mat->GetName())
       return toreturn;
     }
     xmax = mat->GetXaxis()->GetNbins()+1;
     ymax = mat->GetYaxis()->GetNbins()+1;
     break;
   case 1:
     if(axis!=1) {
       fprintf(stderr,"%s offest called on %s with an axis it doesn't have.",
               __PRETTY_FUNCTION__,mat->GetName())
       return toreturn;
     }
     xmax = mat->GetXaxis()->GetNbins()+1;
     break;
 };
 */
 toreturn = (TH2*)mat->Clone(Form("%s_offset",mat->GetName()));
 toreturn->Reset();

  for(int x=0;x<xmax;x++) {
    for(int y=0;y<ymax;y++) {
      double newx = mat->GetXaxis()->GetBinCenter(x);
      double newy = mat->GetYaxis()->GetBinCenter(y);;
      double bcont = mat->GetBinContent(x,y);
      if(axis&kXAxis)
        newx += offset;
      if(axis&kYAxis)
        newy += offset;
      toreturn->Fill(newx,newy,bcont);
     }
   }
  return toreturn;
}
