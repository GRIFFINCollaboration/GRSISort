
#include <stdint.h>

#include <TSystem.h>

#include "TGRSILoop.h"
#include "TGRSIOptions.h"
#include "TDataParser.h"

#include "TFragmentQueue.h"
#include "TGRSIRootIO.h"
#include "TGRSIStats.h"
#include "TGRSIRunInfo.h"

ClassImp(TGRSILoop)

TGRSILoop *TGRSILoop::fTGRSILoop = 0;

bool TGRSILoop::suppress_error = false;

TGRSILoop *TGRSILoop::Get() {
   if(!fTGRSILoop)
      fTGRSILoop = new TGRSILoop();
   return fTGRSILoop;
}

TGRSILoop::TGRSILoop()   { 

   //intiallize flags
   fOffline    = true;
   fTestMode   = false;

   fMidasThreadRunning    = false;
   fFillTreeThreadRunning = false;

   fFragsSentToTree = 0;
   fFragsReadFromMidas = 0;

   fMidasThread = 0;
   fFillTreeThread = 0;
   fOdb = 0;
}

TGRSILoop::~TGRSILoop()  {  }

void TGRSILoop::BeginRun(int transition,int runnumber,int time)   { 
  if(fOffline)    {
  }
  //fMidasThreadRunning = true;
}

void TGRSILoop::EndRun(int transition,int runnumber,int time)     { 


   fMidasThreadRunning = false;

   if(fFillTreeThread) {

      //printf("\n\nJoining Fill Tree Thread.\n\n");
      fFillTreeThread->join();
      fFillTreeThreadRunning = false;
      //printf("\n\nFinished Fill Tree Thread.\n\n");
      delete fFillTreeThread; fFillTreeThread = 0;
   }

   //printf("\n\nFragments in que = %i \n\n",TFragmentQueue::GetQueue()->FragsInQueue());

   TGRSIRootIO::Get()->CloseRootOutFile();  

}


bool TGRSILoop::SortMidas() {
   if(fMidasThread) //already sorting.
      return true;
    
    if(TGRSIOptions::GetInputMidas().size()>0)  { //we have offline midas files to sort.
      TMidasFile *mfile = new TMidasFile;
      for(int x=0;x<TGRSIOptions::GetInputMidas().size();x++) {
         if(mfile->Open(TGRSIOptions::GetInputMidas().at(x).c_str()))  {
            //std::sting filename = mfile->GetName();
            fMidasThread = new std::thread(&TGRSILoop::ProcessMidasFile,this,mfile);
            fMidasThreadRunning = true;
            fFillTreeThread = new std::thread(&TGRSILoop::FillFragmentTree,this,mfile);
            fFillTreeThreadRunning = true;
            //printf("\n\nJoining Midas Thread.\n\n");
            fMidasThread->join();
            //printf("\n\nFinished Midas Thread.\n\n");
  //          printf("\n");
            delete fMidasThread; fMidasThread = 0;
   
          }
	     	mfile->Close();
      }
	   delete mfile;
   }
   TGRSIOptions::GetInputMidas().clear();
	//


   return true;
}


void TGRSILoop::FillFragmentTree(TMidasFile *midasfile) {

   if(!TGRSIRootIO::Get()->GetRootOutFile())
      TGRSIRootIO::Get()->SetUpRootOutFile(midasfile->GetRunNumber(),midasfile->GetSubRunNumber());
   
   fFragsSentToTree = 0;
   TFragment *frag = 0;
   while(TFragmentQueue::GetQueue()->FragsInQueue() !=0 || fMidasThreadRunning)
   {
      frag = TFragmentQueue::GetQueue()->PopFragment();
      if(frag) {
         TGRSIRootIO::Get()->FillFragmentTree(frag);
 	      delete frag;
         fFragsSentToTree++;
      }
      if(!fMidasThreadRunning && TFragmentQueue::GetQueue()->FragsInQueue()%5000==0) {
         printf(DYELLOW HIDE_CURSOR " \t%i" RESET_COLOR "/"
                DBLUE   "%i"   RESET_COLOR
                "     frags left to write to tree/frags written to tree.        " SHOW_CURSOR "\r",
                TFragmentQueue::GetQueue()->FragsInQueue(),fFragsSentToTree);
      }
   }

   printf("\n");
   //printf(" \n\n quiting fill tree thread \n\n");
   return;
}

