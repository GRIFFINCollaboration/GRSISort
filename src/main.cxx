
#include <cstdio>
#include <string>

#include "TEnv.h"
#include "TGRSIint.h"

void SetGRSIEnv() {
   std::string grsi_path = getenv("GRSISYS");
   if(grsi_path.length()>0) {
      grsi_path += "/";
   }
   grsi_path +=  ".grsirc";
   gEnv->ReadFile(grsi_path.c_str(),kEnvChange);
}


int main(int argc, char **argv) {

   SetGRSIEnv();
   TGRSIint *input = 0;

   input = TGRSIint::instance(argc,argv);
   //input->GetOptions(&argc,argv);
   input->Run("true");
   printf("\nbye,bye\n");

   return 0;
}

