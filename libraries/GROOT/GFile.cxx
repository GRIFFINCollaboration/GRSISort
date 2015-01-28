
#include "GFile.h"

ClassImp(GFile)

GFile::GFile() {
  TFile::TFile();
  GFileInit();
}

GFile::GFile(const char *name,Option_t *opt,const char *ftitle,Int_t compress) {
   TFile::TFile(name,opt,ftitle,compress);
   GFileInit();
}

GFile::~GFile() { }

void GFile::GFileInit() {


}

