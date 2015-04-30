
#include "Globals.h"

#include <iostream>

#include <GDirectory.h>

ClassImp(GDirectory)

//GDirectory::GDirectory() { }

//GDirectory::GDirectory(const char* name, const char* title, Option_t* option, TDirectory* motherDir) { }

GDirectory::~GDirectory() { }


void GDirectory::Build(TFile *motherFile,TDirectory *motherDir) {
  std::cout << "\nGDirectory build called\n" << std::endl;
  TDirectory::Build(motherFile,motherDir);
}



void GDirectory::Append(TObject *obj,Bool_t replace) {
  std::cout << "\nGDirectory append called\n" << std::endl;
  TDirectory::Append(obj,replace);
}

