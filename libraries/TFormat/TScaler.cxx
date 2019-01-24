#include "TScaler.h"

#include <iomanip>

#include "TROOT.h"

/// \cond CLASSIMP
ClassImp(TScalerData)
ClassImp(TScaler)
/// \endcond

TScalerData::TScalerData()
{
   Clear();
   fScaler.resize(4); // we expect to have four scaler values
}

TScalerData::TScalerData(const TScalerData& rhs) : TObject()
{
   rhs.Copy(*this);
}

void TScalerData::Copy(TObject& rhs) const
{
   static_cast<TScalerData&>(rhs).fAddress         = fAddress;
   static_cast<TScalerData&>(rhs).fScaler          = fScaler;
   static_cast<TScalerData&>(rhs).fNetworkPacketId = fNetworkPacketId;
   static_cast<TScalerData&>(rhs).fLowTimeStamp    = fLowTimeStamp;
   static_cast<TScalerData&>(rhs).fHighTimeStamp   = fHighTimeStamp;
}

void TScalerData::Clear(Option_t*)
{
   /// Clears the TScalerData.
   fNetworkPacketId = -1;
   fAddress         = 0;
   fScaler.clear();
   fLowTimeStamp    = 0;
   fHighTimeStamp   = 0;
}

void TScalerData::Print(Option_t*) const
{
   printf("time: %16lld, address: 0x%04x", GetTimeStamp(), fAddress);
   for(size_t i = 0; i < fScaler.size(); ++i) {
      printf("\t Scaler[%lu]: 0x%07x", i, fScaler[i]);
   }
   printf("\n");
}

TScaler::TScaler(bool loadIntoMap)
{
   /// This constructor tries to find the "ScalerTree" and uses it (if requested) to load the scaler data into the map.
   ///\param[in] loadIntoMap Flag telling TScaler to load all scaler data into fScalerMap.
   Clear();
   fTree = static_cast<TTree*>(gROOT->FindObject("ScalerTree"));
   if(fTree != nullptr) {
      fEntries = fTree->GetEntries();
      fTree->SetBranchAddress("TScalerData", &fScalerData);
      if(loadIntoMap) {
         for(Long64_t entry = 0; entry < fEntries; ++entry) {
            fTree->GetEntry(entry);
            fScalerMap[fScalerData->GetAddress()][fScalerData->GetTimeStamp()] = fScalerData->GetScaler();
         }
      }
   }
}

TScaler::TScaler(TTree* tree, bool loadIntoMap)
{
   Clear();
   fTree = tree;
   if(fTree != nullptr) {
      fEntries = fTree->GetEntries();
      fTree->SetBranchAddress("TScalerData", &fScalerData);
      if(loadIntoMap) {
         for(Long64_t entry = 0; entry < fEntries; ++entry) {
            fTree->GetEntry(entry);
            fScalerMap[fScalerData->GetAddress()][fScalerData->GetTimeStamp()] = fScalerData->GetScaler();
         }
      }
   }
}

TScaler::TScaler(const TScaler& rhs) : TObject()
{
   rhs.Copy(*this);
}

TScaler::~TScaler()
{
   // Clear() deletes histograms which can cause problems with root assuming ownership of all histograms
   fTree       = nullptr;
   fScalerData = nullptr;
   fEntries    = 0;
   fTimePeriod.clear();
   fNumberOfTimePeriods.clear();
   fTotalTimePeriod = 0;
   fTotalNumberOfTimePeriods.clear();
   fPPG = nullptr;
   fHist.clear();
   fHistRange.clear();
   fScalerMap.clear();
}

