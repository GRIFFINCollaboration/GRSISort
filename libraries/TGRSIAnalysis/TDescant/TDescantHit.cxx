#include "TDescant.h"
#include "TDescantHit.h"
#include "Globals.h"

#include <iostream>
#include <algorithm>

ClassImp(TDescantHit)

TDescantHit::TDescantHit()	{	
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   //Class()->AddRule("TDescantHit waveform attributes=NotOwner");
   Clear();
}

TDescantHit::~TDescantHit()	{	}

TDescantHit::TDescantHit(const TDescantHit &rhs) : TGRSIDetectorHit() {
   Clear();
   ((TDescantHit&)rhs).Copy(*this);
}

void TDescantHit::Copy(TObject &rhs) const {
   TGRSIDetectorHit::Copy(rhs);
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	static_cast<TDescantHit&>(rhs).filter = filter;
	static_cast<TDescantHit&>(rhs).zc = zc;
	static_cast<TDescantHit&>(rhs).ccShort = ccShort;
	static_cast<TDescantHit&>(rhs).ccLong = ccLong;
	static_cast<TDescantHit&>(rhs).psd = psd;
   return;
}

TVector3 TDescantHit::GetChannelPosition(double dist) const {
   //This should not be called by the user. Instead use 
   //TGRSIDetectorHit::GetPosition
	return TDescant::GetPosition(GetDetector());
}

bool TDescantHit::InFilter(Int_t wantedfilter) {
   // check if the desired filter is in wanted filter;
   // return the answer;
   return true;
}

void TDescantHit::Clear(Option_t *opt)	{
   filter = 0;
   psd    = -1;
   TGRSIDetectorHit::Clear();
}

void TDescantHit::Print(Option_t *opt) const	{
   printf("Descant Detector: %i\n",GetDetector());
	printf("Descant hit energy: %.2f\n",GetEnergy());
	printf("Descant hit time:   %.f\n",GetTime());
}

/*
bool TDescantHit::CompareEnergy(TDescantHit *lhs, TDescantHit *rhs)	{
   return(lhs->GetEnergy()) > rhs->GetEnergy();
}
*/
/*
void TDescantHit::Add(TDescantHit *hit)	{
  if(!CompareEnergy(this,hit)) {
      this->cfd    = hit->GetCfd();    
      this->psd    = hit->GetPsd();    
      this->time   = hit->GetTime();
  	 	//this->position = hit->GetPosition();
   }
   this->SetCharge(0);

   this->SetEnergy(this->GetEnergy() + hit->GetEnergy());
}
*/
bool TDescantHit::AnalyzeWaveform() {
   bool error = false;
   
   std::vector<Short_t> *waveform = GetWaveform();
   std::vector<Int_t> baseline_corrections (8, 0);
   std::vector<Short_t> smoothedwaveform;

   // all timing algorithms use interpolation with this many steps between two samples (all times are stored as integers)
   int interpolation_steps = 256;
   int delay = 8;
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

   this->SetCfd(CalculateCfd(attenuation, delay, halfsmoothingwindow, interpolation_steps));

   // PSD
   // time to zero-crossing algorithm: time when sum reaches n% of the total sum minus the cfd time
   double fraction = 0.85;

   this->psd = CalculatePsd(fraction, interpolation_steps);

   if(psd < 0) {
      error = true;
   }

   return !(error);

}

Int_t TDescantHit::CalculateCfd(double attenuation, unsigned int delay, int halfsmoothingwindow, int interpolation_steps) {

   std::vector<Short_t> monitor;

   return CalculateCfdAndMonitor(attenuation, delay, halfsmoothingwindow, interpolation_steps, monitor);

}

