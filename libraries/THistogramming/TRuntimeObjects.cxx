#include "TRuntimeObjects.h"

#include <iostream>

#include "TClass.h"
#include "TCutG.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TDirectoryFile.h"
#include "TProfile.h"

#include "GH1D.h"
#include "GH2D.h"
#include "GH2D.h"

#include "GValue.h"

std::map<std::string, TRuntimeObjects*> TRuntimeObjects::fRuntimeMap;

TRuntimeObjects::TRuntimeObjects(std::shared_ptr<const TFragment> frag, TList* objects, TList* gates,
                                 std::vector<TFile*>& cut_files, TDirectory* directory, const char* name)
   : fFrag(frag), // detectors(nullptr),
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
   TH1* hist = (TH1*)GetObjects().FindObject(name);
   if(!hist) {
      hist = new GH1D(name, name, bins, low, high);
      if(fDirectory) {
         hist->SetDirectory(fDirectory);
      }
      GetObjects().Add(hist);
   }
   if(!(std::isnan(value))) {
      hist->Fill(value, weight);
   }
   return hist;
}

TH2* TRuntimeObjects::FillHistogram(const char* name, int Xbins, double Xlow, double Xhigh, double Xvalue, int Ybins,
                                    double Ylow, double Yhigh, double Yvalue, double weight)
{
   TH2* hist = (TH2*)GetObjects().FindObject(name);
   if(!hist) {
      hist = new GH2D(name, name, Xbins, Xlow, Xhigh, Ybins, Ylow, Yhigh);
      if(fDirectory) hist->SetDirectory(fDirectory);
      GetObjects().Add(hist);
   }
   if(!std::isnan(Xvalue) && !std::isnan(Yvalue)) {
      hist->Fill(Xvalue, Yvalue, weight);
   }
   return hist;
}

TProfile* TRuntimeObjects::FillProfileHist(const char* name, int Xbins, double Xlow, double Xhigh, double Xvalue,
                                           double Yvalue)
{
   TProfile* prof = (TProfile*)GetObjects().FindObject(name);
   if(!prof) {
      prof = new TProfile(name, name, Xbins, Xlow, Xhigh);
      if(fDirectory) prof->SetDirectory(fDirectory);
      GetObjects().Add(prof);
   }
   if(!(std::isnan(Xvalue)))
      if(!(std::isnan(Yvalue))) prof->Fill(Xvalue, Yvalue);
   return prof;
}

TH2* TRuntimeObjects::FillHistogramSym(const char* name, int Xbins, double Xlow, double Xhigh, double Xvalue, int Ybins,
                                       double Ylow, double Yhigh, double Yvalue)
{
   TH2* hist = (TH2*)GetObjects().FindObject(name);
   if(!hist) {
      hist = new GH2D(name, name, Xbins, Xlow, Xhigh, Ybins, Ylow, Yhigh);
      if(fDirectory) hist->SetDirectory(fDirectory);
      GetObjects().Add(hist);
   }

   if(!(std::isnan(Xvalue))) {
      if(!(std::isnan(Yvalue))) {
         hist->Fill(Xvalue, Yvalue);
         hist->Fill(Yvalue, Xvalue);
      }
   }
   return hist;
}

//-------------------------------------------------------------------------
TDirectory* TRuntimeObjects::FillHistogram(const char* dirname, const char* name, int bins, double low, double high,
                                           double value, double weight)
{

   TDirectory* dir = (TDirectory*)GetObjects().FindObject(dirname);
   if(!dir) {
      dir = new TDirectory(dirname, dirname);
      GetObjects().Add(dir);
   }
   dir->cd();
   TH1* hist = (TH1*)dir->FindObject(name);
   if(!hist) {
      hist = new GH1D(name, name, bins, low, high);
      hist->SetDirectory(dir);
      dir->Add(hist);
   }

   if(!std::isnan(value)) {
      hist->Fill(value, weight);
   }
   dir->cd("../");
   // return hist;
   return dir;

   /*
   std::cout << "1" << std::endl;
   if(!(gDirectory->cd(dirname))){
       gDirectory->mkdir(dirname);
         std::cout << "2" << std::endl;
   }
   std::cout << "3" << std::endl;
   TDirectory *dir = gDirectory->GetDirectory(dirname);
   //gDirectory->pwd()
   std::cout << "4" << std::endl;;
   TH1* hist = (TH1*)dir->FindObject(name);
   std::cout << "4a" << std::endl;
   if(!hist){
     std::cout << "4b" << std::endl;
     hist = new TH1I(name,name,bins,low,high);
     //    GetObjects().Add(hist);
     std::cout << "5" << std::endl;
     dir->Add(hist);
     std::cout << "6" << std::endl;
   }
   std::cout << "7" << std::endl;
   hist->Fill(value);
   std::cout << "8" << std::endl;
   gDirectory->cd("../");
   std::cout << "9" << std::endl;
   return dir;
   //return hist;*/
}

