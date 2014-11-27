
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

int clone_test(){
   
   TFile f("tcl.root","recreate");
   TTree T("T","test tcl");
 //  TClonesArray *arr = new TClonesArray("TSceptarHit");
   //TClonesArray &ar = *arr;
   //T.Branch("TSceptarHit",&arr,256000,99);
            std::cout << "Made it here?" << std::endl;
   TSceptar *sc = new TSceptar;
   T.Branch("TSceptar",&sc,256000,99);

   for (Int_t ev=0;ev<10;ev++) {
      //arr->Clear("C");
      sc->Clear();
            std::cout << "Made it here1?" << std::endl;
      Int_t nlines = Int_t(gRandom->Gaus(50,10));
      printf("There were %d hits in event %d\n",nlines,ev);
      if(nlines < 0) nlines = 1;
         for (Int_t i=0;i<nlines;i++) {
        /*    TSceptarHit *schit = (TSceptarHit*)(arr->ConstructedAt(arr->GetEntries()));   
            schit->SetDetectorNumber(nlines);
            schit->SetAddress(1);
            schit->SetCharge(gRandom->Rndm()*100);
            schit->SetTime(gRandom->Rndm()*10.0);
            
         */ 
            std::cout << sc->sceptar_hits.GetEntries() << std::endl;
            TSceptarHit *hit = (TSceptarHit*)((sc->sceptar_hits.ConstructedAt(sc->sceptar_hits.GetEntries())));
            //sc->sceptar_hits.ConstructedAt(sceptar_hits->GetEntries());
            std::cout << hit << std::endl;
            hit->SetDetectorNumber(nlines);
            hit->SetAddress(1);
            hit->SetCharge(gRandom->Rndm()*100);
            hit->SetTime(gRandom->Rndm()*10.0);


         }
         T.Fill();
   }
   T.Print();
   T.Write();
   return 1;
}