Int_t TDescantHit::CalculateCfdAndMonitor(double attenuation, unsigned int delay, int halfsmoothingwindow, int interpolation_steps, std::vector<Short_t> &monitor) {

   Short_t monitormax = 0;

   bool armed = false;

   Int_t cfd = 0;
   std::vector<Short_t> *waveform = GetWaveform();
   std::vector<Short_t> smoothedwaveform;

   if(waveform->size() > delay+1) {

      if(halfsmoothingwindow > 0) {
         smoothedwaveform = TDescantHit::CalculateSmoothedWaveform(halfsmoothingwindow);
      }
      else{
         smoothedwaveform = *waveform;
      }

      monitor.resize(smoothedwaveform.size()-delay);
      monitor[0] = attenuation*smoothedwaveform[delay]-smoothedwaveform[0];
      if(monitor[0] > monitormax) {
         armed = true;
         monitormax = monitor[0];
      }

      for(size_t i = delay + 1; i < smoothedwaveform.size(); ++i) {
         monitor[i-delay] = attenuation*smoothedwaveform[i]-smoothedwaveform[i-delay];
         if(monitor[i-delay] > monitormax) {
            armed=true;
            monitormax = monitor[i-delay];
         }
         else {
            if(armed == true && monitor[i-delay] < 0) {
               armed = false;
               if(monitor[i-delay-1]-monitor[i-delay] != 0) {
                  //Linear interpolation.
                  cfd = (i-delay)*interpolation_steps + (monitor[i-delay-1]*interpolation_steps)/(monitor[i-delay-1]-monitor[i-delay]);
               }
               else {
                  //Should be impossible, since monitor[i-delay-1] => 0 and monitor[i-delay] > 0
                  cfd = 0;
               }
            }
         }
      }
   }
   else {
      monitor.resize(0);
   }

   return cfd;
}

std::vector<Short_t> TDescantHit::CalculateSmoothedWaveform(unsigned int halfsmoothingwindow) {

   std::vector<Short_t> *waveform = GetWaveform();
   std::vector<Short_t> smoothedwaveform(std::max((size_t)0, waveform->size()-2*halfsmoothingwindow), 0);

   for(size_t i = halfsmoothingwindow; i < waveform->size()-halfsmoothingwindow; ++i) {
      for(int j = -(int)halfsmoothingwindow; j <= (int)halfsmoothingwindow; ++j) {
         smoothedwaveform[i-halfsmoothingwindow] += waveform->at(i+j);
      }
//      double temp = smoothedwaveform[i-halfsmoothingwindow]/(2.0*halfsmoothingwindow+1);
//      smoothedwaveform[i-halfsmoothingwindow] = (temp > 0.0) ? (temp + 0.5) : (temp - 0.5);
   }

   return smoothedwaveform;

}

std::vector<Short_t> TDescantHit::CalculateCfdMonitor(double attenuation, int delay, int halfsmoothingwindow) {

   std::vector<Short_t> *waveform = GetWaveform();
   std::vector<Short_t> monitor(std::max((size_t)0, waveform->size()-delay), 0);
   std::vector<Short_t> smoothedwaveform;

   if(halfsmoothingwindow > 0) {
      smoothedwaveform = TDescantHit::CalculateSmoothedWaveform(halfsmoothingwindow);
   }
   else{
      smoothedwaveform = *waveform;
   }

   for(size_t i = delay; i < waveform->size(); ++i) {
      monitor[i-delay] = attenuation*smoothedwaveform[i]-smoothedwaveform[i-delay];
   }

   return monitor;

}

std::vector<Int_t> TDescantHit::CalculatePartialSum() {

   std::vector<Short_t> *waveform = GetWaveform();
   std::vector<Int_t> partialsums(waveform->size(), 0);

   if(waveform->size() > 0) {
      partialsums[0] = waveform->at(0);
      for(size_t i = 1; i < waveform->size(); ++i) {
         partialsums[i] = partialsums[i-1] + waveform->at(i);
      }
   }
   return partialsums;
}

Int_t TDescantHit::CalculatePsd(double fraction, int interpolation_steps) {

   std::vector<Int_t> partialsums;

   return CalculatePsdAndPartialSums(fraction, interpolation_steps, partialsums);

}

Int_t TDescantHit::CalculatePsdAndPartialSums(double fraction, int interpolation_steps, std::vector<Int_t>& partialsums) {

   Int_t psd;
   std::vector<Short_t> *waveform = GetWaveform();
   partialsums = CalculatePartialSum();
   if(partialsums.empty() == true) {
      return -1;
   }
   int total_sum = partialsums.back();

   psd = -1;
   if(partialsums[0] < fraction*total_sum) {
      for(size_t i = 1; i < partialsums.size(); ++i) {
         if(partialsums[i] >= fraction*total_sum) {
            psd = i*interpolation_steps - ((partialsums[i]-fraction*total_sum)*interpolation_steps)/waveform->at(i);
            break;
         }
      }
   }

   return psd;

}