TDirectory* TRuntimeObjects::FillHistogram(const char* dirname, const char* name, int Xbins, double Xlow, double Xhigh,
                                           double Xvalue, int Ybins, double Ylow, double Yhigh, double Yvalue,
                                           double weight)
{
   TDirectory* dir = (TDirectory*)GetObjects().FindObject(dirname);
   if(!dir) {
      dir = new TDirectory(dirname, dirname);
      GetObjects().Add(dir);
   }
   dir->cd();
   TH2* hist = (TH2*)dir->FindObject(name);
   if(!hist) {
      hist = new GH2D(name, name, Xbins, Xlow, Xhigh, Ybins, Ylow, Yhigh);
      hist->SetDirectory(dir);
      dir->Add(hist);
   }

   if(!std::isnan(Xvalue) && !std::isnan(Yvalue)) {
      hist->Fill(Xvalue, Yvalue, weight);
   }
   dir->cd("../");
   // return hist;
   return dir; /*
    TH2* hist = (TH2*) GetObjects().FindObject(name);
    if(!hist){
      hist = new GH2D(name.c_str(),name.c_str(),
                              Xbins, Xlow, Xhigh,
                              Ybins, Ylow, Yhigh);
      GetObjects().Add(hist);
    }
    hist->Fill(Xvalue, Yvalue);
    return hist;*/
}

TDirectory* TRuntimeObjects::FillProfileHist(const char* dirname, const char* name, int Xbins, double Xlow,
                                             double Xhigh, double Xvalue, double Yvalue)
{

   TDirectory* dir = (TDirectory*)GetObjects().FindObject(dirname);
   if(!dir) {
      dir = new TDirectory(dirname, dirname);
      GetObjects().Add(dir);
   }
   dir->cd();
   TProfile* prof = (TProfile*)dir->FindObject(name);
   if(!prof) {
      prof = new TProfile(name, name, Xbins, Xlow, Xhigh);
      prof->SetDirectory(dir);
      dir->Add(prof);
   }

   if(!(std::isnan(Xvalue)))
      if(!(std::isnan(Yvalue))) prof->Fill(Xvalue, Yvalue);
   dir->cd("../");
   return dir;
}

TDirectory* TRuntimeObjects::FillHistogramSym(const char* dirname, const char* name, int Xbins, double Xlow,
                                              double Xhigh, double Xvalue, int Ybins, double Ylow, double Yhigh,
                                              double Yvalue)
{
   TDirectory* dir = (TDirectory*)GetObjects().FindObject(dirname);
   if(!dir) {
      dir = new TDirectory(dirname, dirname);
      GetObjects().Add(dir);
   }
   dir->cd();
   TH2* hist = (TH2*)dir->FindObject(name);
   if(!hist) {
      hist = new GH2D(name, name, Xbins, Xlow, Xhigh, Ybins, Ylow, Yhigh);
      hist->SetDirectory(dir);
      dir->Add(hist);
   }
   if(!(std::isnan(Xvalue))) {
      if(!(std::isnan(Yvalue))) {
         hist->Fill(Xvalue, Yvalue);
         hist->Fill(Yvalue, Xvalue);
      }
   }
   dir->cd("../");
   // return hist;
   return dir; /*
    TH2* hist = (TH2*) GetObjects().FindObject(name);
    if(!hist){
      hist = new GH2D(name.c_str(),name.c_str(),
                              Xbins, Xlow, Xhigh,
                              Ybins, Ylow, Yhigh);
      GetObjects().Add(hist);
    }
    hist->Fill(Xvalue, Yvalue);
    return hist;*/
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
      if(obj) {
         TCutG* cut = dynamic_cast<TCutG*>(obj);
         if(cut) {
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
