#include "TECal.h"

ClassImp(TECal)

TECal::TECal(){
   this->OpenFile("GRSICal.root");
}


TECal::TECal(const char * filename){
   this->OpenFile(filename);
}

TECal::~TECal(){
   effFile->Close();
   delete effFile;

}

void TECal::OpenFile(const char * filename){
   if(!effFile){
      printf("Opening file: %s\n",filename);
      effFile = new TFile(filename,"UPDATE");
   }
   else{ 
      if(effFile->IsOpen()){
         printf("Closing file: %s\n",effFile->GetName());
         effFile->Close();
      }
      printf("Opening file: %s\n",filename);
      effFile->Open(filename,"UPDATE");
   }


      
}

void TECal::AddEnergyGraph(Int_t channum,const char * nucname,TGraphErrors *graph){
   std::string name = TNucleus::SortName(nucname);
 //  graph->SetTitle(nucname);
   graph->SetName(Form("ener_%d_%s",channum,name.c_str()));
   fenergyMap[channum][name] = graph;
   if(effFile->IsOpen()){
      effFile->cd(); 
      effFile->WriteObject(&fenergyMap,"fenergyMap"); 
      effFile->Write(); 
   }
}

void TECal::AddEfficiencyGraph(Int_t channum, const char * nucname, TGraphErrors *graph){
   std::string name = TNucleus::SortName(nucname);
   graph->SetName(Form("eff_%d_%s",channum,name.c_str()));
   fefficiencyMap[channum][name] = graph;
}

void TECal::AutoFitSource(){

}


void TECal::AddEnergyGraph(TGraphErrors *graph, Int_t channum,const char* nucname){
   //This function exists because who can remember the order of these things?
   AddEnergyGraph(channum,nucname,graph);
}

void TECal::CalibrateEnergy(){


}

void TECal::CalibrateEfficiency(){


}

Bool_t TECal::FitEnergyCal(){


   return true;
}

