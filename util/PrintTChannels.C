//g++ PrintTChannels.C -oPrintTChannels -I${GRSISYS}/include -L${GRSISYS}/libraries -lGRSIFormat `root-config --libs --cflags`


#include <cstdio>

#include <TFile.h>
#include <TTree.h>
#include <TList.h>


#include <TChannel.h>



void PrintTChannels(TTree *tree) {
   if(!tree)
      return;

   TList *list = tree->GetUserInfo();
   TIter iter(list);
   while(TObject *obj = iter.Next()) {
      if(!obj->InheritsFrom("TChannel"))
         continue;
      TChannel *chan = (TChannel*)obj;
      chan->Print();
   }
   return;
};


#ifndef __CINT__

int main(int argc, char **argv) {
   if(argc<2) {
      printf("try: PrintTChannel fragment#####.root instead.\n");
      return 1;
   } else if(argc>2) {
      printf("only one input file allowed.\n");
      printf("try: PrintTChannel fragment#####.root instead.\n");
      return 1;
   }
   TFile *infile = new TFile(argv[1]);
   TTree *tree = (TTree*)infile->Get("FragmentTree");
   if(!tree) {
      printf("FragmentTree not found in file %s.\n",argv[1]);
      return 1;
   }
   PrintTChannels(tree);

   return 0;
}


#endif


