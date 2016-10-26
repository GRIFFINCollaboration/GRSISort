#include "TUnpackingLoop.h"

#include <chrono>
#include <thread>
#include <sstream>

#include "TGRSIOptions.h"

//ClassImp(TUnpackingLoop)

TUnpackingLoop *TUnpackingLoop::Get(std::string name) {
	if(name.length()==0) {
		name = "unpacking_loop";
	}

	TUnpackingLoop *loop = static_cast<TUnpackingLoop*>(StoppableThread::Get(name));
	if(!loop) {
		loop = new TUnpackingLoop(name);
	}
	return loop;
}

TUnpackingLoop::TUnpackingLoop(std::string name)
	: StoppableThread(name),
	fInputQueue(std::make_shared<ThreadsafeQueue<TMidasEvent> >()),
	fFragsReadFromMidas(0),fGoodFragsRead(0) {
	}

TUnpackingLoop::~TUnpackingLoop() { }

void TUnpackingLoop::ClearQueue() {
	TMidasEvent singleEvent;
	while(fInputQueue->Size()) {
		fInputQueue->Pop(singleEvent);
	}

   fParser.ClearQueue();
	//while(fParser.GoodOutputQueue()->Size()){
	//	fParser.GoodOutputQueue()->Pop(frag);
	//	delete frag;
	//}
}

bool TUnpackingLoop::Iteration(){
	TMidasEvent event;
	int error = fInputQueue->Pop(event);
	if(error<0) {
		fInputSize = 0;
		if(fInputQueue->IsFinished()){
			// Source is dead, push the last event and stop.
			fParser.SetFinished();
			BadOutputQueue()->SetFinished();
			ScalerOutputQueue()->SetFinished();
			return false;
		} else {
			// Wait for the source to give more data.
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			return true;
		}
	}
	fInputSize = error;
	++fItemsPopped;

	ProcessMidasEvent(&event);
	return true;
}

bool TUnpackingLoop::ProcessMidasEvent(TMidasEvent* mEvent)   {
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
					if(!ProcessTIGRESS((uint32_t*)ptr, banksize, mEvent)) { }
					//(unsigned int)(mEvent->GetSerialNumber()),
					//(unsigned int)(mEvent->GetTimeStamp()))) { }
				} else if((banksize = mEvent->LocateBank(NULL,"GRF1",&ptr))>0) {
					if(!ProcessGRIFFIN((uint32_t*)ptr,banksize,TDataParser::EBank::kGRF1, mEvent)) { }
					//(unsigned int)(mEvent->GetSerialNumber()),
					//(unsigned int)(mEvent->GetTimeStamp()))) { }
				} else if((banksize = mEvent->LocateBank(NULL,"GRF2",&ptr))>0) {
					if(!ProcessGRIFFIN((uint32_t*)ptr,banksize,TDataParser::EBank::kGRF2, mEvent)) { }
				} else if((banksize = mEvent->LocateBank(NULL,"GRF3",&ptr))>0) {
					if(!ProcessGRIFFIN((uint32_t*)ptr,banksize,TDataParser::EBank::kGRF3, mEvent)) { }
				} else if((banksize = mEvent->LocateBank(NULL,"GRF4",&ptr))>0) {
					if(!ProcessGRIFFIN((uint32_t*)ptr,banksize,TDataParser::EBank::kGRF4, mEvent)) { }
				} else if(!TGRSIOptions::Get()->SuppressErrors()) {
					printf(DRED "\nUnknown bank in midas event #%d" RESET_COLOR "\n", mEvent->GetSerialNumber());
				}
				break;
			case 2:
				// if(!fIamGriffin) {
				//   break;
				// }
				mEvent->SetBankList();
				break;
			case 4:
			case 5:
				// mEvent->SetBankList();
				// if((banksize = mEvent->LocateBank(NULL,"MSRD",&ptr))>0) {
				//   if(!ProcessEPICS((float*)ptr, banksize, mEvent)) { }
				//   //(unsigned int)(mEvent->GetSerialNumber()),
				//   //(unsigned int)(mEvent->GetTimeStamp()))) { }
				// }

				break;
		};
	}
	catch(const std::bad_alloc&) {   }
	//printf("I AM HERE!\n");fflush(stdout);
	return true;

}

