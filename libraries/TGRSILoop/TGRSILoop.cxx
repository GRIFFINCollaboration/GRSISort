#include "TGRSILoop.h"

#include <stdint.h>
#include <signal.h>

#include "TSystem.h"
#include "TStopwatch.h"

#include "TGRSIOptions.h"
#include "TDataParser.h"
#include "TEpicsFrag.h"

#include "TFragmentQueue.h"
#include "TScalerQueue.h"
#include "TGRSIRootIO.h"
#include "TGRSIRunInfo.h"

#include "GRSIVersion.h"

/// \cond CLASSIMP
ClassImp(TGRSILoop)
/// \endcond

TGRSILoop* TGRSILoop::fTGRSILoop = 0;

bool TGRSILoop::fSuppressError = false;

std::mutex TGRSILoop::fMutex;

TGRSILoop* TGRSILoop::Get() {
   if(!fTGRSILoop)
      fTGRSILoop = new TGRSILoop();
   return fTGRSILoop;
}

TGRSILoop::TGRSILoop() {
   //intiallize flags
   fOffline    = true;
   fTestMode   = false;

   fMidasThreadRunning    = false;
   fFillTreeThreadRunning = false;
   fFillScalerThreadRunning = false;

   fFragsSentToTree = 0;
   fFragsReadFromMidas = 0;

   fBadFragsSentToTree = 0;

   fMidasThread = 0;
   fFillTreeThread = 0;
   fFillScalerThread = 0;
   fOdb = 0;

   fIamTigress = false;
   fIamGriffin = false;
}

TGRSILoop::~TGRSILoop()  {  }

void TGRSILoop::BeginRun(int transition,int runnumber,int time) {
  if(fOffline) {
  }
  //fMidasThreadRunning = true;
}

void TGRSILoop::EndRun(int transition,int runnumber,int time) {
   fMidasThreadRunning = false;

   if(fFillTreeThread) {
      //printf("\n\nJoining Fill Tree Thread.\n\n");
      fFillTreeThread->join();
      fFillTreeThreadRunning = false;
      //printf("\n\nFinished Fill Tree Thread.\n\n");
      delete fFillTreeThread; 
		fFillTreeThread = 0;
   }

   if(fFillScalerThread) {
      fFillScalerThread->join();
      fFillScalerThreadRunning = false;
      delete fFillScalerThread; 
		fFillScalerThread = 0;
   }

   //printf("\n\nFragments in que = %i \n\n",TFragmentQueue::GetQueue()->FragsInQueue());

   TGRSIRootIO::Get()->CloseRootOutFile();  
}

bool TGRSILoop::SortMidas() {
   if(fMidasThread) //already sorting.
      return true;
    
	if(TGRSIOptions::GetInputMidas().size() > 0) { //we have offline midas files to sort.
      TMidasFile* mFile = new TMidasFile;
      for(size_t x=0;x<TGRSIOptions::GetInputMidas().size();x++) {
         if(mFile->Open(TGRSIOptions::GetInputMidas().at(x).c_str()))  {
            //std::sting filename = mFile->GetName();
            fMidasThread = new std::thread(&TGRSILoop::ProcessMidasFile,this,mFile);
            fMidasThreadRunning = true;
            fFillTreeThread = new std::thread(&TGRSILoop::FillFragmentTree,this,mFile);
            fFillTreeThreadRunning = true;
            fFillScalerThread = new std::thread(&TGRSILoop::FillScalerTree,this);
            fFillScalerThreadRunning = true;
            //printf("\n\nJoining Midas Thread.\n\n");
            fMidasThread->join();
            //printf("\n\nFinished Midas Thread.\n\n");
            delete fMidasThread;
				fMidasThread = 0;
         }
			mFile->Close();
		}
	   delete mFile;
   }
   TGRSIOptions::GetInputMidas().clear();
	//

   return true;
}

