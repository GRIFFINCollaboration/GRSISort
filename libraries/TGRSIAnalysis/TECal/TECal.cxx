#include "TECal.h"

ClassImp(TECal)

TECal::TECal(){
}


TECal::TECal(const char * filename){
   if(!(effFile->IsOpen())){
      printf("Opening file: %s\n",filename);
      TFile *effFile = new TFile(filename,"UPDATE");
   }
}

TECal::~TECal(){
   effFile->Close();
   delete effFile;

}

void TECal::AddEnergyGraph(Int_t channum,TGraphErrors *graph){
   
}

void TECal::AddEnergyGraph(TGraphErrors *graph, Int_t channum){
   //This function exists because who can remember the order of these things?
   AddEnergyGraph(channum,graph);
}

void TECal::CalibrateEnergy(){


}


void TECal::CalibrateEfficiency(){


}

Bool_t TECal::FitEnergyCal(){


   return true;
}

