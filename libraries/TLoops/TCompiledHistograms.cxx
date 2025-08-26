#include "TCompiledHistograms.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <utility>

#include <sys/stat.h>

#include "TH1.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TObject.h"
#include "TROOT.h"
#include "TKey.h"

#include "GRootCommands.h"
#include "TPreserveGDirectory.h"

using void_alias = void*;

TCompiledHistograms::TCompiledHistograms()
   : fFunc(nullptr), fObj(&fObjects, &fGates, fCutFiles)
{
}

TCompiledHistograms::TCompiledHistograms(std::string inputLib, std::string funcName)
   : fLibName(std::move(inputLib)), fFuncName(std::move(funcName)), fFunc(nullptr), fObj(&fObjects, &fGates, fCutFiles)
{
   fLibrary = std::make_shared<DynamicLibrary>(fLibName.c_str(), true);
   // Casting required to keep gcc from complaining.
   *reinterpret_cast<void_alias*>(&fFunc) = fLibrary->GetSymbol(fFuncName.c_str());

   if(fFunc == nullptr) {
      std::cout << "Could not find " << fFuncName << "() inside "
                << R"(")" << inputLib << R"(")" << std::endl;
   }
   fLastModified = get_timestamp();
   fLastChecked  = time(nullptr);
}

void TCompiledHistograms::ClearHistograms()
{
   std::lock_guard<std::mutex> lock(fMutex);

   TIter    next(&fObjects);
   TObject* obj = nullptr;
   while((obj = next()) != nullptr) {
      if(obj->InheritsFrom(TH1::Class())) {
         auto* hist = static_cast<TH1*>(obj);
         hist->Reset();
      } else if(obj->InheritsFrom(TDirectory::Class())) {
         auto*    dir = static_cast<TDirectory*>(obj);
         TIter    dirnext(dir->GetList());
         TObject* dirobj = nullptr;
         while((dirobj = dirnext()) != nullptr) {
            if(dirobj->InheritsFrom(TH1::Class())) {
               auto* hist = static_cast<TH1*>(dirobj);
               hist->Reset();
            }
         }
      }
   }
   std::cout << "ended " << std::endl;
}

time_t TCompiledHistograms::get_timestamp()
{
   struct stat buf{};
   stat(fLibName.c_str(), &buf);
   return buf.st_mtime;
}

bool TCompiledHistograms::file_exists()
{
   std::ifstream infile(fLibName);
   return infile.is_open();
}

Int_t TCompiledHistograms::Write(const char*, Int_t, Int_t)
{
   fObjects.Sort();

   TIter    next(&fObjects);
   TObject* obj = nullptr;
   while((obj = next()) != nullptr) {
      if(obj->InheritsFrom(TDirectory::Class())) {
         // WATCH OUT: THIS DOESN'T SEEM THREAD-SAFE DUE TO gDIRECTORY BEING USED.
         TPreserveGDirectory preserve;
         auto*               dir = static_cast<TDirectory*>(obj);
         gDirectory->mkdir(dir->GetName())->cd();
         TIter    dir_next(dir->GetList());
         TObject* dir_obj = nullptr;
         while((dir_obj = dir_next()) != nullptr) {
            dir_obj->Write();
         }
      } else {
         obj->Write();
      }
   }

   //  objects.Write();
   TPreserveGDirectory preserve;
   gDirectory->mkdir("variables")->cd();
   return 1;
   // variables.Write();
}

void TCompiledHistograms::Load(const std::string& libName, const std::string& funcName)
{
   TCompiledHistograms other(libName, funcName);
   swap_lib(other);
}

void TCompiledHistograms::Reload()
{
   if(file_exists() && get_timestamp() > fLastModified) {
      TCompiledHistograms other(fLibName, fFuncName);
      swap_lib(other);
   }
   fLastChecked = time(nullptr);
}

void TCompiledHistograms::swap_lib(TCompiledHistograms& other)
{
   std::swap(fLibName, other.fLibName);
   std::swap(fFuncName, other.fFuncName);
   std::swap(fLibrary, other.fLibrary);
   std::swap(fFunc, other.fFunc);
   std::swap(fLastModified, other.fLastModified);
   std::swap(fLastChecked, other.fLastChecked);
   std::swap(fCheckEvery, other.fCheckEvery);
}

void TCompiledHistograms::Fill(std::shared_ptr<const TFragment> frag)
{
   std::lock_guard<std::mutex> lock(fMutex);
   if(time(nullptr) > fLastChecked + fCheckEvery) {
      Reload();
   }

   if(!fLibrary || (fFunc == nullptr) || (fDefaultDirectory == nullptr)) {
      return;
   }

   TPreserveGDirectory preserve;
   // fDefaultDirectory->cd();
   fObj.SetDirectory(fDefaultDirectory);

   fObj.SetFragment(std::move(frag));
   fFunc(fObj);
   fObj.SetFragment(nullptr);
}

void TCompiledHistograms::Fill(std::shared_ptr<TUnpackedEvent> detectors)
{
   std::lock_guard<std::mutex> lock(fMutex);
   if(time(nullptr) > fLastChecked + fCheckEvery) {
      Reload();
   }

   if(!fLibrary || (fFunc == nullptr) || (fDefaultDirectory == nullptr)) {
      return;
   }

   TPreserveGDirectory preserve;
   // fDefaultDirectory->cd();
   fObj.SetDirectory(fDefaultDirectory);

   fObj.SetDetectors(std::move(detectors));
   fFunc(fObj);
   fObj.SetDetectors(nullptr);
}

void TCompiledHistograms::AddCutFile(TFile* cut_file)
{
   if(cut_file != nullptr) {
      fCutFiles.push_back(cut_file);
   }
}

void TCompiledHistograms::SetDefaultDirectory(TDirectory* dir)
{
   fDefaultDirectory = dir;

   TObject* obj = nullptr;
   TIter    next(&fObjects);
   while((obj = next()) != nullptr) {
      TH1* hist = static_cast<TH1*>(obj);
      if(hist != nullptr) {
         hist->SetDirectory(dir);
      }
   }
}
