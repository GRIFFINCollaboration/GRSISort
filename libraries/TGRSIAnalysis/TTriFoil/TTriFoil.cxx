


#include "TTriFoil.h"


ClassImp(TTriFoil)


TTriFoil::TTriFoil():data(0) 	{
	Clear();
}

TTriFoil::~TTriFoil() 	{	}

void TTriFoil::Clear(Option_t *opt)	{
//Clears all of the hits and data
   TDetector::Clear(opt);
   if(data) data->Clear();
   tf_wave.clear();
   timestamp =0;
   beam = false;
   tbeam = 0;
}

void TTriFoil::Copy(TObject &rhs) const {
   TDetector::Copy(rhs);
   static_cast<TTriFoil&>(rhs).data      = 0;
   static_cast<TTriFoil&>(rhs).tf_wave    = tf_wave;
   static_cast<TTriFoil&>(rhs).timestamp = timestamp;
   static_cast<TTriFoil&>(rhs).beam      = beam;
   static_cast<TTriFoil&>(rhs).tbeam     = tbeam;
  return;                                      
}                                       

void TTriFoil::FillData(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic) {
	if(!data)
   	data = new TTriFoilData();
	data->SetData(frag,channel,mnemonic);
	//TTriFoil::Set();
}

TTriFoil::TTriFoil(const TTriFoil& rhs) : TDetector() {
  Class()->IgnoreTObjectStreamer(kTRUE);
  rhs.Copy(*this);
}

void TTriFoil::BuildHits(TDetectorData *ddata,Option_t *opt)	{
  TTriFoilData *tfdata = (TTriFoilData*)ddata;
   if(tfdata==0)
     tfdata = (this->data);
   if(!tfdata)
      return;

	if(!tfdata->GetWaveBuffer().empty())	{
		tf_wave = tfdata->GetWaveBuffer();
	}
	//timestamp = frag->MidasTimeStamp;
	int max = 0;
	int imax =0;
	for(size_t i=0;i<tf_wave.size();i++){
		if(tf_wave[i]>max){
			max = tf_wave[i];
			imax = i;
		}
	}
	if(max>2000){
		beam = true;
		tbeam = imax;
	}

}

void TTriFoil::Print(Option_t *opt) const { } 





