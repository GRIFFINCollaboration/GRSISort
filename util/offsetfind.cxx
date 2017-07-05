// g++ offsetfind.cxx `root-config --cflags --libs` -I${GRSISYS}/include -L${GRSISYS}/libraries -lMidasFormat
// -lXMLParser  -ooffsetfind

#include "TMidasFile.h"
#include "TMidasEvent.h"
#include "GFile.h"
#include "TFragment.h"
#include "TTree.h"
#include "TChannel.h"
#include "TH2D.h"
#include "TTreeIndex.h"
#include "TVirtualIndex.h"
#include "Globals.h"
#include "TF1.h"
#include "TMath.h"
#include "TString.h"
#include "TPolyMarker.h"

#include <vector>
#include <iostream>

class TEventTime {
public:
   explicit TEventTime(std::shared_ptr<TMidasEvent> event)
   {
      event->SetBankList();

      void* ptr;
      int   banksize = event->LocateBank(nullptr, "GRF1", &ptr);

      uint32_t type  = 0xffffffff;
      uint32_t value = 0xffffffff;

      // int64_t time = 0;

      for(int x = 0; x < banksize; x++) {
         value = *(reinterpret_cast<int*>(ptr) + x);
         type  = value & 0xf0000000;

         switch(type) {
         case 0x80000000:
            dettype = value & 0x0000000F;
            chanadd = (value & 0x0003fff0) >> 4;
            break;
         case 0xa0000000: timelow  = value & 0x0fffffff; break;
         case 0xb0000000: timehigh = value & 0x00003fff; break;
         };
      }
      timemidas = (unsigned int)(event->GetTimeStamp());
      if(timemidas < low_timemidas) {
         low_timemidas = timemidas;
      }

      SetDigitizer();

      if(GetTimeStamp() < lowest_time || lowest_time == -1) {
         lowest_time = GetTimeStamp();
         best_dig    = Digitizer();
      }
   }

   ~TEventTime() = default;

   int64_t GetTimeStamp()
   {
      long time = timehigh;
      time      = time<<28;
      time |= timelow & 0x0fffffff;
      return time;
   }
   int TimeStampHigh() { return timehigh; }

   unsigned long MidasTime() { return timemidas; }

   int Digitizer() { return digitizernum; }

   int DetectorType() { return dettype; }

   void SetDigitizer()
   {
      // Maybe make a map somewhere of digitizer vs address
      digitizernum = chanadd & 0x0000ff00;
      digmap.insert(std::pair<int, int>(digitizernum, digmap.size()));
   }

   static void OrderDigitizerMap()
   {
      std::map<int, int>::iterator it;
      int index = 0;
      for(it = digmap.begin(); it != digmap.end(); it++) {
         it->second = index++;
      }
   }

   inline static int NDigitizers() { return digmap.size(); }

   inline static int GetBestDigitizer() { return best_dig; }

   static unsigned long GetLowestMidasTime() { return low_timemidas; }

   int DigIndex() { return digmap.find(digitizernum)->second; }

   static std::map<int, int> digmap;
   static unsigned long low_timemidas;
   static int           best_dig;
   static int64_t       lowest_time;

private:
   int           timelow;
   int           timehigh;
   unsigned long timemidas;
   int           dettype;
   int           chanadd;
   int           digitizernum{};
};

unsigned long TEventTime::low_timemidas = -1;
int64_t       TEventTime::lowest_time   = -1;
int           TEventTime::best_dig      = 0;
std::map<int, int> TEventTime::digmap;

int QueueEvents(TMidasFile* infile, std::vector<TEventTime*>* eventQ)
{
   int                          events_read  = 0;
   const int                    total_events = 1E7;
   std::shared_ptr<TMidasEvent> event        = std::make_shared<TMidasEvent>();
   eventQ->reserve(total_events);

   while(infile->Read(event) > 0 && eventQ->size() < total_events) {
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
         if(event->GetEventId() != 1) {
            break;
         }
         events_read++;
         eventQ->push_back(
            new TEventTime(event)); // I'll keep 3G data in here for now in case we need to use it for time stamping
         break;
      };
      if(events_read % 250000 == 0) {
         printf(DYELLOW "\tQUEUEING EVENT %d/%d  \r" RESET_COLOR, events_read, total_events);
         fflush(stdout);
      }
   }
   TEventTime::OrderDigitizerMap();
   printf("\n");
   return 0;
}

