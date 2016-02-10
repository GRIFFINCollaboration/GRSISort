#include "TZeroDegreeHit.h"

#include <iostream>
#include <algorithm>
#include <climits>

#include "Globals.h"
#include "TZeroDegree.h"

/// \cond CLASSIMP
ClassImp(TZeroDegreeHit)
/// \endcond

TZeroDegreeHit::TZeroDegreeHit()	{
   //Default Constructor
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TZeroDegreeHit::~TZeroDegreeHit()	{	}

TZeroDegreeHit::TZeroDegreeHit(const TZeroDegreeHit &rhs) : TGRSIDetectorHit() {
   //Copy Constructor
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
   rhs.Copy(*this);
}

void TZeroDegreeHit::Copy(TObject &rhs) const {
  ///Copies a TZeroDegreeHit
   TGRSIDetectorHit::Copy(rhs);
   static_cast<TZeroDegreeHit&>(rhs).fFilter = fFilter;
}

TVector3 TZeroDegreeHit::GetChannelPosition(double dist) const {
   ///Gets the position of the current TZeroDegreeHit
   ///This position returns is of the center of the paddle
   ///This should not be called externally, only TGRSIDetector::GetPosition should be
   return TZeroDegree::GetPosition(dist);
}

bool TZeroDegreeHit::InFilter(Int_t wantedfilter) {
	/// check if the desired filter is in wanted filter;
   /// return the answer;
   return true;
}

void TZeroDegreeHit::Clear(Option_t *opt)	{
   ///Clears the ZeroDegreeHit
   fFilter = 0;
   TGRSIDetectorHit::Clear();
}

void TZeroDegreeHit::Print(Option_t *opt) const	{
   ////Prints the ZeroDegreeHit. Returns:
   ////Detector
   ////Energy
   ////Time
   printf("ZeroDegree Detector: %i\n",GetDetector());
   printf("ZeroDegree hit energy: %.2f\n",GetEnergy());
   printf("ZeroDegree hit time:   %.lf\n",GetTime());
}

Double_t TZeroDegreeHit::GetTime() const {
  ///returns time using the remainder (difference between 100 MHz timestamp and 125 MHz clock of FPGA), and the CFD word
  return 10.*fTimeStamp + GetRemainder() + (GetCfd() + gRandom->Uniform())/256.;
}

bool TZeroDegreeHit::AnalyzeWaveform() {
   ///Calculates the cfd time from the waveform
   bool error = false;
   std::vector<Short_t> *waveform = GetWaveform();
   if(waveform->empty()) {
      return false; //Error!
   }
   
   std::vector<Int_t> baselineCorrections (8, 0);
   std::vector<Short_t> smoothedWaveform;
   
   // all timing algorithms use interpolation with this many steps between two samples (all times are stored as integers)
   unsigned int interpolationSteps = 256;
   int delay = 8;
   double attenuation = 24./64.;
   int halfsmoothingwindow = 0; //2*halfsmoothingwindow + 1 = number of samples in moving window.
   
   // baseline algorithm: correct each adc with average of first two samples in that adc
   for(size_t i = 0; i < 8 && i < waveform->size(); ++i) {
      baselineCorrections[i] = (*waveform)[i];
   }
   for(size_t i = 8; i < 16 && i < waveform->size(); ++i) {
      baselineCorrections[i-8] = ((baselineCorrections[i-8] + (*waveform)[i]) + ((baselineCorrections[i-8] + (*waveform)[i]) > 0 ? 1 : -1)) >> 1;
   }
   for(size_t i = 0; i < waveform->size(); ++i) {
      (*waveform)[i] -= baselineCorrections[i%8];
   }
   
   this->SetCfd(CalculateCfd(attenuation, delay, halfsmoothingwindow, interpolationSteps));
   
   return !error;
}

Int_t TZeroDegreeHit::CalculateCfd(double attenuation, unsigned int delay, int halfsmoothingwindow, unsigned int interpolationSteps) {
   ///Used when calculating the CFD from the waveform
   std::vector<Short_t> monitor;
   
   return CalculateCfdAndMonitor(attenuation, delay, halfsmoothingwindow, interpolationSteps, monitor);
}

Int_t TZeroDegreeHit::CalculateCfdAndMonitor(double attenuation, unsigned int delay, int halfsmoothingwindow, unsigned int interpolationSteps, std::vector<Short_t> &monitor) {
   ///Used when calculating the CFD from the waveform
   
   Short_t monitormax = 0;
   std::vector<Short_t> *waveform = GetWaveform();
   
   bool armed = false;
   
   Int_t cfd = 0;
   
   std::vector<Short_t> smoothedWaveform;
   
   if(waveform->empty()) {
      return INT_MAX; //Error!
   }
   
   if((unsigned int)waveform->size() > delay+1) {
      
      if(halfsmoothingwindow > 0) {
         smoothedWaveform = TZeroDegreeHit::CalculateSmoothedWaveform(halfsmoothingwindow);
      } else {
         smoothedWaveform = *waveform;
      }
      
      monitor.resize(smoothedWaveform.size()-delay);
      monitor[0] = attenuation*smoothedWaveform[delay]-smoothedWaveform[0];
      if(monitor[0] > monitormax) {
         armed = true;
         monitormax = monitor[0];
      }
      
      for(unsigned int i = delay + 1; i < smoothedWaveform.size(); ++i) {
         monitor[i-delay] = attenuation*smoothedWaveform[i]-smoothedWaveform[i-delay];
         if(monitor[i-delay] > monitormax) {
            armed=true;
            monitormax = monitor[i-delay];
         } else {
            if(armed == true && monitor[i-delay] < 0) {
               armed = false;
               if(monitor[i-delay-1]-monitor[i-delay] != 0) {
                  //Linear interpolation.
                  cfd = (i-delay-1)*interpolationSteps + (monitor[i-delay-1]*interpolationSteps)/(monitor[i-delay-1]-monitor[i-delay]);
               }
               else {
                  //Should be impossible, since monitor[i-delay-1] => 0 and monitor[i-delay] > 0
                  cfd = 0;
               }
            }
         }
      }
   } else {
      monitor.resize(0);
   }
   
   return cfd;
   
}

std::vector<Short_t> TZeroDegreeHit::CalculateSmoothedWaveform(unsigned int halfsmoothingwindow) {
   ///Used when calculating the CFD from the waveform
   
   std::vector<Short_t> *waveform = GetWaveform();
   if(waveform->empty()) {
      return std::vector<Short_t>(); //Error!
   }
   
   std::vector<Short_t> smoothedWaveform(std::max((size_t)0, waveform->size()-2*halfsmoothingwindow), 0);
   
   for(size_t i = halfsmoothingwindow; i < waveform->size()-halfsmoothingwindow; ++i) {
      for(int j = -(int)halfsmoothingwindow; j <= (int)halfsmoothingwindow; ++j) {
         smoothedWaveform[i-halfsmoothingwindow] += (*waveform)[i+j];
      }
   }
   
   return smoothedWaveform;
}

std::vector<Short_t> TZeroDegreeHit::CalculateCfdMonitor(double attenuation, int delay, int halfsmoothingwindow) {
   ///Used when calculating the CFD from the waveform
   
   std::vector<Short_t> *waveform = GetWaveform();
   
   if(waveform->empty()) {
      return std::vector<Short_t>(); //Error!
   }
   
   std::vector<Short_t> smoothedWaveform;
   
   if(halfsmoothingwindow > 0) {
      smoothedWaveform = TZeroDegreeHit::CalculateSmoothedWaveform(halfsmoothingwindow);
   }
   else{
      smoothedWaveform = *waveform;
   }
   
   std::vector<Short_t> monitor(std::max((size_t)0, smoothedWaveform.size()-delay), 0);
   
   for(size_t i = delay; i < smoothedWaveform.size(); ++i) {
      monitor[i-delay] = attenuation*smoothedWaveform[i]-smoothedWaveform[i-delay];
   }
   
   return monitor;
}
