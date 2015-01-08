#ifndef __TCAL_H__
#define __TCAL_H__

#include "TNamed.h"
#include "TH1.h"
#include "TF1.h"
#include "TList.h"
#include "TFitResult.h"
#include "TFitResultPtr.h"
#include "TRandom.h"
#include "TSpectrum.h"
#include "TVirtualFitter.h"
#include "TMath.h"
#include "TCanvas.h"
#include <map>
#include <vector>
#include <utility>
#include "TROOT.h"
#include "TFile.h"
#include "TChannel.h"
#include "TMultiGraph.h"
#include "TGraphErrors.h"
#include "TNucleus.h"
#include "TKey.h"
//#include "../include/TNucleus.h"

#include "../include/TGRSITransition.h"


class TCal : public TNamed {
 public: 
   TCal();
   TCal(const char* name, const char* title);
   virtual ~TCal(); 

 //pure virtual functions  
   virtual Bool_t IsGroupable() const = 0;
   virtual std::vector<Double_t> GetParameters() const = 0;
   virtual Double_t GetParameter(Int_t parameter) const = 0;

 public:
   UInt_t GetChannelNumber() const { return fchanNum; }
   TGraphErrors *Graph() const { return fgraph; }

   virtual void SetFitFunction(void* fnc){};

 protected: 
   virtual void Clear(Option_t *opt = "");
   virtual void Print(Option_t *opt = "") const;
   
   void SetChannelNumber(UInt_t channum) { fchanNum = channum; }

 private:
   UInt_t fchanNum;
   TGraphErrors *fgraph;

   ClassDef(TCal,1);

};

#endif
