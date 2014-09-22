#include"TChannel.h"

#include <stdexcept>
#include<fstream>
#include<fcntl.h>
#include<unistd.h>

#include <vector>
#include <sstream>
#include <algorithm>
/*
 * Author:  P.C. Bender, <pcbend@gmail.com>
 * 
 * Please indicate changes with your initials.
 * 
 *
 */

//////////////////////////////////////////////////////////////
//                                                          //
// TChannel                                                 //
//                                                          //
// A TChannel contains the information about the digitizers //
// used in the current data set. Most of the information is //
// read out of the ODB information in the MIDAS file.       //
//                                                          //
//////////////////////////////////////////////////////////////

ClassImp(TChannel)

std::map<unsigned int,TChannel*> *TChannel::fChannelMap = new std::map<unsigned int,TChannel*>; // global maps of channels
std::map<int,TChannel*> *TChannel::fChannelNumberMap = new std::map<int,TChannel*>;

TChannel::TChannel() { Clear(); }  //default constructor need to write to root file.

TChannel::~TChannel(){}

TChannel::TChannel(const char *temp_name) {
   Clear();
   SetName(temp_name);
   channelname = temp_name;
}

TChannel::TChannel(TChannel *chan) {
//Makes a copy of a the TChannel. 

    this->SetAddress(chan->GetAddress());
    this->SetIntegration(chan->GetIntegration());
    this->SetNumber(chan->GetNumber());
    this->SetStream(chan->GetStream());
    this->SetUserInfoNumber(chan->GetUserInfoNumber());
    this->SetChannelName(chan->GetChannelName());
    this->SetDigitizerType(chan->GetDigitizerType());

    this->SetENGCoefficients(chan->GetENGCoeff());
    this->SetCFDCoefficients(chan->GetCFDCoeff());
    this->SetLEDCoefficients(chan->GetLEDCoeff());
    this->SetTIMECoefficients(chan->GetTIMECoeff());

    this->SetENGChi2(chan->GetENGChi2());
    this->SetCFDChi2(chan->GetCFDChi2());
    this->SetLEDChi2(chan->GetLEDChi2());
    this->SetTIMEChi2(chan->GetTIMEChi2());
}


bool TChannel::Compare(const TChannel &chana,const TChannel &chanb) {
	//printf("here 1.1\n");
   std::string namea; namea.assign(((TChannel)chana).GetChannelName());
	//printf("here 1.2\n");
	
   if(namea.compare(((TChannel)chanb).GetChannelName()) <= 0) return true;
   else return false;
}

void TChannel::DeleteAllChannels() {
//Safely deletes fChannelMap
   std::map < unsigned int, TChannel * >::iterator iter;
   for(iter = fChannelMap->begin(); iter != fChannelMap->end(); iter++)   {
		if(iter->second)
	      delete iter->second;
      iter->second = 0;
   }
   fChannelMap->clear();
   fChannelNumberMap->clear();
   return;
}

void TChannel::AddChannel(TChannel *chan,Option_t *opt) {
//Add a TChannel to fChannelMap. If the TChannel doesn't exist, create a new TChannel and add that the fChannelMap.
//Options:
//        "overwrite" -  The TChannel in the fChannelMap at the same address is overwritten. 
//                       If this option is not specified, an Error is returned if the TChannel already 
//                       exists in the fChannelMap.
//        "save"      -  The temporary channel is not deleted after being placed in the map. 
    if(!chan)
        return;
   if(fChannelMap->count(chan->GetAddress())==1) {// if this channel existss
	   if(strcmp(opt,"overwrite")==0) {
			TChannel *oldchan = GetChannel(chan->GetAddress());
			oldchan->OverWriteChannel(chan);
			return;
	   } else {
	      printf("Trying to add a channel that already exists!\n");
			return;
	   }	
    } else {
		fChannelMap->insert(std::make_pair(chan->GetAddress(),chan));
		if(chan->GetNumber() != 0 && fChannelNumberMap->count(chan->GetNumber())==0)
	   	 fChannelNumberMap->insert(std::make_pair(chan->GetNumber(),chan));
    }

    return;
}

