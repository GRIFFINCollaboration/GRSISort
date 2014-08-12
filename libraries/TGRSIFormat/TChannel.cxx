#include"TChannel.h"

ClassImp(TChannel)

TChannel *TChannel::fTChannel = 0;

TList *TChannel::fChannelList = new TList();
std::map<int,TChannel*> *TChannel::fChannelMap = new std::map<int,TChannel*>;

TChannel::TChannel(const char *temp_name)	{
   Clear();
   this->SetName(temp_name);  
   this->channelname = temp_name;
}

void TChannel::CopyChannel(TChannel *copyto,TChannel *source) {
   if(!copyto || !source)
      return;
   //TChannel *found       = GetChannel(temp_chan->GetAddress());
   copyto->address        = source->GetAddress();
   copyto->integration    = source->GetIntegration();
   copyto->number         = source->GetNumber();
   copyto->stream         = source->GetStream();
   copyto->userinfonumber = source->GetUserInfoNumber();
   copyto->channelname    = source->GetChannelName();
   copyto->SetName(source->GetName());
   
   copyto->ENGCoefficients  = source->GetENGCoeff();
   copyto->CFDCoefficients  = source->GetCFDCoeff();
   copyto->LEDCoefficients  = source->GetLEDCoeff();
   copyto->TIMECoefficients = source->GetTIMECoeff();

}

TChannel::TChannel() {  };


TChannel::~TChannel()	{	}

void TChannel::Clear(Option_t *opt){  
   address				=	0xffffffff;
   integration			=	0;
   number				=	0;
   stream            =  0;
   userinfonumber    =  0xffffffff;

   ENGCoefficients.clear();
   CFDCoefficients.clear();
   LEDCoefficients.clear();
   TIMECoefficients.clear();
}

TChannel *TChannel::GetChannel(int temp_address) {
   if(fChannelMap->count(temp_address) == 1) {
      return fChannelMap->at(temp_address);
   } else {
//      printf("making a new tchannel for: 0x%08x\n",temp_address);
      char buffer[64];
      TChannel *chan;
//      chan->address = temp_address;
      //if(temp_address == 0xffffffff)
      //   sprintf(buffer,"temp");
      // else
      sprintf(buffer,"%i",temp_address);
      if(!(chan = (TChannel*)fChannelList->FindObject(buffer))) {
         //printf("chan = 0x%08x\n",chan);
         chan = new TChannel(buffer);
         fChannelList->Add(chan);
      }
      chan->address = temp_address;
      fChannelMap->insert(std::make_pair(temp_address,chan));
      return chan;
   }
   return 0; //chan;

/*   std::map<int,TChannel*>::const_iterator found;
   if(temp_address == 0xffffffff) {
      if(fTChannel==0)  {
         fTChannel = new TChannel("temp");
      } else {
         fTChannel->Clear();
         fTChannel->SetName("temp");
      }
      return fTChannel;
   } else if( (found=fChannelMap->find(temp_address)) == fChannelMap->end()) { //does not yet exist.
      fChannelMap->at(temp_address) = new TChannel("chan_temp");
      return fChannelMap->at(temp_address);
   } else {  // already exists.
      return found->second;

      //if(strcmp(found->second->GetName(),"chan_temp")==0) {
      //   return found->second;
      //} else if((TChannel *ptr = fChannelList->FindObject(found->second->GetName())) != 0) {
      //   if( ptr == found->second) {
      //      return fChannelMap[temp_address];
      //   } else {
      //      printf(RED "Warning: found two addresses for the smae TChannel!\t%08x\t%08x%\t%s" RESET_COLOR "\n",
      //      ptr,found->second,found->second->GetName());
      //      return ptr;
      //   }
      //}  
   }*/
}


/*
TChannel *TChannel::GetChannel(const char *temp_name) {
   TChannel *temp = 0;
   if(strcmp(temp_name,"")==0) {     
      if(fTChannel==0)  {
         fTChannel = new TChannel("temp");
      } else {
         fTChannel->Clear();
         fTChannel->SetName("temp");
      }
      return fTChannel;
   } else if(!(temp=(TChannel*)(fChannelList->FindObject(temp_name)))) {
      temp = new TChannel(temp_name);
      fChannelList->Add(temp);
   }
   return temp;
}*/





