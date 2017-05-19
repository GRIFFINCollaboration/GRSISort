#ifndef DESCANTHIT_H
#define DESCANTHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <cmath>
#include <vector>

#include "TVector3.h"

#include "TFragment.h"
#include "TChannel.h"

#include "TGRSIDetectorHit.h"

class TDescantHit : public TGRSIDetectorHit {
public:
   TDescantHit();
   virtual ~TDescantHit();
   TDescantHit(const TDescantHit&);
   TDescantHit(const TFragment& frag);

private:
   Int_t              fFilter;
   Int_t              fPsd;
   Int_t              fZc;
   Int_t              fCcShort;
   Int_t              fCcLong;
   std::vector<short> fCfdMonitor;
   std::vector<int>   fPartialSum;

public:
   /////////////////////////		/////////////////////////////////////
   inline void SetFilterPattern(const int& x) { fFilter = x; } //!<!
   inline void SetPsd(const int& x) { fPsd = x; }              //!<!
   inline void SetZc(const int& x) { fZc = x; }                //!<!
   inline void SetCcShort(const int& x) { fCcShort = x; }      //!<!
   inline void SetCcLong(const int& x) { fCcLong = x; }        //!<!

   // 4G 26bit CFD word: 4 bit timestamp remainder, 22 bit actual cfd word
   // inline void SetCfd(const Int_t& x)           { fCfd    = (fCfd & 0x70000000) | (x & 0x1fffffff); } //!<!

   /////////////////////////		/////////////////////////////////////
   inline Int_t               GetFilterPattern() { return fFilter; } //!<!
   inline Int_t               GetPsd() { return fPsd; }              //!<!
   inline Int_t               GetZc() { return fZc; }                //!<!
   inline Int_t               GetCcShort() { return fCcShort; }      //!<!
   inline Int_t               GetCcLong() { return fCcLong; }        //!<!
   inline std::vector<short>& GetCfdMonitor() { return fCfdMonitor; }
   inline std::vector<int>&   GetPartialSum() { return fPartialSum; }

   Int_t    GetCfd() const;
   Int_t    GetRemainder() const;
   Double_t GetTime(const UInt_t& correction_flag = ETimeFlag::kAll,
                    Option_t* opt = "") const; ///< Returns a time value using the CFD with 1/256 ns intrinsic binning
   Double_t GetCorrectedTime()
      const; ///< Returns a time value using the CFD with 1/256 ns intrinsic binning, corrected using GValue

   Int_t CalculateCfd(double attenuation, unsigned int delay, int halfsmoothingwindow,
                      unsigned int interpolation_steps); //!<!
   Int_t CalculateCfdAndMonitor(double attenuation, unsigned int delay, int halfsmoothingwindow,
                                unsigned int interpolation_steps, std::vector<Short_t>& monitor); //!<!
   std::vector<Short_t> CalculateCfdMonitor(double attenuation, unsigned int delay,
                                            unsigned int halfSmoothingWindow);       //!<!
   std::vector<Short_t> CalculateSmoothedWaveform(unsigned int halfsmoothingwindow); //!<!
   std::vector<Int_t> CalculatePartialSum();                                         //!<!
   Int_t CalculatePsd(double fraction, unsigned int interpolation_steps);            //!<!
   Int_t CalculatePsdAndPartialSums(double fraction, unsigned int interpolation_steps,
                                    std::vector<Int_t>& partialsums); //!<!

   bool InFilter(Int_t); //!<!

   bool AnalyzeWaveform(); //!<!

   TVector3 GetPosition(Double_t dist) const; //!<!
   TVector3 GetPosition() const;              //!<!

public:
   void Copy(TObject&) const;            //!<!
   void Copy(TObject&, bool) const;      //!<!
   void Clear(Option_t* opt = "");       //!<!
   void Print(Option_t* opt = "") const; //!<!

private:
   Double_t GetDefaultDistance() const { return 222.; }

   /// \cond CLASSIMP
   ClassDef(TDescantHit, 5)
   /// \endcond
};
/*! @} */
#endif
