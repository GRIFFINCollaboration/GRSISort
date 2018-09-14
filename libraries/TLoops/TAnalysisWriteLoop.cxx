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
#include "TRunInfo.h"
#include "TGRSIOptions.h"
#include "TTreeFillMutex.h"
#include "TSortingDiagnostics.h"
#include "TParallelFileMerger.h"

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
   : StoppableThread(name), fOutputFilename(outputFilename), fCurrentClient(0),
     fInputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent>>>()),
     fOutOfOrderQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<const TFragment>>>())
{
	if(fOutputFilename != "/dev/null") {
		/// Open a server socket looking for connections on a named service or on a specific port.
		fServerSocket = new TServerSocket(0, false, 100); // 0 = scan ports to find free one, false = don't reuse socket, 100 = backlog (queue length for pending connections)
		if(fServerSocket == nullptr || !fServerSocket->IsValid()) throw;
		fServerFuture = std::async(std::launch::async, &TAnalysisWriteLoop::Server, this);
		fClients.resize(TGRSIOptions::Get()->NumberOfClients());
		for(size_t i = 0; i < fClients.size(); ++i) {
			fClients[i] = new TAnalysisWriteLoopClient(Form("write_client_%lu", i), fOutputFilename, fServerSocket->GetLocalPort());
		}
		std::cout<<"created server with "<<fClients.size()<<" clients on port "<<fServerSocket->GetLocalPort()<<std::endl;
	}
	fOutOfOrder = TGRSIOptions::Get()->SeparateOutOfOrder();
}

TAnalysisWriteLoop::~TAnalysisWriteLoop()
{
	if(!fServerFuture.get()) {
		std::cout<<"Server failed!"<<std::endl;
	}
	Write();
	delete fServerSocket;
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
   } else {
		++fItemsPopped;
	}

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
		gROOT->cd();
		TRunInfo* runInfo = TRunInfo::Get();
		TGRSIOptions* options = TGRSIOptions::Get();
		TPPG* ppg = TPPG::Get();
		TSortingDiagnostics* diag = TSortingDiagnostics::Get();

		TFile* outputFile = new TFile(fOutputFilename.c_str(), "update");
		outputFile->cd();

		if(GValue::Size() != 0) {
			GValue::Get()->Write("Values", TObject::kOverwrite);
		}
		if(TChannel::GetNumberOfChannels() != 0) {
			TChannel::WriteToRoot();
		}
		runInfo->WriteToRoot(outputFile);
		options->AnalysisOptions()->WriteToFile(outputFile);
		ppg->Write("PPG", TObject::kOverwrite);

		if(options->WriteDiagnostics()) {
			diag->Write("SortingDiagnostics", TObject::kOverwrite);
		}

		outputFile->Close();
		outputFile->Delete();
	}
}

bool TAnalysisWriteLoop::Server()
{
	TMonitor* monitor = new TMonitor;
	monitor->Add(fServerSocket);

	unsigned int clientIndex = 0; //only counts up
	unsigned int clientCount = 0; //number of connections
	TMemFile* transient = nullptr;

	//TFileMerger merger(false, false);//false, false = isn't local, not 'histOneGo', ParallelFileMerger from parallelMergeServer.C uses false, true
	//merger.SetPrintLevel(1);
	THashTable mergers;

	gErrorIgnoreLevel = kFatal;

	while(true) {
		TMessage* message;
		TSocket* socket;

		socket = monitor->Select();

		if(socket->IsA() == TServerSocket::Class()) {
			if(clientCount > 100) {
				std::cerr<<"accepting only 100 client connections"<<std::endl;
				monitor->Remove(fServerSocket);
				fServerSocket->Close();
			} else {
				TSocket* client = static_cast<TServerSocket*>(socket)->Accept();
				client->Send(clientIndex, 0); //0 = kStartConnection
				client->Send(1, 1); //1 = kProtocol, 1 = kProtocolVersion
				++clientCount;
				++clientIndex;
				monitor->Add(client);
			}
			continue;
		}

		socket->Recv(message);

		if(message == nullptr) {
			std::cerr<<"server: The client did not send a message"<<std::endl;
		} else if(message->What() == kMESS_STRING) {
			char str[64];
			message->ReadString(str,64);
			monitor->Remove(socket);
			std::cout<<"Client "<<clientCount-1<<": received "<<socket->GetBytesRecv()<<" bytes, sent "<<socket->GetBytesSent()<<" bytes"<<std::endl;
			socket->Close();
			--clientCount;
			if(monitor->GetActive() == 0 || clientCount == 0) {
				break;
			}
		} else if(message->What() == kMESS_ANY) {
			Long64_t length;
			TString filename;
			int clientId;
			message->ReadInt(clientId);
			message->ReadTString(filename);
			message->ReadLong64(length);

			//std::cout<<"server: Received input from client "<<clientId<<" for '"<<filename.Data()<<"'"<<std::endl;

			transient = new TMemFile(filename, message->Buffer() + message->Length(), length);
			message->SetBufferOffset(message->Length()+length);

			const Float_t clientThreshold = 0.75; // control how often the histogram are merged.  Here as soon as half the clients have reported.

			TParallelFileMerger* info = static_cast<TParallelFileMerger*>(mergers.FindObject(filename));
			if(info == nullptr) {
				info = new TParallelFileMerger(filename, true); // true = use TFileCacheWrite
				mergers.Add(info);
			}
			if(NeedInitialMerge(transient)) {
				info->InitialMerge(transient);
			}
			info->RegisterClient(clientId, transient);
			if(info->NeedMerge(clientThreshold)) {
				info->Merge();
			}
			transient = nullptr;
		} else if(message->What() == kMESS_OBJECT) {
			std::cout<<"Got object of class '"<<message->GetClass()->GetName()<<"'"<<std::endl;
		} else {
			std::cout<<"Unexpected message!"<<std::endl;
		}

		delete message;
	}

	std::cout<<"final merging ..."<<std::endl;
	TIter next(&mergers);
	TParallelFileMerger* info;
	while((info = static_cast<TParallelFileMerger*>(next())) != nullptr) {
		if(info->NeedFinalMerge()) {
			info->Merge();
		}
	}
	mergers.Delete();
	delete monitor;

	std::cout<<"server stopped"<<std::endl;

	return true;
}

