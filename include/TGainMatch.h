#ifndef __TGAINMATCH_H__
#define __TGAINMATCH_H__

#include "TCal.h"
#include "TPeak.h"
#include "TSpectrum.h"

class TGainMatch : public TCal {
 public: 
   TGainMatch();
   TGainMatch(const char* name, const char* title) : TCal(name,title){}
   ~TGainMatch(); 

 public:
   Bool_t CoarseMatch(TH1 *hist,Int_t channelNum);
   Bool_t FineMatch(TH1 *hist1, TPeak* peak1, TH1 *hist2, TPeak* peak2, Int_t channelNum);
   Bool_t FineMatch(TH1 *hist, TPeak* peak1, TPeak* peak2, Int_t channelNum);
   Bool_t FineMatch(TH1 *hist1, Double_t energy1, Double_t energy2, Int_t channelNum);
   Bool_t FineMatch(TH1 *hist1, Double_t energy1, TH1 *hist2, Double_t energy2, Int_t channelNum);

   void Clear();
   void Print(Option_t *opt = "") const;

   Bool_t IsGroupable() const {return false;}

 private:
   Bool_t fcoarse_match;
   
   ClassDef(TGainMatch,1);

};

#endif
