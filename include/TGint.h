#ifndef TGINT_H
#define TGINT_H

#include "TRint.h"

class TGint : public TRint {
   private:
      TGint(int argc, char** argv,void* options = 0, 
            int numOptions = 0, bool noLogo = false, const char* appClassName = "grsisort") ;

   public:
      static TGint* fTGint;
      static TGint* instance(int argc = 0, char** argv = 0, void* options = 0, 
                             int numOptions = 0, bool noLogo = false, const char* appClassName = "grsisort");
      
      virtual ~TGint();

      void PrintLogo(bool);
      bool HandleTermInput();
      int  TabCompletionHook(char*,int*,ostream&);

   ClassDef(TGint,0);
};

#endif
