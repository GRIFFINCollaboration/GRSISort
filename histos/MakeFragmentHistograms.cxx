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
		      10000, 0, 5000, frag->GetEnergy());
    if(chan->GetClassType() == TSceptar::Class()){
      obj.FillHistogram("sceptar_charge",
                      4000, 0, 4000, frag->GetCharge());
    }
    obj.FillHistogram("channel_ts_shortRuns",
                      2000, 0, 2000, frag->GetChannelNumber(),
                      1000, 0, 1000, frag->GetMidasTimeStamp()-first_timestamp);
    obj.FillHistogram("channel_ts_longRuns",
                      2000, 0, 2000, frag->GetChannelNumber(),
                      5000, 0, 50000, frag->GetMidasTimeStamp()-first_timestamp);
    if(frag->GetChannelNumber()<1199) {
      if(frag->GetSegment()==0) {
        obj.FillHistogram("hpge","core_energy",
	      	           70, 0, 70, frag->GetArrayNumber(),
		           8000, 0, 4000, frag->GetEnergy());
        obj.FillHistogram("hpge","core_charge",
	      	           70, 0, 70, frag->GetArrayNumber(),
		           10000, 0, 30000, frag->GetCharge());
      }
    }else{
      
    }
    int sharc_m = frag->GetSharcMesyBoard();
    
    //printf("address = 0x%08x\n",frag->GetAddress());
    //printf("sharc_m = %i\ni\n",sharc_m); fflush(stdout);
    if(sharc_m>-1) {
      obj.FillHistogram("sharc","mes_trigger_id", 100000, 0, 1000000, frag->GetTriggerId(),
                                                 40, 0, 40, sharc_m);


    }
  }
}

extern "C"
void MakeAnalysisHistograms(TRuntimeObjects& obj) {
  TSharc* sharc = obj.GetDetector<TSharc>();
  TTigress* tigress = obj.GetDetector<TTigress>();
  
  if(tigress){
    for(UInt_t i=0; i<tigress->GetMultiplicity(); i++){
      obj.FillHistogram("gamma_energy",
                        4000, 1, 2001, tigress->GetTigressHit(i).GetEnergy());
    
    }
    		      
  }

  if(sharc){
    for(Int_t i=0; i<sharc->GetSize(); i++){

      obj.FillHistogram("phi_theta",
                         200, -10, 190,  sharc->GetSharcHit(i)->GetThetaDeg(),
                         400, -200, 200, sharc->GetSharcHit(i)->GetPosition().Phi()*180.0/3.1415926535);
      obj.FillHistogram("energy_theta",
                         200, -10, 190,  sharc->GetSharcHit(i)->GetThetaDeg(),
                         5000, 0, 50000, sharc->GetSharcHit(i)->GetEnergy());
      obj.FillHistogram("deltaE_padE",
                         5000, 0, 50000, sharc->GetSharcHit(i)->GetPadE(),
			 1000, 0, 10000, sharc->GetSharcHit(i)->GetDeltaE());
      obj.FillHistogram("deltaE_energy",
                         5000, 0, 50000, sharc->GetSharcHit(i)->GetEnergy(),
			 1000, 0, 10000, sharc->GetSharcHit(i)->GetDeltaE());
      obj.FillHistogram("frontCharge_padCharge",
                         5000, 0, 50000, sharc->GetSharcHit(i)->GetPadCharge(),
			 1000, 0, 10000, sharc->GetSharcHit(i)->GetFrontCharge());
      obj.FillHistogram("charge_detector",
			 50, 0, 50,      sharc->GetSharcHit(i)->GetDetector(),
                         5000, 0, 50000, sharc->GetSharcHit(i)->GetCharge());
      obj.FillHistogram("detector_theta",
                         200, -10, 190,  sharc->GetSharcHit(i)->GetThetaDeg(),
			 50, 0, 50,      sharc->GetSharcHit(i)->GetDetector());
      obj.FillHistogram("detector_phi",
                         400, -200, 200, sharc->GetSharcHit(i)->GetPosition().Phi()*180.0/3.1415926535,
			 50, 0, 50,      sharc->GetSharcHit(i)->GetDetector());
      
      for(Int_t d=5; d<=16; d++){
        if(sharc->GetSharcHit(i)->GetDetector()==d){
          obj.FillHistogram(Form("%02d_frontCharge_backCharge", d),
                           1000, 0, 10000, sharc->GetSharcHit(i)->GetBackCharge(),
          		   1000, 0, 10000, sharc->GetSharcHit(i)->GetFrontCharge());
          obj.FillHistogram(Form("%02d_frontCharge_padCharge", d),
                           5000, 0, 50000, sharc->GetSharcHit(i)->GetPadCharge(),
          		   1000, 0, 10000, sharc->GetSharcHit(i)->GetFrontCharge());
          obj.FillHistogram(Form("%02d_frontCharge_charge", d),
                           5000, 0, 50000, sharc->GetSharcHit(i)->GetCharge(),
          		   1000, 0, 10000, sharc->GetSharcHit(i)->GetFrontCharge());
          obj.FillHistogram(Form("%02d_frontCharge_theta", d),
          		   200, -10, 190,  sharc->GetSharcHit(i)->GetThetaDeg(),
                           1000, 0, 10000, sharc->GetSharcHit(i)->GetFrontCharge());
        }
      }
      
      if(tigress){
        for(UInt_t t=0; t<tigress->GetMultiplicity(); t++){
	  obj.FillHistogram("gamma_sharc_energies",
	                    2000, 0, 10000, sharc->GetSharcHit(i)->GetEnergy(),
			    4000, 1,  2001, tigress->GetTigressHit(t).GetEnergy());
        }
      }
      
    }
  }
}
