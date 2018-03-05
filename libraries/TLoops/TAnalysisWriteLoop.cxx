#include "TAnalysisWriteLoop.h"

#include <chrono>
#include <thread>

#include "TFile.h"
#include "TThread.h"
#include "TMessage.h"
#include "TSocket.h"
#include "TMemFile.h"
#include "TFileMerger.h"
#include "TServerSocket.h"
#include "TMonitor.h"
#include "TFileCacheWrite.h"
#include "TROOT.h"

#include "GValue.h"
#include "TChannel.h"
#include "TGRSIRunInfo.h"
#include "TGRSIOptions.h"
#include "TTreeFillMutex.h"
#include "TSortingDiagnostics.h"
#include "TDescant.h"

TAnalysisWriteLoop* TAnalysisWriteLoop::Get(std::string name, std::string outputFilename)
{
   if(name.length() == 0) {
      name = "write_loop";
   }

   TAnalysisWriteLoop* loop = static_cast<TAnalysisWriteLoop*>(StoppableThread::Get(name));
   if(loop == nullptr) {
      if(outputFilename.length() == 0) {
         outputFilename = "temp.root";
      }
      loop = new TAnalysisWriteLoop(name, outputFilename);
   }

   return loop;
}

TAnalysisWriteLoop::TAnalysisWriteLoop(std::string name, std::string outputFilename)
   : StoppableThread(name), fOutputFilename(outputFilename),
     fInputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent>>>()),
     fOutOfOrderQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<const TFragment>>>())
{
	fServerFuture = std::async(std::launch::async, &TAnalysisWriteLoop::Server, this);
   if(fOutputFilename != "/dev/null") {
		fClients.resize(TGRSIOptions::Get()->NumberOfClients());
		for(size_t i = 0; i < fClients.size(); ++i) {
			fClients[i] = new TAnalysisWriteLoopClient(Form("write_client_%lu", i), fOutputFilename);
		}
   }
	fOutOfOrder = TGRSIOptions::Get()->SeparateOutOfOrder();
}

TAnalysisWriteLoop::~TAnalysisWriteLoop()
{
	if(!fServerFuture.get()) {
		std::cout<<"Server failed!"<<std::endl;
	}
   Write();
}

void TAnalysisWriteLoop::ClearQueue()
{
   while(fInputQueue->Size() != 0u) {
      std::shared_ptr<TUnpackedEvent> event;
      fInputQueue->Pop(event);
   }
	for(auto client : fClients) {
		client->ClearQueue();
	}
}

std::string TAnalysisWriteLoop::EndStatus()
{
   std::stringstream ss;
   ss<<Name()<<":\t"<<std::setw(8)<<fItemsPopped<<"/"<<fInputSize + fItemsPopped<<", "
     <<"??? good events"<<std::endl;
   return ss.str();
}

void TAnalysisWriteLoop::OnEnd()
{
	for(const auto& client : fClients) {
		client->InputQueue()->SetFinished();
	}
}