void TScaler::Copy(TObject& obj) const
{
   static_cast<TScaler&>(obj).Clear();
   static_cast<TScaler&>(obj).fTree                     = fTree;
   static_cast<TScaler&>(obj).fScalerData               = fScalerData;
   static_cast<TScaler&>(obj).fEntries                  = fEntries;
   static_cast<TScaler&>(obj).fTimePeriod               = fTimePeriod;
   static_cast<TScaler&>(obj).fNumberOfTimePeriods      = fNumberOfTimePeriods;
   static_cast<TScaler&>(obj).fTotalTimePeriod          = fTotalTimePeriod;
   static_cast<TScaler&>(obj).fTotalNumberOfTimePeriods = fTotalNumberOfTimePeriods;
   static_cast<TScaler&>(obj).fScalerMap                = fScalerMap;
}

std::vector<UInt_t> TScaler::GetScaler(UInt_t address, ULong64_t time) const
{
   /// Returns the vector of scaler values for address "address" at the time "time".
   /// If the time is after the last entry, we return the last entry, otherwise we return the following entry (or the
   /// current entry if the time is an exact match).
   if(fTree == nullptr || fEntries == 0) {
      printf("Empty\n");
      return std::vector<UInt_t>(0);
   }
   if(!fScalerMap.empty()) {
      // Check that this address exists
      if(fScalerMap.find(address) == fScalerMap.end()) {
         return std::vector<UInt_t>();
      }
      auto currIt = fScalerMap.find(address)->second.lower_bound(time);
      // if the time is after our last entry, we return the last entry
      if(currIt == fScalerMap.find(address)->second.end()) {
         return std::prev(fScalerMap.find(address)->second.end())->second;
      }
      // The lower_bound function returns an iterator to the NEXT map element or the current map element if the time is
      // a perfect match.
      return currIt->second;
   }
   // loop through the tree and find the right entry
   for(Long64_t entry = 0; entry < fEntries; ++entry) {
      fTree->GetEntry(entry);
      // got the exact time match, so we return the current value
      if(fScalerData->GetAddress() == address && fScalerData->GetTimeStamp() == time) {
         return fScalerData->GetScaler();
      }
      // got a scaler that is later than our requested time, so we need to find the previous time and return that one
      if(fScalerData->GetAddress() == address && fScalerData->GetTimeStamp() > time) {
         for(--entry; entry >= 0; --entry) {
            if(fScalerData->GetAddress() == address) {
               return fScalerData->GetScaler();
            }
         }
         // if we failed to find any previous data we return a vector of zeros
         return std::vector<UInt_t>(4, 0);
      }
   }
   // failed to find any matching time so we just return the very last value (or should we search for the last value of
   // this address?)
   return fScalerData->GetScaler();
}

UInt_t TScaler::GetScaler(UInt_t address, ULong64_t time, size_t index) const
{
   std::vector<UInt_t> values = GetScaler(address, time);
   if(index < values.size()) {
      return values[index];
   }
   return 0;
}

