
#include "GFile.h"

ClassImp(GFile)

GFile::GFile() : TFile() {
  GFileInit();
}

GFile::GFile(const char *name,Option_t *opt,const char *ftitle,Int_t compress) : TFile(name,opt,ftitle,compress) {
   GFileInit();
}

GFile::~GFile() { }

void GFile::GFileInit() {


}

