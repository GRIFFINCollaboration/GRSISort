//g++ offsetfind.cxx `root-config --cflags --libs` -I${GRSISYS}/include -L${GRSISYS}/libraries -lMidasFormat -lXMLParser -lSpectrum  -ooffsetfind


#include"TMidasFile.h"
#include"TMidasEvent.h"
#include"GFile.h"
#include"TFragment.h"
#include"TTree.h"
#include"TSpectrum.h"
#include"TChannel.h"
#include"TH2D.h"
#include "TTreeIndex.h"
#include "TVirtualIndex.h"
#include "Globals.h"
#include "TF1.h"
#include "TMath.h"
#include "TString.h"
#include "TPolyMarker.h"
#include "TStopwatch.h"
#include "TSystem.h"

#include<TMidasFile.h>
#include<TMidasEvent.h>
#include<vector>
#include<TDataParser.h>

#include <iostream>
#include <fstream>

Bool_t SplitMezz = false;

class TEventTime {
   public: 
      TEventTime(TMidasEvent* event){
         event->SetBankList();
  
         void *ptr;
         int banksize = event->LocateBank(NULL,"GRF2",&ptr);
         int bank = 2;

         if(!banksize){
            banksize = event->LocateBank(NULL,"GRF1",&ptr);
            bank = 1;
         }
         uint32_t type  = 0xffffffff;
         uint32_t value = 0xffffffff;
    
         uint64_t time = 0;

         for(int x=0;x<banksize;x++) {
            value = *((int*)ptr+x);
            type  = value & 0xf0000000; 
         
            switch(type) {
               case 0x80000000:
                 switch(bank){
                    case 1: // header format from before May 2015 experiments
                       //Sets: 
                       //     The number of filters
                       //     The Data Type
                       //     Number of Pileups
                       //     Channel Address
                       //     Detector Type
                       if( (value&0xf0000000) != 0x80000000) {
                //          return false;
                       }
                       chanadd  =  (value &0x0003fff0)>> 4;
                       dettype    =  (value &0x0000000f);
                         
                       // if(frag-DetectorType==2)
                       //    frag->ChannelAddress += 0x8000;
                       break;
                    case 2:
                       //Sets: 
                       //     The number of filters
                       //     The Data Type
                       //     Number of Pileups
                       //     Channel Address
                       //     Detector Type
                       if( (value&0xf0000000) != 0x80000000) {
                  //        return false;
                       }
                       chanadd  =  (value &0x000ffff0)>> 4;
                       dettype    =  (value &0x0000000f);
                         
                       // if(frag-DetectorType==2)
                       //    frag->ChannelAddress += 0x8000;
                       break;
                    default:
                       printf("This bank not yet defined.\n");
                       break;
               }
               case 0xa0000000:
                  timelow = value & 0x0fffffff;
                  break;
               case 0xb0000000:
                  timehigh = value & 0x00003fff;
                  break;

            };
            
         }
         timemidas = event->GetTimeStamp();
         
         SetDigitizer();
  
         if(!(digset.find(Digitizer())->second)){
            digset.find(Digitizer())->second = true;
            if(GetTimeStamp() < lowest_time || lowest_time == -1){
  /*             if(Digitizer() == 0x0000 ||
                  Digitizer() == 0x0100 ||
                  Digitizer() == 0x0200 ||
                  Digitizer() == 0x1000 ||
                  Digitizer() == 0x1200 ||
                  Digitizer() == 0x1100 ||
                  Digitizer() == 0x1300){
     */             lowest_time = GetTimeStamp();
                  best_dig = Digitizer();
                  if(timemidas < low_timemidas)
                     low_timemidas = timemidas;
           //    }
            }
         }
      }


      ~TEventTime(){}

      uint64_t GetTimeStamp(){
         uint64_t time = timehigh;
         time  = time << 28;
         time |= timelow & 0x0fffffff;
         return time;
   
      }  
      unsigned int TimeStampHigh(){
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
         digitizernum = chanadd&0x0000ff00;
         if(dettype > 1 && ((chanadd&0xF) > 1) && ((chanadd&0xF00)>1) && SplitMezz) {
            digitizernum+=2;
         }

         digmap.insert( std::pair<int,int>(digitizernum, digmap.size()));
         digset.insert( std::pair<int,bool>(digitizernum,false));
         correctionmap.insert(std::pair<int,int64_t>(digitizernum,0));
      }