UInt_t TScaler::GetScalerDifference(UInt_t address, ULong64_t time, size_t index) const
{
   /// Returns the difference between "index"th scaler value for address "address" at the time "time" and the previous
   /// scaler value.
   /// If the time is after our last entry, we return the last entry divided by the number of entries,
   /// if this is before the first scaler, we just return the first scaler, otherwise we return the scaler minus the
   /// previous scaler.
   if(fTree == nullptr || fEntries == 0) {
      printf("Empty\n");
      return 0;
   }
   if(!fScalerMap.empty()) {
      // Check that this address exists
      if(fScalerMap.find(address) == fScalerMap.end()) {
         return 0;
      }
      auto currIt = fScalerMap.find(address)->second.lower_bound(time);
      // if the time is after our last entry, we return the last entry divided by the number of entries
      if(currIt == fScalerMap.find(address)->second.end()) {
         return std::prev(fScalerMap.find(address)->second.end())->second[index] /
                fScalerMap.find(address)->second.size();
      }
      // if this is the before or at the first scaler, we just return the first scaler
      if(currIt == fScalerMap.find(address)->second.begin()) {
         return fScalerMap.find(address)->second.begin()->second[index];
      }
      // otherwise we return the scaler minus the previous scaler
      return (currIt->second[index]) - (std::prev(currIt)->second[index]);
   }
   // loop through the tree and find the right entry
   for(Long64_t entry = 0; entry < fEntries; ++entry) {
      fTree->GetEntry(entry);
      // got the exact time match, so we look for the previous value
      if(fScalerData->GetAddress() == address && fScalerData->GetTimeStamp() == time) {
         UInt_t currentValue = fScalerData->GetScaler(index);
         for(--entry; entry >= 0; --entry) {
            if(fScalerData->GetAddress() == address) {
               return currentValue - fScalerData->GetScaler(index);
            }
         }
         return currentValue;
      }
      // got a scaler that is later than our requested time, so we need to find the previous time and return that one
      if(fScalerData->GetAddress() == address && fScalerData->GetTimeStamp() > time) {
         for(--entry; entry >= 0; --entry) {
            if(fScalerData->GetAddress() == address) {
               // got the current entry, so we look for the previous value
               UInt_t currentValue = fScalerData->GetScaler(index);
               for(--entry; entry >= 0; --entry) {
                  if(fScalerData->GetAddress() == address) {
                     return currentValue - fScalerData->GetScaler(index);
                  }
               }
               // failed to find a previous entry, so we return the current value
               return currentValue;
            }
         }
         // if we failed to find any previous data we return zero
         return 0;
      }
   }
   // failed to find any matching time, so we return 0 (???)
   return 0;
}

void TScaler::Clear(Option_t*)
{
   fTree       = nullptr;
   fScalerData = nullptr;
   fEntries    = 0;
   fTimePeriod.clear();
   fNumberOfTimePeriods.clear();
   fTotalTimePeriod = 0;
   fTotalNumberOfTimePeriods.clear();
   fPPG = nullptr;
   for(auto& addrIt : fHist) {
      if(addrIt.second != nullptr) {
         delete(addrIt.second);
         addrIt.second = nullptr;
      }
   }
   fHist.clear();
   for(auto& addrIt : fHistRange) {
      if(addrIt.second != nullptr) {
         delete(addrIt.second);
         addrIt.second = nullptr;
      }
   }
   fHistRange.clear();
   fScalerMap.clear();
}

TH1D* TScaler::Draw(UInt_t address, size_t index, Option_t* option)
{
   /// Draw scaler differences (i.e. current scaler minus last scaler) vs. time in cycle.
   /// Passing "redraw" as option forces re-drawing of the histogram (e.g. for a different index).
   if(fTree == nullptr || fEntries == 0) {
      printf("Empty\n");
      return nullptr;
   }

   // if the address doesn't exist in the histogram map, insert a null pointer
   if(fHist.find(address) == fHist.end()) {
      fHist[address] = nullptr;
   }
   // try and find the ppg (if we haven't already done so)
   if(fPPG == nullptr) {
      fPPG = TPPG::Get(); // static_cast<TPPG*>(gROOT->FindObject("TPPG"));
      // if we can't find the ppg we're done here
      if(fPPG == nullptr) {
         return nullptr;
      }
   }

   TString opt = option;
   opt.ToLower();
   // if the histogram hasn't been created yet or the "redraw" option has been given, we create the histogram
   Int_t opt_index = opt.Index("redraw");
   if(fHist[address] == nullptr || opt_index >= 0) {
      // we only need to create a new histogram if this is the first time drawing it, if we're just re-drawing we can
      // re-use the existing histogram
      if(fHist[address] == nullptr) {
         // printf("failed to find histogram for address 0x%04x\n",address);
         int nofBins = fPPG->GetCycleLength() / GetTimePeriod(address);
         fHist[address] =
            new TH1D(Form("TScalerHist_%04x", address),
                     Form("scaler %d vs time in cycle for address 0x%04x; time in cycle [ms]; counts/%.0f ms",
                          static_cast<int>(index), address, fPPG->GetCycleLength() / 1e5 / nofBins),
                     nofBins, 0., fPPG->GetCycleLength() / 1e5);
         // fHist[address]->ResetBit(kMustCleanup);
      }
      // we have to skip the first data point in case this is a sub-run
      // loop over the remaining scaler data for this address
      UInt_t previousValue = 0;
      for(Long64_t entry = 0; entry < fEntries; ++entry) {
         fTree->GetEntry(entry);
         if(fScalerData->GetAddress() == address) {
            // fill the difference between the current and the next scaler (if we found a previous value and that one is
            // smaller than the current one)
            if(previousValue != 0 && previousValue < fScalerData->GetScaler(index)) {
               fHist[address]->Fill(fPPG->GetTimeInCycle(fScalerData->GetTimeStamp()) / 1e5,
                                    fScalerData->GetScaler(index) - previousValue);
            }
            previousValue = fScalerData->GetScaler(index);
         }
         if(entry % 1000 == 0) {
            std::cout<<std::setw(3)<<(100 * entry) / fEntries<<" % done\r"<<std::flush;
         }
      }
      std::cout<<"100 % done\r"<<std::flush;
   }
   // if redraw was part of the original options, remove it from the options passed on
   if(opt_index >= 0) {
      opt.Remove(opt_index, 6);
   }

   fHist[address]->Draw(opt);

   return fHist[address];
}

