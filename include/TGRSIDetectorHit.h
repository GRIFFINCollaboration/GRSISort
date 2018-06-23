#ifndef TGRSIDETECTORHIT_H
#define TGRSIDETECTORHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>

#include "Globals.h"

#include "TChannel.h"
#include "TVector3.h"
#include "TObject.h"
#include "TRef.h"
#include "Rtypes.h"
#include "TFile.h"
#include "TString.h"

#include "TPPG.h"
#include "TTransientBits.h"

class TGRSIDetector;

/////////////////////////////////////////////////////////////////
///
/// \class TGRSIDetectorHit
///
/// This is class that contains the basic info about detector
/// hits:
/// 1. An address.         The whoami for the detector. This is the value used by TChannel::GetChannel(address);
///
/// 2. An "Energy value."  What this is left the parent class, but it is going to return a double.
///
/// 3. A   Time  value.     This should be a time value common for detectors (derived from the timestamp)
///                        Units matter here, I am adobting the ns as the standard.
///
/// 4. A   Position.       Tvector3s are nice, they make doing geometery trival.  Each hit needs one to determine
///                        where it is in space, the actual memory of the thing will be stored here.
///                        ***  This is not actually needed here unless we start do waveform analysis to
///                        ***  better determine where the hit is.
///
/// 5. The waveform.       Since we are dealing with digital daqs, a waveform is a fairly common thing to have.  It
///                        may not always be present, put it is echoed enough that the storage for it belongs here.
///
/////////////////////////////////////////////////////////////////

class TGRSIDetectorHit : public TObject {

   //
public:
   enum class EBitFlag {
      kIsEnergySet  = BIT(0), // same as BIT(0);
      kIsChannelSet = BIT(1),
      kBit2         = BIT(2),
      kBit3         = BIT(3),
      kIsPPGSet     = BIT(4),
      kIsTimeSet    = BIT(5),
      kBit6         = BIT(6),
      kBit7         = BIT(7),
      kBit8         = BIT(8),
      // reserved for derived class.
      kDetHitBit0 = BIT(9),
      kDetHitBit1 = BIT(10),
      kDetHitBit2 = BIT(11),
      kDetHitBit3 = BIT(12),
      kDetHitBit4 = BIT(13),
      kDetHitBit5 = BIT(14),
      kDetHitBit6 = BIT(15),
      kBase       = BIT(9),
      kIsAllSet   = 0xFFFF
   };
	//EBitFlag operator &(EBitFlag lhs, EBitFlag rhs)
	//{
	//	return static_cast<EBitFlag>(static_cast<std::underlying_type<EBitFlag>::type>(lhs) &
	//		                          static_cast<std::underlying_type<EBitFlag>::type>(rhs));
	//}

   enum class ETimeFlag { kNoneSet = BIT(0), kCFD = BIT(1), kWalk = BIT(2), kOffset = BIT(3), kAll = 0xFFFF };

public:
   TGRSIDetectorHit(const int& Address = 0xffffffff);
   TGRSIDetectorHit(const TGRSIDetectorHit&, bool copywave = true);
   // TGRSIDetectorHit(const TFragment& frag) { Class()->IgnoreTObjectStreamer(); this->CopyFragment(frag); }
   // void CopyFragment(const TFragment&);
   // void CopyWaveform(const TFragment&);
   ~TGRSIDetectorHit() override;

   static void SetPPGPtr(TPPG* ptr) { fPPG = ptr; }

   bool operator<(const TGRSIDetectorHit& rhs) const { return GetEnergy() > rhs.GetEnergy(); } // sorts large->small

public:
   void         Copy(TObject&) const override;       //!<!
   virtual void Copy(TObject&, bool copywave) const; //!<!
   virtual void CopyWave(TObject&) const;            //!<!
   void Clear(Option_t* opt = "") override;          //!<!
   virtual void ClearTransients() const { fBitflags = 0; }
   void Print(Option_t* opt = "") const override;                                 //!<!
   virtual bool HasWave() const { return (fWaveform.size() > 0) ? true : false; } //!<!

   static bool CompareEnergy(TGRSIDetectorHit* lhs, TGRSIDetectorHit* rhs);
   // We need a common function for all detectors in here
   // static bool Compare(TGRSIDetectorHit* lhs,TGRSIDetectorHit* rhs); //!<!

   void SetAddress(const UInt_t& temp_address) { fAddress = temp_address; }                 //!<!
   void SetKValue(const Short_t& temp_kval) { fKValue = temp_kval; }                        //!<!
   void SetCharge(const Float_t& temp_charge) { fCharge = temp_charge; }                    //!<!
   void SetCharge(const Int_t& temp_charge) { fCharge = temp_charge + gRandom->Uniform(); } //!<!
   virtual void SetCfd(const Int_t& x) { fCfd = x; }                                        //!<!
   void SetWaveform(const std::vector<Short_t>& x) { fWaveform = x; }                       //!<!
   void AddWaveformSample(const Short_t& x) { fWaveform.push_back(x); }                     //!<!
   virtual void SetTimeStamp(const Long64_t& x) { fTimeStamp = x; }                         //!<!
   virtual void AppendTimeStamp(const Long64_t& x) { fTimeStamp += x; }                     //!<!

