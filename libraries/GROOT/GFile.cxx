
#include "GFile.h"

/// \cond CLASSIMP
ClassImp(GFile)
/// \endcond

GFile::GFile() : TFile() {
  GFileInit();
}

GFile::GFile(const char *name,Option_t *opt,const char *ftitle,Int_t compress) : TFile(name,opt,ftitle,compress) {
   GFileInit();
}

GFile::~GFile() { }

void GFile::GFileInit() {


}