void TChannel::OverWriteChannel(TChannel *chan){

    this->SetAddress(chan->GetAddress());
    this->SetIntegration(chan->GetIntegration());
    this->SetNumber(chan->GetNumber());
    this->SetStream(chan->GetStream());
    this->SetUserInfoNumber(chan->GetUserInfoNumber());
    this->SetChannelName(chan->GetChannelName());
    this->SetDigitizerType(chan->GetDigitizerType());

    this->SetENGCoefficients(chan->GetENGCoeff());
    this->SetCFDCoefficients(chan->GetCFDCoeff());
    this->SetLEDCoefficients(chan->GetLEDCoeff());
    this->SetTIMECoefficients(chan->GetTIMECoeff());

    this->SetENGChi2(chan->GetENGChi2());
    this->SetCFDChi2(chan->GetCFDChi2());
    this->SetLEDChi2(chan->GetLEDChi2());
    this->SetTIMEChi2(chan->GetTIMEChi2());

	return;
}

void TChannel::AppendChannel(TChannel *chan){

    if(chan->GetIntegration()!=0) 
		this->SetIntegration(chan->GetIntegration()); 
    if(chan->GetNumber()!=0)
		this->SetNumber(chan->GetNumber());
    if(chan->GetStream()!=0)
		this->SetStream(chan->GetStream());
    if(chan->GetUserInfoNumber()!=0 && chan->GetUserInfoNumber()!=0xffffffff)
		this->SetUserInfoNumber(chan->GetUserInfoNumber());
    if(strlen(chan->GetChannelName())>0)
		this->SetChannelName(chan->GetChannelName());
    if(strlen(chan->GetDigitizerType())>0)
		this->SetDigitizerType(chan->GetDigitizerType());

    if(chan->GetENGCoeff().size()>0)
		this->SetENGCoefficients(chan->GetENGCoeff());
    if(chan->GetCFDCoeff().size()>0)
		this->SetCFDCoefficients(chan->GetCFDCoeff());
    if(chan->GetLEDCoeff().size()>0)
		this->SetLEDCoefficients(chan->GetLEDCoeff());
    if(chan->GetTIMECoeff().size()>0)
		this->SetTIMECoefficients(chan->GetTIMECoeff());

    if(chan->GetENGChi2() != 0.0)
		this->SetENGChi2(chan->GetENGChi2());
    if(chan->GetCFDChi2() != 0.0)
		this->SetCFDChi2(chan->GetCFDChi2());
    if(chan->GetLEDChi2() != 0.0)
		this->SetLEDChi2(chan->GetLEDChi2());
    if(chan->GetTIMEChi2() != 0.0)
		this->SetTIMEChi2(chan->GetTIMEChi2());

	return;
}



int TChannel::UpdateChannel(TChannel *chan,Option_t *opt) {
    //If there is information in the chan, the current TChannel with the same address is updated with that information.
   if(!chan)
   	return 0;
//   printf("temp_address = 0x%08x\n",chan->GetAddress());
   TChannel *oldchan = GetChannel(chan->GetAddress()); // look for already existing channel at this address
	if(oldchan==0)
		return 0;

	oldchan->AppendChannel(chan);

	return 0;
}



void TChannel::Clear(Option_t *opt){
//Clears all fields of a TChannel. There are currently no options to be specified.
    address           =  0xffffffff;
    integration       =  0;
    number            =  0;
    stream            =  0;
    ENGChi2           =  0.0;
    userinfonumber    =  0xffffffff;

    ENGCoefficients.clear();
    CFDCoefficients.clear();
    LEDCoefficients.clear();
    TIMECoefficients.clear();
}

//TChannel *TChannel::GetChannel(int temp_add) {int x = temp_add; return GetChannel(x);}

TChannel *TChannel::GetChannel(unsigned int temp_address) {
//Returns the TChannel at the specified address. If the address doesn't exist, returns an empty gChannel.

    TChannel *chan = 0;
//    if(temp_address == 0 || temp_address == 0xffffffff) {//default (NULL) address, return 0;
//	      return chan;
//    }
	if(fChannelMap->count(temp_address)==1){// found channel
		chan = fChannelMap->at(temp_address);
	}
	return chan;
}

//TChannel *TChannel::GetChannelByNumber(int temp_num) {int x = temp_num; return GetChannel(x);}

TChannel *TChannel::GetChannelByNumber(int temp_num) {
//Returns the TChannel based on the channel number and not the channel address.
    if(fChannelMap->size() != fChannelNumberMap->size()) {
	UpdateChannelNumberMap();
    }
    TChannel *chan  = 0;
    try {
	chan = fChannelNumberMap->at(temp_num);
    } 
    catch(const std::out_of_range& oor) {
	return 0;
    }
    return chan;
}

