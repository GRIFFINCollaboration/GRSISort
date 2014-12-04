

#include "TGint.h"

#include "Globals.h"

ClassImp(TGint)


TGint *TGint::fTGint = NULL;

TGint *TGint::instance(int argc,char** argv, void *options, int numOptions, bool noLogo, const char *appClassName) {
   if(!fTGint)
      fTGint = new TGint(argc,argv,options,numOptions,true,appClassName);
   return fTGint;
}

TGint::TGint(int argc, char **argv,void *options, Int_t numOptions, Bool_t noLogo,const char *appClassName)
      :TRint(appClassName, &argc, argv, options, numOptions,noLogo) {

      PrintLogo(false);
      SetPrompt( DYELLOW "GRSI [%d]" RESET_COLOR);

}

TGint::~TGint()   {
// SafeDelete();
}

bool TGint::HandleTermInput() {
   return TRint::HandleTermInput();
}

int TGint::TabCompletionHook(char* buf, int* pLoc, ostream& out) {
   return TRint::TabCompletionHook(buf,pLoc,out);
}

void TGint::PrintLogo(bool lite) {

   if(!lite)   {
     #if PLATFORM == Linux
      const std::string &ref = ProgramName();
     #else
      const std::strinf &ref = "Nuclear Data";
     #endif

     const unsigned int reflength = ref.length() - 78;
     const unsigned int width = reflength + (reflength % 2);



     printf("\t*%s*\n", std::string(width,'*').c_str());   
     printf("\t*%*s%*s*\n",width/2+5,"GRSI SPOON", width/2-5, "");
     printf("\t*%*s%*s*\n",width/2+reflength/2, ref.c_str(), width/2-reflength/2, "");
     printf("\t*%*s%*s*\n",width/2+5,"A lean-mean sorting machine", width/2-5, "");
     printf("\t*%*s%*s*\n",width/2+5, "version 2.1.0 beta", width/2-5, "");
     printf("\t*%s*\n", std::string(width,'*').c_str());   
   }
}