void CheckHighTimeStamp(std::vector<TEventTime*>* eventQ, int64_t* correction)
{
   // This function should return an array of corrections

   auto* midvshigh = new TList;
   printf(DBLUE "Correcting High time stamps...\n" RESET_COLOR);
   // These first four are for looking to see if high time-stamp reset
   std::map<int, int>::iterator mapit;
   for(mapit = TEventTime::digmap.begin(); mapit != TEventTime::digmap.end(); mapit++) {
      auto* midvshighhist = new TH2D(Form("midvshigh_0x%04x", mapit->first), Form("midvshigh_0x%04x", mapit->first),
                                     5000, 0, 5000, 5000, 0, 5000);
      midvshigh->Add(midvshighhist);
   }

   unsigned int lowmidtime = TEventTime::GetLowestMidasTime();

   // MidasTimeStamp is the only time we can trust at this level.

   // int fEntries = eventQ->size();

   int FragsIn = 0;

   FragsIn++;
   int* lowest_hightime;
   lowest_hightime = new int[TEventTime::NDigitizers()];
   // Clear lowest hightime
   for(int i = 0; i < TEventTime::NDigitizers(); i++) {
      lowest_hightime[i] = 0;
   }

   std::vector<TEventTime*>::iterator it;

   for(it = eventQ->begin(); it != eventQ->end(); it++) {
      // This makes the plot, might not be required
      int           hightime = (*it)->TimeStampHigh();
      unsigned long midtime  = (*it)->MidasTime() - lowmidtime;
      if(midtime > 20) {
         break; // 20 seconds seems like plenty enough time
      }

      if((*it)->DetectorType() == 1) {
         (dynamic_cast<TH2D*>(midvshigh->At((*it)->DigIndex())))->Fill(midtime, hightime);
         if(hightime < lowest_hightime[(*it)->DigIndex()]) {
            lowest_hightime[TEventTime::digmap.at((*it)->DigIndex())] = hightime;
         }
      }
   }

   // find lowest digitizer
   int lowest_dig = 0;
   for(mapit = TEventTime::digmap.begin(); mapit != TEventTime::digmap.end(); mapit++) {
      if(lowest_hightime[mapit->second] < lowest_hightime[lowest_dig]) {
         lowest_dig = mapit->second;
      }
   }

   midvshigh->Print();
   printf("The lowest digitizer is %d\n", lowest_dig);
   printf("*****  High time shifts *******\n");
   for(mapit = TEventTime::digmap.begin(); mapit != TEventTime::digmap.end(); mapit++) {
      printf("0x%04x:\t %d\n", mapit->first, lowest_hightime[mapit->second]);
      // Calculate the shift to 0 all digitizers
      correction[mapit->second] = ((lowest_hightime[mapit->second] - lowest_hightime[lowest_dig]) & 0x00003fff)<<28;
   }
   printf("********************\n");

   midvshigh->Write();
   midvshigh->Delete();
   delete[] lowest_hightime;
}