      static void OrderDigitizerMap(){
         std::map<int,int>::iterator it;
         int index = 0;
         for(it = digmap.begin(); it != digmap.end();it++){
            it->second = index++;
         }
      }
      
      inline static int NDigitizers(){
         return digmap.size();
      }

      inline static int GetBestDigitizer(){
        // return 0x1000;
         return best_dig;
      }
      
      static unsigned long GetLowestMidasTime(){
         return low_timemidas;
      }

      int DigIndex(){
         return digmap.find(digitizernum)->second;
      }

      inline static uint64_t GetLowestTime(){
         return lowest_time;
      }

      static std::map<int,int> digmap;
      static std::map<int,bool> digset;
      static std::map<int,int64_t> correctionmap;
      static unsigned long low_timemidas;
      static int best_dig;
      static uint64_t lowest_time;
 
   private:
      unsigned int timelow;
      unsigned int timehigh;
      unsigned long timemidas;
      int dettype;
      int chanadd;
      int digitizernum;

};


unsigned long TEventTime::low_timemidas = -1;
uint64_t TEventTime::lowest_time = -1;
int TEventTime::best_dig = 0;
std::map<int,int> TEventTime::digmap;
std::map<int,bool> TEventTime::digset;
std::map<int,int64_t> TEventTime::correctionmap;

void PrintError(TMidasEvent *event, int frags,bool verb){
   if(verb){
      printf(DRED "\n//**********************************************//" RESET_COLOR "\n");
      printf(DRED "\nBad things are happening. Failed on datum %i" RESET_COLOR "\n", (-1*frags));
	   if(event)  event->Print(Form("a%i",(-1*frags)-1));
      printf(DRED "\n//**********************************************//" RESET_COLOR "\n");
   }
}

int QueueEvents(TMidasFile *infile, std::vector<TEventTime*> *eventQ){
   int events_read = 0;
   const int total_events = 1E6;
   //const int event_start = 1E5;
   const int event_start = 1E5;
   TMidasEvent *event  = new TMidasEvent;
   eventQ->reserve(total_events);
   void *ptr;
   int banksize;

   //Do checks on the event
	unsigned int mserial=0; if(event) mserial = (unsigned int)(event->GetSerialNumber());
	unsigned int mtime=0;   if(event) mtime   = event->GetTimeStamp();

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
            event->SetBankList();
            

               banksize = event->LocateBank(NULL,"GRF2",&ptr);
            if(!banksize)
               banksize = event->LocateBank(NULL,"GRF1",&ptr);

            if(banksize>0) {
               int frags = TDataParser::GriffinDataToFragment((uint32_t*)(ptr),banksize,2,mserial,mtime);
               if(frags > -1){
                  events_read++;
                  if(events_read > event_start)
                     eventQ->push_back(new TEventTime(event));//I'll keep 4G data in here for now in case we need to use it for time stamping 
               }
               else{
                  PrintError(event,frags,0);
               }
            }
            break;
       };
      if(events_read % 250000 == 0){
         printf(DYELLOW "\tQUEUEING EVENT %d/%d  \r" RESET_COLOR,events_read,total_events); fflush(stdout);
      }
   }
   TEventTime::OrderDigitizerMap();
   printf("\n");
   delete event;
   return 0;
}

