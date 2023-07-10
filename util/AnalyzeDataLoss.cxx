// g++ AnalyzeDataLoss.C -I$GRSISYS/include -L$GRSISYS/libraries `root-config --cflags --libs` -lTreePlayer -o
// AnalyzeDataLoss

// Takes in a FragmentTree, makes data loss spectra
//
//
#include <utility>
#include <vector>
#include <cstdio>
#include <iostream>
#include <iomanip>

#include "TTree.h"
#include "TTreeIndex.h"
#include "TVirtualIndex.h"
#include "TFile.h"
#include "TList.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TStopwatch.h"
#include "TMath.h"

#include "TChannel.h"

#include "TFragment.h"

TList* AnalyzeDataLoss(TTree* tree, long entries = 0, TStopwatch* w = nullptr)
{

   if(w == nullptr) {
      w = new TStopwatch;
      w->Start();
   }

   auto* list = new TList;

   TFragment* currentFrag = nullptr;

   TChannel::ReadCalFromTree(tree);

   TBranch* branch = tree->GetBranch("TFragment");
   branch->SetAddress(&currentFrag);

   int fEntries = tree->GetEntries();

   tree->GetEntry(0);
   long      entry;
   long      skip     = 1000; // skip this many entries before beginning
   const int channels = 150;  // number of channels
   // long lasttime = 0;

   //--------------- parameters for dealing with the roll-over of the AcceptedChannelId ----------------------//
   unsigned long int acceptedMax = TMath::Power(2, 14); // this is the maximum number that the AcceptedChannelId can be
   int               rollovers[channels];               // this is how many roll-overs we have had
   // long int lastAccepted[channels];
   bool         rolling[channels]; // array that tells us if we're rolling over in that channel
   int          rollnum[channels]; // array that tells us how many times we've had accepted ID over the threshold
   unsigned int rollingthreshold  = 1000;
   int          rollnum_threshold = 20; // if we have this many numbers above the threshold, turn rolling on or off

   long int channelIds[channels][3];
   long int acceptedChannelIds[channels][3];
   long int timestamp[channels][3];
   int      networkPacketNumber[3] = {0, 0, 0};
   long int networkPacketTS[3]     = {0, 0, 0};
   int      timebins               = 10000;
   double   timemin                = 0;    // in seconds
   double   timemax                = 1000; // in seconds
   auto*    accepted_hst =
      new TH2D("accepted_hst", "Accepted Channel Id vs. Channel Number;Channel Number;Accepted Channel Id", channels, 0,
               channels, 10000, 0, 10e5);
   list->Add(accepted_hst);
   auto* lostNetworkPackets =
      new TH1D("lostNetworkPackets", "lost network packets;time [s];lost network packets", timebins, timemin, timemax);
   list->Add(lostNetworkPackets);
   auto* lostChannelIds =
      new TH2D("lostChannelIds", "Lost Channel Id vs. Channel Number;Channel Number;Lost Channel Id", channels, 0,
               channels, 10000, 0, 10e5);
   list->Add(lostChannelIds);
   auto* lostAcceptedIds =
      new TH2D("lostAcceptedIds", "Lost Accepted Channel Id vs. Channel Number;Channel Number;Lost Accepted Channel Id",
               channels, 0, channels, 10000, 0, 10e5);
   list->Add(lostAcceptedIds);
   auto* lostChannelIdsTime =
      new TH2D("lostChannelIdsTime", "Lost Channel Id time vs. Channel Number;Channel Number;time [s]", channels, 0,
               channels, timebins, timemin, timemax);
   list->Add(lostChannelIdsTime);
   auto* lostAcceptedIdsTime =
      new TH2D("lostAcceptedIdsTime", "Lost Accepted Channel Id time vs. Channel Number;Channel Number;time [s]",
               channels, 0, channels, timebins, timemin, timemax);
   list->Add(lostAcceptedIdsTime);

   // initialize acceptedID array
   // for (int i=0;i<channels;i++) lastAccepted[i] = 0;
   // initialize rolling array
   for(bool& i : rolling) {
      i = kFALSE;
   }
   // initialize rollnum array
   for(int& i : rollnum) {
      i = 0;
   }
   // initialize rollovers array
   for(int& rollover : rollovers) {
      rollover = 0;
   }
   // initialize timestamp array
   for(auto& i : timestamp) {
      for(long& j : i) {
         j = 0;
      }
   }

   for(entry = skip; entry < fEntries; entry++) {
      // if(entry == skip) {
      //	tree->GetEntry(entry);
      //	continue;
      //}
      // TFragment myFrag  = *currentFrag;         //Set myfrag to be the x'th fragment before incrementing it.

      tree->GetEntry(entry);

      if(currentFrag->GetChannelNumber() == 0 && currentFrag->GetAddress() != 0) {
         continue;
      }

      long time = currentFrag->GetTimeStamp(); // Get the timestamp of the x'th fragment
      int           chan      = currentFrag->GetChannelNumber();
      unsigned long accepted  = currentFrag->GetAcceptedChannelId();
      unsigned long chanid    = currentFrag->GetChannelId();
      unsigned long netpacket = currentFrag->GetNetworkPacketNumber();

      //---------------- this section deals with the rolling over of the AcceptedChannelId. -------------------//
      if(!rolling[chan] && accepted > (acceptedMax - rollingthreshold)) {
         ++rollnum[chan];
      }
      if(rolling[chan] && accepted > rollingthreshold && accepted < (acceptedMax / 2)) {
         ++rollnum[chan];
      }
      if(!rolling[chan] && rollnum[chan] > rollnum_threshold) {
         rolling[chan] = kTRUE;
         rollnum[chan] = 0;
      }
      if(rolling[chan] && rollnum[chan] > rollnum_threshold) {
         rolling[chan] = kFALSE;
         rollnum[chan] = 0;
         ++rollovers[chan];
      }

      if(rolling[chan] && accepted <= rollingthreshold * 2) {
         // printf("found rollover: changing %li to %li\n",accepted,accepted+acceptedMax);
         accepted += acceptedMax;
      }
      accepted += rollovers[chan] * acceptedMax;

      // lasttime = time;
      // if (accepted-lastAccepted[chan]>10 && chan!=0) printf("%i\t%li\t%li\n",chan,accepted,lastAccepted[chan]);
      // lastAccepted[chan] = accepted;

      //----------------- end of section: AcceptedChannelId is now corrected for roll-over -----------------//

      // if we have a new network packet number
      if(netpacket != 0 && networkPacketNumber[0] != 0) {
         networkPacketNumber[2] = netpacket;
         networkPacketTS[2]     = time;
         if(networkPacketNumber[0] < networkPacketNumber[1] && networkPacketNumber[1] < networkPacketNumber[2]) {
            for(int packet = networkPacketNumber[0] + 1; packet < networkPacketNumber[1]; ++packet) {
               lostNetworkPackets->Fill(networkPacketTS[1] / 1e8);
            }
            // things look fine, so prepare for next time
            networkPacketNumber[0] = networkPacketNumber[1];
            networkPacketNumber[1] = networkPacketNumber[2];
            networkPacketTS[0]     = networkPacketTS[1];
            networkPacketTS[1]     = networkPacketTS[2];
         } else if(networkPacketNumber[0] < networkPacketNumber[2]) {
            std::cout<<"found wrong network packet number 0x"<<std::hex<<networkPacketNumber[1]
                     <<" (not between 0x"<<networkPacketNumber[0]<<" and 0x"<<networkPacketNumber[2]<<std::dec
                     <<")"<<std::endl;
            networkPacketNumber[1] = networkPacketNumber[2];
            networkPacketTS[1]     = networkPacketTS[2];
         }
      } else if(netpacket != 0 && networkPacketNumber[0] == 0) {
         networkPacketNumber[0] = networkPacketNumber[1];
         networkPacketNumber[1] = networkPacketNumber[2];
         networkPacketNumber[2] = netpacket;
         networkPacketTS[0]     = networkPacketTS[1];
         networkPacketTS[1]     = networkPacketTS[2];
         networkPacketTS[2]     = time;
      }

      // check if the "middle" channel ID is reasonable and fill all IDs we've missed between the first and middle ID
      if(0 <= chan && chan < channels && timestamp[chan][0] != 0) {
         timestamp[chan][2]  = time;
         channelIds[chan][2] = chanid;
         if(timestamp[chan][0] > 0 && channelIds[chan][0] < channelIds[chan][1] &&
            channelIds[chan][1] < channelIds[chan][2]) {
            for(int id = channelIds[chan][0] + 1; id < channelIds[chan][1]; ++id) {
               lostChannelIds->Fill(chan, id);
               lostChannelIdsTime->Fill(chan, timestamp[chan][1] / 1e8);
            }
            timestamp[chan][0]  = timestamp[chan][1];
            timestamp[chan][1]  = timestamp[chan][2];
            channelIds[chan][0] = channelIds[chan][1];
            channelIds[chan][1] = channelIds[chan][2];
         } else {
            // now, we only want to overwrite the bad timestamp
            timestamp[chan][1]  = timestamp[chan][2];
            channelIds[chan][1] = channelIds[chan][2];
         }
      } else if(0 <= chan && chan < channels && timestamp[chan][0] == 0) {
         timestamp[chan][0]          = timestamp[chan][1];
         timestamp[chan][1]          = timestamp[chan][2];
         channelIds[chan][0]         = channelIds[chan][1];
         channelIds[chan][1]         = channelIds[chan][2];
         timestamp[chan][2]          = time;
         channelIds[chan][2]         = chanid;
         acceptedChannelIds[chan][0] = acceptedChannelIds[chan][1];
         acceptedChannelIds[chan][1] = acceptedChannelIds[chan][2];
         acceptedChannelIds[chan][2] = accepted;
      }

      accepted_hst->Fill(chan, accepted);

      // check if the "middle" accepted channel ID is reasonable and fill all IDs we've missed between the first and
      // middle ID
      if(0 <= chan && chan < channels && timestamp[chan][0] != 0) {
         acceptedChannelIds[chan][2] = accepted;
         if(timestamp[chan][0] > 0 && acceptedChannelIds[chan][0] < acceptedChannelIds[chan][1] &&
            acceptedChannelIds[chan][1] < acceptedChannelIds[chan][2]) {
            for(int id = acceptedChannelIds[chan][0] + 1; id < acceptedChannelIds[chan][1]; ++id) {
               lostAcceptedIds->Fill(chan, id);
               lostAcceptedIdsTime->Fill(chan, timestamp[chan][1] / 1e8);
            }
            acceptedChannelIds[chan][0] = acceptedChannelIds[chan][1];
            acceptedChannelIds[chan][1] = acceptedChannelIds[chan][2];
         } else {
            acceptedChannelIds[chan][1] = acceptedChannelIds[chan][2];
         }
      }

      // check if channel number and detector type agrees (this relies on a good ODB/cal-file)
      if(chan < 64 && currentFrag->GetDetectorType() != 0) {
         std::cout<<entry<<": Found channel 0 - 63 without detector type zero: address 0x"<<std::hex
                  <<currentFrag->GetAddress()<<std::dec<<", detector type "<<currentFrag->GetDetectorType()
                  <<", time stamp "<<currentFrag->GetTimeStamp()<<std::endl;
      }

      if(entry % 25000 == 0) {
         std::cout<<"\t"<<entry<<" / "<<entries<<" = "<<static_cast<float>(entry) / entries * 100.0<<"%. "
                  <<w->RealTime()<<" seconds"
                  <<"\r"<<std::flush;
         w->Continue();
      }
   }
   std::cout<<"\t"<<entry<<" / "<<entries<<" = "<<static_cast<float>(entry) / entries * 100.0<<"%. "
            <<w->RealTime()<<" seconds"<<std::endl
            <<std::endl;
   w->Continue();

   return list;
}

