
#include "TFragmentQueue.h"

//#include "CalibrationManager.h"
//#include "RootIOManager.h"


std::mutex TFragmentQueue::All;
std::mutex TFragmentQueue::Sorted;

ClassImp(TFragmentQueue);

TFragmentQueue *TFragmentQueue::fFragmentQueueClassPointer = NULL;
std::map<std::string,TFragmentQueue*> *TFragmentQueue::fFragmentMap = new std::map<std::string,TFragmentQueue*>;
std::map<int,int> TFragmentQueue::fragment_id_map;



TFragmentQueue *TFragmentQueue::GetQueue(std::string quename)	{
	while(TFragmentQueue::All.try_lock())	{
		//do nothing
	}
   
   
   if(fFragmentMap->count(quename) == 0)
      fFragmentMap->insert(std::pair<std::string,TFragmentQueue*>(quename,new TFragmentQueue));

   
   /*
   if(fFragmentQueueClassPointer==NULL)	{
	//printf(BLUE "\nfrag que ptr = 0x%08x" RESET_COLOR "\n",fFragmentQueueClassPointer);
		fFragmentQueueClassPointer = new TFragmentQueue();
	//printf(RED "\nfrag que ptr = 0x%08x" RESET_COLOR "\n",fFragmentQueueClassPointer);
	}
   */


   TFragmentQueue::All.unlock();
	//return fFragmentQueueClassPointer;
	return fFragmentMap->at(quename);

}

TFragmentQueue::TFragmentQueue()	{	
	//printf(DRED "\n\tFragment Queue Created." RESET_COLOR "\n");
	fFragsInQueue = 0;

	sw = new TStopwatch();
	sw->Start();

	fStop = false;

	Clear();

	//StartStatusUpdate();

}

TFragmentQueue::~TFragmentQueue()	{	}


void TFragmentQueue::Print(Option_t *opt) { 
   CheckStatus();   
}

void TFragmentQueue::Clear(Option_t *opt)	{
	bool locked =false;
	if(!fFragmentQueue.empty())	{
		while(!TFragmentQueue::All.try_lock())	{
			//do nothing
		}
		locked = true;
	}

	if(fFragsInQueue != 0)	{
//		if(strncmp(opt,"force",5))	{
			printf(RED "\n\tWarning, discarding %i Fragemnts!" RESET_COLOR  "\n",fFragsInQueue); 
			while(!fFragmentQueue.empty()){
				fFragmentQueue.pop();
			}	
			fFragsInQueue = 0;
//		}
//		else	{
//			printf(RED "\n\tCan not reset Q Counters; %i Fragments still in Queue!!" RESET_COLOR  "\n",fFragsInQueue);	
//			return;
//		}
	}

	fragments_in  = 0;		
	fragments_out = 0;

	fTotalFragsIn = 0;
	fTotalFragsOut = 0;

	sw->Reset();

	if(locked)
		TFragmentQueue::All.unlock();
	return;
}


void TFragmentQueue::StartStatusUpdate()	{
	fStatusUpdateOn = true;

	std::thread statusUpdate(&TFragmentQueue::StatusUpdate, this);
	statusUpdate.detach();

}

void TFragmentQueue::StopStatusUpdate()	{
	fStatusUpdateOn = false;
}



void TFragmentQueue::Add(TFragment *frag)	{

    //when we move to multithreaded parsing, these three lines will 
    //need to move inside the lock.  pcb.
//	CalibrationManager::instance()->CalibrateFragment(frag);
		fragment_id_map[frag->TriggerId]++;
		frag->FragmentId = fragment_id_map[frag->TriggerId];
		
	
		
		while(!TFragmentQueue::Sorted.try_lock())	{
			//do nothing	
		}

		fFragmentQueue.push(frag);

		fTotalFragsIn++;
		fFragsInQueue++;
		fragments_in++;
		//sorted.unlock();
	
		TFragmentQueue::Sorted.unlock();

	return;
	//printf("Adding frag:\t%i\tNumber in Q = %i\n",frag->MidasId,fFragsInQueue);
}