void TGRSILoop::ProcessMidasFile(TMidasFile *midasfile) {
   if(!midasfile)
      return;

   fOffline = true;

   std::ifstream in(midasfile->GetFilename(), std::ifstream::in | std::ifstream::binary);
   in.seekg(0, std::ifstream::end);
   long long filesize = in.tellg();
   in.close();

   TMidasEvent fMidasEvent;

   Initialize();
   fMidasEvent.Clear();

   int bytes = 0;
   long long bytesread = 0;
   int currenteventnumber = 0;

   while(true) {
      bytes = midasfile->Read(&fMidasEvent);
      currenteventnumber++;
      if(bytes == 0){
         printf(DMAGENTA "\tfile: %s ended on %s" RESET_COLOR "\n",midasfile->GetFilename(),midasfile->GetLastError());
			if(midasfile->GetLastErrno()==-1)  //try to read some more...
				continue;
         break;
      }
      bytesread += bytes;
      int eventId = fMidasEvent.GetEventId();
      switch(eventId)   {
         case 0x8000: {
               printf( DGREEN );
               fMidasEvent.Print();
               printf( RESET_COLOR );
               BeginRun(0,0,0);

               std::string incalfile;
               incalfile.assign(TGRSIOptions::GetXMLODBFile(midasfile->GetRunNumber(),midasfile->GetSubRunNumber()));
               if(incalfile.length()>0) {
						printf("using xml file: %s\n",incalfile.c_str());
						std::ifstream inputxml; inputxml.open(incalfile.c_str()); inputxml.seekg(0,std::ios::end);
						int length = inputxml.tellg(); inputxml.seekg(0,std::ios::beg);
						char buffer[length]; inputxml.read(buffer,length);
						SetFileOdb(buffer,length);
                  TGRSIRunInfo::SetXMLODBFileName(incalfile.c_str());
                  TGRSIRunInfo::SetXMLODBFileData(buffer);
               } else {
	            	SetFileOdb(fMidasEvent.GetData(),fMidasEvent.GetDataSize());
					}
               incalfile.clear();
					incalfile.assign(TGRSIOptions::GetCalFile(midasfile->GetRunNumber(),midasfile->GetSubRunNumber()));
					if(incalfile.length()>0) {
						TChannel::ReadCalFile(incalfile.c_str());
                  TGRSIRunInfo::SetXMLODBFileName(incalfile.c_str());
						std::ifstream inputcal; inputcal.open(incalfile.c_str()); inputcal.seekg(0,std::ios::end);
						int length = inputcal.tellg(); inputcal.seekg(0,std::ios::beg);
						char buffer[length]; inputcal.read(buffer,length);
                  TGRSIRunInfo::SetXMLODBFileData(buffer);
               }
               TGRSIRunInfo::SetRunInfo(midasfile->GetRunNumber(),midasfile->GetSubRunNumber());
            }
            break;
         case 0x8001:
            printf(" Processing event %i have processed %.2fMB/%.2fMB\n",currenteventnumber,(bytesread/1000000.0),(filesize/1000000.0));
            printf( DRED );
            fMidasEvent.Print();
            printf( RESET_COLOR );
            EndRun(0,0,0);
            break;
         default:
            ProcessMidasEvent(&fMidasEvent,midasfile);
            break;
      };
      if((currenteventnumber%5000)== 0) {
         gSystem->ProcessEvents();
         printf(HIDE_CURSOR " Processing event %i have processed %.2fMB/%.2f MB               " SHOW_CURSOR "\r",
					 currenteventnumber,(bytesread/1000000.0),(filesize/1000000.0));
      }
   }

   Finalize();
   return;
}


void TGRSILoop::SetFileOdb(char *data, int size) {
   //chaeck if we have already set the tchannels....
   //
   if(fOdb) 
   	delete fOdb;
	 
	fOdb = new TXMLOdb(data,size);

	TChannel::DeleteAllChannels();

   TXMLNode *node = fOdb->FindPath("/Experiment");
   if(!node->HasChildren())
      return;
   node = node->GetChildren();
   std::string expt;
   while(1) {
      std::string key = fOdb->GetNodeName(node);
      if(key.compare("Name")==0) {
         expt = node->GetText();
         break;
      }
      if(!node->HasNextNode())
         break;
      node = node->GetNextNode();
   }
   if(expt.compare("tigress")==0)
      SetTIGOdb();
   else if(expt.compare("griffin")==0)
      SetGRIFFOdb();
}

