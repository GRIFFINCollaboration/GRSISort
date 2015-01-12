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
   TGraphErrors *Graph() const { return fgraph; }
   virtual void WriteToChannel() const {Error("WriteToChannel","Not defined for %s",ClassName());}

   TChannel *GetChannel() const;
   Bool_t SetChannel(const TChannel* chan);
   Bool_t SetChannel(UInt_t channum);
   virtual void SetFitFunction(void* fnc){};
   virtual void Print(Option_t *opt = "") const;
   virtual void Clear(Option_t *opt = "");

 private:
   void InitTCal();
   TGraphErrors *fgraph;
   TChannel *fchan;

   ClassDef(TCal,1);

};

#endif
