#include "TFragmentQueue.h"

std::mutex TFragmentQueue::All;
std::mutex TFragmentQueue::Sorted;

/// \cond CLASSIMP
ClassImp(TFragmentQueue)
/// \endcond

////////////////////////////////////////////////////////////////
//                                                            //
// TFragmentQueue                                             //
//                                                            //
// This class is where we store fragments that are being      //
// built into events by the TAnalysisTree. It is thread-safe  //
// and returns it's status in order to monitor progress.      //
//                                                            //
////////////////////////////////////////////////////////////////

TFragmentQueue* TFragmentQueue::fFragmentQueueClassPointer = NULL;
std::map<std::string,TFragmentQueue*>* TFragmentQueue::fFragmentMap = new std::map<std::string,TFragmentQueue*>;
std::map<int,int> TFragmentQueue::fFragmentIdMap;

TFragmentQueue* TFragmentQueue::GetQueue(std::string queuename)	{
//Get a pointer to the global event Q with the name queuename. 
  if(fFragmentMap->count(queuename) == 0) {
      while(!TFragmentQueue::All.try_lock())	{
         //try to lock Q, if another thread is using it do nothing
      }
      if(fFragmentMap->count(queuename) == 0) {
		  fFragmentMap->insert(std::pair<std::string,TFragmentQueue*>(queuename,new TFragmentQueue));
      }
   
      //unlock the Q when this thread is done with it.
      TFragmentQueue::All.unlock();
   }

   return fFragmentMap->at(queuename);
}

TFragmentQueue::TFragmentQueue()	{	
	fFragsInQueue = 0;
	//When the Global Q is created, start a timer to see how long we are using it.
	fSw = new TStopwatch();
	fSw->Start();

	fStop = false;

	Clear();
}

TFragmentQueue::~TFragmentQueue() { }

void TFragmentQueue::Print(Option_t* opt) const { 
//Print the status of the Fragment Queue
   CheckStatus();   
}

void TFragmentQueue::Clear(Option_t* opt)	{
//Clear the entire Queue, Queue counters, and timer.
	bool locked = false;
	if(!fFragmentQueue.empty()) {
		while(!TFragmentQueue::All.try_lock())	{
			//do nothing
		}
		locked = true;
	}

	if(fFragsInQueue != 0) {
		printf(RED "\n\tWarning, discarding %i Fragemnts!" RESET_COLOR  "\n",fFragsInQueue); 
		while(!fFragmentQueue.empty()) {
			fFragmentQueue.pop();
		}	
		fFragsInQueue = 0;
	}

	fFragmentsIn  = 0;		
	fFragmentsOut = 0;

	fTotalFragsIn = 0;
	fTotalFragsOut = 0;

	fSw->Reset();

	if(locked)
		TFragmentQueue::All.unlock();
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

void TFragmentQueue::Add(TVirtualFragment* frag)	{
//Add a Fragment to the fragment Queue.

    //when we move to multithreaded parsing, these three lines will 
    //need to move inside the lock.  pcb.
	fFragmentIdMap[frag->GetTriggerId()]++;

	while(!TFragmentQueue::Sorted.try_lock())	{
		//do nothing	
	}

	fFragmentQueue.push(frag);

	fTotalFragsIn++;
	fFragsInQueue++;
	fFragmentsIn++;
	//sorted.unlock();
	
	TFragmentQueue::Sorted.unlock();
}

void TFragmentQueue::Pop()	{	
//Take a fragment out of the Queue
	while(!TFragmentQueue::Sorted.try_lock())	{ 
		//do nothing
	}	

	fFragmentQueue.pop();
	fFragsInQueue--;
	fFragmentsOut++;
	TFragmentQueue::Sorted.unlock();
}

TVirtualFragment* TFragmentQueue::PopFragment(){
//Take a fragment out of the Queue and return a pointer to it.
	while(!TFragmentQueue::Sorted.try_lock())	{
		//do nothing
	}
	if(Size()>0) {	
		TVirtualFragment* frag = fFragmentQueue.front();
		if(frag)	{
			fFragmentQueue.pop();
			fFragsInQueue--;
			fFragmentsOut++;
			fTotalFragsOut++;
		}
		TFragmentQueue::Sorted.unlock();
		return frag;
	} else {
		TFragmentQueue::Sorted.unlock();
		return 0;
	}
}

int TFragmentQueue::Size() const	{
//Returns the number of fragments in the Queue
	return fFragsInQueue;
}

void TFragmentQueue::CheckStatus() const	{
//Checks the status of the Queue. This is called by the Print() function.
	while(!TFragmentQueue::All.try_lock())	{
		//do nothing
	}
	
	printf( BLUE   "# Fragments currently in Q     = %d" RESET_COLOR "\n",Size());
	printf( BLUE   "# Total Fragments put in Q     = %d" RESET_COLOR "\n",fTotalFragsIn);
	printf( DGREEN "# Total Fragments taken from Q = %d" RESET_COLOR "\n",fTotalFragsOut);

	TFragmentQueue::All.unlock();
}

void TFragmentQueue::StatusUpdate()	{
//Updates the status of the fragment Queue
	float time = 0;
	float fragRateIn = 0;
	float fragRateOut = 0;

	while(fStatusUpdateOn)	{
		time = fSw->RealTime();
		fragRateIn = fFragmentsIn/time; 
		fragRateOut = fFragmentsOut/time;
		while(!TFragmentQueue::All.try_lock()){
			//do nothing
		}

		printf(BLUE "\n\tfrags rate in  = %.2f/sec, nqueue = %d\n" RESET_COLOR,fragRateIn, Size());
		printf(DGREEN "\tfrags rate out = %.2f/sec\n" RESET_COLOR,fragRateOut);
		TFragmentQueue::All.unlock();
		ResetRateCounter();
		fSw->Start();
	}
}

void TFragmentQueue::ResetRateCounter()	{
//Resets the number of fragments in and fragments out counter. This is useful for checking to see if the rate
//of fragment parsing is changing.
	while(!TFragmentQueue::All.try_lock())	{
		//do nothing
	}

	fFragmentsIn	 =	0;		
	fFragmentsOut = 0;
	TFragmentQueue::All.unlock();
}