void TGRSILoop::SetGRIFFOdb() {
   std::string path = "/DAQ/MSC"; 
   printf("using GRIFFIN path to analyzer info: %s...\n",path.c_str());
   
   std::string temp = path; temp.append("/MSC");
   TXMLNode *node = fOdb->FindPath(temp.c_str());
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

   for(int x=0;x<address.size();x++) {
      TChannel *tempchan = TChannel::GetChannel(address.at(x));   //names.at(x).c_str());
		if(!tempchan) {
			tempchan = new TChannel();		
		}		
      tempchan->SetChannelName(names.at(x).c_str());
      tempchan->SetAddress(address.at(x));
      tempchan->SetNumber(x);
      //printf("temp chan(%s) number set to: %i\n",tempchan->GetChannelName(),tempchan->GetNumber());
      
      tempchan->SetUserInfoNumber(x);
      tempchan->AddENGCoefficient(offsets.at(x));
      tempchan->AddENGCoefficient(gains.at(x));
      //TChannel::UpdateChannel(tempchan);
      TChannel::AddChannel(tempchan,"overwrite");
   } 
   printf("\t%i tchannels created.\n",TChannel::GetNumberOfChannels());

   return;
}

void TGRSILoop::SetTIGOdb()  {
  
   std::string typepath = "/Equipment/Trigger/settings/Detector Settings";
   std::map<int,std::pair<std::string,std::string> >typemap;
   TXMLNode *typenode = fOdb->FindPath(typepath.c_str());
   int typecounter = 0;
   if(typenode->HasChildren()) {
      TXMLNode *typechild = typenode->GetChildren();
      while(1) {
         std::string tname = fOdb->GetNodeName(typechild);
         if(tname.length()>0 && typechild->HasChildren()) {
            typecounter++;
            TXMLNode *grandchild = typechild->GetChildren();
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
	TXMLNode *test = fOdb->FindPath(path.c_str());
	if(!test)
		path.assign("/Analyzer/Parameters/Cathode/Config");  //the old path to the useful odb info.
   printf("using TIGRESS path to analyzer info: %s...\n",path.c_str());

   std::string temp = path; temp.append("/FSCP");
   TXMLNode *node = fOdb->FindPath(temp.c_str());
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

   for(int x=0;x<address.size();x++) {
      TChannel *tempchan = TChannel::GetChannel(address.at(x));   //names.at(x).c_str());
		if(!tempchan)
			tempchan = new TChannel();		
      if(x<names.size()) { tempchan->SetChannelName(names.at(x).c_str()); }
		//printf("address: 0x%08x\n",address.at(x));
      tempchan->SetAddress(address.at(x));
      tempchan->SetNumber(x);
      //printf("temp chan(%s) 0x%08x  number set to: %i\n",tempchan->GetChannelName(),tempchan->GetAddress(),tempchan->GetNumber());
      int temp_integration = 0;
      if(type.at(x) != 0) {
         tempchan->SetTypeName(typemap[type.at(x)].first);
         tempchan->SetDigitizerType(typemap[type.at(x)].second.c_str());
         if(strcmp(tempchan->GetDigitizerType(),"Tig64")==0)
            temp_integration = 25;
         else if(strcmp(tempchan->GetDigitizerType(),"Tig10")==0)
            temp_integration = 125;
      }
      tempchan->SetIntegration(temp_integration);      
      tempchan->SetUserInfoNumber(x);
      tempchan->AddENGCoefficient(offsets.at(x));
      tempchan->AddENGCoefficient(gains.at(x));

      TChannel::AddChannel(tempchan,"overwrite");
      //TChannel *temp2 = TChannel::GetChannel(address.at(x));
      //temp2->Print();
			//printf("NumberofChannels: %i\n",TChannel::GetNumberOfChannels());
   } 
   printf("\t%i TChannels created.\n",TChannel::GetNumberOfChannels());
   return;
}

bool TGRSILoop::ProcessMidasEvent(TMidasEvent *mevent, TMidasFile *mfile)   {
   if(!mevent)
      return false;
   int banksize;
   void *ptr;
   try {
      switch(mevent->GetEventId())  {
         case 1:
            mevent->SetBankList();
            if((banksize = mevent->LocateBank(NULL,"WFDN",&ptr))>0) {
	       if(!ProcessTIGRESS((uint32_t*)ptr, banksize, mevent, mfile)) { }
                              //(unsigned int)(mevent->GetSerialNumber()),
                              //(unsigned int)(mevent->GetTimeStamp()))) { }
            }
            else if((banksize = mevent->LocateBank(NULL,"GRF1",&ptr))>0) {
            if(!ProcessGRIFFIN((uint32_t*)ptr,banksize, mevent, mfile)) { }
			      //(unsigned int)(mevent->GetSerialNumber()),
			      //(unsigned int)(mevent->GetTimeStamp()))) { }
            }
      };
   }
   catch(const std::bad_alloc&) {   }
   return true;

}

void TGRSILoop::Initialize() {   }

void TGRSILoop::Finalize() { 
   printf("in finalization phase.\n");   
   printf(DMAGENTA "successfully sorted " DBLUE "%0d" DMAGENTA "/" 
          DCYAN "%0d" DMAGENTA "  ---> " DYELLOW " %.2f" DMAGENTA " percent passed." 
          RESET_COLOR "\n",fFragsSentToTree,fFragsReadFromMidas,((double)fFragsSentToTree/(double)fFragsReadFromMidas)*100.);
//   TIter *iter = TChannel::GetChannelIter();   
//   while(TChannel *chan = (TChannel*)iter->Next()) {
//      TGRSIRootIO::Get()->FillChannelTree(chan);
//      TGRSIRootIO::Get()->GetChannelTree()->GetUserInfo()->Add(chan);
//   }
//   TGRSIRootIO::Get()->CloseRootOutFile();
}


bool TGRSILoop::ProcessEPICS() { //TMidasEvent *mevent)   {
   return true;
}


bool TGRSILoop::ProcessTIGRESS(uint32_t *ptr, int &dsize, TMidasEvent *mevent, TMidasFile *mfile)   {
	unsigned int mserial=0; if(mevent) mserial = (unsigned int)(mevent->GetSerialNumber());
	unsigned int mtime=0;   if(mevent) mtime   = (unsigned int)(mevent->GetTimeStamp());
	int frags = TDataParser::TigressDataToFragment(ptr,dsize,mserial,mtime);
	if(frags>-1) {
      fFragsReadFromMidas += frags;
	   return true;
	} else	{
      fFragsReadFromMidas += 1;   // if the midas bank fails, we assume it only had one frag in it... this is just used for a print statement.
	   if(!suppress_error && mevent)  mevent->Print(Form("a%i",(-1*frags)-1));
	   return false;
	}
}

bool TGRSILoop::ProcessGRIFFIN(uint32_t *ptr, int &dsize, TMidasEvent *mevent, TMidasFile *mfile)   {
	unsigned int mserial=0; if(mevent) mserial = (unsigned int)(mevent->GetSerialNumber());
	unsigned int mtime=0;   if(mevent) mtime   = (unsigned int)(mevent->GetTimeStamp());

	int frags = TDataParser::GriffinDataToFragment(ptr,dsize,mserial,mtime);
	if(frags>-1)	{
      fFragsReadFromMidas += frags;
      return true;
	} else {	       
      fFragsReadFromMidas += 1;   // if the midas bank fails, we assume it only had one frag in it... this is just used for a print statement.
		if(!suppress_error) {
			if(!TGRSIOptions::LogErrors()) {
			   printf(DRED "\n//**********************************************//" RESET_COLOR "\n");
			   printf(DRED "\nBad things are happening. Failed on datum %i" RESET_COLOR "\n", (-1*frags));
	    		   if(mevent)  mevent->Print(Form("a%i",(-1*frags)-1));
			   printf(DRED "\n//**********************************************//" RESET_COLOR "\n");
		   } else {
				std::string errfilename; 
				if(mfile) {
               if(mfile->GetSubRunNumber() != -1)
                  errfilename.append(Form("error%05i_%03i.log",mfile->GetRunNumber(),mfile->GetSubRunNumber()));
               else    
                  errfilename.append(Form("error%05i.log",mfile->GetRunNumber()));
            }
            else
					errfilename.append("error_log.log");
            FILE *errfileptr = freopen(errfilename.c_str(),"a",stdout);
			   printf("\n//**********************************************//\n");
				if(mevent) mevent->Print("a");
			   printf("\n//**********************************************//\n");
			   fclose(errfileptr);
		      int fd = open("/dev/tty", O_WRONLY);
	     		stdout = fdopen(fd, "w");
			}
		}
	}
   return false;
}




void TGRSILoop::Print(Option_t *opt) {   }


void TGRSILoop::Clear(Option_t *opt) {   }








