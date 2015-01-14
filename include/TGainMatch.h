#ifndef __TGAINMATCH_H__
#define __TGAINMATCH_H__

#include "TCal.h"
#include "TCalManager.h"
#include "TPeak.h"
#include "TSpectrum.h"
#include "TH2.h"
#include <algorithm>
#include <map>


class TGainMatch : public TCal {
 public: 
   TGainMatch(){}
   TGainMatch(const char* name, const char* title) : TCal(name,title){}
   ~TGainMatch(){} 

   TGainMatch(const TGainMatch &copy);

 public:
   void Copy(TObject &obj) const;

   static Bool_t CoarseMatchAll(TCalManager* cm, TH2 *mat, Double_t energy1 = 1173.228, Double_t energy2 = 1332.492);
   static Bool_t FineMatchAll(TCalManager* cm, TH2 *mat, Double_t energy1, Double_t energy2);
   static Bool_t FineMatchAll(TCalManager* cm, TH2 *mat1, Double_t energy1, TH2 *mat2, Double_t energy2);
   static Bool_t FineMatchAll(TCalManager* cm, TH2 *mat1, TPeak* peak1, TH2 *hist2, TPeak* peak2);
   static Bool_t FineMatchAll(TCalManager* cm, TH2 *mat, TPeak* peak1, TPeak* peak2);

   Bool_t CoarseMatch(TH1 *hist,Int_t channelNum = 9999,Double_t energy1 = 1173.228, Double_t energy2 = 1332.492);
   Bool_t FineMatch(TH1 *hist1, TPeak* peak1, TH1 *hist2, TPeak* peak2, Int_t channelNum = 9999);
   Bool_t FineMatch(TH1 *hist, TPeak* peak1, TPeak* peak2, Int_t channelNum = 9999);
   Bool_t FineMatch(TH1 *hist1, Double_t energy1, Double_t energy2, Int_t channelNum = 9999);
   Bool_t FineMatch(TH1 *hist1, Double_t energy1, TH1 *hist2, Double_t energy2, Int_t channelNum = 9999);

   std::vector<Double_t> GetParameters() const;
   Double_t GetParameter(Int_t parameter) const;

   void Clear(Option_t *opt = "");
   void Print(Option_t *opt = "") const;

   Bool_t IsGroupable() const {return false;}
   void WriteToChannel() const;

 private:
   Bool_t fcoarse_match;

   ClassDef(TGainMatch,1);

};

#endif
