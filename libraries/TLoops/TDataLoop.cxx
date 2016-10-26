#include "TDataLoop.h"

#include <chrono>
#include <thread>
#include <utility>

#include "TGRSIOptions.h"
#include "TString.h"
#include "TMidasFile.h"
#include "TChannel.h"

TDataLoop::TDataLoop(std::string name,TMidasFile* source)
	: StoppableThread(name),
	fSource(source), fSelfStopping(true),
	fOutputQueue(std::make_shared<ThreadsafeQueue<TMidasEvent> >("midas_queue")),
	fOdb(0) {

		SetFileOdb(source->GetFirstEvent().GetData(),
				source->GetFirstEvent().GetDataSize());
		for(auto cal_filename : TGRSIOptions::Get()->CalInputFiles()) {
			TChannel::ReadCalFile(cal_filename.c_str());
		}
	}

TDataLoop::~TDataLoop(){
	//delete source;
}

TDataLoop *TDataLoop::Get(std::string name,TMidasFile* source) {
	if(name.length()==0)
		name = "input_loop";

	TDataLoop *loop = dynamic_cast<TDataLoop*>(StoppableThread::Get(name));
	if(!loop && source) {
		loop = new TDataLoop(name,source);
	}
	return loop;
}

void TDataLoop::SetFileOdb(char* data, int size) {
	//check if we have already set the TChannels....
	//
	if(fOdb) {
		delete fOdb;
		fOdb = 0;
	}

	if(TGRSIOptions::Get()->IgnoreFileOdb()) {
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
		//		fIamTigress = true;
		SetTIGOdb();
	} else if(expt.compare("griffin") == 0) {
		//		fIamGriffin = true;
		SetGRIFFOdb();
	}
}

void TDataLoop::SetGRIFFOdb() {
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
		tempChan->SetName(names.at(x).c_str());
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

void TDataLoop::SetTIGOdb()  {

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
		if(x<names.size()) { tempChan->SetName(names.at(x).c_str()); }
		//printf("address: 0x%08x\n",address.at(x));
		tempChan->SetAddress(address.at(x));
		tempChan->SetNumber(x);
		//printf("temp chan(%s) 0x%08x  number set to: %i\n",tempChan->GetChannelName(),tempChan->GetAddress(),tempChan->GetNumber());
		int temp_integration = 0;
		if(type.at(x) != 0) {
			tempChan->SetTypeName(typemap[type.at(x)].first);
			tempChan->SetDigitizerType(typemap[type.at(x)].second.c_str());
			if(strcmp(tempChan->GetDigitizerTypeString(),"Tig64")==0) // TODO: maybe use enumerations via GetDigitizerType()
				temp_integration = 25;
			else if(strcmp(tempChan->GetDigitizerTypeString(),"Tig10")==0)
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

void TDataLoop::ClearQueue() {
	TMidasEvent event;
	while(fOutputQueue->Size()){
		fOutputQueue->Pop(event);
	}
}

void TDataLoop::ReplaceSource(TMidasFile* new_source) {
	std::lock_guard<std::mutex> lock(fSourceMutex);
	//delete source;
	fSource = new_source;
}

void TDataLoop::ResetSource() {
	std::cerr << "Reset not implemented for TMidasFile" << std::endl;
	// std::lock_guard<std::mutex> lock(fSourceMutex);
	// source->Reset();
}

void TDataLoop::OnEnd() {
	fOutputQueue->SetFinished();
}

bool TDataLoop::Iteration() {
	TMidasEvent evt;
	int bytes_read;
	{
		std::lock_guard<std::mutex> lock(fSourceMutex);
		bytes_read = fSource->Read(evt);
		fItemsPopped = fSource->GetBytesRead()/1000;
		fInputSize = fSource->GetFileSize()/1000 - fItemsPopped; //this way fInputSize+fItemsPopped give the file size
	}

	if(bytes_read <= 0 && fSelfStopping){
		// Error, and no point in trying again.
		std::cout<<std::endl<<"finished sorting all input."<<std::endl;
		return false;
	} else if(bytes_read > 0){
		// A good event was returned
		fOutputQueue->Push(evt);
		return true;
	} else {
		// Nothing returned this time, but I might get something next time.
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		return true;
	}
}

//std::string TDataLoop::Status() {
//  //return fSource->Status(TGRSIOptions::Get()->LongFileDescription());
//}