void CheckHighTimeStamp(std::vector<TEventTime*> *eventQ){
//This function should return an array of corrections

   TList *midvshigh = new TList;
   printf(DBLUE "Correcting High time stamps...\n" RESET_COLOR);
   //These first four are for looking to see if high time-stamp reset
   std::map<int,int>::iterator mapit; //This is an iterator over the digitizer map 
   for(mapit = TEventTime::digmap.begin(); mapit!=TEventTime::digmap.end();mapit++){
      TH2D *midvshighhist = new TH2D(Form("midvshigh_0x%04x",mapit->first),Form("midvshigh_0x%04x",mapit->first), 5000,0,5000,5000,0,5000); 
      midvshigh->Add(midvshighhist);
   }
  
   unsigned int lowmidtime = TEventTime::GetLowestMidasTime();

   //MidasTimeStamp is the only time we can trust at this level. 

   int fEntries = eventQ->size();

   int FragsIn = 0;

   FragsIn++;
   //Clear lowest hightime
   std::map<int,int> lowest_hightime;
   std::vector<TEventTime*>::iterator it;
   
   for(it = eventQ->begin(); it != eventQ->end(); it++) {
      //This makes the plot, might not be required
      int hightime = (*it)->TimeStampHigh();
      unsigned long midtime = (*it)->MidasTime() - lowmidtime;
      if(midtime>20) break;//20 seconds seems like plenty enough time
      if( ((*it)->Digitizer() == 0) && ((*it)->DetectorType()>1)) continue; 
      //The next few lines are probably unnecessary
      ((TH2D*)(midvshigh->FindObject(Form("midvshigh_0x%04x",(*it)->Digitizer()))))->Fill(midtime, hightime);
      if(lowest_hightime.find((*it)->Digitizer()) == lowest_hightime.end()){
         lowest_hightime[(*it)->Digitizer()] = hightime; //initialize this as the first time that is seen.
      }
      else if(hightime < lowest_hightime.find((*it)->Digitizer())->second)
         lowest_hightime.find((*it)->Digitizer())->second = hightime;
   }

//find lowest digitizer 
   int lowest_dig = 0;
   int lowtime = 999999;
 /*  for(mapit = lowest_hightime.begin(); mapit != lowest_hightime.end(); mapit++){
      if(mapit->second < lowtime){
         lowest_dig = mapit->first;
         lowtime = mapit->second;
      }
   }*/

   lowest_dig = TEventTime::GetBestDigitizer();
   lowtime = lowest_hightime.find(lowest_dig)->second;

   midvshigh->Print();  
   printf("The lowest digitizer is 0x%04x\n",lowest_dig);
   printf("*****  High time shifts *******\n");
   for(mapit = lowest_hightime.begin(); mapit != lowest_hightime.end(); mapit++){
      if(mapit->first == lowest_dig){
         continue;
      }
      printf("0x%04x:\t %d \t %lf sec\n",mapit->first,mapit->second -lowtime, static_cast<double>((static_cast<int64_t>(mapit->second-lowtime))*(1<<28))/1.0E8);
      //Calculate the shift to the first event in all digitizers
      TEventTime::correctionmap.find(mapit->first)->second = ((static_cast<uint64_t>(mapit->second-lowtime))*(1<<28));
   }
   printf("********************\n");

   midvshigh->Write();
   midvshigh->Delete();

}


