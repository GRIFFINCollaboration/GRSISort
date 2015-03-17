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
#include "TPolyMarker.h"
#include "TStopwatch.h"
#include "TSystem.h"

#include<TMidasFile.h>
#include<TMidasEvent.h>
#include<vector>

#include <iostream>
#include <fstream>

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
                  dettype = value & 0x0000000F;
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
  
         if(GetTimeStamp() < lowest_time || lowest_time == -1){
            lowest_time = GetTimeStamp();
            best_dig = Digitizer();
         }
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
         digitizernum = chanadd&0x0000ff00;
         digmap.insert( std::pair<int,int>(digitizernum, digmap.size()));
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

      inline static unsigned int GetBestDigitizer(){
         return best_dig;
      }
      
      static unsigned long GetLowestMidasTime(){
         return low_timemidas;
      }

      int DigIndex(){
         return digmap.find(digitizernum)->second;
      }

      static std::map<int,int> digmap;
      static unsigned long low_timemidas;
      static unsigned int best_dig;
      static int64_t lowest_time;
 
   private:
      int timelow;
      int timehigh;
      unsigned long timemidas;
      int dettype;
      int chanadd;
      int digitizernum;

};


unsigned long TEventTime::low_timemidas = -1;
int64_t TEventTime::lowest_time = -1;
unsigned int TEventTime::best_dig = 0;
std::map<int,int> TEventTime::digmap;

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
      if(events_read % 250000 == 0){
         printf(DYELLOW "\tQUEUEING EVENT %d/%d  \r" RESET_COLOR,events_read,total_events); fflush(stdout);
      }
   }
   TEventTime::OrderDigitizerMap();
   printf("\n");
   delete event;
   return 0;
}

void CheckHighTimeStamp(std::vector<TEventTime*> *eventQ, int64_t *correction){
//This function should return an array of corrections

   TList *midvshigh = new TList;
   printf(DBLUE "Correcting High time stamps...\n" RESET_COLOR);
   //These first four are for looking to see if high time-stamp reset
   std::map<int,int>::iterator mapit;
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
    
      ((TH2D*)(midvshigh->At((*it)->DigIndex())))->Fill(midtime, hightime);
      if(lowest_hightime.find((*it)->Digitizer()) == lowest_hightime.end()){
         lowest_hightime[(*it)->Digitizer()] = hightime; //initialize this as the first time that is seen.
      }
     }

   //find lowest digitizer 
   int lowest_dig = 0;
   int lowtime = 999999;
   for(mapit = lowest_hightime.begin(); mapit != lowest_hightime.end(); mapit++){
      if(mapit->second < lowtime){
         lowest_dig = mapit->first;
         lowtime = mapit->second;
      }
   }

   midvshigh->Print();  
   printf("The lowest digitizer is 0x%04x\n",lowest_dig);
   printf("*****  High time shifts *******\n");
   for(mapit = lowest_hightime.begin(); mapit != lowest_hightime.end(); mapit++){
      printf("0x%04x:\t %d\n",mapit->first,mapit->second);
      //Calculate the shift to 0 all digitizers
      correction[TEventTime::digmap.find(mapit->first)->second] = ((int64_t)((mapit->second-lowtime))*(1<<28)) ;
   }
   printf("********************\n");

   midvshigh->Write();
   midvshigh->Delete();

}


