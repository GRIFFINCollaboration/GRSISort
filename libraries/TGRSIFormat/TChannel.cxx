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
std::map<int,TChannel*> *TChannel::fChannelNumberMap = new std::map<int,TChannel*>;

TChannel::TChannel(const char *temp_name)	{
   Clear();
   this->SetName(temp_name);  
   this->channelname = temp_name;
}

void TChannel::DeleteAllChannels() {

   std::map < int, TChannel * >::iterator iter;
   for(iter = fChannelMap->begin(); iter != fChannelMap->end(); iter++)   {
      delete iter->second;
		iter->second = 0;
   }
	fChannelMap->clear();
	fChannelNumberMap->clear();
	return;
}

void TChannel::CopyChannel(TChannel *copyto,TChannel *source) {
   if(!copyto || !source)
      return;
	//printf("in copy\n");
   //TChannel *found       = GetChannel(temp_chan->GetAddress());
	//printf("*****************************\n");
	//printf("*********BEFORE**************\n");
	//printf("*****************************\n");

	//copyto->Print(); source->Print();

	if(copyto->address ==0 )
	   copyto->address        = source->GetAddress();
	if(source->GetIntegration())
	   copyto->integration    = source->GetIntegration();
	//if(copyto->number != 0) // source->GetNumber())
	if(source->GetNumber())
	   copyto->number         = source->GetNumber();
	if(source->GetStream())
	   copyto->stream         = source->GetStream();
	if(source->GetUserInfoNumber())
	   copyto->userinfonumber = source->GetUserInfoNumber();
	if(strlen(source->GetChannelName())>0) {
		//printf("copying channel name: %s\n",source->GetChannelName());
	   copyto->channelname    = source->GetChannelName();
	}
	if(strlen(source->GetDigitizerType())>0)
		copyto->SetDigitizerType(source->GetDigitizerType());
   //copyto->SetName(source->GetName());
   
	if(source->GetENGCoeff().size()>0)
		//copyto->DestroyENGCal();
		copyto->ENGCoefficients = source->GetENGCoeff();
	if(source->GetCFDCoeff().size()>0)
		//copyto->DestroyCFDCal();
	   copyto->CFDCoefficients  = source->GetCFDCoeff();
	if(source->GetLEDCoeff().size()>0)
		//copyto->DestroyLEDCal();
   	copyto->LEDCoefficients  = source->GetLEDCoeff();
	if(source->GetTIMECoeff().size()>0)
		//copyto->DestroyTIMECal();
	   copyto->TIMECoefficients = source->GetTIMECoeff();

	if(source->GetENGChi2())
		copyto->ENGChi2 = source->GetENGChi2();
	if(source->GetLEDChi2())
		copyto->LEDChi2 = source->GetLEDChi2();
	if(source->GetCFDChi2())
		copyto->CFDChi2 = source->GetCFDChi2();
	if(source->GetTIMEChi2())
		copyto->TIMEChi2 = source->GetTIMEChi2();


	//printf("*****************************\n");
	//printf("*********AFTER***************\n");
	//printf("*****************************\n");

	//copyto->Print(); source->Print();

	//printf("-----------------------------\n");
	//printf("-----------------------------\n");
	//printf("-----------------------------\n\n\n");
	//printf("-----------------------------\n");

}

void TChannel::AddChannel(TChannel *toadd,Option_t *opt) {
	TChannel *chan = 0;
	if(fChannelMap->count(toadd->GetAddress())==0) {
		if(!(chan = FindChannelByNumber(toadd->GetNumber()))) 	
			chan = GetChannel(toadd->GetAddress());
		CopyChannel(chan,toadd);
		if(strcmp(opt,"save")!=0)
			delete toadd;
	} else {
		chan = GetChannel(toadd->GetAddress());
		CopyChannel(chan,toadd);
		if(strcmp(opt,"save")!=0)
			delete toadd;
	}	
	return;
}

TChannel::TChannel() {  };


TChannel::~TChannel()	{	}

void TChannel::Clear(Option_t *opt){  
   address				=	0xffffffff;
   integration			=	0;
   number				=	0;
   stream            =  0;
   ENGChi2           =  0.0;
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
	TChannel *chan = 0;
	if(fChannelMap->count(temp_address)==0) {
		fChannelMap->insert(std::make_pair(temp_address,new TChannel));
   	chan = fChannelMap->at(temp_address);
		chan->SetName(Form("0x%08x",temp_address));
   	chan->address = temp_address;
	   fChannelMap->insert(std::make_pair(temp_address,chan));
	} else {
		chan = fChannelMap->at(temp_address);	
		if(chan->GetNumber() != 0)
			fChannelNumberMap->insert(std::make_pair(chan->GetNumber(),chan));

	}
   return chan;
}