bool TAnalysisWriteLoop::Iteration()
{
	std::shared_ptr<TUnpackedEvent> event;
	fInputSize = fInputQueue->Pop(event);
	if(fInputSize < 0) {
		fInputSize = 0;
	}
	++fItemsPopped;

	if(fOutOfOrder) {
		std::shared_ptr<const TFragment> frag;
		fOutOfOrderQueue->Pop(frag, 0);
		if(frag != nullptr) {
			fClients[fCurrentClient]->OutOfOrderQueue()->Push(std::move(frag));
		}
	}

	if(event != nullptr) {
		fClients[fCurrentClient]->InputQueue()->Push(std::move(event));

		++fCurrentClient;
		if(fCurrentClient == fClients.size()) {
			fCurrentClient = 0;
		}
		return true;
	}

	if(fInputQueue->IsFinished()) {
		return false;
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	return true;
}

void TAnalysisWriteLoop::Write()
{
	if(fOutputFilename != "/dev/null") {
		TFile* outputFile = new TFile(fOutputFilename.c_str(), "update");
		outputFile->cd();

		//fEventTree->Write(fEventTree->GetName(), TObject::kOverwrite);

		//if(fOutOfOrderTree != nullptr) {
		//fOutOfOrderTree->Write(fOutOfOrderTree->GetName(), TObject::kOverwrite);
		//}

		if(GValue::Size() != 0) {
			GValue::Get()->Write();
		}
		if(TChannel::GetNumberOfChannels() != 0) {
			TChannel::WriteToRoot();
		}
		TGRSIRunInfo::Get()->WriteToRoot(outputFile);
		TGRSIOptions::Get()->AnalysisOptions()->WriteToFile(outputFile);
		TPPG::Get()->Write();

		if(TGRSIOptions::Get()->WriteDiagnostics()) {
			TSortingDiagnostics::Get()->Write();
		}

		outputFile->Close();
		outputFile->Delete();
	}
}

bool TAnalysisWriteLoop::Server()
{
	TServerSocket* socketServer = new TServerSocket(9090, true);//true = reuse socket
	if(socketServer == nullptr) return false;
	TMonitor* monitor = new TMonitor;
	monitor->Add(socketServer);

	unsigned int clientCount = 0;
	TMemFile* transient = nullptr;

	TFileMerger merger(false, false);//false, false = isn't local, not 'histOneGo'
	merger.SetPrintLevel(0);

	while(true) {
		TMessage* message;
		TSocket* socket;

		socket = monitor->Select();

		if(socket->IsA() == TServerSocket::Class()) {
			if(clientCount > 100) {
				std::cerr<<"accepting only 100 client connections"<<std::endl;
				monitor->Remove(socketServer);
				socketServer->Close();
			} else {
				TSocket* client = static_cast<TServerSocket*>(socket)->Accept();
				client->Send(clientCount, 0); //0 = kStartConnection
				client->Send(1, 1); //1 = kProtocol, 1 = kProtocolVersion
				++clientCount;
				monitor->Add(client);
				std::cout<<"Accepted "<<clientCount<<" connections"<<std::endl;
			}
			continue;
		}

		socket->Recv(message);

		if(message == nullptr) {
			std::cerr<<"server: The client did not send a message"<<std::endl;
		} else if(message->What() == kMESS_STRING) {
			char str[64];
			message->ReadString(str,64);
			std::cout<<"Client "<<clientCount-1<<": '"<<str<<"'"<<std::endl;
			monitor->Remove(socket);
			std::cout<<"Client "<<clientCount-1<<": received "<<socket->GetBytesRecv()<<" bytes, sent "<<socket->GetBytesSent()<<" bytes"<<std::endl;
			socket->Close();
			--clientCount;
			if(monitor->GetActive() == 0 || clientCount == 0) {
				std::cout<<"No more active clients => stopping"<<std::endl;
				break;
			}
		} else if(message->What() == kMESS_ANY) {
			Long64_t length;
			TString filename;
			int clientId;
			message->ReadInt(clientId);
			message->ReadTString(filename);
			message->ReadLong64(length);

			std::cout<<"server: Received input from client "<<clientId<<" for '"<<filename.Data()<<"'"<<std::endl;

			delete transient;
			transient = new TMemFile(filename, message->Buffer() + message->Length(), length);
			message->SetBufferOffset(message->Length()+length);
			if(!merger.OutputFile(filename,"update")) {
				std::cout<<"server: failed to open output-file!"<<std::endl;
				return false;
			}
			if(!merger.AddAdoptFile(transient)) {
				std::cout<<"server: failed to adopt file!"<<std::endl;
				return false;
			}

			if(!merger.PartialMerge(TFileMerger::kAllIncremental)) {
				std::cout<<"server: failed to partial merge!"<<std::endl;
				return false;
			}
			transient = nullptr;
		} else if(message->What() == kMESS_OBJECT) {
			std::cout<<"Got object of class '"<<message->GetClass()->GetName()<<"'"<<std::endl;
		} else {
			std::cout<<"Unexpected message!"<<std::endl;
		}

		delete message;
	}
	std::cout<<"stopping server"<<std::endl;

	return true;
}

