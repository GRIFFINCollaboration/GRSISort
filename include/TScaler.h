#ifndef TSCALER_H
#define TSCALER_H

/** \addtogroup Sorting
 *  @{
 */

/*
 * Author:  V. Bildstein, <vbildste@uoguelph.ca>
 *
 * Based on the TPPG class
 *
 * Please indicate changes with your initials.
 *
 */

//////////////////////////////////////////////////////////////////////////
///
/// \class TScaler
///
/// The TScaler is designed to hold all of the information about the
/// scaler status.
///
//////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <map>

#include "TObject.h"
#include "Globals.h"
#include "TCollection.h"
#include "TTree.h"
#include "TH1D.h"

#include "TPPG.h"

class TScalerData : public TObject {
public:
   TScalerData();
   TScalerData(const TScalerData&);
   ~TScalerData() override = default;

   void Copy(TObject& rhs) const override;

   void SetAddress(UInt_t address) { fAddress = address; }
   void SetNetworkPacketId(UInt_t networkId) { fNetworkPacketId = networkId; }
   void SetLowTimeStamp(UInt_t lowTime) { fLowTimeStamp = lowTime; }
   void SetHighTimeStamp(UInt_t highTime) { fHighTimeStamp = highTime; }
   void SetScaler(size_t index, UInt_t scaler)
   {
      if(index < fScaler.size()) {
         fScaler[index] = scaler;
      } else {
         std::cout << "Failed to set scaler " << scaler << ", index " << index << " is out of range 0 - "
                   << fScaler.size() << std::endl;
      }
   }
   void SetScaler(UInt_t* data, int size)
   {
      fScaler = std::vector<uint32_t>(data, data + size);
   }

   UInt_t              GetAddress() const { return fAddress; }
   UInt_t              GetNetworkPacketId() const { return fNetworkPacketId; }
   UInt_t              GetLowTimeStamp() const { return fLowTimeStamp; }
   UInt_t              GetHighTimeStamp() const { return fHighTimeStamp; }
   std::vector<UInt_t> GetScaler() const { return fScaler; }
   UInt_t              GetScaler(size_t index) const
   {
      if(index < fScaler.size()) {
         return fScaler[index];
      }
		return 0;
   }

   ULong64_t GetTimeStamp() const
   {
      ULong64_t time = GetHighTimeStamp();
      time           = time << 28;
      time |= GetLowTimeStamp() & 0x0fffffff;
      return 10 * time;   // convert from raw 10 ns units to ns
   }

   void ResizeScaler(size_t newSize = 1) { fScaler.resize(newSize); }

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

private:
   UInt_t              fNetworkPacketId{0};
   UInt_t              fAddress{0};
   std::vector<UInt_t> fScaler;
   UInt_t              fLowTimeStamp{0};
   UInt_t              fHighTimeStamp{0};

   /// \cond CLASSIMP
   ClassDefOverride(TScalerData, 2)   // Contains scaler data information
   /// \endcond
};

class TScaler : public TObject {
public:
   TScaler(bool loadIntoMap = false);
   TScaler(TTree*, bool loadIntoMap = false);
   TScaler(const TScaler&);
   ~TScaler() override;

   void Copy(TObject& obj) const override;

   std::vector<UInt_t> GetScaler(UInt_t address, ULong64_t time) const;
   UInt_t              GetScaler(UInt_t address, ULong64_t time, size_t index) const;
   UInt_t              GetScalerDifference(UInt_t address, ULong64_t time, size_t index) const;
   ULong64_t           NumberOfScalerReadouts() const { return fEntries; };

   ULong64_t GetTimePeriod(UInt_t address);

   std::map<UInt_t, ULong64_t>                GetTimePeriodMap() { return fTimePeriod; }
   std::map<UInt_t, std::map<ULong64_t, int>> GetNumberOfTimePeriods() { return fNumberOfTimePeriods; }

   void Clear(Option_t* opt = "") override;
   using TObject::Draw;   // This is to remove hidden overload
   TH1D* Draw(UInt_t address, size_t index = 0, Option_t* option = "");
   TH1D* Draw(UInt_t lowAddress, UInt_t highAddress, size_t index = 0, Option_t* option = "");
   TH1D* DrawRawTimes(UInt_t address, Double_t lowtime, Double_t hightime, size_t index = 0, Option_t* option = "");

   void ListHistograms();

   static double GetLastScaler(TTree* tree = nullptr, UInt_t address = 0xffff, size_t nominator = 2, size_t denominator = 1)
   {
      /// This function returns the ratio of the two scalers nominator and denominator for the last entry with a matching address for a given tree.
      /// If no tree is provided it tries to get the "RateScaler" tree from the current directory.
      if(tree == nullptr) {
         tree = static_cast<TTree*>(gDirectory->Get("RateScaler"));
         if(tree == nullptr) {
            std::cerr << __PRETTY_FUNCTION__ << ": no tree provided and failed to find \"RateScaler\" in " << gDirectory->GetName() << std::endl;
            return -1.;
         }
      }
      TScalerData* scalerData = nullptr;
      tree->SetBranchAddress("ScalerData", &scalerData);

      for(Long64_t entry = tree->GetEntries() - 1; entry >= 0; --entry) {
         tree->GetEntry(entry);
         if(scalerData->GetAddress() != address) { continue; }
         auto scalers = scalerData->GetScaler();
         if(nominator >= scalers.size() || denominator >= scalers.size()) {
            std::cerr << __PRETTY_FUNCTION__ << ": trying to get nominator " << nominator << " and denominator " << denominator << " from vector of size " << scalers.size() << std::endl;
            return -1.;
         }
         return static_cast<double>(scalers[nominator]) / scalers[denominator];
      }
      std::cerr << __PRETTY_FUNCTION__ << ": failed to find any entry for address " << hex(address, 4) << std::endl;
      return -1.;
   }

private:
   void ReadTree(bool loadIntoMap);

   TTree*                                                     fTree;
   TScalerData*                                               fScalerData;
   Long64_t                                                   fEntries;
   std::map<UInt_t, std::map<ULong64_t, std::vector<UInt_t>>> fScalerMap;                  //!<! an address-map of timestamp mapped scaler values
   std::map<UInt_t, ULong64_t>                                fTimePeriod;                 //!<! a map between addresses and time differences (used to calculate the time period)
   std::map<UInt_t, std::map<ULong64_t, int>>                 fNumberOfTimePeriods;        //!<!
   ULong64_t                                                  fTotalTimePeriod;            //!<!
   std::map<ULong64_t, int>                                   fTotalNumberOfTimePeriods;   //!<!
   TPPG*                                                      fPPG;                        //!<!
   std::map<UInt_t, TH1D*>                                    fHist;                       //!<! map to store histograms that have already been drawn
   std::map<std::pair<UInt_t, UInt_t>, TH1D*>                 fHistRange;                  //!<! map to store histograms for address-ranges

   /// \cond CLASSIMP
   ClassDefOverride(TScaler, 2)   // Contains scaler information
                                  /// \endcond
};
/*! @} */
#endif
