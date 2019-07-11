#include "TRuntimeObjects.h"

#include <iostream>
#include <utility>

#include "TClass.h"
#include "TCutG.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TDirectoryFile.h"
#include "TProfile.h"

#include "TH1D.h"
#include "TH2D.h"

#include "GValue.h"

std::map<std::string, TRuntimeObjects*> TRuntimeObjects::fRuntimeMap;

TRuntimeObjects::TRuntimeObjects(std::shared_ptr<const TFragment> frag, TList* objects, TList* gates,
                                 std::vector<TFile*>& cut_files, TDirectory* directory, const char* name)
   : fFrag(std::move(frag)), // detectors(nullptr),
     fObjects(objects), fGates(gates), fCut_files(cut_files), fDirectory(directory)
{
   SetName(name);
   fRuntimeMap.insert(std::make_pair(name, this));

}

// TRuntimeObjects::TRuntimeObjects(TUnpackedEvent *detectors, TList* objects, TList *gates,
//                                  std::vector<TFile*>& cut_files,
//                                  TDirectory* directory,const char *name)
//   : detectors(detectors), objects(objects), gates(gates),
//     cut_files(cut_files),
//     directory(directory) {
//   SetName(name);
//   fRuntimeMap.insert(std::make_pair(name,this));
// }

TRuntimeObjects::TRuntimeObjects(TList* objects, TList* gates, std::vector<TFile*>& cut_files, TDirectory* directory,
                                 const char* name)
   : fFrag(nullptr), // detectors(0),
     fObjects(objects), fGates(gates), fCut_files(cut_files), fDirectory(directory)
{
   SetName(name);
   fRuntimeMap.insert(std::make_pair(name, this));
}

TH1* TRuntimeObjects::FillHistogram(const char* name, int bins, double low, double high, double value, double weight)
{
	FillHistogram("General",name,bins,low,high,value,weight);
	return nullptr;
}

TH2* TRuntimeObjects::FillHistogram(const char* name, int Xbins, double Xlow, double Xhigh, double Xvalue, int Ybins,
                                    double Ylow, double Yhigh, double Yvalue, double weight)
{
	FillHistogram("General",name,Xbins,Xlow,Xhigh,Xvalue,Ybins,Ylow,Yhigh,Yvalue,weight);
	return nullptr;
}

TProfile* TRuntimeObjects::FillProfileHist(const char* name, int Xbins, double Xlow, double Xhigh, double Xvalue,
                                           double Yvalue)
{
   FillProfileHist("General",name,Xbins,Xlow,Xhigh,Xvalue,Yvalue);
   return nullptr;
}

TH2* TRuntimeObjects::FillHistogramSym(const char* name, int Xbins, double Xlow, double Xhigh, double Xvalue, int Ybins,
                                       double Ylow, double Yhigh, double Yvalue)
{
	FillHistogramSym("General",name,Xbins,Xlow,Xhigh,Xvalue,Ybins,Ylow,Yhigh,Yvalue);
   return nullptr;
}

//-------------------------------------------------------------------------
TDirectory* TRuntimeObjects::FillHistogram(const char* dirname, const char* name, int bins, double low, double high,
                                           double value, double weight)
{
	
	TDirectory * dir = FindDirectory(dirname);

   TH1* hist = static_cast<TH1*>(dir->FindObject(name));
   if(hist == nullptr) {
      hist = new TH1D(name, name, bins, low, high);
      hist->SetDirectory(dir);
      dir->Add(hist);
   }

   if(!std::isnan(value)) {
      hist->Fill(value, weight);
   }
   return dir;
}

TDirectory* TRuntimeObjects::FillHistogram(const char* dirname, const char* name, int Xbins, double Xlow, double Xhigh,
                                           double Xvalue, int Ybins, double Ylow, double Yhigh, double Yvalue,
                                           double weight)
{
	TDirectory * dir = FindDirectory(dirname);
   TH2* hist = static_cast<TH2*>(dir->FindObject(name));
   if(hist == nullptr) {
      hist = new TH2D(name, name, Xbins, Xlow, Xhigh, Ybins, Ylow, Yhigh);
      hist->SetDirectory(dir);
      dir->Add(hist);
   }

   if(!std::isnan(Xvalue) && !std::isnan(Yvalue)) {
      hist->Fill(Xvalue, Yvalue, weight);
   }
   return dir; 
}

TDirectory* TRuntimeObjects::FillProfileHist(const char* dirname, const char* name, int Xbins, double Xlow,
                                             double Xhigh, double Xvalue, double Yvalue)
{

   TDirectory *dir = FindDirectory(dirname);
	TProfile* prof = dynamic_cast<TProfile*>(dir->FindObject(name));
   if(prof == nullptr) {
      prof = new TProfile(name, name, Xbins, Xlow, Xhigh);
      prof->SetDirectory(dir);
      dir->Add(prof);
   }

   if(!(std::isnan(Xvalue))) {
      if(!(std::isnan(Yvalue))) {
         prof->Fill(Xvalue, Yvalue);
      }
   }
   return dir;
}

TDirectory* TRuntimeObjects::FillHistogramSym(const char* dirname, const char* name, int Xbins, double Xlow,
                                              double Xhigh, double Xvalue, int Ybins, double Ylow, double Yhigh,
                                              double Yvalue)
{
   TDirectory *dir = FindDirectory(dirname);
   TH2* hist = dynamic_cast<TH2*>(dir->FindObject(name));
   if(hist == nullptr) {
      hist = new TH2D(name, name, Xbins, Xlow, Xhigh, Ybins, Ylow, Yhigh);
      hist->SetDirectory(dir);
      dir->Add(hist);
   }
   if(!(std::isnan(Xvalue))) {
      if(!(std::isnan(Yvalue))) {
         hist->Fill(Xvalue, Yvalue);
         hist->Fill(Yvalue, Xvalue);
      }
   }
   return dir; 
}
//-------------------------------------------------------------------------

TList& TRuntimeObjects::GetObjects()
{
   return *fObjects;
}

TList& TRuntimeObjects::GetGates()
{
   return *fGates;
}

TCutG* TRuntimeObjects::GetCut(const std::string& name)
{
   for(auto& tfile : fCut_files) {
      TObject* obj = tfile->Get(name.c_str());
      if(obj != nullptr) {
         TCutG* cut = dynamic_cast<TCutG*>(obj);
         if(cut != nullptr) {
            return cut;
         }
      }
   }
   return nullptr;
}

double TRuntimeObjects::GetVariable(const char* name)
{
   return GValue::Value(name);
}

TDirectory * TRuntimeObjects::FindDirectory(const char* dirname)
{
	/*static bool found_default = false;
	if(!strcmp(dirname,"")){
		if(!found_default && !GetObjects().FindObject(fDirectory)){
			found_default = true;
			GetObjects().Add(fDirectory);
		}	
		return fDirectory;
	}*/

   TDirectory* dir = static_cast<TDirectory*>(GetObjects().FindObject(dirname));
   if(dir == nullptr) {
      dir = new TDirectory(dirname, dirname);
      GetObjects().Add(dir);
   }
	return dir;

}
