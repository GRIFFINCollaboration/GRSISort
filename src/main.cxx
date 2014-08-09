
#include <cstdio>

#include "TGRSIint.h"

int main(int argc, char **argv) {

   TGRSIint *input = 0;

   input = TGRSIint::instance(argc,argv);
   //input->GetOptions(&argc,argv);
   input->Run("true");
   printf("\nbye,bye\n");

   return 0;
}

