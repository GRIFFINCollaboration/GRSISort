#include "TDataLoop.h"

#include <chrono>
#include <thread>
#include <utility>
#include <cstdio>
#include <sstream>

#include "TGRSIOptions.h"
#include "TString.h"
#include "TRawFile.h"
#include "TMidasFile.h"
#include "TChannel.h"
#include "TGRSIRunInfo.h"
#include "TPriorityValue.h"

TDataLoop::TDataLoop(std::string name, TRawFile* source)
   : StoppableThread(name), fSource(source), fSelfStopping(true),
     fOutputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<TRawEvent>>>("midas_queue"))
#ifdef HAS_XML
     , fOdb(nullptr)
#endif
{
   TMidasFile* midasFile = dynamic_cast<TMidasFile*>(source);
   if(midasFile != nullptr) {
      SetFileOdb(midasFile->GetFirstEvent()->GetTimeStamp(), midasFile->GetFirstEvent()->GetData(), midasFile->GetFirstEvent()->GetDataSize());
   }
}

TDataLoop::~TDataLoop()
{
   // delete source;
}

TDataLoop* TDataLoop::Get(std::string name, TRawFile* source)
{
   if(name.length() == 0) {
      name = "input_loop";
   }
   TDataLoop* loop = static_cast<TDataLoop*>(StoppableThread::Get(name));
   if((loop == nullptr) && (source != nullptr)) {
      loop = new TDataLoop(name, source);
   }
   return loop;
}

void TDataLoop::SetFileOdb(uint32_t time, char* data, int size)
{
#ifdef HAS_XML
   // check if we have already set the TChannels....
   //
	delete fOdb;
	fOdb = nullptr;

   if(TGRSIOptions::Get()->IgnoreFileOdb()) {
      printf(DYELLOW "\tskipping odb information stored in file.\n" RESET_COLOR);
      return;
   }

   fOdb = new TXMLOdb(data, size);
   TChannel::DeleteAllChannels();

   // Check to see if we are running a GRIFFIN or TIGRESS experiment
   TXMLNode* node = fOdb->FindPath("/Experiment");
   if(!node->HasChildren()) {
      return;
   }
   node = node->GetChildren();
   std::string expt;
   while(true) {
      std::string key = fOdb->GetNodeName(node);
      if(key.compare("Name") == 0) {
         expt = node->GetText();
         break;
      }
      if(!node->HasNextNode()) {
         break;
      }
      node = node->GetNextNode();
   }
   if(expt.compare("tigress") == 0) {
      //		fIamTigress = true;
      SetTIGOdb();
   } else if(expt.compare("griffin") == 0) {
      //		fIamGriffin = true;
      SetGRIFFOdb();
   }

   SetRunInfo(time);

   // Check for EPICS variables
   SetEPICSOdb();
#endif
}

void TDataLoop::SetRunInfo(uint32_t time)
{
#ifdef HAS_XML
   TGRSIRunInfo* runInfo = TGRSIRunInfo::Get();
   TXMLNode*     node    = fOdb->FindPath("/Runinfo/Start time binary");
   if(node != nullptr) {
		std::stringstream str(node->GetText());
		unsigned int odbTime;
		str>>odbTime;
		if(runInfo->SubRunNumber() == 0 && time != odbTime) {
			std::cout<<"Warning, ODB start time of first subrun ("<<odbTime<<") does not match midas time of first event in this subrun ("<<time<<")!"<<std::endl;
		}
      runInfo->SetRunStart(time);
   }

   node = fOdb->FindPath("/Experiment/Run parameters/Run Title");
   if(node != nullptr) {
      runInfo->SetRunTitle(node->GetText());
      std::cout<<"Title: "<<DBLUE<<node->GetText()<<RESET_COLOR<<std::endl;
   }

   if(node != nullptr) {
      node = fOdb->FindPath("/Experiment/Run parameters/Comment");
      runInfo->SetRunComment(node->GetText());
      std::cout<<"Comment: "<<DBLUE<<node->GetText()<<RESET_COLOR<<std::endl;
   }
#endif
}

void TDataLoop::SetEPICSOdb()
{
#ifdef HAS_XML
   TXMLNode*                node  = fOdb->FindPath("/Equipment/Epics/Settings/Names");
   std::vector<std::string> names = fOdb->ReadStringArray(node);
   TEpicsFrag::SetEpicsNameList(names);
#endif
}

