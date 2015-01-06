#include "TGainMatch.h"

ClassImp(TGainMatch)

TGainMatch::TGainMatch(){}

TGainMatch::~TGainMatch(){}

Bool_t TGainMatch::CoarseMatch(TH1* hist, Int_t chanNum){
//This functions is used to perform a rough gain matching on a 60Co
//source. This makes gain matching over a wide range much easier to do afterwards
   fcoarse_match = true;
   if(!hist) return false;

   //I might want to do a clear of the gainmatching parameters at this point.

   //Check to see that the histogram isn't empty
   if(hist->GetEntries() < 1){
      printf("The histogram is empty\n");
      return false;
   }

   //See if the channel exists. There is no point in finding the gains if we don't have anywhere to write it
   TChannel *chan = TChannel::GetChannelByNumber(chanNum);
   if(!chan){
      printf("Channel Number %d does not exist in current memory.\n",chanNum);
      return false;
   }

   //Set the channel number
   SetChannelNumber(chanNum);

   std::vector<Double_t> engvec; //This is the vector of expected energies
   //We do coarse gain matching on 60Co. So I have hardcoded the energies for now
   engvec.push_back(1173.228);
   engvec.push_back(1332.492);
 
   //Use a TSpectrum to find the two largest peaks in the spectrum
   TSpectrum *s = new TSpectrum; //This might not have to be allocated
   Int_t nfound = s->Search(hist); 

   //If we didn't find two peaks, it is likely we gave it garbage
   if(nfound <2){
      std::cout << "Did not find enough peaks" << std::endl;
      delete s;
      return false;
   }

   //We want to store the centroids of the found peaks
   std::vector<Double_t> foundbin;
   for(int x=0;x<nfound;x++){
      foundbin.push_back((Double_t)(s->GetPositionX()[x]));
      printf("Found peak at bin %lf\n",foundbin[x]);
   }
   //Get Bin Width for use later. I am using the first peak found to set the bin width
   //If you are using antisymmetric binning... god help you.
   Double_t binWidth = hist->GetBinWidth(foundbin[0]);

   //Set the number of data points to 2. In the gain matching graph.
   this->Set(2);

   //We now want to create a peak for each one we found (2) and fit them.
   for(int x=0; x<nfound; x++){
      TPeak tmpPeak(foundbin[x],foundbin[x] - 20./binWidth, foundbin[x] + 20./binWidth);
      tmpPeak.SetName(Form("GM_Chan%u_%lf",GetChannelNumber(),foundbin[x]));//Change the name of the TPeak to know it's origin
      tmpPeak.Fit(hist,"EM+");
      this->SetPoint(x,engvec[x],tmpPeak.GetParameter("centroid"));
   }

   this->Fit("pol1","C0");

   delete s;
   return true;
}

void TGainMatch::Print(Option_t *opt) const {
   printf("GainMatching: ");
   if(fcoarse_match) 
      printf("COARSE\n");
   else              
      printf("FINE\n");
   TCal::Print();
}

void TGainMatch::Clear() {
   this->fcoarse_match = true;
   TCal::Clear();
}

   /*
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

*/