void TGRSILoop::FillFragmentTree(TMidasFile* midasfile) {
   fFragsSentToTree = 0;
   fBadFragsSentToTree = 0;
   TFragment* frag = 0;
   while(TFragmentQueue::GetQueue()->FragsInQueue() !=0      || 
         TFragmentQueue::GetQueue("BAD")->FragsInQueue() !=0 ||
         fMidasThreadRunning) {
      frag = TFragmentQueue::GetQueue("GOOD")->PopFragment();
      if(frag) {
		   while(!fMutex.try_lock()) {
				//do nothing
			}
         TGRSIRootIO::Get()->FillFragmentTree(frag);
			fMutex.unlock();
 	      delete frag;
         fFragsSentToTree++;
      }

      frag = TFragmentQueue::GetQueue("BAD")->PopFragment();
      if(frag) {
		   while(!fMutex.try_lock()) {
				//do nothing
			}
         TGRSIRootIO::Get()->FillBadFragmentTree(frag);
			fMutex.unlock();
         delete frag;
         fBadFragsSentToTree++;
      } 

      if(!fMidasThreadRunning && TFragmentQueue::GetQueue()->FragsInQueue()%5000==0) {
         printf(DYELLOW HIDE_CURSOR " \t%i" RESET_COLOR "/"
                DBLUE   "%i"   RESET_COLOR
                "     frags left to write to tree/frags written to tree.        " SHOW_CURSOR "\r",
                TFragmentQueue::GetQueue()->FragsInQueue(),fFragsSentToTree);
      }
   }


   printf("\n");
}

void TGRSILoop::FillScalerTree() {
   fDeadtimeScalersSentToTree = 0;
   fRateScalersSentToTree = 0;
   TScalerData* scalerData = 0;
   while(TDeadtimeScalerQueue::Get()->ScalersInQueue() != 0 || 
         TRateScalerQueue::Get()->ScalersInQueue() != 0 || 
         fMidasThreadRunning) {
      if(TDeadtimeScalerQueue::Get()->ScalersInQueue() > 0) {
		   scalerData = TDeadtimeScalerQueue::Get()->PopScaler();
		   if(scalerData) {
				while(!fMutex.try_lock()) {
					//do nothing
				}
				TGRSIRootIO::Get()->FillDeadtimeScalerTree(scalerData);
				fMutex.unlock();
				delete scalerData;
            fDeadtimeScalersSentToTree++;
         }

         if(!fMidasThreadRunning && TDeadtimeScalerQueue::Get()->ScalersInQueue()%5000==0) {
            printf(DYELLOW HIDE_CURSOR " \t%i" RESET_COLOR "/"
                  DBLUE   "%i"   RESET_COLOR
                  "     deadtime scalers left to write to tree/frags written to tree.        " SHOW_CURSOR "\r",
                  TDeadtimeScalerQueue::Get()->ScalersInQueue(),fDeadtimeScalersSentToTree);
         }
      }
      if(TRateScalerQueue::Get()->ScalersInQueue() > 0) {
		   scalerData = TRateScalerQueue::Get()->PopScaler();
		   if(scalerData) {
				while(!fMutex.try_lock()) {
					//do nothing
				}
            TGRSIRootIO::Get()->FillRateScalerTree(scalerData);
				fMutex.unlock();
 	         delete scalerData;
            fRateScalersSentToTree++;
         }

         if(!fMidasThreadRunning && TRateScalerQueue::Get()->ScalersInQueue()%5000==0) {
            printf(DYELLOW HIDE_CURSOR " \t%i" RESET_COLOR "/"
                  DBLUE   "%i"   RESET_COLOR
                  "     rate scalers left to write to tree/frags written to tree.        " SHOW_CURSOR "\r",
                  TRateScalerQueue::Get()->ScalersInQueue(),fRateScalersSentToTree);
         }
      }
   }


   printf("\n");
}

