


#include "TRF.h"


ClassImp(TRF)


TRF::TRF():data(0) 	{
	Clear();
}

TRF::~TRF() { if(data) delete data;	}

void TRF::FillData(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic) {
	if(!data)
   	data = new TRFFitter();
	data->FindPhase((TFragment&)(*frag));
	//TRF::Set();
	
//	TPulseAnalyzer pulse((TFragment&)(*frag));	    
//	if(pulse.IsSet()){
//		phasesfu = pulse.fit_newT0();
//	}
}


void TRF::BuildHits(TDetectorData *ddata,Option_t *opt)	{
   //TRFData *tfdata = (TRFData*)ddata;
   //if(data==0)
   //  tfdata = (this->data);
   if(!data)
      return;
   if(!data->IsSet()) 
      return;
   phase     = data->GetPhase(); 
   midastime = data->GetMidasTime();
   timestamp = data->GetTimeStamp();
   time      = data->GetTime();
}

void TRF::Clear(Option_t *opt)	{
  if(data) data->Clear(); //!
		
   phase     = -1.0; 
   //phasesfu  = -1.0; 
   midastime =  0.0;
   timestamp =  0.0;
   time      =  0.0;


}

void TRF::Print(Option_t *opt) const { } 





