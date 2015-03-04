//g++ offsetfind.cxx `root-config --cflags --libs` -I${GRSISYS}/include -L${GRSISYS}/libraries -lMidasFormat -lXMLParser  -ooffsetfind


#include"TMidasFile.h"
#include"TMidasEvent.h"
#include"GFile.h"
#include"TFragment.h"
#include"TTree.h"
#include"TChannel.h"
#include"TH2D.h"
#include "TTreeIndex.h"
#include "TVirtualIndex.h"
#include "Globals.h"
#include "TF1.h"
#include "TMath.h"
#include "TString.h"

#include<TMidasFile.h>
#include<TMidasEvent.h>
#include<vector>

#include <iostream>


class TEventTime {
   public: 
      TEventTime(TMidasEvent* event){
         event->SetBankList();
  
         void *ptr;
         int banksize = event->LocateBank(NULL,"GRF1",&ptr);

         int type  = 0xffffffff;
         int value = 0xffffffff;

         int64_t time = 0;

         for(int x=0;x<banksize;x++) {
            value = *((int*)ptr+x);
            type  = value & 0xf0000000; 
         

            switch(type) {
               case 0x80000000:
                  dettype = value & 0x0000000f;
                  chanadd = (value &0x0003fff0)>> 4;
                  break;
               case 0xa0000000:
                  timelow = value & 0x0fffffff;
                  break;
               case 0xb0000000:
                  timehigh = value & 0x00003fff;
                  break;

            };
         }
         timemidas = (unsigned int)(event->GetTimeStamp());
         if(timemidas < low_timemidas)
            low_timemidas = timemidas;

         SetDigitizer();
      }

      ~TEventTime(){}

      int64_t GetTimeStamp(){
         long time = timehigh;
         time  = time << 28;
         time |= timelow & 0x0fffffff;
         return time;
   
      }  
      int TimeStampHigh(){
         return timehigh;
      }
   
      unsigned long MidasTime(){
         return timemidas;
      }

      int Digitizer(){
         return digitizernum;
      }

      int DetectorType(){
         return dettype;
      }

      void SetDigitizer(){
      //Maybe make a map somewhere of digitizer vs address
         switch(chanadd&0x0000ff00) {
            case 0x00000000: // if the first GRIF-16
               digitizernum =  0;
               break;
            case 0x00000100: // if the second GRIF-16
               digitizernum =  1;
               break;
            case 0x00001000: // if the third GRIF-16
               digitizernum =  2;
               break;
            case 0x00001100: // if the fourth GRIF-16
               digitizernum =  3;
               break;
            case 0x00001200: // if the fifth GRIF-16
               digitizernum =  4;
               break;
            default:
               return;
         };
            digmap.insert( std::pair<int,int>(digitizernum, digmap.size()));
      }
      
      inline static int NDigitizers(){
         return digmap.size();
      }
      
      static unsigned long GetLowestMidasTime(){
         return low_timemidas;
      }

      int DigIndex(){
         return digmap.find(digitizernum)->second;
      }

      static std::map<int,int> digmap;
      static unsigned long low_timemidas; 
 
   private:
      int timelow;
      int timehigh;
      unsigned long timemidas;
      int dettype;
      int chanadd;
      char digitizernum;

};


int QueueEvents(TMidasFile *infile, std::vector<TEventTime*> *eventQ){
   int events_read = 0;
   const int total_events = 1E6;
   TMidasEvent *event  = new TMidasEvent;
   eventQ->reserve(total_events);

   while(infile->Read(event)>0 && eventQ->size()<total_events) {
      switch(event->GetEventId()) {
         case 0x8000:
            printf(DRED "start of run\n");
            event->Print();
            printf(RESET_COLOR);
            break;
         case 0x8001:
            printf(DGREEN "end of run\n");
            event->Print();
            printf(RESET_COLOR);
            break;
         default:
            if(event->GetEventId() !=1 ) {
               break;
            }
            events_read++;
            eventQ->push_back(new TEventTime(event));//I'll keep 3G data in here for now in case we need to use it for time stamping 
            break;
       };
      if(events_read % 25000 == 0){
         printf(DYELLOW "\tQUEUEING EVENT %d/%d  \r" RESET_COLOR,events_read,total_events); fflush(stdout);
      }
   }
   printf("\n");
   delete event;
   return 0;
}