/*
void TFragmentQueue::Add(TGrifFragment *frag)	{

    //when we move to multithreaded parsing, these three lines will 
    //need to move inside the lock.  pcb.
	fragment_id_map[frag->TriggerId]++;
	frag->FragmentId = fragment_id_map[frag->TriggerId];

		while(!TFragmentQueue::Sorted.try_lock())	{
			//do nothing	
		}

		fFragmentQueue.push(frag);

		fTotalFragsIn++;
		fFragsInQueue++;
		fragments_in++;
		//sorted.unlock();
	
		TFragmentQueue::Sorted.unlock();


	return;
	//printf("Adding frag:\t%i\tNumber in Q = %i\n",frag->MidasId,fFragsInQueue);
}
*/

/*
TFragment *TFragmentQueue::Get()	{	

    //std::unique_lock<std::mutex> sorted(Sorted,std::defer_lock);
    //sorted.lock();
	while(TFragmentQueue::Sorted.try_lock())	{
		//do nothing
	}


	TFragment *frag = (fFragmentQueue.back());	

   TFragmentQueue::Sorted.unlock();
	//printf("\tGetting frag:\t%i\tNumber in Q = %i\n",frag->MidasId,fFragsInQueue);
	return frag;
}*/

void TFragmentQueue::Pop()	{	

	while(!TFragmentQueue::Sorted.try_lock())	{ 
		//do nothing
	}	

	TFragment *frag = (fFragmentQueue.front());	
	fFragmentQueue.pop();
	fFragsInQueue--;
	fragments_out++;
	//fTotalFragsOut++;
	TFragmentQueue::Sorted.unlock();
	//delete frag;
}

TFragment *TFragmentQueue::PopFragment()
{
	//std::unique_lock<std::mutex> sorted(Sorted,std::defer_lock);
	//sorted.lock();
	while(!TFragmentQueue::Sorted.try_lock())	{
		//do nothing
	}
	if(Size()>0) {	
		TFragment *frag = fFragmentQueue.front();
		if(frag)	{
			fFragmentQueue.pop();
			fFragsInQueue--;
			fragments_out++;
			fTotalFragsOut++;
		}
	       TFragmentQueue::Sorted.unlock();
		return frag;
	} else {
	       TFragmentQueue::Sorted.unlock();
		return 0;
	}
}	

int TFragmentQueue::Size()	{
	return fFragsInQueue;
}

void TFragmentQueue::CheckStatus()	{
	//std::unique_lock<std::mutex> all(All,std::defer_lock);
	//all.lock();
	while(!TFragmentQueue::All.try_lock())	{
		//do nothing
	}
	

	printf( BLUE   "# Fragments currently in Q     = %d" RESET_COLOR "\n",Size());
//	printf( DGREEN "# Fragments currently in T     = %d" RESET_COLOR "\n",RootIOManager::instance()->TreeSize());

	printf( BLUE   "# Total Fragments put in Q     = %d" RESET_COLOR "\n",fTotalFragsIn);
	printf( DGREEN "# Total Fragments taken from Q = %d" RESET_COLOR "\n",fTotalFragsOut);

	TFragmentQueue::All.unlock();
	return;
};

void TFragmentQueue::StatusUpdate()	{

	float time = 0;
	float frag_rate_in = 0;
	float frag_rate_out = 0;

	 //std::unique_lock<std::mutex> all(All,std::defer_lock);


	while(fStatusUpdateOn)	{
		//printf("Fragments in que: %i\n",frag_q_ptr->Size());
		time = sw->RealTime();
		frag_rate_in = fragments_in/time; 
		frag_rate_out = fragments_out/time;
		while(!TFragmentQueue::All.try_lock()){
			//do nothing
		}



		printf(BLUE "\n\tfrags rate in  = %.2f/sec, nqueue = %d\n" RESET_COLOR,frag_rate_in, Size());
		printf(DGREEN "\tfrags rate out = %.2f/sec\n" RESET_COLOR,frag_rate_out);
		TFragmentQueue::All.unlock();
//		TThread::ps();
		ResetRateCounter();
		sw->Start();
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	}
	//TThread::CleanUp();
}

void TFragmentQueue::ResetRateCounter()	{
	//std::unique_lock<std::mutex> all(All,std::defer_lock);
	//all.lock();
	while(!TFragmentQueue::All.try_lock())	{
		//do nothing
	}

	fragments_in	=	0;		
	fragments_out	= 0;
	TFragmentQueue::All.unlock();
}



