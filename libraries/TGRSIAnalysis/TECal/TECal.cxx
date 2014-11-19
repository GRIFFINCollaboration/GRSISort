#include "TECal.h"

ClassImp(TECal)

TECal::TECal(){
   this->OpenFile("GRSICal.root");
}


TECal::TECal(const char * filename) : effFile(0){
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


void TECal::AddGraph(Int_t channum,const char * nucname,TGraphErrors *graph,const char* directory){
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
   graph->SetName(Form("raw%s",name.c_str()));
   graph->Write(graph->GetName(),kOverwrite);
}

void TECal::AddEnergyGraph(Int_t channum,const char * nucname,TGraphErrors *graph,const char* directory){
   effFile->cd();
   effFile->cd("energy");
   AddGraph(channum,nucname,graph,directory);
   effFile->cd();
}

void TECal::AddEfficiencyGraph(Int_t channum, const char * nucname, TGraphErrors *graph,const char* directory){
   effFile->cd();
   effFile->cd("efficiency");
   AddGraph(channum,nucname,graph,directory);
   effFile->cd();
}

void TECal::AutoFitSource(){

}


//This does not make me happy. I will make a directory loop file and return keys maybe?
void TECal::ColorGraphsBySource(Bool_t colflag, TDirectory *source){
   //Might want to have a descend into directories function, that takes function pointers?
   Bool_t toplevel_flag = false;
   TDirectory *savdir = gDirectory;
   if(!source){
      effFile->cd("/");
      source = gDirectory;
      toplevel_flag = true;
   }
  source->ls();
   TIter next(gDirectory->GetListOfKeys());
   TKey *key;
   while((key = (TKey*)next())){
      const char *classname = key->GetClassName();
      TClass *cl = gROOT->GetClass(classname);
      if(!cl) continue;
      if(cl->InheritsFrom(TDirectory::Class())){
         source->cd(key->GetName());
         TDirectory *subdir = gDirectory;
         ColorGraphsBySource(colflag,subdir);
         //Might have to do something else ehre
      } 
      else if (cl->InheritsFrom(TGraphErrors::Class())) {
         TGraphErrors *graph = (TGraphErrors*)key->ReadObj();
         std::cout << graph->GetName() << std::endl;        
         if(colflag){
            if(!strcmp(graph->GetName(),"raw152Eu"))
            { std::cout << "coloring "<< graph->GetName() << " Blue" << std::endl;
               graph->SetLineColor(kBlue);
            }
            else if(!strcmp(graph->GetName(),"raw133Ba"))
            { std::cout << "coloring "<< graph->GetName() << " Red" << std::endl;
               graph->SetLineColor(kRed);
            }
            else if(!strcmp(graph->GetName(),"raw56Co")){
               std::cout << "coloring "<< graph->GetName() << " Green" << std::endl;
               graph->SetLineColor(kGreen);
            }
            else if(!strcmp(graph->GetName(),"raw66Ga")){
               std::cout << "coloring "<< graph->GetName() << " Magenta" << std::endl;
               graph->SetLineColor(kMagenta);
            }
            else{
               std::cout << "coloring "<< graph->GetName() << " Black" << std::endl;
               graph->SetLineColor(kBlack);
            }
         }
         else{
            std::cout << "coloring "<< graph->GetName() << " Black" << std::endl;
            graph->SetLineColor(kBlack);
         }
        // graph->Write("",kOverwrite);
        // graph->Write();
      }
      else {
         //Do nothing stuff
      }


   }
   if(toplevel_flag ==true){
      savdir->cd();
      effFile->Write();
   }
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
