#ifndef __TCAL_H__
#define __TCAL_H__

#include <TNamed.h>
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


class TCal : public TGraphErrors {
 public: 
   TCal();
   TCal(const char* name, const char* title){SetNameTitle(name,title);}
   virtual ~TCal(); 

 public:
   UInt_t GetChannelNumber() const { return fchanNum; }

 protected: 
   virtual void Clear();
   virtual void Print() const;
   
   void SetChannelNumber(UInt_t channum) { fchanNum = channum; }
   virtual Bool_t IsGroupable() const = 0;

 protected:
   std::vector<Double_t> fcoeffs;
   std::vector<Double_t> fdcoeffs;

 private:
   UInt_t fchanNum;
   
   /*
   void OpenFile(const char * filename);
 public:
   void CalibrateEfficiency();
   void CalibrateEnergy();
   Bool_t FitEnergyCal();
   void AddEnergyGraph(TGraphErrors *graph, Int_t channum, const char *nucname, const char* directory = "");
   void AddEnergyGraph(Int_t channum, const char *nucname, TGraphErrors *graph, const char* directory = "");
   void AddEfficiencyGraph(Int_t channum, const char * nucname, TGraphErrors *graph,const char* directory = "");
   void AutoFitSource();


   void ColorGraphsBySource(Bool_t colflag = kTRUE, TDirectory* source = NULL);

 private:
   TFile *effFile; //= NULL;
   std::map<Int_t,std::map<std::string,TGraphErrors*>> fenergyMap;
   std::map<Int_t,std::map<std::string,TGraphErrors*>> fefficiencyMap;

   void AddGraph(Int_t channum, const char *nucname, TGraphErrors *graph, const char* directory = "");
*/
  ClassDef(TCal,1);

};

#endif
