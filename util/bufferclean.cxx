// Author: Ryan Dunlop

// g++ offsetfind.cxx `root-config --cflags --libs` -I${GRSISYS}/include -L${GRSISYS}/libraries -lMidasFormat
// -lXMLParser  -ooffsetfind
#include "TMidasFile.h"
#include "TMidasEvent.h"
#include "GFile.h"
#include "TStopwatch.h"
#include "TSystem.h"
#include <map>
#include <iostream>
#include <fstream>
#include <chrono>
#include <utility>

UInt_t chanId_threshold = 100;

Bool_t CheckEvent(const std::shared_ptr<TMidasEvent>& evt)
{
   // This function does not work if a Midas event contains multiple fragments
   static std::map<Int_t, Bool_t> triggermap; // Map of Digitizer vs have we had a triggerId < threshold yet?
   // First parse the  Midas Event.
   evt->SetBankList();

   // Need to put something in that says "if not a Griffin fragment (ie epics) return true"
   void* ptr;
   int   banksize = evt->LocateBank(nullptr, "GRF2", &ptr);
   int   bank     = 2;

   if(banksize == 0) {
      banksize = evt->LocateBank(nullptr, "GRF1", &ptr);
      bank     = 1;
   }
   uint32_t type  = 0xffffffff;
   uint32_t value = 0xffffffff;

   UInt_t chanadd = 0;
   UInt_t trigId  = 0;
   // UInt_t dettype = 0;
   // Int_t dignum = -1;

   for(int x = 0; x < banksize; x++) {
      value = *(reinterpret_cast<int*>(ptr) + x);
      type  = value & 0xf0000000;

      switch(type) {
      case 0x80000000:
         switch(bank) {
         case 1: // header format from before May 2015 experiments
                 // Sets:
            //     The number of filters
            //     The Data Type
            //     Number of Pileups
            //     Channel Address
            //     Detector Type
            if((value & 0xf0000000) != 0x80000000) {
               return false;
            }
            chanadd = (value & 0x0003fff0) >> 4;
            // dettype    =  (value &0x0000000f);

            // if(frag-DetectorType==2)
            //    frag->ChannelAddress += 0x8000;
            break;
         case 2:
            // Sets:
            //     The number of filters
            //     The Data Type
            //     Number of Pileups
            //     Channel Address
            //     Detector Type
            if((value & 0xf0000000) != 0x80000000) {
               return false;
            }
            chanadd = (value & 0x000ffff0) >> 4;
            // dettype    =  (value &0x0000000f);

            // if(frag-DetectorType==2)
            //    frag->ChannelAddress += 0x8000;
            break;
         default: printf("This bank not yet defined.\n"); break;
         };
         // dignum = chanadd&0x0000ff00;
         break;
      case 0x90000000: trigId = value & 0x0fffffff;
      };
   }
   if(triggermap.find(chanadd) == triggermap.end()) {
      triggermap[chanadd] = false; // initialize the new digitizer number to false.
   }
   // Check to make sure we aren't getting any triggerId's = 0. I think these are corrupt events RD.
   if(trigId == 0) {
      return false;
   }

   // Check against map of trigger Id's to see if we have hit the elusive < threshold mark yet.
   if(triggermap.find(chanadd)->second) {
      return true;
   }
   if(trigId < chanId_threshold) {
      triggermap.find(chanadd)->second = true;
      return true;
   } else {
      return false;
   }
}

/*void AddToQueue(std::shared_ptr<TMidasEvent> evt){
   evtQ.push(evt);
}
*/

void Write(std::shared_ptr<TMidasEvent> evt, TMidasFile* outfile)
{
   outfile->FillBuffer(std::move(evt));

   // if(outfile->GetBufferSize() > 100000){
   //    outfile->WriteBuffer();
   // }
}

int main(int argc, char** argv)
{
   if(argc < 2) {
      printf("Usage: ./bufferclear <input.mid> <output.mid>\n");
      return 1;
   }

   if(argv[1] == argv[2]) {
      printf("input.mid and output.mid must have different names\n");
      return 1;
   }

   auto* file = new TMidasFile;
   // TMidasFile *outfile = new TMidasFile;
   file->Open(argv[1]);
   int runnumber    = file->GetRunNumber();
   int subrunnumber = file->GetSubRunNumber();
   if(argc < 3) {
      file->OutOpen(Form("cleanrun%05d_%03d.mid", runnumber, subrunnumber));
   } else {
      file->OutOpen(argv[2]);
   }

   std::ifstream in(file->GetFilename(), std::ifstream::in | std::ifstream::binary);
   in.seekg(0, std::ifstream::end);
   long long filesize = in.tellg();
   in.close();

   int       bytes     = 0;
   long long bytesread = 0;

   TStopwatch w;
   w.Start();

   UInt_t                       num_bad_evt = 0;
   UInt_t                       num_evt     = 0;
   std::shared_ptr<TMidasEvent> event = std::make_shared<TMidasEvent>(); // need to new for each event

   while(true) {
      bytes = file->Read(event);
      if(bytes == 0) {
         printf(DMAGENTA "\tfile: %s ended on %s" RESET_COLOR "\n", file->GetFilename(), file->GetLastError());
         if(file->GetLastErrno() == -1) { // try to read some more...
            continue;
         }
         break;
      }
      bytesread += bytes;

      switch(event->GetEventId()) {
      case 0x8000:
         printf("start of run\n");
         // file->Write(event,"q");
         Write(event, file);
         printf(DGREEN);
         event->Print();
         printf(RESET_COLOR);
         break;
      case 0x8001:
         printf("end of run\n");
         printf(DRED);
         event->Print();
         printf(RESET_COLOR);
         //  file->Write(event,"q");
         Write(event, file);
         break;
      case 0x0001: // This is a GRIFFIN digitizer event
         if(CheckEvent(event)) {
            Write(event, file);
            // file->Write(event,"q");
         } else {
            num_bad_evt++;
         }
         num_evt++;
         break;
      default: // Probably epics
         Write(event, file);
         //  file->Write(event,"q");
         break;
      };

      if(num_evt % 5000 == 0) {
         gSystem->ProcessEvents();
         printf(HIDE_CURSOR " bad events %u/%u have processed %.2fMB/%.2f MB => %.1f MB/s              " SHOW_CURSOR
                            "\r",
                num_bad_evt, num_evt, (bytesread / 1000000.0), (filesize / 1000000.0),
                (bytesread / 1000000.0) / w.RealTime());
         w.Continue();
      }
   }
   file->WriteBuffer();

   printf("\n");

   file->Close();
   file->OutClose();
   delete file;
   // delete outfile;
}
// void WriteEventToFile(TMidasFile*,std::shared_ptr<TMidasEvent> Option_t);
