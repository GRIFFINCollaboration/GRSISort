//g++ clone_test.cxx -I$GRSISYS/include -L$GRSISYS/libraries -lGriffin -lGRSIFormat `root-config --cflags --libs` 

#include "TGriffin.h"
#include "TGriffinHit.h"

#include "TRandom.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

#include <iostream>
using namespace std;

int clone_test(){
   
   TFile f("tcl.root","recreate");
   TTree T("T","test tcl");
 //  TClonesArray *arr = new TClonesArray("TGriffinHit");
   //TClonesArray &ar = *arr;
   //T.Branch("TGriffinHit",&arr,256000,99);
  //          std::cout << "Made it here?" << std::endl;
   TGriffin *sc = new TGriffin;
   T.Branch("TGriffin",&sc,256000,99);

   for (Int_t ev=0;ev<100000;ev++) {
      //arr->Clear("C");
      sc->Clear();
    //        std::cout << "Made it here1?" << std::endl;
      Int_t nlines = Int_t(gRandom->Gaus(50,10));
//      printf("There were %d hits in event %d\n",nlines,ev);
      if(nlines < 0) nlines = 1;
         for (Int_t i=0;i<nlines;i++) {
        /*    TGriffinHit *schit = (TGriffinHit*)(arr->ConstructedAt(arr->GetEntries()));   
            schit->SetDetectorNumber(nlines);
            schit->SetAddress(1);
            schit->SetCharge(gRandom->Rndm()*100);
            schit->SetTime(gRandom->Rndm()*10.0);
            
         */ 
//            std::cout << sc->griffin_hits.GetEntries() << std::endl;
            TGriffinHit *hit = (TGriffinHit*)((sc->griffin_hits.ConstructedAt(sc->griffin_hits.GetEntries())));
            //sc->griffin_hits.ConstructedAt(griffin_hits->GetEntries());
//            std::cout << hit << std::endl;
            hit->SetDetectorNumber(nlines);
            hit->SetAddress(1);
            hit->SetChargeLow(gRandom->Rndm()*100);
            hit->SetTime(gRandom->Rndm()*10.0);


         }
         T.Fill();
   }
   T.Print();
   T.Write();
   return 1;
}

#ifndef __CINT__ 

int main (int argc,char**argv) {

   clone_test();
   return 0;
}
#endif