void GetRoughTimeDiff(std::vector<TEventTime*> *eventQ){
   //We want the MIDAS time stamps to still be the way we index these events, but we want to index on low time stamps next
   printf(DBLUE "Looking for rough time differences...\n" RESET_COLOR);

   TList *roughlist = new TList;
   //These first four are for looking to see if high time-stamp reset

   std::map<int,bool> keep_filling;
   std::map<int,int>::iterator mapit;
   for(mapit = TEventTime::digmap.begin(); mapit!=TEventTime::digmap.end();mapit++){
     // TH1F *roughhist = new TH1F(Form("rough_0x%04x",mapit->first),Form("rough_0x%04x",mapit->first), 6E7,-3E8,3E8); 
      TH1D *roughhist = new TH1D(Form("rough_0x%04x",mapit->first),Form("rough_0x%04x",mapit->first), 3E7,-3E8,3E8); 
      roughhist->SetTitle(Form("rough_0x%04x against 0x%04x",mapit->first,TEventTime::GetBestDigitizer()));
      roughlist->Add(roughhist);
      keep_filling[mapit->first] = true;
   }
   
   //The "best digitizer" is set when we fill the event Q
   printf(DYELLOW "Using the best digitizer 0x%04x\n" RESET_COLOR, TEventTime::GetBestDigitizer());

   TH1D* fillhist; //This pointer is useful later to clean up a lot of messiness

   std::vector<TEventTime*>::iterator hit1;
   std::vector<TEventTime*>::iterator hit2;
   const int range = 5000;
   int event1count = 0;
 //  for(hit1 = (eventQ->begin()+8000); hit1 != eventQ->end(); hit1++) { //This steps hit1 through the eventQ
   for(hit1 = (eventQ->begin()); hit1 != eventQ->end(); hit1++) { //This steps hit1 through the eventQ
      //We want to have the first hit be in the "good digitizer"
      if(event1count%75000 == 0)
         printf("Processing Event %d /%lu      \r",event1count,eventQ->size()); fflush(stdout);
      event1count++;

      if( ((*hit1)->Digitizer() == 0) && ((*hit1)->DetectorType()>1)) continue; 

      if((*hit1)->Digitizer() != TEventTime::GetBestDigitizer())  continue;

      int64_t time1 = static_cast<int64_t>((*hit1)->GetTimeStamp());
  
      if(event1count > range){
         hit2 = hit1 - range;
      }
      else{
         hit2 = eventQ->begin();
      }
      //Now that we have the best digitizer, we can start looping through the events 
      int event2count = 0;
      while((hit2 != eventQ->end()) && (event2count < 2*range)){

         if( ((*hit2)->Digitizer() == 0) && ((*hit2)->DetectorType()>1)){
            ++hit2;
            continue; 
         }
         if(hit1 == hit2){
            ++hit2;
            continue;
         }
         event2count++;
         int digitizer = (*hit2)->Digitizer();
         if(keep_filling[digitizer]){
            fillhist = (TH1D*)(roughlist->At((*hit2)->DigIndex())); //This is where that pointer comes in handy
            int64_t time2 = static_cast<int64_t>((*hit2)->GetTimeStamp()) - TEventTime::correctionmap.find((*hit2)->Digitizer())->second;
            Int_t bin = static_cast<Int_t>(time2 - time1);
               
            if((fillhist->FindBin(bin) > 0) && (fillhist->FindBin(bin) < fillhist->GetNbinsX())){
               fillhist->Fill(bin);
            //   if(fillhist->GetBinContent(fillhist->Fill(bin))>126){
              //    keep_filling[digitizer] = false;
              //    printf("\nDigitizer 0x%04x is done filling\n",digitizer);
              // }
            }
            
         }
         ++hit2;
     }
   }

   for(mapit = TEventTime::digmap.begin(); mapit != TEventTime::digmap.end(); mapit++){
      if(mapit->first == TEventTime::GetBestDigitizer())
         continue;
      fillhist = (TH1D*)(roughlist->At(mapit->second));
      std::cout << static_cast<int64_t>(fillhist->GetBinCenter(fillhist->GetMaximumBin())) << std::endl;
      TEventTime::correctionmap.find(mapit->first)->second +=  static_cast<int64_t>(fillhist->GetBinCenter(fillhist->GetMaximumBin()));
   }

   printf("*****  Rough time shifts *******\n");
   std::map<int,int64_t>::iterator cit;
   for(cit = TEventTime::correctionmap.begin(); cit != TEventTime::correctionmap.end(); cit++){
      if(cit->first == TEventTime::GetBestDigitizer())
         printf("0x%04x:\t BEST\n",cit->first);
      else
         printf("0x%04x:\t %lld\n",cit->first,cit->second);
   }
   printf("********************\n");

   roughlist->Print();
   roughlist->Write();
   roughlist->Delete();

}

