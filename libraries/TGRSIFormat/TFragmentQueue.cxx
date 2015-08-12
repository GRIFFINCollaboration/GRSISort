
#include "TFragmentQueue.h"

std::mutex TFragmentQueue::All;
std::mutex TFragmentQueue::Sorted;

ClassImp(TFragmentQueue);

////////////////////////////////////////////////////////////////
//                                                            //
// TFragmentQueue                                             //
//                                                            //
// This class is where we store fragments that are being      //
// built into events by the TAnalysisTree. It is thread-safe  //
// and returns it's status in order to monitor progress.      //
//                                                            //
////////////////////////////////////////////////////////////////

TFragmentQueue *TFragmentQueue::fFragmentQueueClassPointer = NULL;
std::map<std::string,TFragmentQueue*> *TFragmentQueue::fFragmentMap = new std::map<std::string,TFragmentQueue*>;
std::map<int,int> TFragmentQueue::fragment_id_map;

TFragmentQueue *TFragmentQueue::GetQueue(std::string quename)	{
//Get a pointer to the global event Q with the name quename. 
   
   if(fFragmentMap->count(quename) == 0) {
      while(!TFragmentQueue::All.try_lock())	{
         //try to lock Q, if another thread is using it do nothing
      }
      if(fFragmentMap->count(quename) == 0) {
	 fFragmentMap->insert(std::pair<std::string,TFragmentQueue*>(quename,new TFragmentQueue));
      }
   
      //unlock the Q when this thread is done with it.
      TFragmentQueue::All.unlock();
   }

   return fFragmentMap->at(quename);
}

TFragmentQueue::TFragmentQueue()	{	
	fFragsInQueue = 0;
	//When the Global Q is created, start a timer to see how long we are using it.
	sw = new TStopwatch();
	sw->Start();

	fStop = false;

	Clear();

}

TFragmentQueue::~TFragmentQueue()	{	}


void TFragmentQueue::Print(Option_t *opt) { 
//Print the status of the Fragment Queue
   CheckStatus();   
}

void TFragmentQueue::Clear(Option_t *opt)	{
//Clear the entire Queue, Queue counters, and timer.
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
//The status thread runs the status update at various intervals to show the progress of the analysis tree.
	fStatusUpdateOn = true;

	std::thread statusUpdate(&TFragmentQueue::StatusUpdate, this);
	statusUpdate.detach();

}

void TFragmentQueue::StopStatusUpdate()	{
//Stops the status update
	fStatusUpdateOn = false;
}



void TFragmentQueue::Add(TFragment *frag)	{
//Add a Fragment to the fragment Queue.


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
//Take a fragment out of the Queue
	while(!TFragmentQueue::Sorted.try_lock())	{ 
		//do nothing
	}	

	TObject *frag = (fFragmentQueue.front());	
	fFragmentQueue.pop();
	fFragsInQueue--;
	fragments_out++;
	//fTotalFragsOut++;
	TFragmentQueue::Sorted.unlock();
	//delete frag;
}

TFragment *TFragmentQueue::PopFragment(){
//Take a fragment out of the Queue and return a pointer to it.
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
//Returns the number of fragments in the Queue
	return fFragsInQueue;
}

void TFragmentQueue::CheckStatus()	{
//Checks the status of the Queue. This is called by the Print() function.
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
//Updates the status of the fragment Queue
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
//Resets the number of fragments in and fragments out counter. This is useful for checking to see if the rate
//of fragment parsing is changing.
	//std::unique_lock<std::mutex> all(All,std::defer_lock);
	//all.lock();
	while(!TFragmentQueue::All.try_lock())	{
		//do nothing
	}

	fragments_in	=	0;		
	fragments_out	= 0;
	TFragmentQueue::All.unlock();
}



