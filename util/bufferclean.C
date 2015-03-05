// Author: Ryan Dunlop 

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

void WriteEventToFile(TMidasFile* file, TMidasEvent* event){


}

Bool_t CheckEvent(TMidasEvent *evt){
   //This function does not work if a Midas event contains multiple fragments
   static std::map<Int_t, Bool_t> triggermap; //Map of Digitizer vs have we had a triggerId < 10 yet?
   //First parse the  Midas Event.
   evt->SetBankList();
 

   //Need to put something in that says "if not a Griffin fragment (ie epics) return true"
   void *ptr;
   int banksize = evt->LocateBank(NULL,"GRF1",&ptr);

   int type  = 0xffffffff;
   int value = 0xffffffff;

   UInt_t chanadd = 0;
   UInt_t trigId = 0;
   UInt_t dettype = 0;
   Int_t dignum = -1;

   for(int x=0;x<banksize;x++) {
      value = *((int*)ptr+x);
      type  = value & 0xf0000000; 

      switch(type) {
         case 0x80000000:
            dettype = value & 0x0000000f;
            chanadd = (value &0x0003fff0)>> 4;
            dignum = chanadd&0x0000ff00;
            break;
         case 0x90000000:
            trigId = value & 0x0fffffff;
      };
   }
   if(triggermap.find(dignum) == triggermap.end()){
      triggermap[dignum] = false; //initialize the new digitizer number to false.
   }
   //Check to make sure we aren't getting any triggerId's = 0. I think these are corrupt events RD.
   if(trigId == 0){
      return false;
   }

   //Check against map of trigger Id's to see if we have hit the elusive < 10 mark yet.
   if(triggermap.find(dignum)->second == true){ 
      return true;
   }
   else if(trigId < 10){
      triggermap.find(dignum)->second = true;
      evt->Print();
      return true;
   }
   else 
      return false;
}

int main(int argc, char **argv) {

   if(argc!=3) {
      printf("Usage: ./bufferclear <input.mid> <output.mid>\n");
      return 1;
   }

   if(argv[1] == argv[2]){
      printf("input.mid and output.mid must have different names\n");
      return 1;
   }

   TMidasFile *infile  = new TMidasFile;
   TMidasFile *outfile = new TMidasFile;
   infile->Open(argv[1]);
   outfile->OutOpen(argv[2]);

   TMidasEvent *event = new TMidasEvent;

   while(infile->Read(event)>0) {
      if(CheckEvent(event)){
         printf("TRUE\n\n");
         WriteEventToFile(outfile,event);
      }
      else printf("FALSE\n\n");
   }

   infile->Close();
   outfile->Close();
   delete infile;
   delete outfile;

}
//void WriteEventToFile(TMidasFile*,TMidasEvent*,Option_t);