void GetTimeDiff(std::vector<TEventTime*> *eventQ){
   //We want the MIDAS time stamps to still be the way we index these events, but we want to index on low time stamps next
   printf(DBLUE "Looking for final time differences...\n" RESET_COLOR);

   TList *list = new TList;
   //These first four are for looking to see if high time-stamp reset

   std::map<int,int>::iterator mapit;
   for(mapit = TEventTime::digmap.begin(); mapit!=TEventTime::digmap.end();mapit++){
      TH1D *hist = new TH1D(Form("timediff_0x%04x",mapit->first),Form("timediff_0x%04x",mapit->first), 1000,-500,500); 
      hist->SetTitle(Form("timediff_0x%04x Against 0x%04x",mapit->first,TEventTime::GetBestDigitizer()));
      list->Add(hist);
   }
   
   //The "best digitizer" is set when we fill the event Q
   printf(DYELLOW "Using the best digitizer 0x%04x\n" RESET_COLOR, TEventTime::GetBestDigitizer());

   TH1D* fillhist; //This pointer is useful later to clean up a lot of messiness

   std::vector<TEventTime*>::iterator hit1;
   std::vector<TEventTime*>::iterator hit2;
   int event1count = 0;
   const int range = 2000;
   for(hit1 = eventQ->begin(); hit1 != eventQ->end(); hit1++) { //This steps hit1 through the eventQ
      //We want to have the first hit be in the "good digitizer"
      if(event1count%75000 == 0)
         printf("Processing Event %d / %lu       \r",event1count,eventQ->size()); fflush(stdout);
      
      event1count++;
      //We need to make sure that that if we have a digitizer of 0, we have a detector type of 1
      if( (*hit1)->Digitizer() == 0 && (*hit1)->DetectorType()>1) continue; 
         
      if((*hit1)->Digitizer() != TEventTime::GetBestDigitizer()) continue;

      int64_t time1 = static_cast<int64_t>((*hit1)->GetTimeStamp());
  
      if(event1count > range){
         hit2 = hit1 - range;
      }
      else{
         hit2 = eventQ->begin();
      }
      //Now that we have the best digitizer, we can start looping through the events 
      int event2count = 0;
      while((hit2 != eventQ->end()) && (event2count < range*2)){
         event2count++;
         //We need to make sure that that if we have a digitizer of 0, we have a detector type of 1
         if( ((*hit2)->Digitizer() == 0) && ((*hit2)->DetectorType()>1)){
            hit2++;
            continue; 
         }
 
         if(hit1 != hit2 ){
            int digitizer = (*hit2)->Digitizer();
            fillhist = (TH1D*)(list->At((*hit2)->DigIndex())); //This is where that pointer comes in handy
            int64_t time2 = static_cast<int64_t>((*hit2)->GetTimeStamp()) - TEventTime::correctionmap.find((*hit2)->Digitizer())->second;
            if((time2-time1 < 2147483647) && (time2-time1 > -2147483647)){//Make sure we are casting this to 32 bit properly
               Int_t bin = static_cast<Int_t>(time2 - time1);
               fillhist->Fill(bin);
            }
         }
         hit2++;
      }
   }
   for(mapit = TEventTime::digmap.begin(); mapit!=TEventTime::digmap.end();mapit++){
      if(mapit->first == TEventTime::GetBestDigitizer())
         continue;
      fillhist = (TH1D*)(list->At(mapit->second));
      TSpectrum* spec = new TSpectrum();
      spec->Search(fillhist);
      double peak = spec->GetPositionX()[0];
      std::cout << static_cast<int64_t>(floor(peak+0.5)) << std::endl;
      TEventTime::correctionmap.find(mapit->first)->second +=  static_cast<int64_t>(floor(peak+0.5));
 //     fillhist->Draw();

   }
      
   printf("*****  Rough time shifts *******\n");
   std::map<int,int64_t>::iterator cit;
   for(cit = TEventTime::correctionmap.begin(); cit != TEventTime::correctionmap.end(); cit++){
      if(cit->first == TEventTime::GetBestDigitizer())
         printf("0x%04x:\t BEST\n",cit->first);
      else
         printf("0x%04x:\t %lld\n",cit->first,cit->second);
        // printf("0x%04x:\t %lld\n",mapit->first,correction[mapit->second]);
   }
   printf("********************\n");

   list->Print();
   list->Write();
   list->Delete();

}

