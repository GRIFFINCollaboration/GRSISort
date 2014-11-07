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

   //Make these directories if they don't already exist
   if(!effFile->GetDirectory("energy"))
      effFile->mkdir("energy");
   if(!effFile->GetDirectory("efficiency"))
      effFile->mkdir("efficiency");
   effFile->cd();
      
}

void TECal::AddEnergyGraph(Int_t channum,const char * nucname,TGraphErrors *graph,const char* directory){
   //Make a directory for the Channel
   effFile->cd("energy");
   if(!strcmp(directory,"")){
      if(!gDirectory->GetDirectory(Form("Channel_%d",channum)))
         gDirectory->mkdir(Form("Channel_%d",channum));
      gDirectory->cd(Form("Channel_%d",channum));
   }
   else{
      if(!gDirectory->GetDirectory(Form("%s",directory)))
         gDirectory->mkdir(Form("%s",directory));
      gDirectory->cd(Form("%s",directory));
   }
   std::string name = TNucleus::SortName(nucname);
   graph->SetName(Form("ener_%s",name.c_str()));
   graph->Write(graph->GetName(),kOverwrite);
   effFile->cd();
}

void TECal::AddEfficiencyGraph(Int_t channum, const char * nucname, TGraphErrors *graph,const char* directory){
   //Make a directory for the Channel
   effFile->cd("efficiency");
   if(!strcmp(directory,"")){
      if(!gDirectory->GetDirectory(Form("Channel_%d",channum)))
         gDirectory->mkdir(Form("Channel_%d",channum));
      gDirectory->cd(Form("Channel_%d",channum));
   }
   else{
      if(!gDirectory->GetDirectory(Form("%s",directory)))
         gDirectory->mkdir(Form("%s",directory));
      gDirectory->cd(Form("%s",directory));
   }
   std::string name = TNucleus::SortName(nucname);
   graph->SetName(Form("effic_%s",name.c_str()));
   graph->Write(graph->GetName(),kOverwrite);
   effFile->cd();
}

void TECal::AutoFitSource(){

}


void TECal::AddEnergyGraph(TGraphErrors *graph, Int_t channum,const char* nucname,const char* directory){
   //This function exists because who can remember the order of these things?
   AddEnergyGraph(channum,nucname,graph,directory);
}

void TECal::CalibrateEnergy(){


}

void TECal::CalibrateEfficiency(){


}

Bool_t TECal::FitEnergyCal(){


   return true;
}
Bool_t TECal::Write(){
   if(effFile->IsOpen()){
      effFile->cd(); //I'm going to change this stuff and just write the TGRAPHERRORS directly with unique names
      //effFile->WriteObject(&fenergyMap,"fenergyMap"); 
     // effFile->WriteObject(&fefficiencyMap,"fefficiencyMap");
      effFile->Write(); 
   }
   return true;
}