TChannel *TChannel::FindChannelByName(const char *cc_name){

  TChannel *chan = NULL;
  if(!cc_name)
    return chan;
  
  std::string name = cc_name;
  if(name.length()==0)
    return chan;
    
  std::map < unsigned int, TChannel * >::iterator iter;    
  for(iter = fChannelMap->begin(); iter != fChannelMap->end(); iter++) {
    chan = iter->second;
    std::string channel_name = chan->GetChannelName();
    if(channel_name.compare(0,name.length(),name)==0)
      break;
    chan = NULL;
  }
  // either comes out normally as null or breaks out with some TChannel [SC]
  
  return chan;
}

void TChannel::UpdateChannelNumberMap() {
//Updates the fChannelNumberMap based on the entries in the fChannelMap. This should be called before using the fChannelNumberMap.
    std::map < unsigned int, TChannel * >::iterator iter1;
    for(iter1 = fChannelMap->begin(); iter1 != fChannelMap->end(); iter1++) {
	if(fChannelNumberMap->count(iter1->second->GetNumber())==0) {
            fChannelNumberMap->insert(std::make_pair(iter1->second->GetNumber(),iter1->second));
        }
    }
}


void TChannel::DestroyENGCal()   {
//Erases the ENGCoefficients vector
   ENGCoefficients.erase(ENGCoefficients.begin(),ENGCoefficients.end());
}

void TChannel::DestroyCFDCal()   {
//Erases the CFDCoefficients vector
   CFDCoefficients.erase(CFDCoefficients.begin(),CFDCoefficients.end());
}

void TChannel::DestroyLEDCal()   {
//Erases the LEDCoefficients vector
   LEDCoefficients.erase(LEDCoefficients.begin(),LEDCoefficients.end());
}

void TChannel::DestroyTIMECal()  {
//Erases the TimeCal vector
   LEDCoefficients.erase(TIMECoefficients.begin(),TIMECoefficients.end());
}

void TChannel::DestroyCalibrations()   {
//Erases all Cal vectors
   DestroyENGCal();
   DestroyCFDCal();
   DestroyLEDCal();
   DestroyTIMECal();
};

double TChannel::CalibrateENG(int charge) {
   return CalibrateENG((double)charge) + gRandom->Uniform();
};

double TChannel::CalibrateENG(double charge) {
   if(ENGCoefficients.size()==0)
      return charge;

   double temp_int = 1.0; //125.0;
   if(integration != 0)
      temp_int = (double)integration;  //the 4 is the dis. 

   double cal_chg = 0.0;
   for(int i=0;i<ENGCoefficients.size();i++){
      cal_chg += ENGCoefficients[i] * pow((charge/temp_int),i);
   }
  // printf("(%.02f/%0.2f) *%.05f + %.05f = %.05f\n",charge,temp_int,ENGCoefficients[1],ENGCoefficients[0], cal_chg);
	//Print();
	//printf("ENGCoefficients.size() = %i\n\n\n",ENGCoefficients.size());
   return cal_chg;
};

double TChannel::CalibrateCFD(int cfd) {
   return CalibrateCFD((double)cfd + gRandom->Uniform());
};

double TChannel::CalibrateCFD(double cfd) {
   if(CFDCoefficients.size()==0)
      return cfd;

   double cal_cfd = 0.0;
   for(int i=0;i<CFDCoefficients.size();i++){
      cal_cfd += CFDCoefficients[i] * pow(cfd,i);
   }
   return cal_cfd;
};


double TChannel::CalibrateLED(int led) {
   return CalibrateLED((double)led + gRandom->Uniform());
};

double TChannel::CalibrateLED(double led) {
   if(LEDCoefficients.size()==0)
      return led;

   double cal_led = 0.0;
   for(int i=0;i<LEDCoefficients.size();i++){
      cal_led += LEDCoefficients[i] * pow(led,i);
   }
   return cal_led;
}

double TChannel::CalibrateTIME(int time)  {
   return CalibrateTIME((double)time + gRandom->Uniform());
};