bool ProcessEvent(TMidasEvent *event,TMidasFile *outfile) {
   if(event->GetEventId() !=1 ) {
      outfile->FillBuffer(event);
      return false;
   }
   event->SetBankList();
   int size;
   int data[1024];
  
   void *ptr;
   
   int banksize = event->LocateBank(NULL,"GRF2",&ptr);
   int bank = 2;
   if(!banksize){
      banksize = event->LocateBank(NULL,"GRF1",&ptr);
      bank =1;
   }
   uint32_t type  = 0xffffffff;
   uint32_t value = 0xffffffff;

   int dettype = 0;
   int chanadd = 0;

   unsigned int timelow  = 0;
   unsigned int timehigh = 0;

   uint64_t time = 0;

   for(int x=0;x<banksize;x++) {
      value = *((int*)ptr+x);
      type  = value & 0xf0000000; 

      switch(type) {
          case 0x80000000:
            switch(bank){
                case 1: // header format from before May 2015 experiments
                   //Sets: 
                    //     The number of filters
                  //     The Data Type
                  //     Number of Pileups
                 //     Channel Address
                 //     Detector Type
                  if( (value&0xf0000000) != 0x80000000) {
                     return false;
                   }
                  chanadd  =  (value &0x0003fff0)>> 4;
                  dettype    =  (value &0x0000000f);
                       
                   // if(frag-DetectorType==2)
                  //    frag->ChannelAddress += 0x8000;
                  break;
               case 2:
                  //Sets: 
                  //     The number of filters
                  //     The Data Type
                  //     Number of Pileups
                  //     Channel Address
                  //     Detector Type
                  if( (value&0xf0000000) != 0x80000000) {
                     return false;
                  }
                  chanadd  =  (value &0x000ffff0)>> 4;
                  dettype    =  (value &0x0000000f);
                       
                  // if(frag-DetectorType==2)
                  //    frag->ChannelAddress += 0x8000;
                  break;
               default:
                  printf("This bank not yet defined.\n");
                  break;
             };
             case 0xa0000000:
                timelow = value & 0x0fffffff;
                break;
             case 0xb0000000:
                timehigh = value & 0x00003fff;
                break;

           };
   }

   time = timehigh;
   time = time << 28;
   time |= timelow &0x0fffffff;

//   if((chanadd&0x0000ff00) != TEventTime::GetBestDigitizer()){
 //  if((dettype<2) || ((chanadd&0xf) < 2) ){   
   if(dettype > 1 && ((chanadd&0xF) > 1) && ((chanadd&0xF00)>1) && SplitMezz) {
      time -= TEventTime::correctionmap.find(chanadd&0x0000ff00)->second;    
   }
   else{
      time -= TEventTime::correctionmap.find((chanadd&0x0000ff00)+2)->second;    
    }

//  }

   if(time<0)
      time += 0x3ffffffffff;
   else if(time>0x3ffffffffff)
      time -= 0x3ffffffffff;

   // moving these inside the next switch, to account for doubly printed words.
   // (hey, it happens.)
   // -JKS, 14 January 2015
   //timelow = time&0x0fffffff;
   //timehigh = (time&0x3fff0000000) >> 28;

//   printf(DRED);
//   event->Print("a");
//   printf(RESET_COLOR);

   TMidasEvent copyevent = *event;
   copyevent.SetBankList();

   banksize = copyevent.LocateBank(NULL,"GRF2",&ptr);
   if(!banksize)
      banksize = copyevent.LocateBank(NULL,"GRF1",&ptr);

   for(int x=0;x<banksize;x++) {
      value = *((int*)ptr+x);
      type  = value & 0xf0000000; 

      switch(type) {
         case 0xa0000000:
            timelow = time&0x0fffffff;
            timelow += 0xa0000000;
            *((int*)ptr+x) = timelow;
            break;
         case 0xb0000000: {
            timehigh = (time&0x3fff0000000) >> 28;
            int tempdead = value & 0xffffc000;
            timehigh +=tempdead;
            *((int*)ptr+x) = timehigh;
            break;
         }
      };



       //printf( "0x%08x ",*((int*)ptr+x));
       //if(x!=0 && (x%7)==0)
       //   printf("\n");
   }
   //printf("===================\n");

   outfile->FillBuffer(&copyevent);

//   printf(DBLUE);
//   copyevent.Print("a");
//   printf(RESET_COLOR);
   return true;
}

void WriteEvents(TMidasFile* file) {

   std::ifstream in(file->GetFilename(), std::ifstream::in | std::ifstream::binary);
   in.seekg(0, std::ifstream::end);
   long long filesize = in.tellg();
   in.close();

   int bytes = 0;
   long long bytesread = 0;

   UInt_t num_evt = 0;

   TStopwatch w;
   w.Start();

   TMidasEvent* event = new TMidasEvent;

   while(true) {
      bytes = file->Read(event);
      if(bytes == 0){
         printf(DMAGENTA "\tfile: %s ended on %s" RESET_COLOR "\n",file->GetFilename(),file->GetLastError());
      if(file->GetLastErrno()==-1)  //try to read some more...
         continue;
      break;
      }
      bytesread += bytes;
                                          
      switch(event->GetEventId()) {
         case 0x8000:
            printf("start of run\n");
            file->FillBuffer(event);
            printf(DGREEN);
            event->Print();
            printf(RESET_COLOR);
            break;
         case 0x8001:
            printf("end of run\n");
            printf(DRED);
            event->Print();
            printf(RESET_COLOR);
            file->FillBuffer(event);
            break;
         default: 
            num_evt++;
            ProcessEvent(event,file);
            break;
       };
       if(num_evt %5000 == 0){
         gSystem->ProcessEvents();
         printf(HIDE_CURSOR " Events %d have processed %.2fMB/%.2f MB => %.1f MB/s              " SHOW_CURSOR "\r",num_evt,(bytesread/1000000.0),(filesize/1000000.0),(bytesread/1000000.0)/w.RealTime());
         w.Continue();
       }
   }
   printf("\n");
   
   delete event;
   file->WriteBuffer();

}

