#include "TDescant.h"
#include "TDescantHit.h"

ClassImp(TDescantHit)

TDescantHit::TDescantHit()	{	
   //Class()->IgnoreTObjectStreamer(true);
   //Class()->AddRule("TSceptarHit waveform attributes=NotOwner");
   Clear();
}

TDescantHit::~TDescantHit()	{	}

bool TDescantHit::InFilter(Int_t wantedfilter) {
   // check if the desired filter is in wanted filter;
   // return the answer;
   return true;
}

void TDescantHit::Clear(Option_t *opt)	{
	detector = 0;
   address = 0xffffffff;
   filter = 0;
   charge = -1;
   cfd    = -1;
   psd    = -1;
   energy = 0.0;
   time   = 0;

   position.SetXYZ(0,0,1);

   waveform.clear();
}

void TDescantHit::Print(Option_t *opt)	{
   printf("Descant Detector: %i\n",detector);
	printf("Descant hit energy: %.2f\n",GetEnergy());
	printf("Descant hit time:   %.ld\n",GetTime());
}


bool TDescantHit::CompareEnergy(TDescantHit *lhs, TDescantHit *rhs)	{
   return(lhs->GetEnergy()) > rhs->GetEnergy();
}


void TDescantHit::Add(TDescantHit *hit)	{
   if(!CompareEnergy(this,hit)) {
      this->cfd    = hit->GetCfd();    
      this->psd    = hit->GetPsd();    
      this->time   = hit->GetTime();
      this->position = hit->GetPosition();
   }
   this->SetCharge(0);

   this->SetEnergy(this->GetEnergy() + hit->GetEnergy());
}

bool TDescantHit::AnalyzeWaveform() {
   bool error = false;
  
   // baseline algorithm: average of the first <n> samples
   size_t baseline_length = 10;
   double baseline = 0;
   //printf("here %d\n",waveform.size());
   if(waveform.size() < baseline_length) {
      error = true;
   } else {
      for(size_t i = 0; i < baseline_length; ++i) {
         baseline += ((double)waveform[i]);
      }
      baseline /= ((double)baseline_length);
   }
   //printf("now here %d\n",error);

   // all timing algorithms use interpolation with this many steps between two samples (all times are stored as integers)
   int interpolation_steps = 256;
      
   // cfd algorithm: attenuated minus delayed sample
   // the zero-crossing is determined by a positive sample followed by two negative samples
   int delay = 10;
   double attenuation = 11./64.;

   //printf("here2\n");
   if(waveform.size() < delay+2) {
      error = true;
   } else {
      double monitor[3] = {attenuation*(waveform[delay+2] - baseline) - (waveform[2] - baseline),
		                     attenuation*(waveform[delay+1] - baseline) - (waveform[1] - baseline),
		                     attenuation*(waveform[delay]   - baseline) - (waveform[0] - baseline)};

      for(size_t i = delay+3; i < waveform.size(); ++i) {
         if(monitor[2] >= 0. && monitor[1] < 0. && monitor[0] < monitor[1]) {
            //interpolate cfd time
            //the zero crossing is between i-2 and i-1
            cfd = (i-2. + monitor[2]/(monitor[2]-monitor[1]))*interpolation_steps;
            break;
         }
         if(i == waveform.size() -1) {
            //printf("failed to get zero-crossing of cfd monitor signal\n");
            error = true;
         }
         monitor[2] = monitor[1];
         monitor[1] = monitor[0];
         monitor[0] = attenuation*(waveform[i] - baseline) - (waveform[i-delay] - baseline);
      }
   }
   //printf("now here2 %d\n",error);

   // PSD
   // time to zero-crossing algorithm: time when sum reaches n% of the total sum minus the cfd time
   double fraction = 0.9;
   int total_sum = 0;
   for(size_t i = 0; i < waveform.size(); ++i) {
      total_sum += waveform[i] - baseline;
   }
   
   double sum = -fraction*total_sum + waveform[0] - baseline;
   for(size_t i = 1; i < waveform.size(); ++i) {
      sum += waveform[i] - baseline;
      if(sum >= 0) {
         // interpolate between i-1 and i
         psd = (i - sum/(waveform[i] - baseline))*interpolation_steps;
         break;
      }
      if(i == waveform.size() -1) {
         //printf("failed to get psd\n");
         error = true;
      }
   }
  
   return !error;
}