TH1D* TScaler::Draw(UInt_t lowAddress, UInt_t highAddress, size_t index, Option_t* option)
{
   /// Draw scaler differences (i.e. current scaler minus last scaler) vs. time in cycle.
   /// Passing "redraw" as option forces re-drawing of the histogram (e.g. for a different index).
   /// The "single" options creates spectra for all single addresses in the given range (instead of one accumulative
   /// spectrum).
   if(fTree == nullptr || fEntries == 0) {
      printf("Empty\n");
      return nullptr;
   }

   // try and find the ppg (if we haven't already done so)
   if(fPPG == nullptr) {
      fPPG = TPPG::Get(); // static_cast<TPPG*>(gROOT->FindObject("TPPG"));
      // if we can't find the ppg we're done here
      if(fPPG == nullptr) {
         return nullptr;
      }
   }

   TString opt = option;
   opt.ToLower();
   // if the histogram hasn't been created yet or the "redraw" option has been given, we create the histogram
   Int_t draw_index   = opt.Index("redraw");
   Int_t single_index = opt.Index("single");

   if(single_index < 0) {
      // draw the whole range of addresses in a single histogram
      // if the address doesn't exist in the histogram map, insert a null pointer
      if(fHistRange.find(std::make_pair(lowAddress, highAddress)) == fHistRange.end()) {
         fHistRange[std::make_pair(lowAddress, highAddress)] = nullptr;
      }
      if(fHistRange[std::make_pair(lowAddress, highAddress)] == nullptr || draw_index >= 0) {
         // printf("failed to find histogram for address range 0x%04x - 0x%04x\n",lowAddress,highAddress);
         int nofBins =
            fPPG->GetCycleLength() / GetTimePeriod(lowAddress); // the time period should be the same for all channels
         fHistRange[std::make_pair(lowAddress, highAddress)] =
            new TH1D(Form("TScalerHist_%04x_%04x", lowAddress, highAddress),
                     Form("scaler %d vs time in cycle for address 0x%04x - 0x%04x; time in cycle [ms]; counts/%.0f ms",
                          static_cast<int>(index), lowAddress, highAddress, fPPG->GetCycleLength() / 1e5 / nofBins),
                     nofBins, 0., fPPG->GetCycleLength() / 1e5);
         // fHistRange[std::make_pair(lowAddress, highAddress)]->ResetBit(kMustCleanup);
         // we have to skip the first data point in case this is a sub-run
         // loop over the remaining scaler data for this address
         std::map<UInt_t, UInt_t> previousValue; // we could make this a vector, since we know there can only be
                                                 // highAddress-lowAddress+1 different addresses
         for(Long64_t entry = 0; entry < fEntries; ++entry) {
            fTree->GetEntry(entry);
            if(lowAddress <= fScalerData->GetAddress() && fScalerData->GetAddress() <= highAddress) {
               // fill the difference between the current and the next scaler (if we found a previous value and that one
               // is smaller than the current one)
               if(previousValue[fScalerData->GetAddress()] != 0 &&
                  previousValue[fScalerData->GetAddress()] < fScalerData->GetScaler(index)) {
                  fHistRange[std::make_pair(lowAddress, highAddress)]->Fill(
                     fPPG->GetTimeInCycle(fScalerData->GetTimeStamp()) / 1e5,
                     fScalerData->GetScaler(index) - previousValue[fScalerData->GetAddress()]);
               }
               previousValue[fScalerData->GetAddress()] = fScalerData->GetScaler(index);
            }
            if(entry % 1000 == 0) {
               std::cout<<std::setw(3)<<(100 * entry) / fEntries<<" % done\r"<<std::flush;
            }
         }
         std::cout<<"100 % done\r"<<std::flush;
      }
      // if "redraw" was part of the original options, remove it from the options passed on
      if(draw_index >= 0) {
         opt.Remove(draw_index, 6);
      }
      // if "single" was part of the original options, remove it from the options passed on
      if(single_index >= 0) {
         opt.Remove(single_index, 6);
      }

      fHistRange[std::make_pair(lowAddress, highAddress)]->Draw(opt);

      return fHistRange[std::make_pair(lowAddress, highAddress)];
   }
   // loop over all addresses and create one histogram for each
   for(UInt_t address = lowAddress; address <= highAddress; ++address) {
      // if the address doesn't exist in the histogram map, create a new histogram
      if(fHist.find(address) == fHist.end()) {
         // printf("failed to find histogram for address 0x%04x\n",address);
         int nofBins = fPPG->GetCycleLength() / GetTimePeriod(address);
         fHist[address] =
            new TH1D(Form("TScalerHist_%04x", address),
                     Form("scaler %d vs time in cycle for address 0x%04x; time in cycle [ms]; counts/%.0f ms",
                          static_cast<int>(index), address, fPPG->GetCycleLength() / 1e5 / nofBins),
                     nofBins, 0., fPPG->GetCycleLength() / 1e5);
      } else {
         // if the histogram already exist, reset it
         fHist[address]->Reset();
      }
      fHist[address]->SetLineColor(address - lowAddress + 1);
   }
   // now we have all histograms, so we loop through the tree (once) and create all that are in the range
   std::map<UInt_t, UInt_t> previousValue; // we could make this a vector, since we know there can only be
                                           // highAddress-lowAddress+1 different addresses
   for(Long64_t entry = 0; entry < fEntries; ++entry) {
      fTree->GetEntry(entry);
      if(lowAddress <= fScalerData->GetAddress() && fScalerData->GetAddress() <= highAddress) {
         // fill the difference between the current and the next scaler (if we found a previous value and that one is
         // smaller than the current one)
         if(previousValue[fScalerData->GetAddress()] != 0 &&
            previousValue[fScalerData->GetAddress()] < fScalerData->GetScaler(index)) {
            fHist[fScalerData->GetAddress()]->Fill(fPPG->GetTimeInCycle(fScalerData->GetTimeStamp()) / 1e5,
                                                   fScalerData->GetScaler(index) -
                                                      previousValue[fScalerData->GetAddress()]);
         }
         previousValue[fScalerData->GetAddress()] = fScalerData->GetScaler(index);
      }
      if(entry % 1000 == 0) {
         std::cout<<std::setw(3)<<(100 * entry) / fEntries<<" % done\r"<<std::flush;
      }
   }
   std::cout<<"100 % done\r"<<std::flush;
   Double_t max = fHist[lowAddress]->GetMaximum();
   for(UInt_t address = lowAddress + 1; address <= highAddress; ++address) {
      if(max < fHist[address]->GetMaximum()) {
         max = fHist[address]->GetMaximum();
      }
   }
   fHist[lowAddress]->GetYaxis()->SetRangeUser(0., 1.05 * max);
   fHist[lowAddress]->Draw();
   for(UInt_t address = lowAddress + 1; address <= highAddress; ++address) {
      fHist[address]->Draw("same");
   }

   return fHist[lowAddress];
}

