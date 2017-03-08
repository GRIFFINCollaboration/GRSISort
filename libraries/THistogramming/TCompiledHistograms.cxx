#include "TCompiledHistograms.h"

#include <algorithm>
#include <fstream>
#include <iostream>

#include <sys/stat.h>

#include "TH1.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TObject.h"
#include "TROOT.h"
#include "TKey.h"

#include "GValue.h"
#include "GRootCommands.h"
#include "TPreserveGDirectory.h"

typedef void* __attribute__((__may_alias__)) void_alias;

TCompiledHistograms::TCompiledHistograms()
  : fLibname(""), fFunc_name(""), fLibrary(nullptr), fFunc(nullptr),
    fLast_modified(0), fLast_checked(0), fCheck_every(5),
    fDefault_directory(0),fObj(&fObjects, &fGates, fCut_files) { }

TCompiledHistograms::TCompiledHistograms(std::string input_lib, std::string func_name)
  : TCompiledHistograms() {

  fFunc_name = func_name;
  fLibname = input_lib;
  fLibrary = std::make_shared<DynamicLibrary>(fLibname.c_str(), true);
  // Casting required to keep gcc from complaining.
  *(void_alias*)(&fFunc) = fLibrary->GetSymbol(fFunc_name.c_str());

  if(!fFunc){
    std::cout << "Could not find " << fFunc_name << "() inside "
              <<"\"" << input_lib << "\"" << std::endl;
  }
  fLast_modified = get_timestamp();
  fLast_checked = time(nullptr);
}

void TCompiledHistograms::ClearHistograms() {
  std::lock_guard<std::mutex> lock(fMutex);

  TIter next(&fObjects);
  TObject* obj;
  while((obj = next())){
    if(obj->InheritsFrom(TH1::Class())){
      TH1* hist = (TH1*)obj;
      hist->Reset();
    }
    else if(obj->InheritsFrom(TDirectory::Class())){
      TDirectory* dir = (TDirectory*)obj;
      TIter dirnext(dir->GetList());
      TObject* dirobj;
      while((dirobj=dirnext())){
	if(dirobj->InheritsFrom(TH1::Class())){
	  TH1* hist = (TH1*)dirobj;
	  hist->Reset();
	}
      }
    }

  }
  std::cout << "ended " << std::endl;
}

time_t TCompiledHistograms::get_timestamp() {
  struct stat buf;
  stat(fLibname.c_str(), &buf);
  return buf.st_mtime;
}

bool TCompiledHistograms::file_exists() {
  std::ifstream infile(fLibname);
  return infile.is_open();
}

Int_t TCompiledHistograms::Write(const char* name, Int_t option, Int_t bufsize) {
  fObjects.Sort();

  TIter next(&fObjects);
  TObject *obj;
  while((obj=next())){
    if(obj->InheritsFrom(TDirectory::Class())){
      TPreserveGDirectory preserve;
      TDirectory *dir = (TDirectory*)obj;
      gDirectory->mkdir(dir->GetName())->cd();
      TIter dir_next(dir->GetList());
      TObject *dir_obj;
      while((dir_obj=dir_next())){
	dir_obj->Write();
      }
    }
    else obj->Write();
  }



  //  objects.Write();
  TPreserveGDirectory preserve;
  gDirectory->mkdir("variables")->cd();
  return 1;
  //variables.Write();
}

void TCompiledHistograms::Load(std::string libname, std::string func_name) {
  TCompiledHistograms other(libname, func_name);
  swap_lib(other);
}

void TCompiledHistograms::Reload() {
  if (file_exists() &&
      get_timestamp() > fLast_modified) {
    TCompiledHistograms other(fLibname,fFunc_name);
    swap_lib(other);
  }
  fLast_checked = time(nullptr);
}

void TCompiledHistograms::swap_lib(TCompiledHistograms& other) {
  std::swap(fLibname, other.fLibname);
  std::swap(fFunc_name, other.fFunc_name);
  std::swap(fLibrary, other.fLibrary);
  std::swap(fFunc, other.fFunc);
  std::swap(fLast_modified, other.fLast_modified);
  std::swap(fLast_checked, other.fLast_checked);
  std::swap(fCheck_every, other.fCheck_every);
}

void TCompiledHistograms::Fill(std::shared_ptr<const TFragment> frag) {
  std::lock_guard<std::mutex> lock(fMutex);
  if(time(nullptr) > fLast_checked + fCheck_every){
    Reload();
  }

  if(!fLibrary || !fFunc || !fDefault_directory){
    return;
  }

  TPreserveGDirectory preserve;
  fDefault_directory->cd();
  fObj.SetDirectory(fDefault_directory);

  fObj.SetFragment(frag);
  fFunc(fObj);
  fObj.SetFragment(nullptr);
}

void TCompiledHistograms::Fill(std::shared_ptr<TUnpackedEvent> detectors) {
  std::lock_guard<std::mutex> lock(fMutex);
  if(time(nullptr) > fLast_checked + fCheck_every){
    Reload();
  }

  if(!fLibrary || !fFunc || !fDefault_directory){
    return;
  }

  TPreserveGDirectory preserve;
  fDefault_directory->cd();
  fObj.SetDirectory(fDefault_directory);

  fObj.SetDetectors(detectors);
  fFunc(fObj);
  fObj.SetDetectors(nullptr);
}

void TCompiledHistograms::AddCutFile(TFile* cut_file) {
  if(cut_file) {
    fCut_files.push_back(cut_file);
  }
}

void TCompiledHistograms::SetDefaultDirectory(TDirectory* dir) {
  fDefault_directory = dir;

  TObject* obj = nullptr;
  TIter next(&fObjects);
  while((obj = next())) {
    TH1* hist = dynamic_cast<TH1*>(obj);
    if(hist) {
      hist->SetDirectory(dir);
    }
  }
}