   Double_t SetEnergy(const double& en) const
   {
      fEnergy = en;
      SetHitBit(EBitFlag::kIsEnergySet, true);
      return fEnergy;
   }
   Double_t SetTime(const Double_t& time) const
   {
      fTime = time;
      SetHitBit(EBitFlag::kIsTimeSet, true);
      return fTime;
   }

   virtual TVector3 GetPosition(Double_t) const { return TVector3(0., 0., 0.); } //!<!
   virtual TVector3 GetPosition() const { return TVector3(0., 0., 0.); }         //!<!
   virtual double GetEnergy(Option_t* opt = "") const;
   virtual Long64_t GetTimeStamp(Option_t* opt = "") const;
   Long64_t         GetRawTimeStamp(Option_t* = "") const { return fTimeStamp; }
   virtual Double_t GetTime(const ETimeFlag& correct_flag = ETimeFlag::kAll,
                            Option_t*     opt          = "") const; ///< Returns a time value to the nearest nanosecond!
   // TODO: Fix Getters to have non-const types
   virtual Int_t               GetCfd() const { return fCfd; }            //!<!
   virtual UInt_t              GetAddress() const { return fAddress; }    //!<!
   virtual Float_t             GetCharge() const;                         //!<!
   virtual Float_t             Charge() const { return fCharge; }         //!<!
   virtual Short_t             GetKValue() const { return fKValue; }      //!<!
   const std::vector<Short_t>* GetWaveform() const { return &fWaveform; } //!<!
   TChannel*                   GetChannel() const
   {
      if(!IsChannelSet()) {
         fChannel = TChannel::GetChannel(fAddress);
         SetHitBit(EBitFlag::kIsChannelSet, true);
      }
      return fChannel;
   } //!<!

   // stored in the tchannel (things common to all hits of this address)
   virtual Int_t    GetDetector() const;      //!<!
   virtual Int_t    GetSegment() const;       //!<!
   virtual Int_t    GetCrystal() const;       //!<!
   const char*      GetName() const override; //!<!
   virtual UShort_t GetArrayNumber() const;   //!<!

   // virtual void GetSegment() const;

   virtual Double_t GetEnergyNonlinearity(double) const { return 0.0; }

   // The PPG is only stored in events that come out of the GRIFFIN DAQ
	EPpgPattern GetPPGStatus() const;
   Long64_t GetCycleTimeStamp() const;

   void ClearEnergy()
   {
      fEnergy = 0.0;
      SetHitBit(EBitFlag::kIsEnergySet, false);
   }
   void ClearChannel()
   {
      fChannel = nullptr;
      SetHitBit(EBitFlag::kIsChannelSet, false);
   }

   static TVector3* GetBeamDirection() { return &fBeamDirection; }

   virtual void Add(const TGRSIDetectorHit*) {} //!<!
private:
   //     virtual TVector3 GetChannelPosition(Double_t dist) const { AbstractMethod("GetChannelPosition"); return
   //     TVector3(0., 0., 0.); }

protected:
   Bool_t IsEnergySet() const { return (fBitflags.TestBit(EBitFlag::kIsEnergySet)); }
   Bool_t IsChannelSet() const { return (fBitflags.TestBit(EBitFlag::kIsChannelSet)); }
   Bool_t IsTimeSet() const { return (fBitflags.TestBit(EBitFlag::kIsTimeSet)); }
   Bool_t IsPPGSet() const { return (fBitflags.TestBit(EBitFlag::kIsPPGSet)); }

public:
   void SetHitBit(EBitFlag, Bool_t set = true) const; // const here is dirty
   bool TestHitBit(EBitFlag flag) const { return fBitflags.TestBit(flag); }

protected:
   UInt_t               fAddress{0};   ///< address of the the channel in the DAQ.
   Float_t              fCharge{0.};   ///< charge collected from the hit
   Short_t              fKValue{0};    ///< integration value.
   Int_t                fCfd{0};       ///< CFD time of the Hit
   Long64_t             fTimeStamp{0}; ///< Timestamp given to hit
   std::vector<Short_t> fWaveform;     ///<

private:
   mutable Double_t fTime{0.}; //!<! Calibrated Time of the hit
   mutable Double_t fEnergy{0.};    //!<! Energy of the Hit.
   mutable EPpgPattern fPPGStatus{EPpgPattern::kJunk}; //!<!

   mutable Long64_t  fCycleTimeStamp{0}; //!<!
   mutable TChannel* fChannel{nullptr};        //!<!

protected:
   static TPPG* fPPG;

private:
   // flags
   mutable TTransientBits<UChar_t> fBitflags;
   static TVector3                 fBeamDirection; //!

   /// \cond CLASSIMP
   ClassDefOverride(TGRSIDetectorHit, 10) // Stores the information for a detector hit
   /// \endcond
};
/*! @} */
#endif