bool TUnpackingLoop::ProcessEPICS(float* ptr,int& dSize,TMidasEvent* mEvent) {
	unsigned int mserial=0; if(mEvent) mserial = (unsigned int)(mEvent->GetSerialNumber());
	unsigned int mtime=0;   if(mEvent) mtime   = (unsigned int)(mEvent->GetTimeStamp());
	//int epics_banks =
	fParser.EPIXToScalar(ptr,dSize,mserial,mtime);

	return true;
}

bool TUnpackingLoop::ProcessTIGRESS(uint32_t* ptr, int& dSize, TMidasEvent* mEvent)   {
	unsigned int mserial=0; if(mEvent) mserial = (unsigned int)(mEvent->GetSerialNumber());
	unsigned int mtime=0;   if(mEvent) mtime   = (unsigned int)(mEvent->GetTimeStamp());
	int frags = fParser.TigressDataToFragment(ptr,dSize,mserial,mtime);
	if(frags>0) {
		fFragsReadFromMidas += frags;
		fGoodFragsRead += frags;
		return true;
	} else  {
		fFragsReadFromMidas += 1;   // if the midas bank fails, we assume it only had one frag in it... this is just used for a print statement.
		if(!TGRSIOptions::Get()->SuppressErrors() && mEvent) {
			mEvent->Print(Form("a%i",(-1*frags)-1));
		}
		return false;
	}
}


bool TUnpackingLoop::ProcessGRIFFIN(uint32_t* ptr, int& dSize, TDataParser::EBank bank, TMidasEvent* mEvent) {
	unsigned int mserial=0; if(mEvent) mserial = (unsigned int)(mEvent->GetSerialNumber());
	unsigned int mtime=0;   if(mEvent) mtime   = (unsigned int)(mEvent->GetTimeStamp());
	//loop over words in event to find fragment header
	for(int index = 0; index < dSize;) {
		if(((ptr[index])&0xf0000000) == 0x80000000) {
			//if we found a fragment header we pass the data to the data parser which returns the number of words read
			int words = fParser.GriffinDataToFragment(&ptr[index],dSize-index,bank,mserial,mtime);
			if(words>0) {
				//we successfully read one event with <words> words, so we advance the index by words
				++fFragsReadFromMidas;
				++fGoodFragsRead;
				index += words;
			} else {
				//we failed to read the fragment on word <-words>, so advance the index by -words and we create an error message
				++fFragsReadFromMidas;   // if the midas bank fails, we assume it only had one frag in it... this is just used for a print statement.
				index -= words;

				if(!TGRSIOptions::Get()->SuppressErrors()) {
					if(!TGRSIOptions::Get()->LogErrors()) {
						printf(DRED "\n//**********************************************//" RESET_COLOR "\n");
						printf(DRED "\nBad things are happening. Failed on datum %i" RESET_COLOR "\n", index);
						if(mEvent)  mEvent->Print(Form("a%i",index-1));
						printf(DRED "\n//**********************************************//" RESET_COLOR "\n");
					} else {
						std::string errfilename = "error.log";
						// if(mFile) {
						//   if(mFile->GetSubRunNumber() != -1) {
						//     errfilename.append(Form("error%05i_%03i.log",mFile->GetRunNumber(),mFile->GetSubRunNumber()));
						//   } else {
						//     errfilename.append(Form("error%05i.log",mFile->GetRunNumber()));
						//   }
						// } else {
						//   errfilename.append("error_log.log");
						// }
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

std::string TUnpackingLoop::EndStatus() {
	std::stringstream ss;
	if(fFragsReadFromMidas > 0) {
		ss<<"\r"<<Name()<<":\t"<<fGoodFragsRead<<" good fragments out of "<<fFragsReadFromMidas<<" fragments => "<<(100.*fGoodFragsRead)/fFragsReadFromMidas<<"% passed"<<std::endl;
	} else {
		ss<<"\rno fragments read from midas => none parsed!"<<std::endl;
	}
	return ss.str();
}

