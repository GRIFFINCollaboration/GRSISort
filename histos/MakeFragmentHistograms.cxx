#include "TRuntimeObjects.h"
#include "TFragment.h"
#include "TSceptar.h"
#include "TChannel.h"

extern "C"
void MakeFragmentHistograms(TRuntimeObjects& obj) {
  TFragment* frag = obj.GetFragment();
  TChannel* chan = frag->GetChannel();

  static long first_timestamp = 0;

  if(frag && chan) {
    if(!first_timestamp) {
      first_timestamp = frag->GetMidasTimeStamp();
    }

    obj.FillHistogram("channel",
		      2000, 0, 2000, frag->GetChannelNumber());
    obj.FillHistogram("channel_charge",
		      2000, 0, 2000, frag->GetChannelNumber(),
		      3000, 0, 30000, frag->GetCharge());
    if(chan->GetClassType() == TSceptar::Class()){
      obj.FillHistogram("sceptar_charge",
                      4000,0,4000, frag->GetCharge());
    }
    obj.FillHistogram("channel_ts",
                      2000, 0, 2000, frag->GetChannelNumber(),
                      3600, 0, 3600, frag->GetMidasTimeStamp()-first_timestamp);

  }
}
