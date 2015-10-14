#ifndef TGAINMATCH_H__
#define TGAINMATCH_H__

#include "TCal.h"
#include "TCalManager.h"
#include "TPeak.h"
#include "TSpectrum.h"
#include "TH2.h"
#include "TF1.h"


class TGainMatch : public TCal {
 public: 
   TGainMatch(){}
   TGainMatch(const char* name, const char* title) : TCal(name,title){Clear();}
   ~TGainMatch(){} 

   TGainMatch(const TGainMatch &copy);

 public:
   void Copy(TObject &obj) const;

   void CalculateGain(Double_t cent1, Double_t cent2, Double_t eng1, Double_t eng2);

   static Bool_t CoarseMatchAll(TCalManager* cm, TH2 *mat, Double_t energy1 = 1173.228, Double_t energy2 = 1332.492);
   static Bool_t FineMatchFastAll(TCalManager* cm, TH2 *mat, Double_t energy1, Double_t energy2);
   static Bool_t FineMatchFastAll(TCalManager* cm, TH2 *mat1, Double_t energy1, TH2 *mat2, Double_t energy2);
   static Bool_t FineMatchFastAll(TCalManager* cm, TH2 *mat1, TPeak* peak1, TH2 *hist2, TPeak* peak2);
   static Bool_t FineMatchFastAll(TCalManager* cm, TH2 *mat, TPeak* peak1, TPeak* peak2);

   static Bool_t FineMatchAll(TCalManager* cm, TH2 *charge_mat, TH2* eng_mat, Int_t testchan, Double_t energy1, Double_t energy2, Int_t low_range=100, Int_t high_range = 600);

   static Bool_t AlignAll(TCalManager*cm, TH1* hist, TH2* mat, Int_t low_range = 100, Int_t high_range = 600); 

   Bool_t CoarseMatch(TH1 *hist,Int_t channelNum = 9999,Double_t energy1 = 1173.228, Double_t energy2 = 1332.492);
   Bool_t FineMatchFast(TH1 *hist1, TPeak* peak1, TH1 *hist2, TPeak* peak2, Int_t channelNum = 9999);
   Bool_t FineMatchFast(TH1 *hist, TPeak* peak1, TPeak* peak2, Int_t channelNum = 9999);
   Bool_t FineMatchFast(TH1 *hist1, Double_t energy1, Double_t energy2, Int_t channelNum = 9999);
   Bool_t FineMatchFast(TH1 *hist1, Double_t energy1, TH1 *hist2, Double_t energy2, Int_t channelNum = 9999);

   Bool_t FineMatch(TH1 *energy_hist, TH1* test_hist, TH1* charge_hist, Double_t energy1, Double_t energy2, Int_t low_range = 100, Int_t high_range = 600, Int_t channelNum = 9999);
 
   Bool_t Align(TH1* testhist, TH1* hist,Int_t low_range = 100, Int_t high_range = 600);

   void Clear(Option_t *opt = "");
   void Print(Option_t *opt = "") const;

   Bool_t IsGroupable() const {return false;}
   void WriteToChannel() const;

   void SetNucleus(TNucleus* nuc) { Warning("SetNucleus","Is not used in TGainMatching");} 
   TNucleus* GetNucleus() const { Warning("GetNucleus","Is not used in TGainMatching"); return 0; }

   void SetHist(TH1* nuc) { Warning("SetHist","Is not used in TGainMatching");} 
   TH1* GetHist() const { Warning("GetHist","Is not used in TGainMatching"); return 0; }

 private:
   Bool_t fcoarse_match;
   Bool_t faligned;
   TH1* fhist;
   Double_t fAlign_coeffs[2];
   Double_t fGain_coeffs[2];
   Double_t HistCompare(Double_t *x, Double_t *par);
 //  Double_t HistCompare(const Double_t *xx);

   ClassDef(TGainMatch,1);

};

#endif