TH1D* TScaler::DrawRawTimes(UInt_t address, Double_t lowtime, Double_t hightime, size_t index, Option_t* option)
{
   /// Draw scaler differences (i.e. current scaler minus last scaler) vs. time.
   if(fTree == nullptr || fEntries == 0) {
      printf("Empty\n");
      return nullptr;
   }

   TString opt = option;
   opt.ToLower();
   int nofBins = std::abs(static_cast<int>(1e8 * (hightime - lowtime) / GetTimePeriod(address)));
   std::cout<<nofBins<<"nofbins"<<std::endl;
   // This scHist could be leaky as the outside user has ownership of it.
   auto* scHist =
      new TH1D(Form("TScalerHistRaw_%04x", address),
               Form("scaler %d vs time for address 0x%04x; time in [ms]; counts/ ms", static_cast<int>(index), address),
               nofBins, lowtime, hightime);
   // we have to skip the first data point in case this is a sub-run
   // loop over the remaining scaler data for this address
   UInt_t previousValue = 0;
   for(Long64_t entry = 0; entry < fEntries; ++entry) {
      fTree->GetEntry(entry);
      if(fScalerData->GetAddress() == address) {
         // fill the difference between the current and the next scaler (if we found a previous value and that one is
         // smaller than the current one)
         if(previousValue != 0 && previousValue < fScalerData->GetScaler(index)) {
            scHist->Fill(fScalerData->GetTimeStamp() / 1e8, fScalerData->GetScaler(index) - previousValue);
         }
         previousValue = fScalerData->GetScaler(index);
      }
      if(entry % 1000 == 0) {
         std::cout<<std::setw(3)<<(100 * entry) / fEntries<<" % done\r"<<std::flush;
      }
   }
   std::cout<<"100 % done\r"<<std::flush;

   scHist->Draw(opt);

   return scHist;
}