void GetRoughTimeDiff(std::vector<TEventTime*> *eventQ, int64_t *correction){
   //We want the MIDAS time stamps to still be the way we index these events, but we want to index on low time stamps next
   printf(DBLUE "Looking for rough time differences...\n" RESET_COLOR);

   TList *roughlist = new TList;
   //These first four are for looking to see if high time-stamp reset

   std::map<int,bool> keep_filling;
   std::map<int,int>::iterator mapit;
   for(mapit = TEventTime::digmap.begin(); mapit!=TEventTime::digmap.end();mapit++){
      TH1C *roughhist = new TH1C(Form("rough_0x%04x",mapit->first),Form("rough_0x%04x",mapit->first), 6E8,-3E8,3E8); 
      roughhist->SetTitle(Form("rough_0x%04x against 0x%04x",mapit->first,TEventTime::GetBestDigitizer()));
      roughlist->Add(roughhist);
      keep_filling[mapit->first] = true;
   }
   
   //The "best digitizer" is set when we fill the event Q
   printf(DYELLOW "Using the best digitizer 0x%04x\n" RESET_COLOR, TEventTime::GetBestDigitizer());

   TH1C* fillhist; //This pointer is useful later to clean up a lot of messiness

   std::vector<TEventTime*>::iterator hit1;
   std::vector<TEventTime*>::iterator hit2;
   int event1count = 0;
   const int range = 250;
   for(hit1 = eventQ->begin(); hit1 != eventQ->end(); hit1++) { //This steps hit1 through the eventQ
      //We want to have the first hit be in the "good digitizer"
      if(event1count%250000 == 0)
         printf("Processing Event %d /%d      \r",event1count,eventQ->size()); fflush(stdout);
         event1count++;

      if((*hit1)->Digitizer() != TEventTime::GetBestDigitizer()) 
         continue;

      int64_t time1 = (*hit1)->GetTimeStamp() - correction[(*hit1)->DigIndex()];
  
      if(event1count > range){
         hit2 = hit1 - range;
      }
      else{
         hit2 = eventQ->begin();
      }
      //Now that we have the best digitizer, we can start looping through the events 
      int event2count = 0;
      while(hit2 != eventQ->end() && event2count < range*2){
         event2count++;
         if(hit1 == hit2) continue;
         int digitizer = (*hit2)->Digitizer();
         if(keep_filling[digitizer]){
            fillhist = (TH1C*)(roughlist->At((*hit2)->DigIndex())); //This is where that pointer comes in handy
            int64_t time2 = (*hit2)->GetTimeStamp() - correction[(*hit2)->DigIndex()];
            Int_t bin = static_cast<Int_t>(time2 - time1);
               
            if(fillhist->FindBin(bin) > 0 && fillhist->FindBin(bin) < fillhist->GetNbinsX()){
               if(fillhist->GetBinContent(fillhist->Fill(bin))>126){
                  keep_filling[digitizer] = false;
                  printf("\nDigitizer 0x%04x is done filling\n",digitizer);
               }
            }
            
            hit2++;
      }
     }
   }

   for(mapit = TEventTime::digmap.begin(); mapit != TEventTime::digmap.end(); mapit++){
      fillhist = (TH1C*)(roughlist->At(mapit->second));
      correction[mapit->second] +=  (int64_t)fillhist->GetBinCenter(fillhist->GetMaximumBin());
   }

   printf("*****  Rough time shifts *******\n");
   for(mapit = TEventTime::digmap.begin(); mapit != TEventTime::digmap.end(); mapit++){
      printf("0x%04x:\t %d\n",mapit->first,correction[mapit->second]);
   }
   printf("********************\n");

   roughlist->Print();
   roughlist->Write();
   roughlist->Delete();

}

