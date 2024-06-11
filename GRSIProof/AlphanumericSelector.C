#define AlphanumericSelector_cxx
// The class definition in AlphanumericSelector.h has been generated automatically
#include "AlphanumericSelector.h"

void AlphanumericSelector::CreateHistograms()
{
   // get the channel map and loop over it, building a map of array number to channel address
   std::unordered_map<unsigned int, TChannel*>* channelMap = TChannel::GetChannelMap();
   std::cout<<"Got "<<channelMap->size()<<" channels"<<std::endl;
   std::map<int, unsigned int> address;

   for(auto channel : *channelMap) {
      // use only GRIFFIN channels
      if(channel.second->GetClassType() == TGriffin::Class()) {
         // use only the "A" channel, not the "B" channel
         if(channel.second->GetMnemonic()->OutputSensor() == TMnemonic::EMnemonic::kA) {
            address[4 * (channel.second->GetDetectorNumber() - 1) + channel.second->GetCrystalNumber() + 1] = channel.second->GetAddress();
         }
      }
   }

   std::cout<<"x-axis: "<<address.rbegin()->first - address.begin()->first + 1<<" bins from "<<address.begin()->first<<" to "<<address.rbegin()->first + 1<<std::endl;

   // Define Histograms
   fH2["EnergyVsChannel"] = new TH2D("EnergyVsChannel", "#gamma singles energy vs. channel", address.rbegin()->first - address.begin()->first + 1, address.begin()->first, address.rbegin()->first + 1, 4000, 0, 2000);
   fH2["ChargeVsChannel"] = new TH2D("ChargeVsChannel", "#gamma singles charge vs. channel", address.rbegin()->first - address.begin()->first + 1, address.begin()->first, address.rbegin()->first + 1, 4000, 0, 4000);

   for(int bin = 1; bin <= fH2["EnergyVsChannel"]->GetNbinsX(); ++bin) {
      if(address.find(bin) == address.end()) {
         std::cout<<"Couldn't find array number "<<bin<<" in address map"<<std::endl;
         continue;
      }
      fH2["EnergyVsChannel"]->GetXaxis()->SetBinLabel(bin, Form("0x%x", address.at(bin)));
      fH2["ChargeVsChannel"]->GetXaxis()->SetBinLabel(bin, Form("0x%x", address.at(bin)));
   }

   // Send histograms to Output list to be added and written.
   for(auto it : fH1) {
      GetOutputList()->Add(it.second);
   }
   for(auto it : fH2) {
      GetOutputList()->Add(it.second);
   }
   for(auto it : fHSparse) {
      GetOutputList()->Add(it.second);
   }
}

void AlphanumericSelector::FillHistograms()
{
   // Loop over all Griffin Hits
   for(auto i = 0; i < fGrif->GetMultiplicity(); ++i) {
      auto grif = fGrif->GetGriffinHit(i);
      fH2.at("EnergyVsChannel")->Fill(grif->GetArrayNumber(), grif->GetEnergy());
      fH2.at("ChargeVsChannel")->Fill(grif->GetArrayNumber(), grif->GetCharge());
   }
}