ULong64_t TScaler::GetTimePeriod(UInt_t address)
{
   /// Get time period of scaler readouts for address "address" by calculating all time differences and choosing the one
   /// that occurs most often.
   /// Returns 0 if the address doesn't exist in the map.
   if(fTimePeriod[address] == 0) {
      ULong64_t previousValue = 0;
      for(Long64_t entry = 0; entry < fEntries; ++entry) {
         fTree->GetEntry(entry);
         if(fScalerData->GetAddress() == address) {
            // fill the difference between the current and the next scaler (if we found a previous value and that one is
            // smaller than the current one)
            if(previousValue != 0 && previousValue < fScalerData->GetTimeStamp()) {
               // compare timestamp of current element with that of the previous element
               ULong64_t diff = fScalerData->GetTimeStamp() - previousValue;
               fNumberOfTimePeriods[address][diff]++;
            }
            previousValue = fScalerData->GetTimeStamp();
         }
      }
      int counter = 0;
      for(auto it = fNumberOfTimePeriods[address].begin(); it != fNumberOfTimePeriods[address].end(); ++it) {
         if(it->second > counter) {
            counter              = it->second;
            fTimePeriod[address] = it->first;
         }
      }
   }

   return fTimePeriod[address];
}

void TScaler::ListHistograms()
{
   printf("single address histograms:\n");
   for(auto& it : fHist) {
      printf("\t0x%04x: %s, %s\n", it.first, it.second->GetName(), it.second->GetTitle());
   }

   printf("range histograms:\n");
   for(auto& it : fHistRange) {
      printf("\t0x%04x, %d: %s, %s\n", it.first.first, it.first.second, it.second->GetName(), it.second->GetTitle());
   }
}