void GetTimeDiff(std::vector<TEventTime*> *eventQ, int64_t *correction){
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
   const int range = 250;
   for(hit1 = eventQ->begin(); hit1 != eventQ->end(); hit1++) { //This steps hit1 through the eventQ
      //We want to have the first hit be in the "good digitizer"
      if(event1count%75000 == 0)
         printf("Processing Event %d / %d       \r",event1count,eventQ->size()); fflush(stdout);
      
      event1count++;
      //We need to make sure that that if we have a digitizer of 0, we have a detector type of 1
      if( (*hit1)->Digitizer() == 0 && (*hit1)->DetectorType()!=1) continue; 
         
      if((*hit1)->Digitizer() != TEventTime::GetBestDigitizer()) 
         continue;

      int64_t time1 = (*hit1)->GetTimeStamp() - correction[(*hit1)->DigIndex()];;
  
      if(event1count > range){
         hit2 = hit1 - range;
      }
      else{
         hit2 = eventQ->begin();
      }
      //Now that we have the best digitizer, we can start looping through the events 
      int event2count = 0;
      while(hit2 != eventQ->end() && event2count < range*2){
         event2count++;
         //We need to make sure that that if we have a digitizer of 0, we have a detector type of 1
         if( (*hit2)->Digitizer() == 0 && (*hit2)->DetectorType()!=1) continue; 
 
         if(hit1 != hit2 ){
            int digitizer = (*hit2)->Digitizer();
            fillhist = (TH1D*)(list->At((*hit2)->DigIndex())); //This is where that pointer comes in handy
            int64_t time2 = (*hit2)->GetTimeStamp() - correction[(*hit2)->DigIndex()];
            if(time2-time1 < 2147483647 && time2-time1 > -2147483647){//Make sure we are casting this to 32 bit properly
               Int_t bin = static_cast<Int_t>(time2 - time1);
               
               fillhist->Fill(bin);
            }
         }
         hit2++;
      }
   }
   for(mapit = TEventTime::digmap.begin(); mapit!=TEventTime::digmap.end();mapit++){
      fillhist = (TH1D*)(list->At(mapit->second));
      correction[mapit->second] +=  (int64_t)fillhist->GetBinCenter(fillhist->GetMaximumBin());
      TPolyMarker *pm = new TPolyMarker;
      pm->SetNextPoint(fillhist->GetBinCenter(fillhist->GetMaximumBin()),fillhist->GetBinContent(fillhist->GetMaximumBin())+10);
      pm->SetMarkerStyle(23);
      pm->SetMarkerColor(kRed);
      pm->SetMarkerSize(1.3);
      fillhist->GetListOfFunctions()->Add(pm);
 //     fillhist->Draw();

   }
      
   
   printf("*****  Final time shifts *******\n");
   for(mapit = TEventTime::digmap.begin(); mapit != TEventTime::digmap.end(); mapit++){
      printf("0x%04x:\t %d\n",mapit->first,correction[mapit->second]);
   }
   printf("********************\n");
        
   list->Print();
   list->Write();
   list->Delete();

}

void ProcessEvent(TMidasEvent *event,TMidasFile *outfile,int64_t* correction) {
   if(event->GetEventId() !=1 ) {
      outfile->Write(event,"q");
      return;
   }
   event->SetBankList();
   int size;
   int data[1024];
  
   void *ptr;
   int banksize = event->LocateBank(NULL,"GRF1",&ptr);

   int type  = 0xffffffff;
   int value = 0xffffffff;

   int dettype = 0;
   int chanadd = 0;

   int timelow  = 0;
   int timehigh = 0;

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

   //printf("chanadd = 0x%08x
   //event->Print("a");

   //printf("dettype  = 0x%08x\n",dettype);
   //printf("chanadd  = 0x%08x\n",chanadd);
                                    
   //printf("timelow  = 0x%08x\n",timelow);
   //printf("timehigh = 0x%08x\n",timehigh);



  /* if( (dettype==1) || (dettype ==5) ) { // 1 for GRIFFIN, 5 for PACES
     //do nothing.
   } else {
    outfile->Write(event,"q");
    return;
   }

   */
 /*
   if(((chanadd&0x0000ff00) == 0x00000000) ||
      ((chanadd&0x0000ff00) == 0x00001000) ||
      ((chanadd&0x0000ff00) == 0x00001100) ||
      ((chanadd&0x0000ff00) == 0x00001200)) {
      //do nothing.
    } else {
      outfile->Write(event,"q");
      return;
    }
*/
   time = timehigh;
   time = time << 28;
   time |= timelow &0x0fffffff;

   

   int dig_index = TEventTime::digmap.find(chanadd&0x0000ff00)->second; //This is where in the corrections list we find this digitzer
                                                                //I know, I know...This is the worst way to do this...
                                                                //This code morphed over time and it's hacked together in the
                                                                //worst way possible...get over it...I hate it too.... RD


   if((chanadd&0x0000ff00) != TEventTime::GetBestDigitizer()){
      time -= correction[dig_index];
   }

   //printf("time = 0x%016x\n",time);
   //std::cout << "time    = " << std::hex << time << std::endl;

   // Here's where we change the values of the time stamps!!!!
 /*  switch(chanadd&0x0000ff00) {
      case 0x00000000: // if the first GRIF-16
//         time -= 10919355323; // run 2369 correction
         time -= 87; // run 2394 correction
         break;
      case 0x00000100: // if the second GRIF-16
         break;
      case 0x00001000: // if the third GRIF-16
//         time -= 10919355323; // run 2369 correction
         break;
      case 0x00001100: // if the fourth GRIF-16
//         time -= 10919355239; // run 2369 correction
         break;
      case 0x00001200: // if the fifth GRIF-16
//         time += 7;
         time -= 87; // run 2394 correction
         break;
   };*/
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

   outfile->Write(&copyevent,"q");

//   printf(DBLUE);
//   copyevent.Print("a");
//   printf(RESET_COLOR);

}

