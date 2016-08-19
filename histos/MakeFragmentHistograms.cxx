#include "TRuntimeObjects.h"
#include "TFragment.h"

extern "C"
void MakeFragmentHistograms(TRuntimeObjects& obj) {  
  TFragment* frag = obj.GetFragment();

  obj.FillHistogram("x",
		    100,0,100,75);
  if(frag) {
    obj.FillHistogram("channel",
		      2000, 0, 2000, frag->GetChannelNumber());
    obj.FillHistogram("channel_energy",
		      2000, 0, 2000, frag->GetChannelNumber(),
		      30000, 0, 30000, frag->GetCharge());
  }
}
