#ifndef TCAL_H__
#define TCAL_H__

#include "Varargs.h"
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
#include "TChannel.h"
#include "TMultiGraph.h"
#include "TGraphErrors.h"
#include "TNucleus.h"
#include "TRef.h"
//#include "../include/TNucleus.h"

#include "../include/TGRSITransition.h"

class TCal : public TGraphErrors {
 public: 
   TCal();
   TCal(const char* name, const char* title);
   virtual ~TCal(); 

   TCal(const TCal& copy);

 //pure virtual functions  
   virtual Bool_t IsGroupable() const = 0;

 public:
   virtual void Copy(TObject &obj) const;
   //TGraphErrors *Graph() const { return fgraph; }
   virtual void WriteToChannel() const {Error("WriteToChannel","Not defined for %s",ClassName());}
   virtual TF1* GetFitFunction() const { return ffitfunc; } 
   virtual void SetFitFunction(const TF1* func){ ffitfunc = (TF1*)func; };
   virtual std::vector<Double_t> GetParameters() const;
   virtual Double_t GetParameter(Int_t parameter) const;

   //static TGraphErrors MergeGraphs(TCal *cal,...);

   TChannel* const GetChannel() const;
   Bool_t SetChannel(const TChannel* chan);
   Bool_t SetChannel(UInt_t channum);
   virtual void Print(Option_t *opt = "") const;
   virtual void Clear(Option_t *opt = "");
   //virtual void Draw(Option_t *chopt = "");

   virtual void WriteToAllChannels(std::string mnemonic = "");

   virtual void SetHist(TH1* hist);
   TH1* GetHist() const {return fhist;}
   virtual void SetNucleus(TNucleus* nuc,Option_t *opt = "");
   virtual TNucleus* GetNucleus() const { return fnuc; }

 protected:
   void InitTCal();

 private:
   //TGraphErrors *fgraph; //->
   TRef fchan; //This points at the TChannel
   TF1* ffitfunc; //-> Fit function representing calibration
   TH1* fhist; //Histogram that was fit by the TPeak.
   TNucleus* fnuc; //Nucleus that we are calibrating against

   ClassDef(TCal,1); //Abstract Class for Calibrations

};

#endif
