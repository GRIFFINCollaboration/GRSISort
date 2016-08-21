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
    obj.FillHistogram("channel_energy",
		      2000, 0, 2000, frag->GetChannelNumber(),
		      5000, 0, 50000, frag->GetEnergy());
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
    int sharc_m = frag->GetSharcMesyBoard();

    //printf("address = 0x%08x\n",frag->GetAddress());
    //printf("sharc_m = %i\ni\n",sharc_m); fflush(stdout);
    if(sharc_m>-1) {
      obj.FillHistogram("sharc","mes_trigger_id",100000,0,1000000,frag->GetTriggerId(),
                                                 40,0,40,sharc_m);


    }
  }
}

extern "C"
void MakeAnalysisHistograms(TRuntimeObjects& obj) {
  TSharc* sharc = obj.GetDetector<TSharc>();
  if(sharc){
    for(Int_t i=0; i<sharc->GetSize(); i++){

      obj.FillHistogram("sharc_diag", "phi_theta",
                        200, -10, 190,  sharc->GetSharcHit(i)->GetTheta()*180.0/3.14159,
                        400, -200, 200, sharc->GetSharcHit(i)->GetPosition().Phi()*180.0/3.14159);
      obj.FillHistogram("sharc_diag", "energy_theta",
                        200, -10, 190,  sharc->GetSharcHit(i)->GetTheta()*180.0/3.14159,
                        5000, 0, 50000, sharc->GetSharcHit(i)->GetEnergy());
      obj.FillHistogram("sharc_diag", "detnum",
                        20, 0, 20, sharc->GetSharcHit(i)->GetDetector());

      obj.FillHistogram("sharc_diag", Form("front_back_%02d",sharc->GetSharcHit(i)->GetDetector()),
                        60, 0, 60, sharc->GetSharcHit(i)->GetFrontStrip(),
                        60, 0, 60, sharc->GetSharcHit(i)->GetBackStrip());

    }
  }
}