unsigned long TEventTime::low_timemidas = -1;
std::map<int,int> TEventTime::digmap;

void CheckHighTimeStamp(std::vector<TEventTime*> *eventQ, int64_t *correction){
//This function should return an array of corrections

   TList *midvshigh = new TList;
   printf(DBLUE "Correcting High time stamps...\n" RESET_COLOR);
   //These first four are for looking to see if high time-stamp reset
   std::map<int,int>::iterator mapit;
   for(mapit = TEventTime::digmap.begin(); mapit!=TEventTime::digmap.end();mapit++){
      TH2D *midvshighhist = new TH2D(Form("midvshigh%d",mapit->first),Form("midvshigh%d",mapit->first), 5000,0,5000,5000,0,5000); midvshigh->Add(midvshighhist);
   }
  
   unsigned int lowmidtime = TEventTime::GetLowestMidasTime();

   //MidasTimeStamp is the only time we can trust at this level. 

   int fEntries = eventQ->size();

   int FragsIn = 0;

   FragsIn++;
   int *lowest_hightime;
   lowest_hightime = new int[TEventTime::NDigitizers()];
   //Clear lowest hightime
   for(int i=0;i<TEventTime::NDigitizers();i++){
      lowest_hightime[i] = 0;
   }

   std::vector<TEventTime*>::iterator it;
   for(it = eventQ->begin(); it != eventQ->end(); it++) {
      //This makes the plot, might not be required
      int hightime = (*it)->TimeStampHigh();
      unsigned long midtime = (*it)->MidasTime() - lowmidtime;
      if(midtime>20) break;//20 seconds seems like plenty enough time
    
      if((*it)->DetectorType() == 1){
         ((TH2D*)(midvshigh->At((*it)->DigIndex())))->Fill(midtime, hightime);
         if(hightime < lowest_hightime[(*it)->DigIndex()])
            lowest_hightime[TEventTime::digmap.at((*it)->DigIndex())] = hightime;
      }
     }

   //find lowest digitizer
   int lowest_dig = 0;
   for(mapit = TEventTime::digmap.begin(); mapit != TEventTime::digmap.end(); mapit++){
      if(lowest_hightime[mapit->second] < lowest_hightime[lowest_dig])
         lowest_dig = mapit->second;
   }

   midvshigh->Print();  
   printf("The lowest digitizer is %d\n",lowest_dig);
   printf("*****  High time shifts *******\n");
   for(mapit = TEventTime::digmap.begin(); mapit != TEventTime::digmap.end(); mapit++){
      printf("%d:\t %d\n",mapit->first,lowest_hightime[mapit->second]);
      //Calculate the shift to 0 all digitizers
      correction[mapit->second] = ((lowest_hightime[mapit->second] - lowest_hightime[lowest_dig]) & 0x00003fff) << 28;
   }
   printf("********************\n");

   midvshigh->Write();
   midvshigh->Delete();
   delete[] lowest_hightime;

}

int main(int argc, char **argv) {

   if(argc!=2) {
      printf("Usage: ./offsetfind <input.mid>\n");
      return 1;
   }

   TMidasFile *infile  = new TMidasFile;
   infile->Open(argv[1]);

   GFile *outfile = new GFile("outfile.root","RECREATE");

   std::cout << "SIZE: " << TEventTime::digmap.size() << std::endl;
   std::vector<TEventTime*> *eventQ = new std::vector<TEventTime*>;
   QueueEvents(infile,eventQ);
   std::cout << "SIZE: " << TEventTime::digmap.size() << std::endl;

   int64_t *correction;
   correction = new int64_t[TEventTime::NDigitizers()];
   CheckHighTimeStamp(eventQ,correction);


   //Have to do deleting on Q if we move to a next step of fixing the MIDAS File
   infile->Close();
   outfile->Close();
   delete[] correction;
   delete infile;

}
//void WriteEventToFile(TMidasFile*,TMidasEvent*,Option_t);

