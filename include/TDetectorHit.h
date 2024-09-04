#ifndef TDETECTORHIT_H
#define TDETECTORHIT_H

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

class TDetector;

/////////////////////////////////////////////////////////////////
///
/// \class TDetectorHit
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

class TDetectorHit : public TObject {
public:
   enum class EBitFlag {
      kIsEnergySet  = BIT(0),   // same as BIT(0);
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

   enum class ETimeFlag { kNoneSet = BIT(0),
                          kCFD     = BIT(1),
                          kWalk    = BIT(2),
                          kOffset  = BIT(3),
                          kAll     = 0xFFFF };

   explicit TDetectorHit(const int& Address = 0xffffffff);
   TDetectorHit(const TDetectorHit&, bool copywave = true);
   TDetectorHit(TDetectorHit&&) noexcept = default;
   ~TDetectorHit();

   TDetectorHit& operator=(const TDetectorHit&)     = default;
   TDetectorHit& operator=(TDetectorHit&&) noexcept = default;

   // static void SetPPGPtr(TPPG* ptr) { fPPG = ptr; }

   bool operator<(const TDetectorHit& rhs) const { return GetEnergy() > rhs.GetEnergy(); }   // sorts large->small

   void                 Copy(TObject&) const override;         //!<!
   virtual void         Copy(TObject&, bool copywave) const;   //!<!
   virtual void         CopyWave(TObject&) const;              //!<!
   void                 Clear(Option_t* opt = "") override;    //!<!
   virtual void         ClearTransients() const { fBitFlags = 0; }
   void                 Print(Option_t* opt = "") const override;   //!<!
   virtual void         Print(std::ostream& out) const;
   friend std::ostream& operator<<(std::ostream& out, const TDetectorHit& hit)
   {
      hit.Print(out);
      return out;
   }
   virtual bool   HasWave() const { return !fWaveform.empty(); }   //!<!
   virtual size_t WaveSize() const { return fWaveform.size(); }    //!<!

   static bool CompareEnergy(TDetectorHit* lhs, TDetectorHit* rhs);
   // We need a common function for all detectors in here
   // static bool Compare(TDetectorHit* lhs,TDetectorHit* rhs); //!<!

   void         SetAddress(const UInt_t& temp_address) { fAddress = temp_address; }                                                               //!<!
   void         SetKValue(const Short_t& temp_kval) { fKValue = temp_kval; }                                                                      //!<!
   void         SetCharge(const Float_t& temp_charge) { fCharge = temp_charge; }                                                                  //!<!
   void         SetCharge(const Int_t& temp_charge) { fCharge = static_cast<Float_t>(temp_charge) + static_cast<Float_t>(gRandom->Uniform()); }   //!<! this function automatically randomizes the integer provided
   virtual void SetCfd(const Float_t& val) { fCfd = val; }                                                                                        //!<!
   virtual void SetCfd(const uint32_t& val) { fCfd = static_cast<Float_t>(val) + static_cast<Float_t>(gRandom->Uniform()); }                      //!<! this function automatically randomizes the integer provided
   virtual void SetCfd(const Int_t& val) { fCfd = static_cast<Float_t>(val) + static_cast<Float_t>(gRandom->Uniform()); }                         //!<! this function automatically randomizes the integer provided
   void         SetWaveform(const std::vector<Short_t>& val) { fWaveform = val; }                                                                 //!<!
   void         AddWaveformSample(const Short_t& val) { fWaveform.push_back(val); }                                                               //!<!
   virtual void SetTimeStamp(const Long64_t& val) { fTimeStamp = val; }                                                                           //!<!
   virtual void AppendTimeStamp(const Long64_t& val) { fTimeStamp += val; }                                                                       //!<!

   Double_t SetEnergy(const double& energy) const
   {
      fEnergy = energy;
      SetHitBit(EBitFlag::kIsEnergySet, true);
      return fEnergy;
   }
   Double_t SetTime(const Double_t& time) const
   {
      fTime = time;
      SetHitBit(EBitFlag::kIsTimeSet, true);
      return fTime;
   }