#ifndef __CINT__

int main(int argc, char** argv)
{

   if(argc != 4 && argc != 3 && argc != 2) {
      printf("try again (usage: %s <fragment tree file> <optional: output file> <max entries>).\n", argv[0]);
      return 0;
   }

   TStopwatch w;
   w.Start();

   std::string fileName;
   if(argc == 2) {
      fileName = argv[1];
      if(fileName.find_last_of('/') != std::string::npos) {
         fileName.insert(fileName.find_last_of('/') + 1, "dataloss_");
      } else {
         fileName.insert(0, "dataloss_");
      }
   } else {
      fileName = argv[2];
   }

   auto* file = new TFile(argv[1]);
   if(file == nullptr) {
      printf("Failed to open file '%s'!\n", argv[1]);
      return 1;
   }
   if(!file->IsOpen()) {
      printf("Failed to open file '%s'!\n", argv[1]);
      return 1;
   }

   TTree* tree = dynamic_cast<TTree*>(file->Get("FragmentTree"));

   if(tree == nullptr) {
      printf("Failed to find fragment tree in file '%s'!\n", argv[1]);
      return 1;
   }

   TTree* badtree = dynamic_cast<TTree*>(file->Get("BadFragmentTree"));

   if(badtree == nullptr) {
      printf("Failed to find bad fragment tree in file '%s'!\n", argv[1]);
   } else {
      std::cout<<badtree->GetEntries()
               <<" bad entries in total = "<<(100. * badtree->GetEntries()) / tree->GetEntries()
               <<"% of the good entries"<<std::endl;
   }

   TTree* epicstree = dynamic_cast<TTree*>(file->Get("EpicsTree"));

   if(epicstree == nullptr) {
      printf("Failed to find epics tree in file '%s'!\n", argv[1]);
   } else {
      std::cout<<epicstree->GetEntries()<<" epics entries"<<std::endl;
   }

   TPPG* ppg = dynamic_cast<TPPG*>(file->Get("TPPG"));

   if(ppg == nullptr) {
      printf("Failed to find ppg in file '%s'!\n", argv[1]);
   } else {
      std::cout<<ppg->PPGSize()<<" ppg events"<<std::endl;
   }

   std::cout<<argv[0]<<": starting AnalyzeDataLoss after "<<w.RealTime()<<" seconds"<<std::endl;
   w.Continue();

   TList* list;
   long   entries = tree->GetEntries();
   if(argc == 4 && atoi(argv[3]) < entries) {
      entries = atoi(argv[3]);
      std::cout<<"Limiting processing of fragment tree to "<<entries<<" entries!"<<std::endl;
   }
   list = AnalyzeDataLoss(tree, entries, &w);

   auto* outfile = new TFile(fileName.c_str(), "recreate");
   list->Write();
   outfile->Close();

   std::cout<<argv[0]<<" done after "<<w.RealTime()<<" seconds"<<std::endl;

   return 0;
}

#endif