void TDataLoop::SetGRIFFOdb()
{
#ifdef HAS_XML
   // get calibrations
   std::string path = "/DAQ/MSC";
   printf("using GRIFFIN path to analyzer info: %s...\n", path.c_str());

   std::string temp = path;
   temp.append("/MSC");
   TXMLNode*        node    = fOdb->FindPath(temp.c_str());
   std::vector<int> address = fOdb->ReadIntArray(node);

   temp = path;
   temp.append("/chan");
   node                           = fOdb->FindPath(temp.c_str());
   std::vector<std::string> names = fOdb->ReadStringArray(node);

   temp = path;
   temp.append("/datatype");
   node                  = fOdb->FindPath(temp.c_str());
   std::vector<int> type = fOdb->ReadIntArray(node);

   temp = path;
   temp.append("/gain");
   node                      = fOdb->FindPath(temp.c_str());
   std::vector<double> gains = fOdb->ReadDoubleArray(node);

   temp = path;
   temp.append("/offset");
   node                        = fOdb->FindPath(temp.c_str());
   std::vector<double> offsets = fOdb->ReadDoubleArray(node);

   if((address.size() == names.size()) && (names.size() == gains.size()) && (gains.size() == offsets.size()) &&
      offsets.size() == type.size()) {
      // all good.
      for(size_t x = 0; x < address.size(); x++) {
         TChannel* tempChan = TChannel::GetChannel(address.at(x)); // names.at(x).c_str());
         if(tempChan == nullptr) {
            tempChan = new TChannel();
         }
         tempChan->SetName(names.at(x).c_str());
         tempChan->SetAddress(address.at(x));
         tempChan->SetNumber(TPriorityValue<int>(x, EPriority::kDefault));
         // printf("temp chan(%s) number set to: %i\n",tempChan->GetChannelName(),tempChan->GetNumber());

         tempChan->SetUserInfoNumber(TPriorityValue<int>(x, EPriority::kDefault));
         tempChan->AddENGCoefficient(offsets.at(x));
         tempChan->AddENGCoefficient(gains.at(x));
         // TChannel::UpdateChannel(tempChan);
         TChannel::AddChannel(tempChan, "overwrite");
      }
      printf("\t%i TChannels created.\n", TChannel::GetNumberOfChannels());
   } else {
      printf(BG_WHITE DRED "problem parsing odb data, arrays are different sizes, channels not set." RESET_COLOR "\n");
   }

   // get cycle information
   // "/Experiment/Edit on start/PPG Cycle" is a link to the PPG cycle used (always "/PPG/Current"???)
   // "/PPG/Current" gives the current PPG cycle used, e.g. 146Cs_S1468
   // "/PPG/Cycles/146Cs_S1468" then has four PPGcodes and four durations
   node = fOdb->FindPath("/PPG/Current");
   if(node == nullptr) {
      std::cerr<<R"(Failed to find "/PPG/Current" in ODB!)"<<std::endl;
      return;
   }

   if(!node->HasChildren()) {
      std::cout<<"Node has no children, can't read ODB cycle"<<std::endl;
      return;
   }
   std::string currentCycle = "/PPG/Cycles/";
   currentCycle.append(node->GetChildren()->GetContent());
   temp = currentCycle;
   temp.append("/PPGcodes");
   node = fOdb->FindPath(temp.c_str());
   if(node == nullptr) {
      std::cerr<<R"(Failed to find ")"<<temp<<R"(" in ODB!)"<<std::endl;
      return;
   }
   std::vector<int> tmpCodes = fOdb->ReadIntArray(node);
   // the codes are 32bit with the 16 high bits being the same as the 16 low bits
   // we check this and only keep the low 16 bits
   std::vector<short> ppgCodes;
   for(auto code : tmpCodes) {
      if(((code >> 16) & 0xffff) != (code & 0xffff)) {
         std::cout<<DRED<<"Found ppg code in the ODB with high bits (0x"<<std::hex<<(code >> 16)
                  <<") != low bits ("<<(code & 0xffff)<<std::dec<<")"<<RESET_COLOR<<std::endl;
      }
      ppgCodes.push_back(code & 0xffff);
   }
   temp = currentCycle;
   temp.append("/durations");
   node = fOdb->FindPath(temp.c_str());
   if(node == nullptr) {
      std::cerr<<R"(Failed to find ")"<<temp<<R"(" in ODB!)"<<std::endl;
      return;
   }
   std::vector<int> durations = fOdb->ReadIntArray(node);

   TPPG::Get()->SetOdbCycle(ppgCodes, durations);
#endif
}