   virtual TVector3            GetPosition(Double_t) const { return GetPosition(); }   //!<!
   virtual TVector3            GetPosition() const { return {0., 0., 0.}; }            //!<!
   virtual double              GetEnergy(Option_t* opt = "") const;
   virtual Double_t            GetEnergyNonlinearity(double energy) const;
   virtual Long64_t            GetTimeStamp(Option_t* = "") const { return fTimeStamp; }
   virtual Long64_t            GetTimeStampNs(Option_t* opt = "") const;
   virtual Double_t            GetTime(const ETimeFlag& correct_flag = ETimeFlag::kAll,
                                       Option_t*        opt          = "") const;           ///< Returns a time value to the nearest nanosecond!
   virtual Float_t             GetCfd() const { return fCfd; }              //!<!
   virtual UInt_t              GetAddress() const { return fAddress; }      //!<!
   virtual Float_t             GetCharge() const;                           //!<!
   virtual Float_t             Charge() const { return fCharge; }           //!<!
   virtual Short_t             GetKValue() const { return fKValue; }        //!<!
   const std::vector<Short_t>* GetWaveform() const { return &fWaveform; }   //!<!
   TChannel*                   GetChannel() const
   {
      if(!IsChannelSet()) {
         fChannel = TChannel::GetChannel(fAddress, false);
         SetHitBit(EBitFlag::kIsChannelSet, true);
      }
      return fChannel;
   }   //!<!

   // stored in the tchannel (things common to all hits of this address)
   virtual int      GetChannelNumber() const;                          //!<!
   virtual Int_t    GetDetector() const;                               //!<!
   virtual Int_t    GetSegment() const;                                //!<!
   virtual Int_t    GetCrystal() const;                                //!<!
   const char*      GetName() const override;                          //!<!
   virtual UShort_t GetArrayNumber() const { return GetDetector(); }   //!<! Simply returns the detector number, overwritten for detectors that have crystals/segments
   virtual Int_t    GetTimeStampUnit() const;                          //!<!

   // The PPG is only stored in events that come out of the GRIFFIN DAQ
   EPpgPattern GetPPGStatus() const;
   Long64_t    GetCycleTimeStamp() const;
   double      GetTimeSinceTapeMove() const;

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

   virtual void Add(const TDetectorHit*) {}   //!<!

   void SetHitBit(EBitFlag, Bool_t set = true) const;   // const here is dirty
   bool TestHitBit(EBitFlag flag) const { return fBitFlags.TestBit(flag); }

protected:
   Bool_t IsEnergySet() const { return (fBitFlags.TestBit(EBitFlag::kIsEnergySet)); }
   Bool_t IsChannelSet() const { return (fBitFlags.TestBit(EBitFlag::kIsChannelSet)); }
   Bool_t IsTimeSet() const { return (fBitFlags.TestBit(EBitFlag::kIsTimeSet)); }
   Bool_t IsPPGSet() const { return (fBitFlags.TestBit(EBitFlag::kIsPPGSet)); }

private:
   UInt_t               fAddress{0};     ///< address of the the channel in the DAQ.
   Float_t              fCharge{0.};     ///< charge collected from the hit
   Short_t              fKValue{0};      ///< integration value.
   Float_t              fCfd{0};         ///< CFD time of the Hit
   Long64_t             fTimeStamp{0};   ///< Timestamp given to hit in ns
   std::vector<Short_t> fWaveform;       ///<
   mutable Double_t     fTime{0.};       //!<! Calibrated Time of the hit

   mutable Double_t    fEnergy{0.};                      //!<! Energy of the Hit.
   mutable EPpgPattern fPPGStatus{EPpgPattern::kJunk};   //!<!

   mutable Long64_t  fCycleTimeStamp{0};   //!<!
   mutable TChannel* fChannel{nullptr};    //!<!

   //  flags
   mutable TTransientBits<UChar_t> fBitFlags;

   static TVector3 fBeamDirection;   //!

   /// \cond CLASSIMP
   ClassDefOverride(TDetectorHit, 1)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