void GetRoughTimeDiff(std::vector<TEventTime*>* eventQ, int64_t* correction)
{
   // We want the MIDAS time stamps to still be the way we index these events, but we want to index on low time stamps
   // next
   printf(DBLUE "Looking for rough time differences...\n" RESET_COLOR);

   auto* roughlist = new TList;
   // These first four are for looking to see if high time-stamp reset

   std::map<int, bool>          keep_filling;
   std::map<int, int>::iterator mapit;
   for(mapit = TEventTime::digmap.begin(); mapit != TEventTime::digmap.end(); mapit++) {
      auto* roughhist =
         new TH1C(Form("rough_0x%04x", mapit->first), Form("rough_0x%04x", mapit->first), 50E6, -25E6, 25E6);
      roughlist->Add(roughhist);
      keep_filling[mapit->first] = true;
   }

   // The "best digitizer" is set when we fill the event Q
   printf(DYELLOW "Using the best digitizer 0x%04x\n" RESET_COLOR, TEventTime::GetBestDigitizer());

   TH1C* fillhist; // This pointer is useful later to clean up a lot of messiness

   std::vector<TEventTime*>::iterator hit1;
   std::vector<TEventTime*>::iterator hit2;
   int                                event1count = 0;
   const int                          range       = 1000;
   for(hit1 = eventQ->begin(); hit1 != eventQ->end(); hit1++) { // This steps hit1 through the eventQ
      // We want to have the first hit be in the "good digitizer"
      if(event1count % 250000 == 0) {
         printf("Processing Event %d /%lu      \r", event1count, eventQ->size());
      }
      fflush(stdout);
      event1count++;

      if((*hit1)->Digitizer() != TEventTime::GetBestDigitizer()) {
         continue;
      }

      int64_t time1 = (*hit1)->GetTimeStamp() - correction[(*hit1)->DigIndex()];

      if(event1count > range) {
         hit2 = hit1 - range;
      } else {
         hit2 = eventQ->begin();
      }
      // Now that we have the best digitizer, we can start looping through the events
      int event2count = 0;
      while(hit2 != eventQ->end() && event2count < range * 2) {
         event2count++;
         if(hit1 == hit2) {
            continue;
         }
         int digitizer = (*hit2)->Digitizer();
         if(keep_filling[digitizer]) {
            fillhist =
               dynamic_cast<TH1C*>(roughlist->At((*hit2)->DigIndex())); // This is where that pointer comes in handy
            int64_t time2 = (*hit2)->GetTimeStamp() - correction[(*hit2)->DigIndex()];
            Int_t   bin   = static_cast<Int_t>(time2 - time1);

            if(fillhist->FindBin(bin) > 0 && fillhist->FindBin(bin) < fillhist->GetNbinsX()) {
               if(fillhist->GetBinContent(fillhist->Fill(bin)) > 126) {
                  keep_filling[digitizer] = false;
                  printf("\nDigitizer 0x%04x is done filling\n", digitizer);
               }
            }

            hit2++;
         }
      }
   }

   for(mapit = TEventTime::digmap.begin(); mapit != TEventTime::digmap.end(); mapit++) {
      fillhist = dynamic_cast<TH1C*>(roughlist->At(mapit->second));
      correction[mapit->second] += static_cast<int64_t>(fillhist->GetBinCenter(fillhist->GetMaximumBin()));
   }

   printf("*****  Rough time shifts *******\n");
   for(mapit = TEventTime::digmap.begin(); mapit != TEventTime::digmap.end(); mapit++) {
#ifdef OS_DARWIN
      printf("0x%04x:\t %lld\n", mapit->first, correction[mapit->second]);
#else
      printf("0x%04x:\t %ld\n", mapit->first, correction[mapit->second]);
#endif
   }
   printf("********************\n");

   roughlist->Print();
   roughlist->Write();
   roughlist->Delete();
}

