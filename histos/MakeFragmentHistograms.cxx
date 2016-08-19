#include "TRuntimeObjects.h"
#include "TFragment.h"
#include "TSceptar.h"
#include "TChannel.h"

extern "C"
void MakeFragmentHistograms(TRuntimeObjects& obj) {  
  TFragment* frag = obj.GetFragment();
  TChannel* chan = frag->GetChannel();

  if(frag && chan) {
    obj.FillHistogram("channel",
		      2000, 0, 2000, frag->GetChannelNumber());
    obj.FillHistogram("channel_energy",
		      2000, 0, 2000, frag->GetChannelNumber(),
		      30000, 0, 30000, frag->GetCharge());
    if(chan->GetClassType() == TSceptar::Class()){
      obj.FillHistogram("sceptar_charge",
                      4000,0,4000, frag->GetCharge());
    }
                
  }
}