void TChannel::SetChannel(int taddress,int tnumber,std::string tname)	{
	address	=	taddress; 
	number	=	tnumber; 
	this->SetChannelName(tname.c_str());								
}





void TChannel::DestroyENGCal()	{
	ENGCoefficients.clear();
}

void TChannel::DestroyCFDCal()	{
	CFDCoefficients.clear();
}

void TChannel::DestroyLEDCal()	{
	LEDCoefficients.clear();
}

void TChannel::DestroyTIMECal()	{
	LEDCoefficients.clear();
}


void TChannel::DestroyCalibrations()	{
	DestroyENGCal();
	DestroyCFDCal();
	DestroyLEDCal();
	DestroyTIMECal();	
};


double TChannel::CalibrateENG(int charge)	{
	return CalibrateENG((double)charge + gRandom->Uniform());
};

double TChannel::CalibrateENG(double charge)	{
	if(ENGCoefficients.size()==0)
		return charge;
	
	double temp_int = 125.0;
	if(integration != 0)
		temp_int = integration/4;  //the 4 is the dis. 
	
	double cal_chg = 0.0;
	for(int i=0;i<ENGCoefficients.size();i++){
		cal_chg += ENGCoefficients[i] * pow((charge/temp_int),i);
	}
	return cal_chg;
};


double TChannel::CalibrateCFD(int cfd)	{
	return CalibrateCFD((double)cfd + gRandom->Uniform());
};

double TChannel::CalibrateCFD(double cfd)	{
	if(CFDCoefficients.size()==0)
		return cfd;
	
	double cal_cfd = 0.0;
	for(int i=0;i<CFDCoefficients.size();i++){
		cal_cfd += CFDCoefficients[i] * pow(cfd,i);
	}
	return cal_cfd;
};


double TChannel::CalibrateLED(int led)	{
	return CalibrateLED((double)led + gRandom->Uniform());
};

double TChannel::CalibrateLED(double led)	{
	if(LEDCoefficients.size()==0)
		return led;
	
	double cal_led = 0.0;
	for(int i=0;i<LEDCoefficients.size();i++){
		cal_led += LEDCoefficients[i] * pow(led,i);
	}
	return cal_led;
}


double TChannel::CalibrateTIME(int time)	{
	return CalibrateTIME((double)time + gRandom->Uniform());
};

double TChannel::CalibrateTIME(double time)	{
	if(TIMECoefficients.size()==0)
		return time;
	
	double cal_time = 0.0;
	for(int i=0;i<TIMECoefficients.size();i++){
		cal_time += TIMECoefficients[i] * pow(time,i);
	}
	return cal_time;
}




//void TChannel::CalibrateFragment(TTigFragment *frag)	{
	
//	frag->ChannelNumber = number;
//	frag->ChannelName	=	name;
	
//	frag->ChargeCal = (float) CalibrateENG(frag->Charge);
	
//}


void TChannel::Print(Option_t *opt)	{
   printf( DBLUE "%s\t" DYELLOW "0x%08x" RESET_COLOR "\n",this->GetChannelName(),this->GetAddress());
	printf( "Channel: %i\t\t" DBLUE "%s" RESET_COLOR "\n",number,channelname.c_str());
	printf( "Address: " DMAGENTA "0x%08x" RESET_COLOR "\n", address);
//	printf( "Energy Coefficients{%i}:\n", ENGCoefficients.size() );
//   printf( "\t");
//	for(int x=0;x<ENGCoefficients.size();x++)
//		printf( DRED "E[%i]: %f  " RESET_COLOR, x, ENGCoefficients[x] );
	printf( "\n====================================\n");
};

void TChannel::PrintAll(Option_t *opt) {
   TIter iter(fChannelList);
   while(TChannel *temp = (TChannel*)(iter.Next()))
      temp->Print();
}


