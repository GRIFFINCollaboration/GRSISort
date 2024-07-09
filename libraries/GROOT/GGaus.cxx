#include "GGaus.h"
#include "TGraph.h"
#include "TVirtualFitter.h"
#include "TFitResult.h"
#include "TFitResultPtr.h"

#include "Globals.h"
#include "GRootFunctions.h"
#include "GCanvas.h"

GGaus::GGaus(Double_t xlow, Double_t xhigh, Option_t*)
   : TF1("gausbg", "gaus(0)+pol1(3)", xlow, xhigh), fBGFit("background", "pol1", xlow, xhigh)
{
   Clear("");
   if(xlow > xhigh) {
      std::swap(xlow, xhigh);
   }

   TF1::SetRange(xlow, xhigh);

   fBGFit.SetNpx(1000);
   fBGFit.SetLineStyle(2);
   fBGFit.SetLineColor(kBlack);

   // Changing the name here causes an infinite loop when starting the FitEditor
   // SetName(Form("gaus_%d_to_%d",(Int_t)(xlow),(Int_t)(xhigh)));
   InitNames();
}

GGaus::GGaus(Double_t xlow, Double_t xhigh, TF1* bg, Option_t*) : TF1("gausbg", "gaus(0)+pol1(3)", xlow, xhigh)
{
   Clear("");
   if(xlow > xhigh) {
      std::swap(xlow, xhigh);
   }
   TF1::SetRange(xlow, xhigh);
   // Changing the name here causes an infinite loop when starting the FitEditor
   // SetName(Form("gaus_%d_to_%d",(Int_t)(xlow),(Int_t)(xhigh)));
   InitNames();

   if(bg != nullptr) {
      fBGFit.Clear();
      fBGFit.Copy(*bg);
   } else {
      fBGFit = TF1("BGFit", "pol1", xlow, xhigh);
   }

   fBGFit.SetNpx(1000);
   fBGFit.SetLineStyle(2);
   fBGFit.SetLineColor(kBlack);
}

GGaus::GGaus() : TF1("gausbg", "gaus(0)+pol1(3)", 0, 1000), fBGFit("background", "pol1", 0, 1000)
{
   Clear();
   InitNames();
   fBGFit.SetNpx(1000);
   fBGFit.SetLineStyle(2);
   fBGFit.SetLineColor(kBlack);
}

GGaus::GGaus(const GGaus& peak) : TF1(peak)
{
   peak.Copy(*this);
}

GGaus::~GGaus()
{
}

void GGaus::InitNames()
{
   TF1::SetParName(0, "Height");
   TF1::SetParName(1, "centroid");
   TF1::SetParName(2, "sigma");
   TF1::SetParName(3, "bg_offset");
   TF1::SetParName(4, "bg_slope");
}

void GGaus::Copy(TObject& obj) const
{
   TF1::Copy(obj);
   (static_cast<GGaus&>(obj)).init_flag = init_flag;
   (static_cast<GGaus&>(obj)).fArea     = fArea;
   (static_cast<GGaus&>(obj)).fDArea    = fDArea;
   (static_cast<GGaus&>(obj)).fSum      = fSum;
   (static_cast<GGaus&>(obj)).fDSum     = fDSum;
   (static_cast<GGaus&>(obj)).fChi2     = fChi2;
   (static_cast<GGaus&>(obj)).fNdf      = fNdf;

   fBGFit.Copy(((static_cast<GGaus&>(obj)).fBGFit));
}

bool GGaus::InitParams(TH1* fithist)
{
   if(fithist == nullptr) {
      std::cout << "No histogram is associated yet, no initial guesses made" << std::endl;
      return false;
   }
   // Makes initial guesses at parameters for the fit. Uses the histogram to
   Double_t xlow, xhigh;
   GetRange(xlow, xhigh);

   Int_t binlow  = fithist->GetXaxis()->FindBin(xlow);
   Int_t binhigh = fithist->GetXaxis()->FindBin(xhigh);

   Double_t highy = fithist->GetBinContent(binlow);
   Double_t lowy  = fithist->GetBinContent(binhigh);
   for(int x = 1; x < 5; x++) {
      highy += fithist->GetBinContent(binlow - x);
      lowy += fithist->GetBinContent(binhigh + x);
   }
   highy = highy / 5.0;
   lowy  = lowy / 5.0;

   if(lowy > highy) {
      std::swap(lowy, highy);
   }

   double largestx = 0.0;
   double largesty = 0.0;
   int    i        = binlow;
   for(; i <= binhigh; i++) {
      if(fithist->GetBinContent(i) > largesty) {
         largesty = fithist->GetBinContent(i);
         largestx = fithist->GetXaxis()->GetBinCenter(i);
      }
   }

   // - par[0]: height of peak
   // - par[1]: cent of peak
   // - par[2]: sigma
   // - par[3]: bg constent
   // - par[4]: bg slope

   // limits.
   TF1::SetParLimits(0, 0, largesty * 2);
   TF1::SetParLimits(1, xlow, xhigh);
   TF1::SetParLimits(2, 0, xhigh - xlow);

   // Make initial guesses
   TF1::SetParameter(0, largesty);                  // fithist->GetBinContent(bin));
   TF1::SetParameter(1, largestx);                  // GetParameter("centroid"));
   TF1::SetParameter(2, (largestx * .01) / 2.35);   // 2,(xhigh-xlow));     //2.0/binWidth); //

   TF1::SetParError(0, 0.10 * largesty);
   TF1::SetParError(1, 0.25);
   TF1::SetParError(2, 0.10 * ((largestx * .01) / 2.35));

   SetInitialized();
   return true;
}