void GetTimeDiff(std::vector<TEventTime*>* eventQ, int64_t* correction)
{
   // We want the MIDAS time stamps to still be the way we index these events, but we want to index on low time stamps
   // next
   printf(DBLUE "Looking for final time differences...\n" RESET_COLOR);

   auto* list = new TList;
   // These first four are for looking to see if high time-stamp reset

   std::map<int, int>::iterator mapit;
   for(mapit = TEventTime::digmap.begin(); mapit != TEventTime::digmap.end(); mapit++) {
      auto* hist =
         new TH1D(Form("timediff_0x%04x", mapit->first), Form("timediff_0x%04x", mapit->first), 1000, -500, 500);
      list->Add(hist);
   }

   // The "best digitizer" is set when we fill the event Q
   printf(DYELLOW "Using the best digitizer 0x%04x\n" RESET_COLOR, TEventTime::GetBestDigitizer());

   TH1D* fillhist; // This pointer is useful later to clean up a lot of messiness

   std::vector<TEventTime*>::iterator hit1;
   std::vector<TEventTime*>::iterator hit2;
   int                                event1count = 0;
   const int                          range       = 1250;
   for(hit1 = eventQ->begin(); hit1 != eventQ->end(); hit1++) { // This steps hit1 through the eventQ
      // We want to have the first hit be in the "good digitizer"
      if(event1count % 75000 == 0) {
         printf("Processing Event %d / %lu       \r", event1count, eventQ->size());
      }
      fflush(stdout);

      event1count++;
      // We need to make sure that that if we have a digitizer of 0, we have a detector type of 1
      if((*hit1)->Digitizer() == 0 && (*hit1)->DetectorType() != 1) {
         continue;
      }

      if((*hit1)->Digitizer() != TEventTime::GetBestDigitizer()) {
         continue;
      }

      int64_t time1 = (*hit1)->GetTimeStamp() - correction[(*hit1)->DigIndex()];
      ;

      if(event1count > range) {
         hit2 = hit1 - range;
      } else {
         hit2 = eventQ->begin();
      }
      // Now that we have the best digitizer, we can start looping through the events
      int event2count = 0;
      while(hit2 != eventQ->end() && event2count < range * 2) {
         event2count++;
         // We need to make sure that that if we have a digitizer of 0, we have a detector type of 1
         if((*hit2)->Digitizer() == 0 && (*hit2)->DetectorType() != 1) {
            continue;
         }

         if(hit1 != hit2) {
            // int digitizer = (*hit2)->Digitizer();
            fillhist      = dynamic_cast<TH1D*>(list->At((*hit2)->DigIndex())); // This is where that pointer comes in handy
            int64_t time2 = (*hit2)->GetTimeStamp() - correction[(*hit2)->DigIndex()];
            if(time2 - time1 < 2147483647 &&
               time2 - time1 > -2147483647) { // Make sure we are casting this to 32 bit properly
               Int_t bin = static_cast<Int_t>(time2 - time1);

               fillhist->Fill(bin);
            }
         }
         hit2++;
      }
   }
   for(mapit = TEventTime::digmap.begin(); mapit != TEventTime::digmap.end(); mapit++) {
      fillhist = dynamic_cast<TH1D*>(list->At(mapit->second));
      correction[mapit->second] += static_cast<int64_t>(fillhist->GetBinCenter(fillhist->GetMaximumBin()));
      auto* pm = new TPolyMarker;
      pm->SetNextPoint(fillhist->GetBinCenter(fillhist->GetMaximumBin()),
                       fillhist->GetBinContent(fillhist->GetMaximumBin()) + 10);
      pm->SetMarkerStyle(23);
      pm->SetMarkerColor(kRed);
      pm->SetMarkerSize(1.3);
      fillhist->GetListOfFunctions()->Add(pm);
      //     fillhist->Draw();
   }

   printf("*****  Final time shifts *******\n");
   for(mapit = TEventTime::digmap.begin(); mapit != TEventTime::digmap.end(); mapit++) {
#ifdef OS_DARWIN
      printf("0x%04x:\t %lld\n", mapit->first, correction[mapit->second]);
#else
      printf("0x%04x:\t %ld\n", mapit->first, correction[mapit->second]);
#endif
   }
   printf("********************\n");

   list->Print();
   list->Write();
   list->Delete();
}

int main(int argc, char** argv)
{

   if(argc != 2) {
      printf("Usage: ./offsetfind <input.mid>\n");
      return 1;
   }

   auto* infile = new TMidasFile;
   infile->Open(argv[1]);

   auto* outfile = new TFile("outfile.root", "RECREATE");

   std::cout<<"SIZE: "<<TEventTime::digmap.size()<<std::endl;
   auto* eventQ = new std::vector<TEventTime*>;
   QueueEvents(infile, eventQ);
   std::cout<<"SIZE: "<<TEventTime::digmap.size()<<std::endl;

   int64_t* correction;
   correction = new int64_t[TEventTime::NDigitizers()];
   CheckHighTimeStamp(eventQ, correction);
   GetRoughTimeDiff(eventQ, correction);
   GetTimeDiff(eventQ, correction);

   // Have to do deleting on Q if we move to a next step of fixing the MIDAS File
   infile->Close();
   outfile->Close();
   delete[] correction;
   delete infile;
}