void TDataLoop::SetTIGOdb()
{
#ifdef HAS_XML
   std::string typepath = "/Equipment/Trigger/settings/Detector Settings";
   std::map<int, std::pair<std::string, std::string>> typemap;
   TXMLNode* typenode    = fOdb->FindPath(typepath.c_str());
   int       typecounter = 0;
   if(typenode->HasChildren()) {
      TXMLNode* typechild = typenode->GetChildren();
      while(true) {
         std::string tname = fOdb->GetNodeName(typechild);
         if(tname.length() > 0 && typechild->HasChildren()) {
            typecounter++;
            TXMLNode* grandchild = typechild->GetChildren();
            while(true) {
               std::string grandchildname = fOdb->GetNodeName(grandchild);
               if(grandchildname.compare(0, 7, "Digitis") == 0) {
                  std::string dname    = grandchild->GetText();
                  typemap[typecounter] = std::make_pair(tname, dname);
                  break;
               }
               if(!grandchild->HasNextNode()) {
                  break;
               }
               grandchild = grandchild->GetNextNode();
            }
         }
         if(!typechild->HasNextNode()) {
            break;
         }
         typechild = typechild->GetNextNode();
      }
   }

   std::string path = "/Analyzer/Shared Parameters/Config";
   TXMLNode*   test = fOdb->FindPath(path.c_str());
   if(test == nullptr) {
      path.assign("/Analyzer/Parameters/Cathode/Config"); // the old path to the useful odb info.
   }
   printf("using TIGRESS path to analyzer info: %s...\n", path.c_str());

   std::string temp = path;
   temp.append("/FSCP");
   TXMLNode*        node    = fOdb->FindPath(temp.c_str());
   std::vector<int> address = fOdb->ReadIntArray(node);

   temp = path;
   temp.append("/Name");
   node                           = fOdb->FindPath(temp.c_str());
   std::vector<std::string> names = fOdb->ReadStringArray(node);

   temp = path;
   temp.append("/Type");
   node                  = fOdb->FindPath(temp.c_str());
   std::vector<int> type = fOdb->ReadIntArray(node);

   temp = path;
   temp.append("/g");
   node                      = fOdb->FindPath(temp.c_str());
   std::vector<double> gains = fOdb->ReadDoubleArray(node);

   temp = path;
   temp.append("/o");
   node                        = fOdb->FindPath(temp.c_str());
   std::vector<double> offsets = fOdb->ReadDoubleArray(node);

   //   if( (address.size() == names.size()) && (names.size() == gains.size()) && (gains.size() == offsets.size()) &&
   //   offsets.size() == type.size() ) {
   if((address.size() == gains.size()) && (gains.size() == offsets.size()) && offsets.size() == type.size()) {
      // all good.
   } else {
      printf(BG_WHITE DRED "problem parsing odb data, arrays are different sizes, channels not set." RESET_COLOR "\n");
      printf(DRED "\taddress.size() = %lu" RESET_COLOR "\n", address.size());
      printf(DRED "\tnames.size()   = %lu" RESET_COLOR "\n", names.size());
      printf(DRED "\tgains.size()   = %lu" RESET_COLOR "\n", gains.size());
      printf(DRED "\toffsets.size() = %lu" RESET_COLOR "\n", offsets.size());
      printf(DRED "\ttype.size()    = %lu" RESET_COLOR "\n", type.size());
      return;
   }

   for(size_t x = 0; x < address.size(); x++) {
      TChannel* tempChan = TChannel::GetChannel(address.at(x)); // names.at(x).c_str());
      if(tempChan == nullptr) {
         tempChan = new TChannel();
      }
      if(x < names.size()) {
         tempChan->SetName(names.at(x).c_str());
      }
      tempChan->SetAddress(address.at(x));
      tempChan->SetNumber(TPriorityValue<int>(x, EPriority::kDefault));
      int temp_integration = 0;
      if(type.at(x) != 0) {
         tempChan->SetDigitizerType(TPriorityValue<std::string>(typemap[type.at(x)].second.c_str(), EPriority::kDefault));
         if(strcmp(tempChan->GetDigitizerTypeString(), "Tig64") ==
            0) { // TODO: maybe use enumerations via GetDigitizerType()
            temp_integration = 25;
         } else if(strcmp(tempChan->GetDigitizerTypeString(), "Tig10") == 0) {
            temp_integration = 125;
         }
      }
      tempChan->SetIntegration(TPriorityValue<int>(temp_integration, EPriority::kDefault));
      tempChan->SetUserInfoNumber(TPriorityValue<int>(x, EPriority::kDefault));
      tempChan->AddENGCoefficient(offsets.at(x));
      tempChan->AddENGCoefficient(gains.at(x));

      TChannel::AddChannel(tempChan, "overwrite");
   }
   printf("\t%i TChannels created.\n", TChannel::GetNumberOfChannels());
#endif
}

void TDataLoop::ClearQueue()
{
   std::shared_ptr<TRawEvent> event;
   while(fOutputQueue->Size() != 0u) {
      fOutputQueue->Pop(event);
   }
}

void TDataLoop::ReplaceSource(TRawFile* new_source)
{
   std::lock_guard<std::mutex> lock(fSourceMutex);
   // delete source;
   fSource = new_source;
}

void TDataLoop::ResetSource()
{
   std::cerr<<"Reset not implemented for TRawFile"<<std::endl;
   // std::lock_guard<std::mutex> lock(fSourceMutex);
   // source->Reset();
}

void TDataLoop::OnEnd()
{
   fOutputQueue->SetFinished();
}

bool TDataLoop::Iteration()
{
   std::shared_ptr<TRawEvent> evt = fSource->NewEvent();
   int                        bytesRead;
   {
      std::lock_guard<std::mutex> lock(fSourceMutex);
      bytesRead   = fSource->Read(evt);
      fItemsPopped = fSource->GetBytesRead() / 1000;
      fInputSize = fSource->GetFileSize() / 1000 - fItemsPopped; // this way fInputSize+fItemsPopped give the file size
   }

   if(bytesRead <= 0 && fSelfStopping) {
      // Error, and no point in trying again.
      return false;
   }
   if(bytesRead > 0) {
      // A good event was returned
      fOutputQueue->Push(evt);
      return true;
   }
   // Nothing returned this time, but I might get something next time.
   std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   return true;
}

