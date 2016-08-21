#include "TRuntimeObjects.h"
#include "TFragment.h"
#include "TSceptar.h"
#include "TChannel.h"
#include "TTigress.h"
#include "TSharc.h"

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
    if(frag->GetChannelNumber()<1199) {
      if(frag->GetSegment()==0) {
        obj.FillHistogram("hpge","core_energy",
	      	           70,0,70, frag->GetArrayNumber(),
		           8000, 0, 4000, frag->GetEnergy());
        obj.FillHistogram("hpge","core_charge",
	      	           70,0,70, frag->GetArrayNumber(),
		           10000, 0, 30000, frag->GetCharge());
      }
    }
  }
}

extern "C"
void MakeAnalysisHistograms(TRuntimeObjects& obj) {
  TSharc* sharc = obj.GetDetector<TSharc>();
  if(sharc){
    for(Int_t i=0; i<sharc->GetSize(); i++){

      obj.FillHistogram("phi_theta",
                        200, -10, 190,  sharc->GetSharcHit(i)->GetTheta()*180.0/3.14159,
                        400, -200, 200, sharc->GetSharcHit(i)->GetPosition().Phi()*180.0/3.14159);
    }
  }
}
