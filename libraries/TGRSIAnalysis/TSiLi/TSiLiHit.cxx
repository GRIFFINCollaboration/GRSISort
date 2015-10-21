#include "TSiLi.h"
#include "TSiLiHit.h"

ClassImp(TSiLiHit)

TSiLiHit::TSiLiHit()  {  }

TSiLiHit::~TSiLiHit()  {  }


void TSiLiHit::Clear(Option_t *opt)  {
  segment = -1;
  energy  = 0.0;
  cfd     = 0.0;
  charge  = -1;
  time    = -1;
  ts      = -1;
  time_fit= -1;

}


void TSiLiHit::Print(Option_t *opt) const {
  printf("===============\n");
  printf("not yet written\n");
  printf("===============\n");
}


Double_t TSiLiHit::fit_time(TFragment &frag){
	
	TWaveformAnalyzer doug(&frag);
	
	return doug.fit_newT0();
	
	
// 	short wave[1024];
// 	WaveFormPar* wpar=new WaveFormPar;
// 
// 
// 	if(frag.wavebuffer.size()>0){
// 
// 		//ONLY FOR DATA WITH PEAK SIZE > 30, ROUGHLY
// 		//ONLY IF RISE IS NOT IN FIRST 30 CHANNELS (large noise can fool this simple check)
// 		if(frag.wavebuffer[frag.wavebuffer.size()-1]-frag.wavebuffer[0]>30&&
// 		frag.wavebuffer[30]-frag.wavebuffer[0]<frag.wavebuffer[frag.wavebuffer.size()-1]-frag.wavebuffer[30]){
// 
// 			//transfer frag.wavebuffer to an array because Kris is oldschool
// 			for(int j=0;(unsigned)j<frag.wavebuffer.size();j++)wave[j]=frag.wavebuffer[j];
// 
// 			//Do the fitting
// 			double XX=fit_newT0(frag.wavebuffer.size(), &wave[0], wpar);						
// 
// 			//ONLY IF THE NOISE (on the baseline) IS LESS THAN X TIMES (simple) SIGNAL SIZE
// 			//Basically a signal to noise gate, but its not great and kind of requires a decent fit
// 			//if(wpar->t0!=0&&wpar->max-wpar->baselinefin > wpar->baselineStDevfin*20){//More strict option
// 			if(wpar->t0!=0&&wpar->max-wpar->baselinefin > wpar->baselineStDevfin*0.8){
// 				// print_WavePar(wpar);
// 				return wpar->t0;
// 
// 			}
// 		}
// 	}
//
//	return 0.0;
}