void TGRSILoop::ProcessMidasFile(TMidasFile* midasFile) {
   if(!midasFile)
      return;

   //This is a new midas sort, so we should start by resetting the version number in the code.
   TGRSIRunInfo::ClearGRSIVersion();

   fOffline = true;
   //Once this is done, we want to set the frags read from midas to 0 for use in the next sort.
   fFragsReadFromMidas = 0;

   std::ifstream in(midasFile->GetFilename(), std::ifstream::in | std::ifstream::binary);
   in.seekg(0, std::ifstream::end);
   long long filesize = in.tellg();
   in.close();

   TMidasEvent fMidasEvent;

   Initialize();
   fMidasEvent.Clear();

   int bytes = 0;
   long long bytesRead = 0;
   int currentEventNumber = 0;

   TStopwatch w;
   w.Start();

   if(!TGRSIRootIO::Get()->GetRootOutFile()) {
     if(!(TGRSIRootIO::Get()->SetUpRootOutFile(midasFile->GetRunNumber(),midasFile->GetSubRunNumber()))) {
        return;
     }
   }
   while(true) {
      bytes = midasFile->Read(&fMidasEvent);
      currentEventNumber++;
      if(bytes == 0){
         if(midasFile->GetLastError()) {
           printf(DMAGENTA "\tfile: %s ended on %s" RESET_COLOR "\n",midasFile->GetFilename(),midasFile->GetLastError());
         } else {//catch(char* e) { 
           printf(DMAGENTA "\tfile: %s ended on unknown state." RESET_COLOR "\n",midasFile->GetFilename());
         }
         break;
      }
      bytesRead += bytes;
      int eventId = fMidasEvent.GetEventId();
      switch(eventId)   {
         case 0x8000: {
               printf( DGREEN );
               fMidasEvent.Print();
               printf( RESET_COLOR );
               BeginRun(0,0,0);

               std::string inCalFile;
               inCalFile.assign(TGRSIOptions::GetXMLODBFile(midasFile->GetRunNumber(),midasFile->GetSubRunNumber()));
               if(inCalFile.length()>0) {
						printf("using xml file: %s\n",inCalFile.c_str());
						std::ifstream inputxml; inputxml.open(inCalFile.c_str()); inputxml.seekg(0,std::ios::end);
						int length = inputxml.tellg(); inputxml.seekg(0,std::ios::beg);
						char* buffer = new char[length]; inputxml.read(buffer,length);
						SetFileOdb(buffer,length);
                  TGRSIRunInfo::SetXMLODBFileName(inCalFile.c_str());
                  TGRSIRunInfo::SetXMLODBFileData(buffer);
               } else {
	            	SetFileOdb(fMidasEvent.GetData(),fMidasEvent.GetDataSize());
					}
               inCalFile.clear();
					inCalFile.assign(TGRSIOptions::GetCalFile(midasFile->GetRunNumber(),midasFile->GetSubRunNumber()));
					if(inCalFile.length()>0) {
						TChannel::ReadCalFile(inCalFile.c_str());
                  TGRSIRunInfo::SetXMLODBFileName(inCalFile.c_str());
						std::ifstream inputCal; inputCal.open(inCalFile.c_str()); inputCal.seekg(0,std::ios::end);
						int length = inputCal.tellg(); inputCal.seekg(0,std::ios::beg);
						char* buffer = new char[length]; inputCal.read(buffer,length);
                  TGRSIRunInfo::SetXMLODBFileData(buffer);
               }
               TGRSIRunInfo::SetRunInfo(midasFile->GetRunNumber(),midasFile->GetSubRunNumber());
               TGRSIRunInfo::SetGRSIVersion(GRSI_RELEASE);
            }
            break;
         case 0x8001:
            printf(" Processing event %i have processed %.2fMB/%.2fMB => %.1f MB/s\n",currentEventNumber,(bytesRead/1000000.0),(filesize/1000000.0),(bytesRead/1000000.0)/w.RealTime());
            w.Continue();
            printf( DRED );
            fMidasEvent.Print();
            printf( RESET_COLOR );
            EndRun(0,0,0);
            break;
         default:
            ProcessMidasEvent(&fMidasEvent,midasFile);
            break;
      };
      if((currentEventNumber%5000)== 0) {
			if(!TGRSIOptions::CloseAfterSort()) {
				gSystem->ProcessEvents();
			}
         printf(HIDE_CURSOR " Processing event %i have processed %.2fMB/%.2f MB => %.1f MB/s              " SHOW_CURSOR "\r",
					 currentEventNumber,(bytesRead/1000000.0),(filesize/1000000.0),(bytesRead/1000000.0)/w.RealTime());
			fflush(stdout);
         w.Continue();
      }
   }

   Finalize();
   return;
}


