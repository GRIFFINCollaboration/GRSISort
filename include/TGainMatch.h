#ifndef __TGAINMATCH_H__
#define __TGAINMATCH_H__

#include "TCal.h"

class TGainMatch : public TCal {
 public: 
   TGainMatch();
   TGainMatch(const char* name, const char* title) : TCal(name,title){}
   ~TGainMatch(); 

 public:
   Bool_t CoarseMatch(TH1 *hist,Int_t channelNum);

   void Clear();
   void Print() const;

 private:
   Bool_t fcoarse_match;
   
   ClassDef(TGainMatch,1);

};

#endif