void WriteEvents(TMidasFile* infile, TMidasFile* outfile,int64_t* correction) {

   std::ifstream in(infile->GetFilename(), std::ifstream::in | std::ifstream::binary);
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
      bytes = infile->Read(event);
      if(bytes == 0){
         printf(DMAGENTA "\tfile: %s ended on %s" RESET_COLOR "\n",infile->GetFilename(),infile->GetLastError());
      if(infile->GetLastErrno()==-1)  //try to read some more...
         continue;
      break;
      }
      bytesread += bytes;
                                          
      switch(event->GetEventId()) {
         case 0x8000:
            printf("start of run\n");
            outfile->Write(event,"q");
            printf(DGREEN);
            event->Print();
            printf(RESET_COLOR);
            break;
         case 0x8001:
            printf("end of run\n");
            printf(DRED);
            event->Print();
            printf(RESET_COLOR);
            outfile->Write(event,"q");
            break;
         default: 
            num_evt++;
            ProcessEvent(event,outfile,correction);
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

}

int main(int argc, char **argv) {

   if(argc!=3) {
      printf("Usage: ./offsetadd <input.mid> <output.mid>\n");
      return 1;
   }
   if(argv[1] == argv[2]){
      printf("ERROR: Cannot overwrite midas file %s\n",argv[1]);
   }

   TMidasFile *infile  = new TMidasFile;
   TMidasFile *outfilemid = new TMidasFile;
   infile->Open(argv[1]);
   outfilemid->OutOpen(argv[2]);
   
   int runnumber = infile->GetRunNumber();
   int subrunnumber = infile->GetSubRunNumber();
   char filename[64];
   if(subrunnumber>-1)
      sprintf(filename,"time_diffs%05i_%03i.root",runnumber,subrunnumber); 
   else
      sprintf(filename,"time_diffs%05i.root",runnumber);
	printf("Creating root outfile: %s\n",filename);

   GFile *outfile = new GFile(filename,"RECREATE");

   std::cout << "SIZE: " << TEventTime::digmap.size() << std::endl;
   std::vector<TEventTime*> *eventQ = new std::vector<TEventTime*>;
   QueueEvents(infile,eventQ);
   std::cout << "SIZE: " << TEventTime::digmap.size() << std::endl;

   int64_t *correction;
   correction = new int64_t[TEventTime::NDigitizers()];
   CheckHighTimeStamp(eventQ,correction);
   GetRoughTimeDiff(eventQ,correction);
   GetTimeDiff(eventQ,correction);
   infile->Close();
   infile->Open(argv[1]);//This seems like the easiest way to reset the file....
   //It might be worth threading the Read/Write Part of this...its slooooooow.
   WriteEvents(infile,outfilemid,correction);


   //Have to do deleting on Q if we move to a next step of fixing the MIDAS File
   infile->Close();
   outfile->Close();
   delete[] correction;
   delete infile;
   delete outfile;
   delete outfilemid;

}