Bool_t GGaus::Fit(TH1* fithist, Option_t* opt)
{
   if(fithist == nullptr) {
      return false;
   }
   TString options = opt;
   if(!IsInitialized()) {
      InitParams(fithist);
   }
   TVirtualFitter::SetMaxIterations(100000);

   bool verbose = !options.Contains("Q");
   bool noprint = options.Contains("no-print");
   if(noprint) {
      options.ReplaceAll("no-print", "");
   }

   if(fithist->GetSumw2()->fN != fithist->GetNbinsX() + 2) {
      fithist->Sumw2();
   }

   TFitResultPtr fitres = fithist->Fit(this, Form("%sRSME", options.Data()));

   if(!fitres.Get()->IsValid()) {
      if(!verbose) {
         std::cout << RED << "fit has failed, trying refit... " << RESET_COLOR;
      }
      fithist->GetListOfFunctions()->Last()->Delete();
      fitres = fithist->Fit(this, Form("%sRSME", options.Data()));   //,Form("%sRSM",options.Data()))
      if(fitres.Get()->IsValid()) {
         if(!verbose && !noprint) {
            std::cout << DGREEN << " refit passed!" << RESET_COLOR << std::endl;
         }
      } else {
         if(!verbose && !noprint) {
            std::cout << DRED << " refit also failed :( " << RESET_COLOR << std::endl;
         }
      }
   }

   Double_t xlow, xhigh;
   TF1::GetRange(xlow, xhigh);

   double bgpars[2];
   bgpars[0] = TF1::GetParameters()[3];
   bgpars[1] = TF1::GetParameters()[4];

   fBGFit.SetParameters(bgpars);

   fArea         = Integral(xlow, xhigh) / fithist->GetBinWidth(1);
   double bgArea = fBGFit.Integral(xlow, xhigh) / fithist->GetBinWidth(1);
   ;
   fArea -= bgArea;

   if(xlow > xhigh) {
      std::swap(xlow, xhigh);
   }
   fSum = fithist->Integral(fithist->GetXaxis()->FindBin(xlow),
                            fithist->GetXaxis()->FindBin(xhigh));   //* fithist->GetBinWidth(1);
   std::cout << "sum between markers: " << fSum << std::endl;
   fDSum = TMath::Sqrt(fSum);
   fSum -= bgArea;
   std::cout << "sum after subtraction: " << fSum << std::endl;

   if(!verbose && !noprint) {
      Print();
   }

   Copy(*fithist->GetListOfFunctions()->FindObject(GetName()));
   fithist->GetListOfFunctions()->Add(fBGFit.Clone());

   return true;
}

void GGaus::Clear(Option_t* opt)
{
   TString options = opt;
   // Clear the GGaus including functions and histogram
   if(options.Contains("all")) {
      TF1::Clear();
   }
   init_flag = false;
   fArea     = 0.0;
   fDArea    = 0.0;
   fSum      = 0.0;
   fDSum     = 0.0;
   fChi2     = 0.0;
   fNdf      = 0.0;
}

void GGaus::Print(Option_t* opt) const
{
   TString options = opt;
   std::cout << GREEN;
   std::cout << "Name: " << GetName() << std::endl;
   std::cout << "Centroid:  " << GetParameter("centroid") << " +- " << GetParError(GetParNumber("centroid")) << std::endl;
   std::cout << "Area:      " << fArea << " +- " << fDArea << std::endl;
   std::cout << "Sum:       " << fSum << " +- " << fDSum << std::endl;
   std::cout << "FWHM:      " << GetFWHM() << " +- " << GetFWHMErr() << std::endl;
   std::cout << "Reso:      " << GetFWHM() / GetParameter("centroid") * 100. << "%%" << std::endl;
   std::cout << "Chi^2/NDF: " << fChi2 / fNdf << std::endl;
   if(options.Contains("all")) {
      TF1::Print(opt);
   }
   std::cout << RESET_COLOR;
}

void GGaus::DrawResiduals(TH1* hist) const
{
   if(hist == nullptr) {
      return;
   }
   if(fChi2 < 0.000000001) {
      std::cout << "No fit performed" << std::endl;
      return;
   }
   Double_t xlow, xhigh;
   GetRange(xlow, xhigh);
   Int_t nbins  = hist->GetXaxis()->GetNbins();
   auto* res    = new Double_t[nbins];
   auto* bin    = new Double_t[nbins];
   Int_t points = 0;
   for(int i = 1; i <= nbins; i++) {
      if(hist->GetBinCenter(i) <= xlow || hist->GetBinCenter(i) >= xhigh) {
         continue;
      }
      res[points] = (hist->GetBinContent(i) - Eval(hist->GetBinCenter(i))) + GetParameter("Height") / 2;
      bin[points] = hist->GetBinCenter(i);
      points++;
   }
   new GCanvas();
   auto* residuals = new TGraph(points, bin, res);
   residuals->Draw("*AC");
   delete[] res;
   delete[] bin;
}
