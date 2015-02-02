#ifndef __GFILE_H__
#define __GFILE_H__

#include "TFile.h"

class GFile : public TFile {
   public:
      GFile();
      GFile(const char *name,Option_t *opt="",const char *ftitle="",Int_t compress=1);
      ~GFile();

 
   private:

      void GFileInit();

   ClassDef(GFile,1)
};

#endif


