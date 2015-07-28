#include "TSceptar.h"
#include "TSceptarHit.h"
#include "Globals.h"

#include <iostream>
#include <algorithm>

ClassImp(TSceptarHit)

////////////////////////////////////////////////////////////////
//                                                            //
// TSceptarHit                                                //
//                                                            //
// This is class that contains the information about a sceptar//
// hit. This class is used to find energy, time, etc.         //
//                                                            //
////////////////////////////////////////////////////////////////

TSceptarHit::TSceptarHit()	{
   //Default Constructor
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   //Class()->AddRule("TSceptarHit waveform attributes=NotOwner");
   Clear();
}

TSceptarHit::~TSceptarHit()	{	}

TSceptarHit::TSceptarHit(const TSceptarHit &rhs)	{	
   //Copy Constructor
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
   ((TSceptarHit&)rhs).Copy(*this);
}

void TSceptarHit::Copy(TSceptarHit &rhs) const {
   //Copies a TSceptarHit
  TGRSIDetectorHit::Copy((TGRSIDetectorHit&)rhs);
	((TSceptarHit&)rhs).filter = filter;
}                                       

/*
void TSceptarHit::SetHit(){
   MNEMONIC mnemonic;
   TChannel *channel = TChannel::GetChannel(GetAddress());
   if(!channel){
      Error("SetHit","No TChannel exists for address %u",GetAddress());
      return;
   }
   ClearMNEMONIC(&mnemonic);
   ParseMNEMONIC(channel->GetChannelName(),&mnemonic);
   SetDetectorNumber(mnemonic.arrayposition);
   fDetectorSet = true;
   SetEnergy(channel->CalibrateENG(GetCharge()));
   fEnergySet = true;
   SetPosition(TSceptar::GetPosition(GetDetectorNumber()));
   fPosSet = true;
   fHitSet = true;

}
*/

TVector3 TSceptarHit::GetPosition(double dist) const {
   //Gets the position of the current TSceptarHit
   //This position returns is of the center of the paddle
	return TSceptar::GetPosition(detector);
}

bool TSceptarHit::InFilter(Int_t wantedfilter) {
   // check if the desired filter is in wanted filter;
   // return the answer;
   return true;
}

double TSceptarHit::GetTime(Option_t *opt) const {
   //Returns the Time of the Sceptar Hit.
   return (double)time;
}

void TSceptarHit::Clear(Option_t *opt)	{
   //Clears the SceptarHit
   filter = 0;
   TGRSIDetectorHit::Clear();
}

void TSceptarHit::Print(Option_t *opt) const	{
   //Prints the SceptarHit. Returns:
   //Detector
   //Energy
   //Time
   printf("Sceptar Detector: %i\n",GetDetector());
	printf("Sceptar hit energy: %.2f\n",GetEnergy());
	printf("Sceptar hit time:   %.lf\n",GetTime());
}

/*
bool TSceptarHit::CompareEnergy(TSceptarHit *lhs, TSceptarHit *rhs)	{
   return(lhs->GetEnergy()) > rhs->GetEnergy();
}
*/
/*
void TSceptarHit::Add(TSceptarHit *hit)	{
   if(!CompareEnergy(this,hit)) {
      this->cfd    = hit->GetCfd();    
      this->time   = hit->GetTime();
      //this->position = hit->GetPosition();
   }
   this->SetCharge(0);

   this->SetEnergy(this->GetEnergy() + hit->GetEnergy());
}
*/
/*
Double_t TSceptarHit::GetEnergy() const {
   if(fEnergySet)
      return energy;
   else{
      TChannel* channel = TChannel::GetChannel(GetAddress());
      if(!channel){
         Error("GetEnergy()","No TChannel exists for address %u",GetAddress());
         return 0.0;
      }
      return channel->CalibrateENG(GetCharge()); 
   }
}
*/

bool TSceptarHit::AnalyzeWaveform() {
   //Calculates the cfd time from the waveform
   bool error = false;
   std::vector<Int_t> baseline_corrections (8, 0);
   std::vector<Short_t> smoothedwaveform;

   // all timing algorithms use interpolation with this many steps between two samples (all times are stored as integers)
   int interpolation_steps = 256;
   int delay = 8;
   double attenuation = 24./64.;
   int halfsmoothingwindow = 0; //2*halfsmoothingwindow + 1 = number of samples in moving window.

   // baseline algorithm: correct each adc with average of first two samples in that adc
   for(size_t i = 0; i < 8 && i < waveform.size(); ++i) {
      baseline_corrections[i] = waveform[i];
   }
   for(size_t i = 8; i < 16 && i < waveform.size(); ++i) {
      baseline_corrections[i-8] = ((baseline_corrections[i-8] + waveform[i]) + ((baseline_corrections[i-8] + waveform[i]) > 0 ? 1 : -1)) >> 1;
   }
   for(size_t i = 0; i < waveform.size(); ++i) {
      waveform[i] -= baseline_corrections[i%8];
   }

   this->cfd = CalculateCfd(attenuation, delay, halfsmoothingwindow, interpolation_steps);

   return !(error);

}

Int_t TSceptarHit::CalculateCfd(double attenuation, int delay, int halfsmoothingwindow, int interpolation_steps) {
   //Used when calculating the CFD from the waveform
   std::vector<Short_t> monitor;

   return CalculateCfdAndMonitor(attenuation, delay, halfsmoothingwindow, interpolation_steps, monitor);

}

Int_t TSceptarHit::CalculateCfdAndMonitor(double attenuation, int delay, int halfsmoothingwindow, int interpolation_steps, std::vector<Short_t> &monitor) {
   //Used when calculating the CFD from the waveform

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
   //Used when calculating the CFD from the waveform

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
   //Used when calculating the CFD from the waveform

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

