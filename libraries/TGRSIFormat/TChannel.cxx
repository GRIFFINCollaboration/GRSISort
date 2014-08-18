#include"TChannel.h"
#include<fstream>
#include<fcntl.h>
#include<unistd.h>

#include <sstream>

/*
 * Author:  P.C. Bender, <pcbend@gmail.com>
 * 
 * Please indicate changes with your initials.
 * 
 *
 */


ClassImp(TChannel)

TChannel *TChannel::fTChannel = 0;

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

	copyto->ENGChi2 = source->GetENGChi2();
	copyto->LEDChi2 = source->GetLEDChi2();
	copyto->CFDChi2 = source->GetCFDChi2();
	copyto->TIMEChi2 = source->GetTIMEChi2();

}

void TChannel::AddChannel(TChannel *toadd) {
	TChannel *chan;
	chan = GetChannel(toadd->GetAddress());
	CopyChannel(chan,toadd);
	delete toadd;
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
	if(temp_address == 0) {
		TChannel *chan = new TChannel;
		return chan;
	}
		
   //if(fChannelMap->count(temp_address) == 1) {
   //   return fChannelMap->at(temp_address);
   //} else {
		// printf("making a new tchannel for: 0x%08x\n",temp_address);
      TChannel *chan;
		chan->SetName(Form("0x%08x",temp_address));
      chan->address = temp_address;
      fChannelMap->insert(std::make_pair(temp_address,chan));
      return chan;
   //}
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




//void TChannel::CalibrateFragment(TFragment *frag)	{
//	frag->ChargeCal = (float) CalibrateENG(frag->Charge);
//}


void TChannel::Print(Option_t *opt)	{
   printf( DBLUE "%s\t" DYELLOW "0x%08x" RESET_COLOR "\n",this->GetChannelName(),this->GetAddress());
	printf( "Channel: %i\t\t" DBLUE "%s" RESET_COLOR "\n",number,channelname.c_str());
	printf( "Address: " DMAGENTA "0x%08x" RESET_COLOR "\n", address);
	printf( "Energy Coefficients:\t"  );
	for(int x=0;x<ENGCoefficients.size();x++)
		printf( DRED "E[%i]: %f  " RESET_COLOR, x, ENGCoefficients[x] );
	printf( "ENGChi2:  %.02f\n");
	printf( "\n//====================================\n");
};

//void TChannel::PrintAll(Option_t *opt) {
//   TIter iter(fChannelList);
//   while(TChannel *temp = (TChannel*)(iter.Next()))
//      temp->Print();
//}



void TChannel::WriteCalFile(std::string outfilename) {
   //prints the context of addresschannelmap formatted correctly to stdout if
   //no file name is passed to the function.  If a file name is passed to the function
   //prints the context of addresschannelmap formatted correctly to a file with the given
   //name.  This will earse and rewrite the file if the file already exisits!
   //

   std::map < int, TChannel * >::iterator iter;
   FILE *c_outputfile;
   if(outfilename.length()>0) {
      c_outputfile = freopen (outfilename.c_str(),"w",stdout);
   }
   for(iter = fChannelMap->begin(); iter != fChannelMap->end(); iter++)   {
      iter->second->Print();
   }
   if(outfilename.length()>0) {
      fclose(c_outputfile);
      int fd = open("/dev/tty", O_WRONLY);
      stdout = fdopen(fd, "w");
   }
	return;
}


void TChannel::ReadCalFile(std::string infilename) {
	//does magic, make tchannels from a cal file.

	if(infilename.length()==0)
		return;

	ifstream infile;
   infile.open(infilename.c_str());
   if (!infile) {
   	printf("could not open file.\n");
      return;
   }

   TChannel *channel = 0;

   std::string line;
   int linenumber = 0;
   int newchannels = 0;

   bool creatednewchannel = false;
   bool brace_open = false;
   int detector = 0;
   std::string name;

   std::pair < int, int >pixel = std::make_pair(0, 0);

   while (std::getline(infile, line)) {
      linenumber++;
		trim(&line);
		int comment = line.find("//");
      if (comment != std::string::npos) {
         line = line.substr(0, comment);
      }
      if (!line.length())
         continue;
      int openbrace = line.find("{");
      int closebrace = line.find("}");

		//*************************************//
		if (closebrace != std::string::npos) {
         brace_open = false;
         if (channel && (channel->GetAddress()!=0) ) {         //(name.length() > 0)) {  //&& pixel.first && pixel.second )
            AddChannel(channel);
            newchannels++;
         } else
            delete channel;
         channel = 0;
         name.clear();
         detector = 0;
		}
		//*************************************//
      if (openbrace != std::string::npos) {
         brace_open = true;
         name = line.substr(0, openbrace).c_str();
         channel = GetChannel(0);
         channel->SetChannelName(name.c_str());
      }
		//*************************************//


      if (brace_open) {
	      int ntype = line.find(":");
         if (ntype != std::string::npos) {
				std::string type = line.substr(0, ntype);
            line = line.substr(ntype + 1, line.length());
            trim(&line);
            std::istringstream ss(line);
				int j = 0;
				while (type[j]) {
					char c = *(type.c_str() + j);
					c = toupper(c);
					type[j++] = c;
				}
				if(type.compare("NAME")==0) {
					channel->SetChannelName(line.c_str());
				} else if(type.compare("ADDRESS")==0) {
					int tempadd; ss>>tempadd;
					channel->SetAddress(tempadd);
				} else if(type.compare("INTEGRATION")==0) {
					int tempint; ss>>tempint;
					channel->SetIntegration(tempint);
				} else if(type.compare("NUMEBR")==0) {
					int tempnum; ss>>tempnum;
					channel->SetNumber(tempnum);
				} else if(type.compare("STREAM")==0) {
					int tempstream; ss>>tempstream;
					channel->SetStream(tempstream);
				} else if(type.compare("DIGITZER")==0) {
					channel->SetDigitizerType(line);
				} else if(type.compare("ENGCHI2")==0) {
					double tempdbl; ss>>tempdbl;
					channel->SetENGChi2(tempdbl);
				} else if(type.compare("CFDCHI2")==0) {
					double tempdbl; ss>>tempdbl;
					channel->SetCFDChi2(tempdbl);
				} else if(type.compare("LEDCHI2")==0) {
					double tempdbl; ss>>tempdbl;
					channel->SetLEDChi2(tempdbl);
				} else if(type.compare("TIMECHI2")==0) {
					double tempdbl; ss>>tempdbl;
					channel->SetTIMEChi2(tempdbl);
				} else if(type.compare("ENGCOEFF")==0) {
	            channel->DestroyENGCal();
               double value;
               while (ss >> value) {	channel->AddENGCoefficient(value); }
				} else if(type.compare("LEDCOEFF")==0) {
	            channel->DestroyLEDCal();
               double value;
               while (ss >> value) {	channel->AddLEDCoefficient(value); }
				} else if(type.compare("CFDCOEFF")==0) {
	            channel->DestroyCFDCal();
               double value;
               while (ss >> value) {	channel->AddCFDCoefficient(value); }
				} else if(type.compare("TIMECOEFF")==0) {
	            channel->DestroyTIMECal();
               double value;
               while (ss >> value) {	channel->AddTIMECoefficient(value); }
				} else  {

				}
			}
		}

	}


	return;
}




void TChannel::trim(std::string * line, const std::string & trimChars) {
   if (line->length() == 0)
      return;
   std::size_t found = line->find_first_not_of(trimChars);
   if (found != std::string::npos)
      *line = line->substr(found, line->length());
   found = line->find_last_not_of(trimChars);
   if (found != std::string::npos)
      *line = line->substr(0, found + 1);
   return;
}








