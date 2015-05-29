#include "TSceptar.h"
#include "TSceptarHit.h"

#include <iostream>
#include <algorithm>

ClassImp(TSceptarHit)

TSceptarHit::TSceptarHit()	{	
   //Class()->IgnoreTObjectStreamer(true);
   //Class()->AddRule("TSceptarHit waveform attributes=NotOwner");
   Clear();
}

TSceptarHit::~TSceptarHit()	{	}

bool TSceptarHit::InFilter(Int_t wantedfilter) {
   // check if the desired filter is in wanted filter;
   // return the answer;
   return true;
}

void TSceptarHit::Clear(Option_t *opt)	{
   detector = 0;
   address = 0xffffffff;
   filter = 0;
   charge = -1;
   cfd    = -1;
   energy = 0.0;
   time   = 0;

   position.SetXYZ(0,0,1);

   waveform.clear();
}

void TSceptarHit::Print(Option_t *opt)	{
   printf("Sceptar Detector: %i\n",detector);
   printf("Sceptar hit energy: %.2f\n",GetEnergy());
   printf("Sceptar hit time:   %.ld\n",GetTime());
}

bool TSceptarHit::CompareEnergy(TSceptarHit *lhs, TSceptarHit *rhs)	{
   return(lhs->GetEnergy()) > rhs->GetEnergy();
}

void TSceptarHit::Add(TSceptarHit *hit)	{
   if(!CompareEnergy(this,hit)) {
      this->cfd    = hit->GetCfd();    
      this->time   = hit->GetTime();
      this->position = hit->GetPosition();
   }
   this->SetCharge(0);

   this->SetEnergy(this->GetEnergy() + hit->GetEnergy());
}

bool TSceptarHit::AnalyzeWaveform() {
   bool error = false;
   std::vector<Int_t> baseline_corrections (8, 0);
   std::vector<Short_t> smoothedwaveform;

   // all timing algorithms use interpolation with this many steps between two samples (all times are stored as integers)
   int interpolation_steps = 256;
   int delay = 8;
   double attenuation = 24./64.;
   int halfsmoothingwindow = 0; //2*halfsmoothingwindow + 1 = number of samples in moving window.

   // baseline algorithm: correct each adc with average of first two samples in that adc
   size_t baseline_length = 8;

   for(size_t i = 0; i < 8 && i < waveform.size(); ++i) {
      baseline_corrections[i] = waveform[i];
   }
   for(size_t i = 8; i < 16 && i < waveform.size(); ++i) {
      baseline_corrections[i-8] = ((baseline_corrections[i-8] + waveform[i]) + ((baseline_corrections[i-8] + waveform[i]) > 0 ? 1 : -1)) >> 1;
   }
   for(size_t i = 0; i < waveform.size(); ++i) {
      waveform[i] -= baseline_corrections[i%8];
   }

//   this->cfd = CalculateCfd(attenuation, delay, halfsmoothingwindow, interpolation_steps);
   this->cfd = CalculateCfdAndMonitor(attenuation, delay, halfsmoothingwindow, interpolation_steps, cfdmonitor);

   return !(error);

}

Int_t TSceptarHit::CalculateCfd(double attenuation, int delay, int halfsmoothingwindow, int interpolation_steps) {

   std::vector<Short_t> monitor;

   return CalculateCfdAndMonitor(attenuation, delay, halfsmoothingwindow, interpolation_steps, monitor);

}

Int_t TSceptarHit::CalculateCfdAndMonitor(double attenuation, int delay, int halfsmoothingwindow, int interpolation_steps, std::vector<Short_t> &monitor) {

   Short_t monitormax = 0;

   bool cfderror = true;
   bool armed = false;

   Int_t cfd = 0;

   std::vector<Short_t> smoothedwaveform;

   if(waveform.size() > delay+1) {

      if(halfsmoothingwindow > 0) {
         smoothedwaveform = TSceptarHit::CalculateSmoothedWaveform(halfsmoothingwindow);
      }
      else{
         smoothedwaveform = waveform;
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
               cfderror = false;
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

std::vector<Short_t> TSceptarHit::CalculateSmoothedWaveform(unsigned int halfsmoothingwindow) {

   std::vector<Short_t> smoothedwaveform(std::max((size_t)0, waveform.size()-2*halfsmoothingwindow), 0);

   for(size_t i = halfsmoothingwindow; i < waveform.size()-halfsmoothingwindow; ++i) {
      for(int j = -(int)halfsmoothingwindow; j <= (int)halfsmoothingwindow; ++j) {
         smoothedwaveform[i-halfsmoothingwindow] += waveform[i+j];
      }
//      double temp = smoothedwaveform[i-halfsmoothingwindow]/(2.0*halfsmoothingwindow+1);
//      smoothedwaveform[i-halfsmoothingwindow] = (temp > 0.0) ? (temp + 0.5) : (temp - 0.5);
   }

   return smoothedwaveform;

}

std::vector<Short_t> TSceptarHit::CalculateCfdMonitor(double attenuation, int delay, int halfsmoothingwindow) {

   std::vector<Short_t> monitor(std::max((size_t)0, waveform.size()-delay), 0);
   std::vector<Short_t> smoothedwaveform;

   if(halfsmoothingwindow > 0) {
      smoothedwaveform = TSceptarHit::CalculateSmoothedWaveform(halfsmoothingwindow);
   }
   else{
      smoothedwaveform = waveform;
   }

   for(size_t i = delay; i < waveform.size(); ++i) {
      monitor[i-delay] = attenuation*smoothedwaveform[i]-smoothedwaveform[i-delay];
   }

   return monitor;

}