void TGRSILoop::SetFileOdb(char* data, int size) {
   //check if we have already set the TChannels....
   //
   if(fOdb) {
   	delete fOdb;
	   fOdb = 0;
   } 

   if(TGRSIOptions::IgnoreFileOdb()) {
      printf(DYELLOW "\tskipping odb information stored in file.\n" RESET_COLOR);
      return;
   }

	fOdb = new TXMLOdb(data,size);
	TChannel::DeleteAllChannels();

   TXMLNode* node = fOdb->FindPath("/Experiment");
   if(!node->HasChildren()){
      return;
   }
   node = node->GetChildren();
   std::string expt;
   while(true) {
		std::string key = fOdb->GetNodeName(node) ;
		if(key.compare("Name")==0) {
			expt = node->GetText();
			break;
      }
      if(!node->HasNextNode())
         break;
      node = node->GetNextNode();
   }
   if(expt.compare("tigress") == 0) {
		fIamTigress = true;
		SetTIGOdb();
   } else if(expt.compare("griffin") == 0) {
		fIamGriffin = true;
		SetGRIFFOdb();
   }
}

void TGRSILoop::SetGRIFFOdb() {
   std::string path = "/DAQ/MSC"; 
   printf("using GRIFFIN path to analyzer info: %s...\n",path.c_str());
   
   std::string temp = path; temp.append("/MSC");
   TXMLNode* node = fOdb->FindPath(temp.c_str());
   std::vector<int> address = fOdb->ReadIntArray(node);

   temp = path; temp.append("/chan");
   node = fOdb->FindPath(temp.c_str());
   std::vector<std::string> names = fOdb->ReadStringArray(node);

   temp = path; temp.append("/datatype");
   node = fOdb->FindPath(temp.c_str());
   std::vector<int> type = fOdb->ReadIntArray(node);

   temp = path; temp.append("/gain");
   node = fOdb->FindPath(temp.c_str());
   std::vector<double> gains = fOdb->ReadDoubleArray(node);
   
   temp = path; temp.append("/offset");
   node = fOdb->FindPath(temp.c_str());
   std::vector<double> offsets = fOdb->ReadDoubleArray(node);


   if( (address.size() == names.size()) && (names.size() == gains.size()) && (gains.size() == offsets.size()) && offsets.size() == type.size() ) {
      //all good.
   }  else {
      printf(BG_WHITE DRED "problem parsing odb data, arrays are different sizes, channels not set." RESET_COLOR "\n");
      return;
   }

   for(size_t x=0;x<address.size();x++) {
      TChannel* tempChan = TChannel::GetChannel(address.at(x));   //names.at(x).c_str());
		if(!tempChan) {
			tempChan = new TChannel();		
		}		
      tempChan->SetChannelName(names.at(x).c_str());
      tempChan->SetAddress(address.at(x));
      tempChan->SetNumber(x);
      //printf("temp chan(%s) number set to: %i\n",tempChan->GetChannelName(),tempChan->GetNumber());
      
      tempChan->SetUserInfoNumber(x);
      tempChan->AddENGCoefficient(offsets.at(x));
      tempChan->AddENGCoefficient(gains.at(x));
      //TChannel::UpdateChannel(tempChan);
      TChannel::AddChannel(tempChan,"overwrite");
   } 
   printf("\t%i TChannels created.\n",TChannel::GetNumberOfChannels());

   return;
}