void TChannel::UpdateChannelNumberMap() {
   std::map < int, TChannel * >::iterator iter1;
	for(iter1 = fChannelMap->begin(); iter1 != fChannelMap->end(); iter1++) {
		if(fChannelNumberMap->count(iter1->second->GetNumber())==0) {
			fChannelNumberMap->insert(std::make_pair(iter1->second->GetNumber(),iter1->second));
		}
	}

}



TChannel *TChannel::FindChannel(int temp_address) {
	if(fChannelMap->count(temp_address)==0)
		return 0;    //not found.
	return fChannelMap->at(temp_address);
}


TChannel *TChannel::FindChannelByNumber(int temp_number) {
	if(fChannelMap->size() != fChannelNumberMap->size()) {
		//printf("fChannelMap->size():%i fChannelNumberMap->size():%i \n",fChannelMap->size(),fChannelNumberMap->size());
		UpdateChannelNumberMap();
	}
	if(fChannelNumberMap->count(temp_number)==0)
		return 0;    //not found.
	return fChannelNumberMap->at(temp_number);
}


void TChannel::SetChannel(int taddress,int tnumber,std::string tname)	{
	address	=	taddress; 
	number	=	tnumber; 
	this->SetChannelName(tname.c_str());								
}





void TChannel::DestroyENGCal()	{
	ENGCoefficients.erase(ENGCoefficients.begin(),ENGCoefficients.end());
}

void TChannel::DestroyCFDCal()	{
	CFDCoefficients.erase(CFDCoefficients.begin(),CFDCoefficients.end());
}

void TChannel::DestroyLEDCal()	{
	LEDCoefficients.erase(LEDCoefficients.begin(),LEDCoefficients.end());
}

void TChannel::DestroyTIMECal()	{
	LEDCoefficients.erase(TIMECoefficients.begin(),TIMECoefficients.end());
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
		temp_int = (double)integration;  //the 4 is the dis. 
	
	double cal_chg = 0.0;
	for(int i=0;i<ENGCoefficients.size();i++){
		cal_chg += ENGCoefficients[i] * pow((charge/temp_int),i);
	}
	//printf("(%.02f/%0.2f) *%.02f + %.02f = %.02f\n",charge,temp_int,ENGCoefficients[1],ENGCoefficients[0], cal_chg);
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
   //printf( DBLUE "%s\t" DYELLOW "0x%08x" RESET_COLOR "\n",this->GetChannelName(),this->GetAddress());
	printf( "%s\t{\n",channelname.c_str());
	printf( "Name:      %s\n",channelname.c_str());
	printf( "Number:    %i\n",number);
	printf( "Address:   0x%08x\n", address);
	printf( "Digitizer: %s\n",digitizertype.c_str()); 
	printf( "EngCoeff:  "  );
	for(int x=0;x<ENGCoefficients.size();x++)
		printf( "%f\t", ENGCoefficients.at(x) );
	printf("\n");
	printf("Integration: %i\n",integration);
	printf( "ENGChi2:   %f\n",ENGChi2);
	printf("\n}\n");
	printf( "//====================================//\n");
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


void TChannel::ReadCalFile(const char *filename) {
	//does magic, make tchannels from a cal file.
	std::string infilename;
	infilename.append(filename);

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
			//printf("brace closed.\n");
			//channel->Print();
         brace_open = false;
         if (channel && (channel->GetAddress()!=0) ) {       
				//channel->Print();
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
				//printf("type = %s\n",type.c_str());
				if(type.compare("NAME")==0) {
					channel->SetChannelName(line.c_str());
				} else if(type.compare("ADDRESS")==0) {
					int tempadd =0; ss>>tempadd;
					if(tempadd == 0) { //maybe it is in hex...
						std::stringstream newss;
						newss << std::hex << line;
						newss >> tempadd;
					}
					tempadd = tempadd &0x00ffffff; //front end number is not included in the odb...
					channel->SetAddress(tempadd);
				} else if(type.compare("INTEGRATION")==0) {
					int tempint; ss>>tempint;
					channel->SetIntegration(tempint);
				} else if(type.compare("NUMBER")==0) {
					int tempnum; ss>>tempnum;
					channel->SetNumber(tempnum);
				} else if(type.compare("STREAM")==0) {
					int tempstream; ss>>tempstream;
					channel->SetStream(tempstream);
				} else if(type.compare("DIGITZER")==0) {
					channel->SetDigitizerType(line.c_str());
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
	printf("parsed %i lines.\n",linenumber);

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








