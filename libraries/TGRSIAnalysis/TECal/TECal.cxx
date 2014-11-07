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
   graph->SetName(Form("%s",name.c_str()));
   graph->Write(graph->GetName(),kOverwrite);
}

void TECal::AddEnergyGraph(Int_t channum,const char * nucname,TGraphErrors *graph,const char* directory){
   effFile->cd("energy");
   AddGraph(channum,nucname,graph,directory);
   effFile->cd();
}

void TECal::AddEfficiencyGraph(Int_t channum, const char * nucname, TGraphErrors *graph,const char* directory){
   effFile->cd("efficiency");
   AddGraph(channum,nucname,graph,directory);
   effFile->cd();
}

void TECal::AutoFitSource(){

}


//This does not make me happy. I will make a directory loop file and return keys maybe?
void TECal::ColorGraphsBySource(Bool_t colflag, TDirectory *source){
   //Might want to have a descend into directories function, that takes function pointers?
   TDirectory *savdir = gDirectory;
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
         //Do stuff here
      }
      else {
         //Do nothing stuff
      }


   }
   
   
   
   
   /*
   
   TDirectory *dir;
   while ((key = (TKey*)next())) {
      if (key->IsFolder()) {
         key->Print();
         dir = (TDirectory*)key->ReadObj();
         dir->cd(dir->GetName());
         dir->Print();
       //  ColorGraphsBySource(colflag);
      }
      else {
         TClass *cl = gROOT->GetClass(key->GetClassName());
         if (cl->InheritsFrom("TGraphErrors")){
            TGraphErrors *graph = (TGraphErrors*)key->ReadObj();
            std::cout << graph->GetName() << std::endl;
            if(colflag){
               if(!strcmp(graph->GetName(),"115Eu"))
                  graph->SetLineColor(kBlue);
               if(!strcmp(graph->GetName(),"133Ba"))
                  graph->SetLineColor(kRed);
               if(!strcmp(graph->GetName(),"56Co"))
                  graph->SetLineColor(kGreen);
               if(!strcmp(graph->GetName(),"66Ga"))
                  graph->SetLineColor(kMagenta);
               else
                  graph->SetLineColor(kBlack);
            }
            else
               graph->SetLineColor(kBlack);
         }
      }
   }*/

   //Might eventually have a ColorSource(color) type function
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