void TGRSILoop::SetTIGOdb()  {
  
   std::string typepath = "/Equipment/Trigger/settings/Detector Settings";
   std::map<int,std::pair<std::string,std::string> >typemap;
   TXMLNode* typenode = fOdb->FindPath(typepath.c_str());
   int typecounter = 0;
   if(typenode->HasChildren()) {
      TXMLNode* typechild = typenode->GetChildren();
      while(1) {
         std::string tname = fOdb->GetNodeName(typechild);
         if(tname.length()>0 && typechild->HasChildren()) {
            typecounter++;
            TXMLNode* grandchild = typechild->GetChildren();
            while(1) {
               std::string grandchildname = fOdb->GetNodeName(grandchild);
               if(grandchildname.compare(0,7,"Digitis")==0) {
                  std::string dname = grandchild->GetText();
                  typemap[typecounter] = std::make_pair(tname,dname);
                  break;
               }
               if(!grandchild->HasNextNode())
                  break;
               grandchild = grandchild->GetNextNode();
            }
         }
         if(!typechild->HasNextNode())
            break;
         typechild = typechild->GetNextNode();
      }
 
   }
   
   std::string path = "/Analyzer/Shared Parameters/Config";
	TXMLNode* test = fOdb->FindPath(path.c_str());
	if(!test)
		path.assign("/Analyzer/Parameters/Cathode/Config");  //the old path to the useful odb info.
   printf("using TIGRESS path to analyzer info: %s...\n",path.c_str());

   std::string temp = path; temp.append("/FSCP");
   TXMLNode* node = fOdb->FindPath(temp.c_str());
   std::vector<int> address = fOdb->ReadIntArray(node);

   temp = path; temp.append("/Name");
   node = fOdb->FindPath(temp.c_str());
   std::vector<std::string> names = fOdb->ReadStringArray(node);

   temp = path; temp.append("/Type");
   node = fOdb->FindPath(temp.c_str());
   std::vector<int> type = fOdb->ReadIntArray(node);

   temp = path; temp.append("/g");
   node = fOdb->FindPath(temp.c_str());
   std::vector<double> gains = fOdb->ReadDoubleArray(node);
   
   temp = path; temp.append("/o");
   node = fOdb->FindPath(temp.c_str());
   std::vector<double> offsets = fOdb->ReadDoubleArray(node);

//   if( (address.size() == names.size()) && (names.size() == gains.size()) && (gains.size() == offsets.size()) && offsets.size() == type.size() ) {
   if( (address.size() == gains.size()) && (gains.size() == offsets.size()) && offsets.size() == type.size() ) {
      //all good.
   }  else {
      printf(BG_WHITE DRED "problem parsing odb data, arrays are different sizes, channels not set." RESET_COLOR "\n");
      printf(DRED "\taddress.size() = %lu" RESET_COLOR "\n",address.size());
      printf(DRED "\tnames.size()   = %lu" RESET_COLOR "\n",names.size());
      printf(DRED "\tgains.size()   = %lu" RESET_COLOR "\n",gains.size());
      printf(DRED "\toffsets.size() = %lu" RESET_COLOR "\n",offsets.size());
      printf(DRED "\ttype.size()    = %lu" RESET_COLOR "\n",type.size());
      return;
   }

   for(size_t x=0;x<address.size();x++) {
      TChannel* tempChan = TChannel::GetChannel(address.at(x));   //names.at(x).c_str());
		if(!tempChan)
			tempChan = new TChannel();		
      if(x<names.size()) { tempChan->SetChannelName(names.at(x).c_str()); }
		//printf("address: 0x%08x\n",address.at(x));
      tempChan->SetAddress(address.at(x));
      tempChan->SetNumber(x);
      //printf("temp chan(%s) 0x%08x  number set to: %i\n",tempChan->GetChannelName(),tempChan->GetAddress(),tempChan->GetNumber());
      int temp_integration = 0;
      if(type.at(x) != 0) {
         tempChan->SetTypeName(typemap[type.at(x)].first);
         tempChan->SetDigitizerType(typemap[type.at(x)].second.c_str());
         if(strcmp(tempChan->GetDigitizerType(),"Tig64")==0)
            temp_integration = 25;
         else if(strcmp(tempChan->GetDigitizerType(),"Tig10")==0)
            temp_integration = 125;
      }
      tempChan->SetIntegration(temp_integration);      
      tempChan->SetUserInfoNumber(x);
      tempChan->AddENGCoefficient(offsets.at(x));
      tempChan->AddENGCoefficient(gains.at(x));

      TChannel::AddChannel(tempChan,"overwrite");
      //TChannel* temp2 = TChannel::GetChannel(address.at(x));
      //temp2->Print();
			//printf("NumberofChannels: %i\n",TChannel::GetNumberOfChannels());
   } 
   printf("\t%i TChannels created.\n",TChannel::GetNumberOfChannels());
   return;
}

