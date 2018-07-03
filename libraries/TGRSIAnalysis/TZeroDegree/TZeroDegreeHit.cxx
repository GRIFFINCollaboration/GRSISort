#include "TZeroDegreeHit.h"

#include <iostream>
#include <algorithm>
#include <climits>

#include "Globals.h"
#include "TZeroDegree.h"
#include "TGRSIOptions.h"
#include "TChannel.h"
#include "GValue.h"

/// \cond CLASSIMP
ClassImp(TZeroDegreeHit)
/// \endcond

TZeroDegreeHit::TZeroDegreeHit()
{
// Default Constructor
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TZeroDegreeHit::~TZeroDegreeHit() = default;

TZeroDegreeHit::TZeroDegreeHit(const TFragment& frag) : TGRSIDetectorHit(frag)
{
   if(TGRSIOptions::Get()->ExtractWaves()) {
      if(frag.GetWaveform()->empty()) {
         printf("Warning, TZeroDegree::SetWave() set, but data waveform size is zero!\n");
      } else {
         frag.CopyWave(*this);
      }
      if(!GetWaveform()->empty()) {
         AnalyzeWaveform();
      }
   }
}

TZeroDegreeHit::TZeroDegreeHit(const TZeroDegreeHit& rhs) : TGRSIDetectorHit()
{
// Copy Constructor
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
   rhs.Copy(*this);
}

void TZeroDegreeHit::Copy(TObject& rhs) const
{
   /// Copies a TZeroDegreeHit
   TGRSIDetectorHit::Copy(rhs);
   if(TGRSIOptions::Get()->ExtractWaves()) {
      TGRSIDetectorHit::CopyWave(rhs);
   }
   static_cast<TZeroDegreeHit&>(rhs).fFilter     = fFilter;
   static_cast<TZeroDegreeHit&>(rhs).fCfdMonitor = fCfdMonitor;
   static_cast<TZeroDegreeHit&>(rhs).fPartialSum = fPartialSum;
}

void TZeroDegreeHit::Copy(TObject& obj, bool waveform) const
{
   Copy(obj);
   if(waveform) {
      CopyWave(obj);
   }
}

bool TZeroDegreeHit::InFilter(Int_t)
{
   /// check if the desired filter is in wanted filter;
   /// return the answer;
   return true;
}

Int_t TZeroDegreeHit::GetCfd() const
{
   /// special function for TZeroDegreeHit to return CFD after mapping out the high bits
   /// which are the remainder between the 125 MHz data and the 100 MHz timestamp clock
   return fCfd & 0x3fffff;
}

Int_t TZeroDegreeHit::GetRemainder() const
{
   /// returns the remainder between 100 MHz/10ns timestamp and 125 MHz/8 ns data in ns
   return fCfd >> 22;
}

void TZeroDegreeHit::Clear(Option_t*)
{
   /// Clears the ZeroDegreeHit
   fFilter = 0;
   TGRSIDetectorHit::Clear();
   fCfdMonitor.clear();
   fPartialSum.clear();
}

void TZeroDegreeHit::Print(Option_t*) const
{
   ////Prints the ZeroDegreeHit. Returns:
   ////Detector
   ////Energy
   ////Time
   printf("ZeroDegree Detector: %i\n", GetDetector());
   printf("ZeroDegree hit energy: %.2f\n", GetEnergy());
   printf("ZeroDegree hit time:   %.lf\n", GetTime());
}

bool TZeroDegreeHit::AnalyzeWaveform()
{
   /// Calculates the cfd time from the waveform
   bool error = false;
   if(fWaveform.empty()) {
      return false; // Error!
   }

   std::vector<Int_t>   baselineCorrections(8, 0);
   std::vector<Short_t> smoothedWaveform;

   // all timing algorithms use interpolation with this many steps between two samples (all times are stored as
   // integers)
   unsigned int interpolationSteps  = 256;
   int          delay               = 2;
   double       attenuation         = 24. / 64.;
   int          halfsmoothingwindow = 0; // 2*halfsmoothingwindow + 1 = number of samples in moving window.

   // baseline algorithm: correct each adc with average of first two samples in that adc
   for(size_t i = 0; i < 8 && i < fWaveform.size(); ++i) {
      baselineCorrections[i] = fWaveform[i];
   }
   for(size_t i = 8; i < 16 && i < fWaveform.size(); ++i) {
      baselineCorrections[i - 8] =
         ((baselineCorrections[i - 8] + fWaveform[i]) + ((baselineCorrections[i - 8] + fWaveform[i]) > 0 ? 1 : -1)) >>
         1;
   }
   for(size_t i = 0; i < fWaveform.size(); ++i) {
      fWaveform[i] -= baselineCorrections[i % 8];
   }

   SetCfd(CalculateCfd(attenuation, delay, halfsmoothingwindow, interpolationSteps));

   SetCharge(CalculatePartialSum().back());

   return !error;
}

Int_t TZeroDegreeHit::CalculateCfd(double attenuation, unsigned int delay, int halfsmoothingwindow,
                                   unsigned int interpolationSteps)
{
   /// Used when calculating the CFD from the waveform
   std::vector<Short_t> monitor;

   return CalculateCfdAndMonitor(attenuation, delay, halfsmoothingwindow, interpolationSteps, monitor);
}

Int_t TZeroDegreeHit::CalculateCfdAndMonitor(double attenuation, unsigned int delay, int halfsmoothingwindow,
                                             unsigned int interpolationSteps, std::vector<Short_t>& monitor)
{
   /// Used when calculating the CFD from the waveform

   Short_t monitormax = 0;

   bool armed = false;

   Int_t cfd = 0;

   std::vector<Short_t> smoothedWaveform;

   if(fWaveform.empty()) {
      return INT_MAX; // Error!
   }

   if(static_cast<unsigned int>(fWaveform.size()) > delay + 1) {

      if(halfsmoothingwindow > 0) {
         smoothedWaveform = TZeroDegreeHit::CalculateSmoothedWaveform(halfsmoothingwindow);
      } else {
         smoothedWaveform = fWaveform;
      }

      monitor.resize(smoothedWaveform.size() - delay);
      monitor[0] = attenuation * smoothedWaveform[delay] - smoothedWaveform[0];
      if(monitor[0] > monitormax) {
         armed      = true;
         monitormax = monitor[0];
      }

      for(unsigned int i = delay + 1; i < smoothedWaveform.size(); ++i) {
         monitor[i - delay] = attenuation * smoothedWaveform[i] - smoothedWaveform[i - delay];
         if(monitor[i - delay] > monitormax) {
            armed      = true;
            monitormax = monitor[i - delay];
         } else {
            if(armed && monitor[i - delay] < 0) {
               armed = false;
               if(monitor[i - delay - 1] - monitor[i - delay] != 0) {
                  // Linear interpolation.
                  cfd = (i - delay - 1) * interpolationSteps +
                        (monitor[i - delay - 1] * interpolationSteps) / (monitor[i - delay - 1] - monitor[i - delay]);
               } else {
                  // Should be impossible, since monitor[i-delay-1] => 0 and monitor[i-delay] > 0
                  cfd = 0;
               }
            }
         }
      }
   } else {
      monitor.resize(0);
   }

   if(TGRSIOptions::Get()->Debug()) {
      fCfdMonitor = monitor;
   }

   // correct for remainder between the 100MHz timestamp and the 125MHz start of the waveform
   // we save this in the upper bits, otherwise we can't correct the waveform themselves
   cfd = (cfd & 0x3fffff) | (fCfd & 0x7c00000);

   return cfd;
}

std::vector<Short_t> TZeroDegreeHit::CalculateSmoothedWaveform(unsigned int halfsmoothingwindow)
{
   /// Used when calculating the CFD from the waveform

   if(fWaveform.empty()) {
      return std::vector<Short_t>(); // Error!
   }

   std::vector<Short_t> smoothedWaveform(std::max(static_cast<size_t>(0), fWaveform.size() - 2 * halfsmoothingwindow),
                                         0);

   for(size_t i = halfsmoothingwindow; i < fWaveform.size() - halfsmoothingwindow; ++i) {
      for(int j = -static_cast<int>(halfsmoothingwindow); j <= static_cast<int>(halfsmoothingwindow); ++j) {
         smoothedWaveform[i - halfsmoothingwindow] += fWaveform[i + j];
      }
   }

   return smoothedWaveform;
}

std::vector<Short_t> TZeroDegreeHit::CalculateCfdMonitor(double attenuation, int delay, int halfsmoothingwindow)
{
   /// Used when calculating the CFD from the waveform

   if(fWaveform.empty()) {
      return std::vector<Short_t>(); // Error!
   }

   std::vector<Short_t> smoothedWaveform;

   if(halfsmoothingwindow > 0) {
      smoothedWaveform = TZeroDegreeHit::CalculateSmoothedWaveform(halfsmoothingwindow);
   } else {
      smoothedWaveform = fWaveform;
   }

   std::vector<Short_t> monitor(std::max(static_cast<size_t>(0), smoothedWaveform.size() - delay), 0);

   for(size_t i = delay; i < smoothedWaveform.size(); ++i) {
      monitor[i - delay] = attenuation * smoothedWaveform[i] - smoothedWaveform[i - delay];
   }

   return monitor;
}

std::vector<Int_t> TZeroDegreeHit::CalculatePartialSum()
{

   if(fWaveform.empty()) {
      return std::vector<Int_t>(); // Error!
   }

   std::vector<Int_t> partialSums(fWaveform.size(), 0);

   if(!fWaveform.empty()) {
      partialSums[0] = fWaveform.at(0);
      for(size_t i = 1; i < fWaveform.size(); ++i) {
         partialSums[i] = partialSums[i - 1] + fWaveform[i];
      }
   }

   if(TGRSIOptions::Get()->Debug()) {
      fPartialSum = partialSums;
   }

   return partialSums;
}