void WriteCorrectionFile(int runnumber){
//I think I can directly write the map, but I was having a bit of trouble, so I'm using this Tree hack
   char filename[64];
   sprintf(filename,"corrections%05i.root",runnumber); 
   GFile *corrfile = new GFile(filename,"RECREATE");

   //Just going to make a corrections map for now...it should be a map throughout....

   int address;
   Long64_t correction;
   TTree *t = new TTree("correctiontree","Tree with map");
   t->Branch("address",&address);
   t->Branch("correction",&correction);

   std::map<int,int64_t>::iterator it;
   for(it = TEventTime::correctionmap.begin();it != TEventTime::correctionmap.end();it++){
      address = it->first;
      correction = it->second;
      t->Fill();
   }
   corrfile->Write();
   
   corrfile->Close();
   delete corrfile;
}

int CorrectionFile(int runnumber){
//I think I can directly write the map, but I was having a bit of trouble, so I'm using this Tree hack
   char filename[64];
   sprintf(filename,"corrections%05i.root",runnumber); 
   GFile *corrfile = new GFile(filename,"READ");
   if(!(corrfile->IsOpen())){
      delete corrfile;
      return 0;
   }
   
   printf(DGREEN "Found Correction File %s\n" RESET_COLOR,filename);

   TTree* t; corrfile->GetObject("correctiontree",t);
   TBranch *baddress = 0;
   TBranch *bcorrection = 0;
   int address;
   Long64_t correction;
   t->SetBranchAddress("address",&address,&baddress);
   t->SetBranchAddress("correction",&correction,&bcorrection);

   int i =0;
   printf("Digitizer \t Correction\n");
   while(true){
      Long64_t tentry = t->LoadTree(++i);
      if(tentry<0) break;
      baddress->GetEntry(tentry);
      bcorrection->GetEntry(tentry);
      printf("0x%04x: \t\t %lld\n",address,correction);
      TEventTime::correctionmap.insert(std::pair<int,int64_t>(address,correction));
   }

   t->ResetBranchAddresses();
   printf(DGREEN "Found %lu digitizers\n" RESET_COLOR,TEventTime::correctionmap.size());
   corrfile->Close();
   delete corrfile;

   return TEventTime::correctionmap.size();

}

int main(int argc, char **argv) {

   if(argc<2) {
      printf("Usage: ./offsetfix <input.mid> <output.mid> <y/n>(read correction file)\n");
      return 1;
   }
   if(argv[1] == argv[2]){
      printf("ERROR: Cannot overwrite midas file %s\n",argv[1]);
   }

   TMidasFile *midfile  = new TMidasFile;
   midfile->Open(argv[1]);
   int runnumber = midfile->GetRunNumber();
   int subrunnumber = midfile->GetSubRunNumber();
   if(argc < 3){
      midfile->OutOpen(Form("fixrun%05d_%03d.mid",runnumber,subrunnumber));
   }
   else{
      midfile->OutOpen(argv[2]);
   }
   char filename[64];
   if(subrunnumber>-1)
      sprintf(filename,"time_diffs%05i_%03i.root",runnumber,subrunnumber); 
   else
      sprintf(filename,"time_diffs%05i.root",runnumber);
	printf("Creating root outfile: %s\n",filename);
   
   int nDigitizers = 0;
   if(argc == 4){
      if(strcmp(argv[3],"n")==0){
         nDigitizers = 0;
      }
      else{
         nDigitizers = CorrectionFile(runnumber);
      }
   }
   else{
      nDigitizers = CorrectionFile(runnumber);
   }

   if(!nDigitizers){
      GFile *outfile = new GFile(filename,"RECREATE");
      std::vector<TEventTime*> *eventQ = new std::vector<TEventTime*>;
      QueueEvents(midfile,eventQ);
      std::cout << "Number of Digitizers Found: " << TEventTime::digmap.size() << std::endl;

      CheckHighTimeStamp(eventQ);
      GetRoughTimeDiff(eventQ);
      GetTimeDiff(eventQ);
      WriteCorrectionFile(runnumber);
      midfile->Close();
      midfile->Open(argv[1]);//This seems like the easiest way to reset the file....
      outfile->Close();
      delete outfile;
   }

   WriteEvents(midfile);


   //Have to do deleting on Q if we move to a next step of fixing the MIDAS File
   midfile->Close();
   midfile->OutClose();
   delete midfile;

}


