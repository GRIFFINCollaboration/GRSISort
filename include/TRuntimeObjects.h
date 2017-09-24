#ifndef _RUNTIMEOBJECTS_H_
#define _RUNTIMEOBJECTS_H_

#include <string>
#include <map>
#ifndef __CINT__
#include <memory>
#include <utility>
#endif

#include "TCutG.h"
#include "TDirectory.h"
#include "TList.h"

#include "TFragment.h"
#include "TUnpackedEvent.h"

class TH1;
class TH2;
class TFile;
class TProfile;

/// Object passed to the online histograms.
/**
   For each event, an instance of this type will be passed to the custom histogrammer.
   This class contains all detectors present, and all existing cuts and histograms.
 */
class TRuntimeObjects : public TNamed {
public:
/// Constructor
#ifndef __CINT__
   TRuntimeObjects(std::shared_ptr<const TFragment> frag, TList* objects, TList* gates, std::vector<TFile*>& cut_files,
                   TDirectory* directory = nullptr, const char* name = "default");
#endif
   TRuntimeObjects(TList* objects, TList* gates, std::vector<TFile*>& cut_files, TDirectory* directory = nullptr,
                   const char* name = "default");

#ifndef __CINT__
   /// Returns a pointer to the detector of type T
   template <typename T>
   std::shared_ptr<T> GetDetector()
   {
      return fDetectors->GetDetector<T>();
   }

   std::shared_ptr<const TFragment> GetFragment() { return fFrag; }
#endif

   TCutG* GetCut(const std::string& name);

   TList& GetObjects();
   TList& GetGates();

   TList* GetObjectsPtr() { return fObjects; }
   TList* GetGatesPtr() { return fGates; }

   TH1* FillHistogram(const char* name, int bins, double low, double high, double value, double weight = 1);
   TH2* FillHistogram(const char* name, int Xbins, double Xlow, double Xhigh, double Xvalue, int Ybins, double Ylow,
                      double Yhigh, double Yvalue, double weight = 1);
   TProfile* FillProfileHist(const char* name, int Xbins, double Xlow, double Xhigh, double Xvalue, double Yvalue);
   TH2* FillHistogramSym(const char* name, int Xbins, double Xlow, double Xhigh, double Xvalue, int Ybins, double Ylow,
                         double Yhigh, double Yvalue);

   TH1* FillHistogram(const std::string& name, int bins, double low, double high, double value, double weight = 1)
   {
      return FillHistogram(name.c_str(), bins, low, high, value, weight);
   }
   TH2* FillHistogram(const std::string& name, int Xbins, double Xlow, double Xhigh, double Xvalue, int Ybins,
                      double Ylow, double Yhigh, double Yvalue, double weight = 1)
   {
      return FillHistogram(name.c_str(), Xbins, Xlow, Xhigh, Xvalue, Ybins, Ylow, Yhigh, Yvalue, weight);
   }
   TProfile* FillProfileHist(const std::string& name, int Xbins, double Xlow, double Xhigh, double Xvalue,
                             double Yvalue)
   {
      return FillProfileHist(name.c_str(), Xbins, Xlow, Xhigh, Xvalue, Yvalue);
   }
   TH2* FillHistogramSym(const std::string& name, int Xbins, double Xlow, double Xhigh, double Xvalue, int Ybins,
                         double Ylow, double Yhigh, double Yvalue)
   {
      return FillHistogramSym(name.c_str(), Xbins, Xlow, Xhigh, Xvalue, Ybins, Ylow, Yhigh, Yvalue);
   }
   //---------------------------------------------------------------------
   TDirectory* FillHistogram(const char* dirname, const char* name, int bins, double low, double high, double value,
                             double weight = 1);
   TDirectory* FillHistogram(const char* dirname, const char* name, int Xbins, double Xlow, double Xhigh, double Xvalue,
                             int Ybins, double Ylow, double Yhigh, double Yvalue, double weight = 1);
   TDirectory* FillProfileHist(const char* dirname, const char* name, int Xbins, double Xlow, double Xhigh,
                               double Xvalue, double Yvalue);
   TDirectory* FillHistogramSym(const char* dirname, const char* name, int Xbins, double Xlow, double Xhigh,
                                double Xvalue, int Ybins, double Ylow, double Yhigh, double Yvalue);

   TDirectory* FillHistogram(const std::string& dirname, const std::string& name, int bins, double low, double high,
                             double value, double weight = 1)
   {
      return FillHistogram(dirname.c_str(), name.c_str(), bins, low, high, value, weight);
   }
   TDirectory* FillHistogram(const std::string& dirname, const std::string& name, int Xbins, double Xlow, double Xhigh,
                             double Xvalue, int Ybins, double Ylow, double Yhigh, double Yvalue, double weight = 1)
   {
      return FillHistogram(dirname.c_str(), name.c_str(), Xbins, Xlow, Xhigh, Xvalue, Ybins, Ylow, Yhigh, Yvalue,
                           weight);
   }
   TDirectory* FillProfileHist(const std::string& dirname, const std::string& name, int Xbins, double Xlow,
                               double Xhigh, double Xvalue, double Yvalue)
   {
      return FillProfileHist(dirname.c_str(), name.c_str(), Xbins, Xlow, Xhigh, Xvalue, Yvalue);
   }
   TDirectory* FillHistogramSym(const std::string& dirname, const std::string& name, int Xbins, double Xlow,
                                double Xhigh, double Xvalue, int Ybins, double Ylow, double Yhigh, double Yvalue)
   {
      return FillHistogramSym(dirname.c_str(), name.c_str(), Xbins, Xlow, Xhigh, Xvalue, Ybins, Ylow, Yhigh, Yvalue);
   }

   double GetVariable(const char* name);

   static TRuntimeObjects* Get(const std::string& name = "default")
   {
      if(fRuntimeMap.count(name)) {
         return fRuntimeMap.at(name);
      }
      return nullptr;
   }

#ifndef __CINT__
   void SetFragment(std::shared_ptr<const TFragment> frag) { fFrag = std::move(frag); }
   void SetDetectors(std::shared_ptr<TUnpackedEvent> det) { fDetectors = std::move(det); }
#endif

   void SetDirectory(TDirectory* dir) { fDirectory = dir; }
   TDirectory*                   GetDirectory() const { return fDirectory; }

private:
   static std::map<std::string, TRuntimeObjects*> fRuntimeMap;
#ifndef __CINT__
   std::shared_ptr<TUnpackedEvent>  fDetectors;
   std::shared_ptr<const TFragment> fFrag;
#endif
   TList*               fObjects;
   TList*               fGates;
   std::vector<TFile*>& fCut_files;

   TDirectory* fDirectory;

   ClassDefOverride(TRuntimeObjects, 0);
};

#endif /* _RUNTIMEOBJECTS_H_ */