bool TGRSILoop::ProcessMidasEvent(TMidasEvent* mEvent, TMidasFile* mFile)   {
   if(!mEvent)
      return false;
   //printf("mEvent->GetSerialNumber = %i\n",mEvent->GetSerialNumber());
   int banksize;
   void* ptr;
   try {
      switch(mEvent->GetEventId())  {
         case 1:
            mEvent->SetBankList();
            if((banksize = mEvent->LocateBank(NULL,"WFDN",&ptr))>0) {
	            if(!ProcessTIGRESS((uint32_t*)ptr, banksize, mEvent, mFile)) { }
                              //(unsigned int)(mEvent->GetSerialNumber()),
                              //(unsigned int)(mEvent->GetTimeStamp()))) { }
            }
            else if((banksize = mEvent->LocateBank(NULL,"GRF1",&ptr))>0) {
               if(!ProcessGRIFFIN((uint32_t*)ptr,banksize,1, mEvent, mFile)) { }
			      //(unsigned int)(mEvent->GetSerialNumber()),
			      //(unsigned int)(mEvent->GetTimeStamp()))) { }
            }
            else if((banksize = mEvent->LocateBank(NULL,"GRF2",&ptr))>0) {
               if(!ProcessGRIFFIN((uint32_t*)ptr,banksize,2, mEvent, mFile)) { }
            }
	         else if((banksize = mEvent->LocateBank(NULL,"GRF3",&ptr))>0) {
               if(!ProcessGRIFFIN((uint32_t*)ptr,banksize,3, mEvent, mFile)) { }
            }
            else if( (banksize = mEvent->LocateBank(NULL,"FME0",&ptr))>0) {
               if(!Process8PI(0,(uint32_t*)ptr,banksize,mEvent,mFile)) {}
            } else if( (banksize = mEvent->LocateBank(NULL,"FME1",&ptr))>0) {
               if(!Process8PI(1,(uint32_t*)ptr,banksize,mEvent,mFile)) {}
            } else if( (banksize = mEvent->LocateBank(NULL,"FME2",&ptr))>0) {
               if(!Process8PI(2,(uint32_t*)ptr,banksize,mEvent,mFile)) {}
            } else if( (banksize = mEvent->LocateBank(NULL,"FME3",&ptr))>0) {
               if(!Process8PI(3,(uint32_t*)ptr,banksize,mEvent,mFile)) {}
            }
            break;
         case 2:
           if(!fIamGriffin) {
              break;
           }
           mEvent->SetBankList();
           break;
         case 4:
         case 5:
            mEvent->SetBankList();
            if((banksize = mEvent->LocateBank(NULL,"MSRD",&ptr))>0) {
	            if(!ProcessEPICS((float*)ptr, banksize, mEvent, mFile)) { }
                              //(unsigned int)(mEvent->GetSerialNumber()),
                              //(unsigned int)(mEvent->GetTimeStamp()))) { }
            }

            
      };
   }
   catch(const std::bad_alloc&) {   }
   return true;

}

void TGRSILoop::Initialize() {   }

void TGRSILoop::Finalize() { 
   int PPGEvents = TGRSIRootIO::Get()->GetTimesPPGCalled();
   printf("in finalization phase.\n");   
   printf(DMAGENTA "successfully sorted " DBLUE "%0d" DMAGENTA "/" 
          DCYAN "%0d" DMAGENTA "  ---> " DYELLOW " %.2f" DMAGENTA " percent passed." 
          RESET_COLOR "\n",fFragsSentToTree+PPGEvents+fDeadtimeScalersSentToTree+fRateScalersSentToTree,fFragsReadFromMidas,
          ((double)(fFragsSentToTree+PPGEvents+fDeadtimeScalersSentToTree+fRateScalersSentToTree)/(double)fFragsReadFromMidas)*100.);
}


bool TGRSILoop::ProcessEPICS(float* ptr,int& dSize,TMidasEvent* mEvent,TMidasFile* mFile) { 
   unsigned int mserial=0; if(mEvent) mserial = (unsigned int)(mEvent->GetSerialNumber());
	unsigned int mtime=0;   if(mEvent) mtime   = (unsigned int)(mEvent->GetTimeStamp());
   //int epics_banks = 
	TDataParser::EPIXToScalar(ptr,dSize,mserial,mtime);

   return true;
}

bool TGRSILoop::ProcessTIGRESS(uint32_t* ptr, int& dSize, TMidasEvent* mEvent, TMidasFile* mFile)   {
	unsigned int mserial=0; if(mEvent) mserial = (unsigned int)(mEvent->GetSerialNumber());
	unsigned int mtime=0;   if(mEvent) mtime   = (unsigned int)(mEvent->GetTimeStamp());
	int frags = TDataParser::TigressDataToFragment(ptr,dSize,mserial,mtime);
	if(frags>-1) {
      fFragsReadFromMidas += frags;
	   return true;
	} else	{
      fFragsReadFromMidas += 1;   // if the midas bank fails, we assume it only had one frag in it... this is just used for a print statement.
	   if(!fSuppressError && mEvent)  mEvent->Print(Form("a%i",(-1*frags)-1));
	   return false;
	}
}

