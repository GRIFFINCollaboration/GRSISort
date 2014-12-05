
#include <cstdio>
#include <string>

#include "TEnv.h"
#include "TGRSIint.h"

void SetGRSIEnv() {
   std::string grsi_path = getenv("GRSISYS"); //Finds the GRSISYS path to be used by other parts of the grsisort code
   if(grsi_path.length()>0) {
      grsi_path += "/";
   }
   //Read in grsirc in the GRSISYS directory to set user defined options on grsisort startup
   grsi_path +=  ".grsirc"; 
   gEnv->ReadFile(grsi_path.c_str(),kEnvChange);
}


int main(int argc, char **argv) {
   //Find the grsisort environment variable so that we can read in .grsirc
   SetGRSIEnv();
   TGRSIint *input = 0;

   //Create an instance of the grsi interpreter so that we can run root-like interpretive mode
   input = TGRSIint::instance(argc,argv);
   //input->GetOptions(&argc,argv);
   //Run the code!
   input->Run("true");
   //Be polite when you leave.
   printf("\nbye,bye\n");

   return 0;
}

