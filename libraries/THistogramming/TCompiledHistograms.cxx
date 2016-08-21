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
  : libname(""), func_name(""), library(nullptr), func(nullptr),
    last_modified(0), last_checked(0), check_every(5),
    default_directory(0),obj(&objects, &gates, cut_files) { }

TCompiledHistograms::TCompiledHistograms(std::string input_lib, std::string func_name)
  : TCompiledHistograms() {

  this->func_name = func_name;
  libname = input_lib;
  library = std::make_shared<DynamicLibrary>(libname.c_str(), true);
  // Casting required to keep gcc from complaining.
  *(void_alias*)(&func) = library->GetSymbol(func_name.c_str());

  if(!func){
    std::cout << "Could not find " << func_name << "() inside "
              <<"\"" << input_lib << "\"" << std::endl;
  }
  last_modified = get_timestamp();
  last_checked = time(NULL);
}

void TCompiledHistograms::ClearHistograms() {
  std::lock_guard<std::mutex> lock(mutex);

  TIter next(&objects);
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
  stat(libname.c_str(), &buf);
  return buf.st_mtime;
}

bool TCompiledHistograms::file_exists() {
  std::ifstream infile(libname);
  return infile.is_open();
}

void TCompiledHistograms::Write() {
  objects.Sort();

  TIter next(&objects);
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
  //variables.Write();
}

void TCompiledHistograms::Load(std::string libname, std::string func_name) {
  TCompiledHistograms other(libname, func_name);
  swap_lib(other);
}

void TCompiledHistograms::Reload() {
  if (file_exists() &&
      get_timestamp() > last_modified) {
    TCompiledHistograms other(libname,func_name);
    swap_lib(other);
  }
  last_checked = time(NULL);
}

void TCompiledHistograms::swap_lib(TCompiledHistograms& other) {
  std::swap(libname, other.libname);
  std::swap(func_name, other.func_name);
  std::swap(library, other.library);
  std::swap(func, other.func);
  std::swap(last_modified, other.last_modified);
  std::swap(last_checked, other.last_checked);
  std::swap(check_every, other.check_every);
}

void TCompiledHistograms::Fill(TFragment& frag) {
  std::lock_guard<std::mutex> lock(mutex);
  if(time(NULL) > last_checked + check_every){
    Reload();
  }

  if(!library || !func || !default_directory){
    return;
  }

  TPreserveGDirectory preserve;
  default_directory->cd();
  obj.SetDirectory(default_directory);

  obj.SetFragment(&frag);
  func(obj);
  obj.SetFragment(NULL);
}

void TCompiledHistograms::Fill(TUnpackedEvent& detectors) {
  std::lock_guard<std::mutex> lock(mutex);
  if(time(NULL) > last_checked + check_every){
    Reload();
  }

  if(!library || !func || !default_directory){
    return;
  }

  TPreserveGDirectory preserve;
  default_directory->cd();

  obj.SetDetectors(&detectors);
  func(obj);
  obj.SetDetectors(NULL);
}

void TCompiledHistograms::AddCutFile(TFile* cut_file) {
  if(cut_file) {
    cut_files.push_back(cut_file);
  }
}

void TCompiledHistograms::SetDefaultDirectory(TDirectory* dir) {
  default_directory = dir;

  TObject* obj = NULL;
  TIter next(&objects);
  while((obj = next())) {
    TH1* hist = dynamic_cast<TH1*>(obj);
    if(hist) {
      hist->SetDirectory(dir);
    }
  }
}
