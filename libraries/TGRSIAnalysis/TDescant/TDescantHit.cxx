#include "TDescantHit.h"

#include <iostream>
#include <algorithm>
#include <climits>

#include "Globals.h"
#include "TDescant.h"

/// \cond CLASSIMP
ClassImp(TDescantHit)
/// \endcond

TDescantHit::TDescantHit()	{
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TDescantHit::~TDescantHit() { }

TDescantHit::TDescantHit(const TDescantHit &rhs) : TGRSIDetectorHit() {
   Clear();
   rhs.Copy(*this);
}

void TDescantHit::Copy(TObject &rhs) const {
   TGRSIDetectorHit::Copy(rhs);
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   static_cast<TDescantHit&>(rhs).fFilter  = fFilter;
   static_cast<TDescantHit&>(rhs).fZc      = fZc;
   static_cast<TDescantHit&>(rhs).fCcShort = fCcShort;
   static_cast<TDescantHit&>(rhs).fCcLong  = fCcLong;
   static_cast<TDescantHit&>(rhs).fPsd     = fPsd;

   static_cast<TDescantHit&>(rhs).fSlopePsd     = fSlopePsd;
   static_cast<TDescantHit&>(rhs).fMonitor      = fMonitor;
   static_cast<TDescantHit&>(rhs).fMax          = fMax;
   static_cast<TDescantHit&>(rhs).fPartialSum   = fPartialSum;
   
   static_cast<TDescantHit&>(rhs).fDiffWaveform   = fDiffWaveform;
   static_cast<TDescantHit&>(rhs).fDiffIntWaveform   = fDiffIntWaveform;
   static_cast<TDescantHit&>(rhs).fShapedWaveform   = fShapedWaveform;
   static_cast<TDescantHit&>(rhs).fPsdMonitor = fPsdMonitor;
   static_cast<TDescantHit&>(rhs).fTimes = fTimes;
   static_cast<TDescantHit&>(rhs).fCfdTime = fCfdTime;
   static_cast<TDescantHit&>(rhs).fMaxElement = fMaxElement;
   static_cast<TDescantHit&>(rhs).fZc2 = fZc2;
}

TVector3 TDescantHit::GetChannelPosition(double dist) const {
   ///This should not be called by the user. Instead use
   ///TGRSIDetectorHit::GetPosition
   return TDescant::GetPosition(GetDetector());
}

bool TDescantHit::InFilter(Int_t wantedfilter) {
   /// check if the desired filter is in wanted filter;
   /// return the answer;
   return true;
}

void TDescantHit::Clear(Option_t *opt)	{
   fFilter  = 0;
   fPsd     = -1;
   fZc      = 0;
   fCcShort = 0;
   fCcLong  = 0;
   
   fSlopePsd = 0;
   fMonitor.clear();
   fMax = 0;
   fPartialSum.clear();

   fDiffWaveform.clear();
   fDiffIntWaveform.clear();
   fShapedWaveform.clear();
   fPsdMonitor.clear();

   fTimes.clear();
   fCfdTime = 0;
   fMaxElement = 0.;
   fZc2 = 0.;
   
   TGRSIDetectorHit::Clear();
}

void TDescantHit::Print(Option_t *opt) const	{
   printf("Descant Detector: %i\n",GetDetector());
   printf("Descant hit energy: %.2f\n",GetEnergy());
   printf("Descant hit time:   %.f\n",GetTime());
}

Double_t TDescantHit::GetTime() const {
  ///returns time using the remainder (difference between 100 MHz timestamp and 125 MHz clock of FPGA), and the CFD word
  return 10.*fTimeStamp + GetRemainder() + (GetCfd() + gRandom->Uniform())/256.;
}

bool TDescantHit::AnalyzeWaveform() {
   bool error = false;
   
   std::vector<Short_t>* waveform = GetWaveform();
   if(waveform->empty()) {
      return false; //Error!
   }

   std::vector<Int_t> baselineCorrections(8, 0);
   std::vector<Short_t> smoothedWaveform;
   
   // all timing algorithms use interpolation with this many steps between two samples (all times are stored as integers)
   unsigned int interpolationSteps = 256;
   int delay = 10;
   double attenuation = 11./64.;
   int halfSmoothingWindow = 0; //2*halfsmoothingwindow + 1 = number of samples in moving window.
   
   // baseline algorithm: correct each adc with average of first two samples in that adc
   for(size_t i = 0; i < 8 && i < waveform->size(); ++i) {
      baselineCorrections[i] = (*waveform)[i];
   }
   for(size_t i = 8; i < 16 && i < waveform->size(); ++i) {
      baselineCorrections[i-8] = ((baselineCorrections[i-8] + (*waveform)[i]) + ((baselineCorrections[i-8] + (*waveform)[i]) > 0 ? 1 : -1)) >> 1; //Average
   }
   for(size_t i = 0; i < waveform->size(); ++i) {
      (*waveform)[i] -= baselineCorrections[i%8];
   }
   if( waveform->size() > 0) {
	  fMax = CalculateWaveformMaximum(2);
   } else {
	  fMax = 0;
   }
   
   //SetCfd(CalculateCfd(attenuation, delay, halfSmoothingWindow, interpolationSteps));
   SetCfd(CalculateCfdAndMonitor(attenuation, delay, halfSmoothingWindow, interpolationSteps, fMonitor));
   
   if(GetCfd() >= 0 && static_cast<size_t>(GetCfd()/256) < fTimes.size()) {
      fCfdTime = fTimes[GetCfd()/256];
   }
   else {
      fCfdTime = INT_MAX;
   }

   //fTimes.resize(std::min(fTimes.size(), static_cast<size_t>(GetCfd()/256+1000)));
   //fTimes.erase(fTimes.begin(), fTimes.begin() + std::max(0,(static_cast<int>(GetCfd()/256-400))));
  
   //waveform->resize(std::min(waveform->size(), static_cast<size_t>(GetCfd()/256+1000)));
   //waveform->erase(waveform->begin(), waveform->begin() + std::max(0,(static_cast<int>(GetCfd()/256-400))));
	//
   //SetCfd(CalculateCfdAndMonitor(attenuation, delay, halfSmoothingWindow, interpolationSteps, fMonitor));

   fMaxElement = std::distance(waveform->begin(), std::max_element(waveform->begin(), waveform->end()));
   
	double timeConstant = 34.;//172.
   fDiffWaveform = Differentiator(ShortVectorToDouble(*waveform), timeConstant);
   fDiffIntWaveform = Integrator(fDiffWaveform, timeConstant);
   fShapedWaveform = Integrator(fDiffIntWaveform, timeConstant);

   fZc = CalculateZeroCrossing(fShapedWaveform, interpolationSteps);
   
   // PSD
   // time to zero-crossing algorithm: time when sum reaches n% of the total sum minus the cfd time
   double fraction = 0.50;
   
   SetPsd(CalculatePsd(fraction, interpolationSteps));
   
   SetCharge(CalculatePartialSum().back());
   
   fSlopePsd = CalculateSlopePsd(63, 120, 256);
   
   fPartialSum = CalculatePartialSum();
   
   SetCharge(fPartialSum.back());
   
   fZc2 = CalculateCfdAndMonitorForSignal(49/64., 139, interpolationSteps, IntVectorToDouble(fPartialSum), fPsdMonitor);
   
   if(fPsd < 0) {
      error = true;
   }
   
   return !error;
}

Int_t TDescantHit::CalculateCfd(double attenuation, unsigned int delay, int halfsmoothingwindow, unsigned int interpolation_steps) {
   std::vector<Short_t> monitor;
   return CalculateCfdAndMonitor(attenuation, delay, halfsmoothingwindow, interpolation_steps, monitor);
}

Int_t TDescantHit::CalculateCfdAndMonitor(double attenuation, unsigned int delay, int halfSmoothingWindow, unsigned int interpolationSteps, std::vector<Short_t>& monitor) {
   Short_t monitormax = 0;
   bool armed = false;
   
   Int_t cfd = 0;
   std::vector<Short_t>* waveform = GetWaveform();
   if(waveform->empty()) {
      return INT_MAX; //Error!
   }
   std::vector<Short_t> smoothedWaveform;
   
   if(waveform->size() > delay+1) {
      if(halfSmoothingWindow > 0) {
         smoothedWaveform = TDescantHit::CalculateSmoothedWaveform(halfSmoothingWindow);
      } else {
         smoothedWaveform = *waveform;
      }
      
      monitor.clear();
      monitor.resize(smoothedWaveform.size()-delay);
      monitor[0] = attenuation*smoothedWaveform[delay]-smoothedWaveform[0];
      if(monitor[0] > monitormax) {
         armed = true;
         monitormax = monitor[0];
      }
      
      for(size_t i = delay + 1; i < smoothedWaveform.size(); ++i) {
         monitor[i-delay] = attenuation*smoothedWaveform[i]-smoothedWaveform[i-delay];
         if(monitor[i-delay] > monitormax) {
            armed=true;
            monitormax = monitor[i-delay];
         } else {
            if(armed == true && monitor[i-delay] < 0) {
               armed = false;
               if(monitor[i-delay-1] - monitor[i-delay] != 0) {
                  //Linear interpolation.
                  cfd = (i-delay-1)*interpolationSteps + (monitor[i-delay-1]*interpolationSteps)/(monitor[i-delay-1]-monitor[i-delay]);
               } else {
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

Int_t TDescantHit::CalculateCfdAndMonitorForSignal(double attenuation, unsigned int delay, unsigned int interpolationSteps, const std::vector<Double_t> inputSignal, std::vector<Double_t> &monitor) {
   
   Double_t monitormax = 0.;
   bool armed = false;
   
   Int_t cfd = 0;

   if(inputSignal.empty()) {
      return INT_MAX; //Error!
   }
   
   if(inputSignal.size() > delay+1) {
      
      monitor.clear();
      monitor.resize(inputSignal.size()-delay);
      monitor[0] = attenuation*inputSignal[delay]-inputSignal[0];
      if(monitor[0] > monitormax) {
         armed = true;
         monitormax = monitor[0];
      }
      
      for(unsigned int i = delay + 1; i < inputSignal.size(); ++i) {
         monitor[i-delay] = attenuation*inputSignal[i]-inputSignal[i-delay];
         if(monitor[i-delay] > monitormax) {
            armed=true;
            monitormax = monitor[i-delay];
         }
         else {
            if(armed == true && monitor[i-delay] < 0) {
               armed = false;
               if(monitor[i-delay-1] - monitor[i-delay] != 0) {
                  //Linear interpolation.
                  cfd = (i-delay-1)*interpolationSteps + (monitor[i-delay-1]*interpolationSteps)/(monitor[i-delay-1]-monitor[i-delay]);
               } else {
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

Int_t TDescantHit::CalculateCfdForPartialSums(double attenuation, unsigned int delay, unsigned int interpolationSteps) {
   
   Int_t monitormax = 0.;
   bool armed = false;
   
   Int_t cfd = 0;
   
   std::vector<Int_t> monitor;
   
   if(fPartialSum.empty()) {
      this->fPartialSum = CalculatePartialSum(0);
      if(fPartialSum.empty()) {
         return INT_MAX; //Error!
      }
   }
   
   if(fPartialSum.size() > delay+1) {
      
      monitor.clear();
      monitor.resize(fPartialSum.size()-delay);
      monitor[0] = attenuation*fPartialSum[delay]-fPartialSum[0];
      if(monitor[0] > monitormax) {
         armed = true;
         monitormax = monitor[0];
      }
      
      for(unsigned int i = delay + 1; i < fPartialSum.size(); ++i) {
         monitor[i-delay] = attenuation*fPartialSum[i]-fPartialSum[i-delay];
         if(monitor[i-delay] > monitormax) {
            armed=true;
            monitormax = monitor[i-delay];
         }
         else {
            if(armed == true && monitor[i-delay] < 0) {
               armed = false;
               if(monitor[i-delay-1] - monitor[i-delay] != 0) {
                  //Linear interpolation.
                  cfd = (i-delay-1)*interpolationSteps + (monitor[i-delay-1]*interpolationSteps)/(monitor[i-delay-1]-monitor[i-delay]);
               } else {
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

std::vector<Short_t> TDescantHit::CalculateSmoothedWaveform(unsigned int halfSmoothingWindow) {
   std::vector<Short_t>* waveform = GetWaveform();
   
   if(waveform->empty()) {
      return std::vector<Short_t>(); //Error!
   }
   
   std::vector<Short_t> smoothedWaveform(std::max((static_cast<size_t>(0)), waveform->size()-2*halfSmoothingWindow), 0);
   
   for(size_t i = halfSmoothingWindow; i < waveform->size() - halfSmoothingWindow; ++i) {
      for(int j = -static_cast<int>(halfSmoothingWindow); j <= static_cast<int>(halfSmoothingWindow); ++j) {
         smoothedWaveform[i-halfSmoothingWindow] += (*waveform)[i+j];
      }
   }
   
   return smoothedWaveform;
}

std::vector<Short_t> TDescantHit::CalculateCfdMonitor(double attenuation, unsigned int delay, unsigned int halfSmoothingWindow) {
   std::vector<Short_t>* waveform = GetWaveform();
   if(waveform->empty()) {
      return std::vector<Short_t>(); //Error!
   }
   std::vector<Short_t> smoothedWaveform;
   
   if(halfSmoothingWindow > 0) {
      smoothedWaveform = TDescantHit::CalculateSmoothedWaveform(halfSmoothingWindow);
   } else {
      smoothedWaveform = *waveform;
   }
   
   std::vector<Short_t> monitor(std::max((static_cast<size_t>(0)), smoothedWaveform.size()-delay), 0);
   
   for(size_t i = delay; i < waveform->size(); ++i) {
      monitor[i-delay] = attenuation*smoothedWaveform[i]-smoothedWaveform[i-delay];
   }
   
   return monitor;
}

std::vector<Int_t> TDescantHit::CalculatePartialSum() {
   std::vector<Short_t>* waveform = GetWaveform();
   if(waveform->empty()) {
      return std::vector<Int_t>(); //Error!
   }
   
   std::vector<Int_t> partialSums(waveform->size(), 0);
   
   if(waveform->size() > 0) {
      partialSums[0] = waveform->at(0);
      for(size_t i = 1; i < waveform->size(); ++i) {
         partialSums[i] = partialSums[i-1] + (*waveform)[i];
      }
   }

   return partialSums;
}

Int_t TDescantHit::CalculatePsd(double fraction, unsigned int interpolationSteps) {
   std::vector<Int_t> partialSums;
   
   return CalculatePsdAndPartialSums(fraction, interpolationSteps, partialSums);
}

Int_t TDescantHit::CalculatePsdAndPartialSums(double fraction, unsigned int interpolationSteps, std::vector<Int_t>& partialSums) {
   
   Int_t psd = 0;
   
   std::vector<Short_t> *waveform = GetWaveform();
   partialSums = CalculatePartialSum();
   if(partialSums.empty()) {
      return -1;
   }
   int totalSum = partialSums.back();
   
   fPsd = -1;
   if(partialSums[0] < fraction*totalSum) {
      for(size_t i = 1; i < partialSums.size(); ++i) {
         if(partialSums[i] >= fraction*totalSum) {
            psd = i*interpolationSteps - ((partialSums[i]-fraction*totalSum)*interpolationSteps)/(*waveform)[i];
            break;
         }
      }
   }
   
   return psd;
}

Int_t TDescantHit::CalculatePsd(double fraction, int halfsmoothingwindow, unsigned interpolation_steps) {
   
   std::vector<Int_t> partialsums;
   
   return CalculatePsdAndPartialSums(fraction, halfsmoothingwindow, interpolation_steps, partialsums);
   
}

Int_t TDescantHit::CalculatePsdAndPartialSums(double fraction, int halfsmoothingwindow, unsigned int interpolation_steps, std::vector<Int_t>& partialsums) {
   
   Int_t psd;
   std::vector<Short_t> *waveform = GetWaveform();
   if(waveform->empty()) {
      return INT_MAX; //Error!
   }
   
   partialsums = CalculatePartialSum(halfsmoothingwindow);
   if(partialsums.empty() == true) {
      return -1;
   }
   int total_sum = partialsums.back();
   
   psd = -1;
   if(partialsums[0] < fraction*total_sum) {
      for(size_t i = 1; i < partialsums.size(); ++i) {
         if(partialsums[i] >= fraction*total_sum) {
            psd = i*interpolation_steps - ((partialsums[i]-fraction*total_sum)*interpolation_steps)/((double)waveform->at(i));
            break;
         }
      }
   }
   
   return psd;
   
}

Double_t TDescantHit::CalculateSlopePsd(unsigned int peakdelay, unsigned int psddelay, unsigned int interpolation_steps) {
   
   std::vector<Short_t> *waveform = GetWaveform();
   if(waveform->empty()) {
      return INT_MAX; //Error!
   }
   
   //   unsigned int peakdelay = 5;
   //   unsigned int psddelay = 23;
   unsigned int halfpsdpeakintegrationwidth = 4;
   unsigned int halfpsddelayedintegrationwidth = 20;
   
   unsigned int roundedcfd = (GetCfd()+interpolation_steps*0.5)/interpolation_steps;
   
   double psdpeak = 0;
   double psddelayed = 0;
   
   //bool psderror = false;
   
   //   this->slopepsd = 0;
   
   if(roundedcfd + peakdelay < halfpsdpeakintegrationwidth || roundedcfd + psddelay + halfpsddelayedintegrationwidth + 1 > waveform->size()) {
	  //psderror = true;
      return -1;
   }
   else {
      psdpeak = 0;
      psddelayed = 0;
      for(size_t i = roundedcfd + peakdelay - halfpsdpeakintegrationwidth; i < roundedcfd + peakdelay + halfpsdpeakintegrationwidth + 1; ++i) {
         psdpeak += waveform->at(i);
      }
      for(size_t i = roundedcfd + psddelay - halfpsddelayedintegrationwidth; i < roundedcfd + psddelay + halfpsddelayedintegrationwidth + 1; ++i) {
         psddelayed += waveform->at(i);
      }
   }
   
   if(psdpeak == 0) {
      return -1;
   }
   else {
      return psddelayed/psdpeak;
   }
   
}

std::vector<Int_t> TDescantHit::CalculatePartialSum(int halfsmoothingwindow) {
   
   std::vector<Short_t> *waveform = GetWaveform();
   if(waveform->empty()) {
      return std::vector<Int_t>(); //Error!
   }
   
   std::vector<Short_t> smoothedwaveform;
   
   if(halfsmoothingwindow > 0) {
      smoothedwaveform = TDescantHit::CalculateSmoothedWaveform(halfsmoothingwindow);
   }
   else{
      smoothedwaveform = *waveform;
   }
   
   std::vector<Int_t> partialsums(smoothedwaveform.size(), 0);
   
   
   if(smoothedwaveform.size() > 0) {
      partialsums[0] = smoothedwaveform[0];
      for(size_t i = 1; i < smoothedwaveform.size(); ++i) {
         partialsums[i] = partialsums[i-1] + smoothedwaveform[i];
      }
   }
   return partialsums;
}

std::vector<Int_t> TDescantHit::CalculateAverageWaveform(unsigned int halfsmoothingwindow) {
   
   std::vector<Short_t> *waveform = GetWaveform();
   if(waveform->empty()) {
      return std::vector<Int_t>(); //Error!
   }
   
   std::vector<Int_t> averagewaveform(std::max((size_t)0, waveform->size()-2*halfsmoothingwindow), 0);
   
   for(size_t i = halfsmoothingwindow; i < waveform->size()-halfsmoothingwindow; ++i) {
      for(int j = -(int)halfsmoothingwindow; j <= (int)halfsmoothingwindow; ++j) {
         averagewaveform[i-halfsmoothingwindow] += waveform->at(i+j);
      }
      
      averagewaveform[i-halfsmoothingwindow] /= (2*(int)halfsmoothingwindow+1);
      
   }
   
   return averagewaveform;
   
}

Short_t TDescantHit::CalculateWaveformMaximum(unsigned int halfwindowsize) {
   
   std::vector<Short_t> *waveform = GetWaveform();
   if(waveform->empty()) {
      return 0; //Error!
   }
   
   std::vector<Int_t> averagewaveform;
   
   if(halfwindowsize > 0) {
      averagewaveform = TDescantHit::CalculateAverageWaveform(halfwindowsize);
      return *std::max_element(averagewaveform.begin(), averagewaveform.end());
   }
   else{
      return *std::max_element(waveform->begin(), waveform->end());
   }
   
}

std::vector<Double_t> TDescantHit::Differentiator(const std::vector<Double_t> inputSignal, double timeconstant) {
   //Assumes that inputSignal[-1] = inputSignal[0] and outputsignal[-1] = 0 - implies that outputsignal[0] = 0

   std::vector<Double_t> outputsignal(inputSignal.size(), 0);
   
   double invertedtimeconstant = 1/timeconstant;
   
   for(size_t i = 1; i < inputSignal.size(); ++i) {
      outputsignal[i] = 1/(1.+invertedtimeconstant)*(outputsignal[i-1]+inputSignal[i]-inputSignal[i-1]);
   }
   
   return outputsignal;
}

std::vector<Double_t> TDescantHit::ShortVectorToDouble(const std::vector<Short_t> inputSignal) {
   
   std::vector<Double_t> outputsignal(inputSignal.size(), 0);
   
   for(size_t i = 0; i < inputSignal.size(); ++i) {
      outputsignal[i] = inputSignal[i];
   }

   return outputsignal;

}

std::vector<Double_t> TDescantHit::IntVectorToDouble(const std::vector<Int_t> inputSignal) {
   
   std::vector<Double_t> outputsignal(inputSignal.size(), 0);
   
   for(size_t i = 0; i < inputSignal.size(); ++i) {
      outputsignal[i] = inputSignal[i];
   }
   
   return outputsignal;
   
}

std::vector<Double_t> TDescantHit::Integrator(const std::vector<Double_t> inputSignal, double timeconstant) {
   //Assumes that outputsignal[-1] = 0
   
   if(inputSignal.size() == 0) {
      return std::vector<Double_t>();
   }

   std::vector<Double_t> outputsignal(inputSignal.size(), 0);
   double invertedtimeconstant = 1/timeconstant;
   
   outputsignal[0] = 1/(1+invertedtimeconstant)*(invertedtimeconstant*inputSignal[0]);
   
   for(size_t i = 1; i < inputSignal.size(); ++i) {
      outputsignal[i] = 1/(1+invertedtimeconstant)*(invertedtimeconstant*inputSignal[i]+outputsignal[i-1]);
   }
   
   return outputsignal;
}

bool TDescantHit::AnalyzeKentuckyWaveform() {
   bool error = false;
   
   std::vector<Short_t> *waveform = GetWaveform();
   if(waveform->empty()) {
      return false; //Error!
   }
   
   std::vector<Int_t> baseline_corrections (8, 0);
   std::vector<Short_t> smoothedwaveform;
   
   // all timing algorithms use interpolation with this many steps between two samples (all times are stored as integers)
   int interpolation_steps = 256;
   //   int delay = 8;
   int delay = 100;
   double attenuation = 24./64.;
   int halfsmoothingwindow = 0; //2*halfsmoothingwindow + 1 = number of samples in moving window.
   
   // baseline algorithm: correct each adc with average of first two samples in that adc
   for(size_t i = 0; i < 8 && i < waveform->size(); ++i) {
      baseline_corrections[i] = waveform->at(i);
   }
   for(size_t i = 8; i < 16 && i < waveform->size(); ++i) {
      baseline_corrections[i-8] = ((baseline_corrections[i-8] + waveform->at(i)) + ((baseline_corrections[i-8] + waveform->at(i)) > 0 ? 1 : -1)) >> 1;
   }
   for(size_t i = 0; i < waveform->size(); ++i) {
      waveform->at(i) -= baseline_corrections[i%8];
   }
   if( waveform->size() > 0) {
      //      max = *std::max_element(waveform->begin(), waveform->end());
      fMax = CalculateWaveformMaximum(2);
   }
   else {
      fMax = 0;
   }
   //   this->cfd = CalculateCfd(attenuation, delay, halfsmoothingwindow, interpolation_steps);
   SetCfd(CalculateCfdAndMonitor(attenuation, delay, halfsmoothingwindow, interpolation_steps, this->fMonitor));
   
   this->fPartialSum = CalculatePartialSum();
   
   // PSD
   // time to zero-crossing algorithm: time when sum reaches n% of the total sum minus the cfd time
   double fraction = 0.935;
   
   this->fPsd = CalculatePsd(fraction, interpolation_steps);
   
   this->fSlopePsd = CalculateSlopePsd(63, 120, 256);
   
   SetCharge(CalculatePartialSum().back());
   
   if(fPsd < 0) {
      error = true;
   }
   
   return !(error);
   
}

Int_t TDescantHit::CalculateZeroCrossing(std::vector<Double_t> inputSignal, unsigned int interpolationSteps) {
   
   int zc = INT_MAX;
   
   bool armed = false;
   double signalmax = 0;
   
   if(inputSignal.size() == 0) {
      return INT_MAX;
   }
   
   if(inputSignal[0] > signalmax) {
      armed = true;
      signalmax = inputSignal[0];
   }
   
   for(unsigned int i = 1; i < inputSignal.size(); ++i) {
      
      if(inputSignal[i] > signalmax) {
         armed = true;
         signalmax = inputSignal[i];
      }
      else {
         if(armed == true && inputSignal[i] < 0) {
            armed = false;
            zc = i*interpolationSteps + (inputSignal[i-1]*interpolationSteps)/(inputSignal[i-1]-inputSignal[i]);
         }
      }
   }
   
   return zc;
   
}

bool TDescantHit::AnalyzeScopeWaveform() {
   bool error = false;
   
   std::vector<Short_t> *waveform = GetWaveform();
   if(waveform->empty()) {
      return false; //Error!
   }
   
   std::vector<Int_t> baseline_corrections (8, 0);
   std::vector<Short_t> smoothedwaveform;
   
   // all timing algorithms use interpolation with this many steps between two samples (all times are stored as integers)
   int interpolation_steps = 256;
   //   int delay = 8;
   int delay = 48;
   double attenuation = 11./64.;
   int halfsmoothingwindow = 0; //2*halfsmoothingwindow + 1 = number of samples in moving window.
   
   // baseline algorithm: correct each adc with average of first two samples in that adc
   for(size_t i = 0; i < 8 && i < waveform->size(); ++i) {
      baseline_corrections[i] = waveform->at(i);
   }
   for(size_t i = 8; i < 16 && i < waveform->size(); ++i) {
      baseline_corrections[i-8] = ((baseline_corrections[i-8] + waveform->at(i)) + ((baseline_corrections[i-8] + waveform->at(i)) > 0 ? 1 : -1)) >> 1;
   }
   for(size_t i = 0; i < waveform->size(); ++i) {
      waveform->at(i) -= baseline_corrections[i%8];
   }
   if( waveform->size() > 0) {
      //      max = *std::max_element(waveform->begin(), waveform->end());
      fMax = CalculateWaveformMaximum(2);
   }
   else {
      fMax = 0;
   }
   //   this->cfd = CalculateCfd(attenuation, delay, halfsmoothingwindow, interpolation_steps);
   SetCfd(CalculateCfdAndMonitor(attenuation, delay, halfsmoothingwindow, interpolation_steps, this->fMonitor));
   
   if(this->GetCfd() >= 0 && static_cast<size_t>(this->GetCfd()/256) < fTimes.size()) {
      fCfdTime = fTimes[this->GetCfd()/256];
   }
   else {
      fCfdTime = INT_MAX;
   }

   fTimes.resize(std::min(fTimes.size(), static_cast<size_t>(this->GetCfd()/256+1000)));
   fTimes.erase(fTimes.begin(), fTimes.begin() + std::max(0,(static_cast<int>(this->GetCfd()/256-400))));
  
   waveform->resize(std::min(waveform->size(), static_cast<size_t>(this->GetCfd()/256+1000)));
   waveform->erase(waveform->begin(), waveform->begin() + std::max(0,(static_cast<int>(this->GetCfd()/256-400))));

   SetCfd(CalculateCfdAndMonitor(attenuation, delay, halfsmoothingwindow, interpolation_steps, this->fMonitor));

   fMaxElement = std::distance(waveform->begin(), std::max_element(waveform->begin(), waveform->end()));
   
   fDiffWaveform = Differentiator(ShortVectorToDouble(*waveform), 172.);
   fDiffIntWaveform = Integrator(fDiffWaveform, 172.);
   fShapedWaveform = Integrator(fDiffIntWaveform, 172.);

//   fZc = CalculateCfdAndMonitorForSignal(56/64., 100, interpolation_steps, fShapedWaveform, this->fPsdMonitor);
   fZc = CalculateZeroCrossing(fShapedWaveform, interpolation_steps);
   
   // PSD
   // time to zero-crossing algorithm: time when sum reaches n% of the total sum minus the cfd time
   double fraction = 0.50; //0.935
   
   this->fPsd = CalculatePsd(fraction, interpolation_steps);
   
   this->fSlopePsd = CalculateSlopePsd(63, 120, 256);
   
   this->fPartialSum = CalculatePartialSum();
   
   SetCharge(this->fPartialSum.back());
   
   this->fZc2 = CalculateCfdAndMonitorForSignal(49/64., 139, interpolation_steps, IntVectorToDouble(this->fPartialSum), this->fPsdMonitor);
   
   if(fPsd < 0) {
      error = true;
   }
   
   return !(error);
   
}
