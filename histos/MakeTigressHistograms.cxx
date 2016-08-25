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

     obj.FillHistogram("sharc_size",sharc->GetSize(),10,0,10);


    for(Int_t i=0; i<sharc->GetSize(); i++){

      TSharcHit *shit = sharc->GetSharcHit(i);

      obj.FillHistogram("sharc_diag", "phi_theta",
                        200, -10, 190,  shit->GetTheta()*180.0/3.14159,
                        400, -200, 200, shit->GetPosition().Phi()*180.0/3.14159);
      obj.FillHistogram("sharc_diag", "energy_theta",
                        200, -10, 190,  shit->GetTheta()*180.0/3.14159,
                        5000, 0, 50000, shit->GetEnergy());
      obj.FillHistogram("sharc_diag", "detnum",
                        20, 0, 20, shit->GetDetector());

      obj.FillHistogram("sharc_diag", Form("front_back_%02d",shit->GetDetector()),
                        60, 0, 60, shit->GetFrontStrip(),
                        60, 0, 60, shit->GetBackStrip());






      obj.FillHistogram("phi_theta",
                         200, -10, 190,  shit->GetThetaDeg(),
                         400, -200, 200, shit->GetPosition().Phi()*180.0/3.1415926535);
      
      obj.FillHistogram("energy_theta",
                         200, -10, 190,  shit->GetThetaDeg(),
                         5000, 0, 50000, shit->GetEnergy());
      
      obj.FillHistogram("denergy_theta",
                         200, -10, 190,  shit->GetThetaDeg(),
                         5000, 0, 50000, shit->GetDeltaE());
      
      obj.FillHistogram("deltaE_padE",
                         5000, 0, 50000, shit->GetPadE(),
			 1000, 0, 10000, shit->GetDeltaE());

      obj.FillHistogram("deltaE_energy",
                         5000, 0, 50000, shit->GetEnergy(),
			 1000, 0, 10000, shit->GetDeltaE());
     
      //obj.FillHistogram("detectors",Form("frontCharge_padCharge_%02i",shit->GetDetector()),
      //                   5000, 0, 50000, shit->GetPadCharge(),
      //                   1000, 0, 10000, shit->GetFrontCharge());
   

      obj.FillHistogram("detectors_energy",Form("energy_theta_%02i",shit->GetDetector()),
                         200, -10, 190,  shit->GetThetaDeg(),
                         5000, 0, 50000, shit->GetEnergy());
      
      obj.FillHistogram("detectors_energy",Form("denergy_theta_%02i",shit->GetDetector()),
                         200, -10, 190,  shit->GetThetaDeg(),
                         5000, 0, 50000, shit->GetDeltaE());
      
      obj.FillHistogram("detectors_energy",Form("deltaE_padE_%02i",shit->GetDetector()),
                         5000, 0, 50000, shit->GetPadE(),
			 1000, 0, 10000, shit->GetDeltaE());





      obj.FillHistogram("detectors_charge",Form("frontCharge_backCharge_%02d", shit->GetDetector()),
                       1000, 0, 10000, shit->GetBackCharge(),
      		   1000, 0, 10000, shit->GetFrontCharge());
      obj.FillHistogram("detectors_charge",Form("frontCharge_padCharge_%02d", shit->GetDetector()),
                       5000, 0, 50000, shit->GetPadCharge(),
      		   1000, 0, 10000, shit->GetFrontCharge());
      obj.FillHistogram("detectors_charge",Form("frontCharge_charge_%02d", shit->GetDetector()),
                       5000, 0, 50000, shit->GetCharge(),
      		   1000, 0, 10000, shit->GetFrontCharge());
      obj.FillHistogram("detectors_charge",Form("frontCharge_theta_%02d", shit->GetDetector()),
      		   200, -10, 190,  shit->GetThetaDeg(),
                       1000, 0, 10000, shit->GetFrontCharge());



      
      if(tigress){
        for(UInt_t t=0; t<tigress->GetMultiplicity(); t++){
	  TTigressHit &thit = tigress->GetTigressHit(t);
	  obj.FillHistogram("tigress","gamma_sharc_energies",
	                    2000, 0, 10000, shit->GetEnergy(),
			    4000, 0,  2000, thit.GetEnergy());
          if(shit->GetDetector()<9) {
	    obj.FillHistogram("tigress","gammas_upstream",4000,0,4000,thit.GetDoppler(0.10));
          } else {
	    obj.FillHistogram("tigress","gammas_downstream",4000,0,4000,thit.GetDoppler(0.10));
          }
	  obj.FillHistogram("tigress","all_doppler",4000,0,8000,thit.GetDoppler(0.10));
          double beta = 0.0;
          for(int j=0;j<50;j++) {
	    beta += 0.12/50.;
  	      obj.FillHistogram("tigress","gamma_sharc_energies",
	                         2000, 0, 10000, thit.GetDoppler(beta),
				 50,0,0.12,beta);
          }
        }
      }
      
    }
  }
}