bool TGRSILoop::Process8PI(uint32_t stream,uint32_t* ptr, int& dSize, TMidasEvent* mEvent, TMidasFile* mFile) {
	unsigned int mserial=0; if(mEvent) mserial = (unsigned int)(mEvent->GetSerialNumber());
	unsigned int mtime=0;   if(mEvent) mtime   = (unsigned int)(mEvent->GetTimeStamp());
  
   std::string banklist = mEvent->GetBankList();
   int frags = 0;
   for(size_t i=0;i<banklist.length();i+=4) {
      std::string bankname;
      bankname = banklist.substr(i,4);
      dSize = mEvent->LocateBank(0,bankname.c_str(),(void**)&ptr);
      if(!dSize)
         continue;
      char str_char = bankname[3];
      stream = atoi(&str_char);
      frags += TDataParser::EightPIDataToFragment(stream,ptr,dSize,mserial,mtime);
   }
   //mEvent->Print("a");
   if(frags>-1) {
      fFragsReadFromMidas += frags;
	   return true;
	} else	{
      fFragsReadFromMidas += 1;   // if the midas bank fails, we assume it only had one frag in it... this is just used for a print statement.
	   if(!fSuppressError && mEvent)  mEvent->Print(Form("a%i",(-1*frags)-1));
	   return false;
	}
}


bool TGRSILoop::ProcessGRIFFIN(uint32_t* ptr, int& dSize, int bank, TMidasEvent* mEvent, TMidasFile* mFile)   {
   unsigned int mserial=0; if(mEvent) mserial = (unsigned int)(mEvent->GetSerialNumber());
   unsigned int mtime=0;   if(mEvent) mtime   = (unsigned int)(mEvent->GetTimeStamp());

   //loop over words in event to find fragment header
   for(int index = 0; index < dSize;) {
      if(((ptr[index])&0xf0000000) == 0x80000000) {
         //if we found a fragment header we pass the data to the data parser which returns the number of words read
         int words = TDataParser::GriffinDataToFragment(&ptr[index],dSize-index,bank,mserial,mtime);
         if(words>0) {
            //we successfully read one event with <words> words, so we advance the index by words
            ++fFragsReadFromMidas;
            index += words;
         } else {	       
            //we failed to read the fragment on word <-words>, so advance the index by -words and we create an error message
            ++fFragsReadFromMidas;   // if the midas bank fails, we assume it only had one frag in it... this is just used for a print statement.
            index -= words;
            if(!fSuppressError) {
               if(!TGRSIOptions::LogErrors()) {
                  printf(DRED "\n//**********************************************//" RESET_COLOR "\n");
                  printf(DRED "\nBad things are happening. Failed on datum %i" RESET_COLOR "\n", index);
       	          if(mEvent)  mEvent->Print(Form("a%i",index-1));
                  printf(DRED "\n//**********************************************//" RESET_COLOR "\n");
               } else {
                  std::string errfilename; 
                  if(mFile) {
                     if(mFile->GetSubRunNumber() != -1) {
                        errfilename.append(Form("error%05i_%03i.log",mFile->GetRunNumber(),mFile->GetSubRunNumber()));
                     } else {
                        errfilename.append(Form("error%05i.log",mFile->GetRunNumber()));
                     }
                  } else {
                     errfilename.append("error_log.log");
                  }
                  FILE* originalstdout = stdout;
                  FILE* errfileptr = freopen(errfilename.c_str(),"a",stdout);
                  printf("\n//**********************************************//\n");
                  if(mEvent) mEvent->Print("a");
                  printf("\n//**********************************************//\n");
                  fclose(errfileptr);
                  stdout = originalstdout;
               }
            }
         }
      } else {
         //this is not a fragment header, so we advance the index
         ++index;
      }
   }

   return true;
}

void TGRSILoop::Print(Option_t* opt) const { printf("Print() Currently Does nothing\n");  }

void TGRSILoop::Clear(Option_t* opt) {   }