/*
const size_t MEM_SIZE = (size_t)1024*(size_t)1024*(size_t)1024*(size_t)8; // 8 GB

void GetRoughTimeDiff(TTree *tree, int64_t *correction){
   //We want the MIDAS time stamps to still be the way we index these events, but we want to index on low time stamps next
   printf(DBLUE "Looking for rough time differences...\n" RESET_COLOR);
   printf(DYELLOW "Tree Index not found, building index on %s/%s..." RESET_COLOR,"MidasTimeStamp","TimeStampLow");  fflush(stdout);
   tree->BuildIndex("MidasTimeStamp","TimeStampLow");
   printf(" done!\n");

   TList *bestvsrough = new TList;
   TH1C *bestvs1rough = new TH1C("bestvs1rough","bestvs1rough",50E6,-25E6,25E6); bestvsrough->Add(bestvs1rough);
   TH1C *bestvs2rough = new TH1C("bestvs2rough","bestvs2rough",50E6,-25E6,25E6); bestvsrough->Add(bestvs2rough);
   TH1C *bestvs3rough = new TH1C("bestvs3rough","bestvs3rough",50E6,-25E6,25E6); bestvsrough->Add(bestvs3rough);
   TH1C *bestvs4rough = new TH1C("bestvs4rough","bestvs4rough",50E6,-25E6,25E6); bestvsrough->Add(bestvs4rough);

   TFragment *currentFrag = 0;
   TBranch *branch = tree->GetBranch("TFragment");
   branch->SetAddress(&currentFrag);
   tree->LoadBaskets(MEM_SIZE);

   int best_digitizer = 0;
   //find the best digitizer
   for(int i =0;i<4;i++){
      printf("Correction %d: %ld\n",i,correction[i]);
      if(correction[best_digitizer] > correction[i])
         best_digitizer = i;
   }
   printf("Assuming the best digitizer is: Digitizer %d\n",best_digitizer);

   TTreeIndex *index = (TTreeIndex*)tree->GetTreeIndex();
   Long64_t *indexvalues = index->GetIndex();
   int fEntries = index->GetN();
   time_t lowmidtime = tree->GetMinimum("MidasTimeStamp");

   int FragsIn = 0;
   int digitizer;
   bool keepfilling[4] = {1,1,1,1};

   tree->GetEntry(indexvalues[0]);
   FragsIn++;
   double low_hightime[4] = {0.0,0.0,0.0,0.0};

   TH1C* fillhist;

      for(long x=1;x<fEntries;x++) {
      if(tree->GetEntry(indexvalues[x]) == -1 ) { //move current frag to the next (x'th) entry in the tree
         printf( "FIRE!!!" "\n");
         continue;
      } 
      FragsIn++;
   
      //This makes the plot, might not be required
      TFragment myFrag  = *currentFrag;         //Set myfrag to be the x'th fragment before incrementing it.
      if(best_digitizer !=(int)((currentFrag->ChannelNumber-1)/16.0)) continue;
      int64_t besttime = currentFrag->GetTimeStamp() - correction[best_digitizer];  //Get the timestamp of the x'th fragment 
      time_t midtime = myFrag.MidasTimeStamp - lowmidtime;
      if(midtime > 3) break;

      for(long y=x+1;y<fEntries;y++) {
         //If the index of the comapred fragment equals the index of the first fragment, do nothing
         if(y == x) {
            continue;
         }
         if(tree->GetEntry(indexvalues[y]) == -1 ) { //move currentfrag to the next fragment
            printf( "FIRE!!!" "\n");
            continue;
         } 
         if(x%250==0){
            printf("\tOn fragment %i/%i                MidasTime:%d\r",x,fEntries,midtime);
         }
         if(currentFrag->MidasTimeStamp - lowmidtime - midtime > 0) break; //If they are 2 midas seconds away, move to the next event
     
         if(myFrag.DetectorType == 1 && currentFrag->DetectorType == 1) {
            int mydigitizer = (int)((currentFrag->ChannelNumber-1)/16.0);
            if(keepfilling[mydigitizer]){
               fillhist = (TH1C*)(bestvsrough->At(mydigitizer));
               Int_t bin = currentFrag->GetTimeStamp()-correction[mydigitizer]-besttime;
               if(fillhist->FindBin(bin) > 0 && fillhist->FindBin(bin) < fillhist->GetNbinsX()){
                  if(fillhist->GetBinContent(fillhist->Fill(bin))>126){
                     keepfilling[mydigitizer] = false;
                  }
                  
               }
            }

         }
      }
     }
   printf("\n\n");
   for(int i=0;i<4;i++){
      fillhist = (TH1C*)(bestvsrough->At(i));
      correction[i] += (int64_t)fillhist->GetBinCenter(fillhist->GetMaximumBin())  ;
      printf("Maximum bin in channel %d = %ld\n",i,correction[i]);
   }
   bestvsrough->Print();
   bestvsrough->Write();
   bestvsrough->Delete();

}

void GetTimeDiff(TTree *tree, int64_t *correction){
   const int range = 1000; //   +/- the amount of events to diff over

   printf(DBLUE "Looking for time differences...\n" RESET_COLOR);
   printf(DYELLOW "Tree Index not found, building index on %s/%s..." RESET_COLOR,"Master Filter Id","TimeStampLow");  fflush(stdout);//I'm not sure this even has to happen
   tree->BuildIndex("TriggerId","TimeStampLow");
   printf(" done!\n");
   TList* bestvs = new TList;

   TH1D *bestvs1 = new TH1D("bestvs1","bestvs1",1000,-500,500); bestvs->AddLast(bestvs1);
   TH1D *bestvs2 = new TH1D("bestvs2","bestvs2",1000,-500,500); bestvs->AddLast(bestvs2);
   TH1D *bestvs3 = new TH1D("bestvs3","bestvs3",1000,-500,500); bestvs->AddLast(bestvs3);
   TH1D *bestvs4 = new TH1D("bestvs4","bestvs4",1000,-500,500); bestvs->AddLast(bestvs4);

   TFragment *currentFrag = 0;
   TBranch *branch = tree->GetBranch("TFragment");
   branch->SetAddress(&currentFrag);
   tree->LoadBaskets(MEM_SIZE);

   int best_digitizer = 0;
   //find the best digitizer
   for(int i =0;i<4;i++){
      printf("Rough Low-time-stamp shift %d: %ld\n",i,correction[i]);
      if(correction[best_digitizer] > correction[i])
         best_digitizer = i;
   }
   printf("Assuming the best digitizer is: Digitizer %d\n",best_digitizer);

   TTreeIndex *index = (TTreeIndex*)tree->GetTreeIndex();
   Long64_t *indexvalues = index->GetIndex();
   int fEntries = index->GetN();

   int FragsIn = 0;
   int digitizer;
   bool keepfilling[4] = {1,1,1,1};
   int histsfull = 0;

   tree->GetEntry(indexvalues[0]);
   FragsIn++;
   double low_hightime[4] = {0.0,0.0,0.0,0.0};

   TH1C* fillhist;

   for(long x=1;x<fEntries;x++) {
      if(tree->GetEntry(indexvalues[x]) == -1 ) { //move current frag to the next (x'th) entry in the tree
         printf( "FIRE!!!" "\n");
         continue;
      } 
      FragsIn++;
   
      //This makes the plot, might not be required
      TFragment myFrag  = *currentFrag;         //Set myfrag to be the x'th fragment before incrementing it.
      if(best_digitizer !=(int)((currentFrag->ChannelNumber-1)/16.0)) continue;
      int64_t besttime = currentFrag->GetTimeStamp() - correction[best_digitizer];  //Get the timestamp of the x'th fragment 
      int myId = myFrag.TriggerId;
      if(histsfull > 3) break;
      for(long y=x-1000;y<fEntries;y++) {
         if(y<0) y =0;
         //If the index of the comapred fragment equals the index of the first fragment, do nothing
         if(y == x) {
            continue;
         }
         if(tree->GetEntry(indexvalues[y]) == -1 ) { //move currentfrag to the next fragment
            printf( "FIRE!!!" "\n");
            continue;
         } 
         if(x%1000==0){
            printf("\tOn fragment %i/%i\r",x,fEntries);
         }
         int currentId = currentFrag->TriggerId;
         if(TMath::Abs(currentId - myId > 1000)) break;

         if(myFrag.DetectorType == 1 && currentFrag->DetectorType == 1) {
            int mydigitizer = (int)((currentFrag->ChannelNumber-1)/16.0);
            if(keepfilling[mydigitizer]){
               fillhist = (TH1C*)(bestvs->At(mydigitizer));
               Int_t bin = currentFrag->GetTimeStamp()-correction[mydigitizer]-besttime;
               if(fillhist->FindBin(bin) > 0 && fillhist->FindBin(bin) < fillhist->GetNbinsX()){
                  if(fillhist->GetBinContent(fillhist->Fill(bin))>150){//This number can be changed to vary the amount of statistics in the time diff peak
                     keepfilling[mydigitizer] = false;
                     histsfull++;
                     printf( DRED "\nDigitizer %d completed\n" RESET_COLOR,mydigitizer);
                  }
                  
               }
            }
            else{
               if(histsfull > 3)
                  break;
            }
         }
      }
     }
   printf("\n\n");
   for(int i=0;i<4;i++){
      fillhist = (TH1C*)(bestvs->At(i));
      TF1* gausfit = new TF1(Form("Fit%d",i+1),"[0]*TMath::Exp(-(x-[1])*(x-[1])/2./([2]*[2])) + [3]",-100,100);
      gausfit->SetParNames("Constant","Mean","Sigma","Flat");
      gausfit->SetParameters(500,  0.0, 10.0, 10);
      gausfit->SetParLimits(1,-50,50);
      gausfit->SetParLimits(2,0.0,30.0);
      gausfit->SetParLimits(3,0,10000000);
      gausfit->SetNpx(1000);
      fillhist->Fit(gausfit,"MLR+");
      if(i!=best_digitizer){
         correction[i] += (int64_t)(gausfit->GetParameter(1));
      }
      else{
         correction[i] = (int64_t)(gausfit->GetParameter(1));//This isn't really a correction.
         printf("Channel %d is the best\n",i);
      }
   }
   int64_t best_time_diff = correction[best_digitizer];
   for(int i=0;i<4;i++){
      correction[i] -= best_time_diff;
      printf("Correction for channel %d = %ld *10ns\n",i,correction[i]);
        
   }
   bestvs->Print();
   bestvs->Write();
   bestvs->Delete();
}


int main(int argc, char **argv) {

   const Int_t ENTRIES = 100000;

   if(argc!=2) {
      printf("Usage: ./offsetfind <input.mid>\n");
      return 1;
   }

   //Open fragment tree
   GFile *f = new GFile(argv[1],"READ");
   TTree *tree = (TTree*)f->Get("FragmentTree");
 
   GFile *outfile = new GFile("outfile.root","RECREATE");
   //We need to read the cal in for digitizer information.
   //Can probably get this information from the addresses, but this is the easiest for now
   TChannel::ReadCalFromTree(tree);

   int64_t correction[4] = {0,0,0,0};
   int64_t dig_number[4] = {0,1,2,3};

   //This does checking of the high time stamps.
   //CheckHighTimeStamp(tree,correction);
   //GetRoughTimeDiff(tree,correction);
   GetTimeDiff(tree,correction);

   outfile->Close();
   f->Close();
   return 0;
}

*/
