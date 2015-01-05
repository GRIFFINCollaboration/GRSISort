#ifndef __TGAINMATCH_H__
#define __TGAINMATCH_H__

#include "TCal.h"

class TGainMatch : public TCal {
 public: 
   TGainMatch();
   ~TGainMatch(); 

 public:
   Bool_t CourseMatch(TH1 *hist,Int_t channelNum);

   void Clear();
   void Print() const;

 private:
   Bool_t fcourse_match;
   
   ClassDef(TGainMatch,1);

};

#endif