double TChannel::CalibrateTIME(double time)  {
   if(TIMECoefficients.size()==0)
      return time;

   double cal_time = 0.0;
   for(int i=0;i<TIMECoefficients.size();i++){
      cal_time += TIMECoefficients[i] * pow(time,i);
   }
   return cal_time;
}

void TChannel::Print(Option_t *opt) {
   //Prints out the current TChannel.

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


void TChannel::WriteCalFile(std::string outfilename) {
   //prints the context of addresschannelmap formatted correctly to stdout if
   //no file name is passed to the function.  If a file name is passed to the function
   //prints the context of addresschannelmap formatted correctly to a file with the given
   //name.  This will earse and rewrite the file if the file already exisits!

   std::map <unsigned int, TChannel * >::iterator iter;
   std::vector<TChannel> chanVec;
   std::vector<TChannel>::iterator iter_vec;
   for(iter = fChannelMap->begin(); iter != fChannelMap->end(); iter++)   {
		if(iter->second)
	      chanVec.push_back(*iter->second);
   }

//printf("here 1\n");
   std::sort(chanVec.begin(),chanVec.end(),TChannel::Compare);
//printf("here 2\n");

   FILE *c_outputfile;
   if(outfilename.length()>0) {
      c_outputfile = freopen (outfilename.c_str(),"w",stdout);
   }
   for(iter_vec = chanVec.begin(); iter_vec != chanVec.end(); iter_vec++)   {
      iter_vec->Print();
   }
   if(outfilename.length()>0) {
      fclose(c_outputfile);
      int fd = open("/dev/tty", O_WRONLY);
      stdout = fdopen(fd, "w");
   }
   return;
}


void TChannel::ReadCalFromTree(TTree *tree,Option_t *opt) {
//Reads the TChannel information for a Tree if it has already been written to a Tree.
    if(!tree)
	return;
    TList *list = tree->GetUserInfo();	
    TIter iter(list);
    int channelsfound = 0;
    while(TObject *obj = iter.Next()) {
	if(!obj->InheritsFrom("TChannel"))
            continue;
	TChannel *chan = new TChannel((TChannel*)obj);
 	AddChannel(chan,opt);// if we read from the tree we want to overwrite any channels found
	channelsfound ++;
    }
    //printf("found %i channels in tree\n",cahnnelsfound);
    return;
}


void TChannel::ReadCalFile(const char *filename) {
   //Makes TChannels from a cal file.
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

   //std::pair < int, int >pixel = std::make_pair(0, 0);

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
		int colon = line.find(":");

		if(openbrace  == std::string::npos &&
			closebrace == std::string::npos &&
			colon  == std::string::npos)
			continue;
		//printf("line : %s\n",line.c_str());

      //*************************************//
      if (closebrace != std::string::npos) {
         //printf("brace closed.\n");
         //channel->Print();
         brace_open = false;
         if (channel && (channel->GetAddress()!=0) ) {
				TChannel *currentchan = GetChannel(channel->GetAddress());      
            if(!currentchan){
					AddChannel(channel);// consider using a default option here
					newchannels++;
				} else {
					 currentchan->Print();
					 currentchan->UpdateChannel(channel);
					 delete channel;
					 newchannels++;
				}
         } else {
			delete channel;
		 }
         channel = 0;
         name.clear();
         detector = 0;
      }
      //*************************************//
      if (openbrace != std::string::npos) {
         brace_open = true;
         name = line.substr(0, openbrace).c_str();
         channel = new TChannel("");//GetChannel(0);
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
               unsigned int tempadd =0; ss>>tempadd;
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
               while (ss >> value) {   channel->AddENGCoefficient(value); }
            } else if(type.compare("LEDCOEFF")==0) {
               channel->DestroyLEDCal();
               double value;
               while (ss >> value) {   channel->AddLEDCoefficient(value); }
            } else if(type.compare("CFDCOEFF")==0) {
               channel->DestroyCFDCal();
               double value;
               while (ss >> value) {   channel->AddCFDCoefficient(value); }
            } else if(type.compare("TIMECOEFF")==0) {
               channel->DestroyTIMECal();
               double value;
               while (ss >> value) {   channel->AddTIMECoefficient(value); }
            } else  {

            }
         }
      }

   }
   printf("parsed %i lines.\n",linenumber);

   return;
}

void TChannel::trim(std::string * line, const std::string & trimChars) {
//Removes the the string "trimCars" from  the string 'line'
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



























