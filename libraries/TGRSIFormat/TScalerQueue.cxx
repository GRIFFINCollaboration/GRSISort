
#include "TScalerQueue.h"

std::mutex TScalerQueue::All;
std::mutex TScalerQueue::Sorted;

ClassImp(TScalerQueue)

////////////////////////////////////////////////////////////////
//                                                            //
// TScalerQueue                                               //
//                                                            //
// This class is where we store scalers. It is thread-safe    //
// and returns it's status in order to monitor progress.      //
//                                                            //
////////////////////////////////////////////////////////////////

TScalerQueue *TScalerQueue::fScalerQueueClassPointer = NULL;

TScalerQueue *TScalerQueue::Get() {
  ///Get a pointer to the global scaler Q. 
  if(fScalerQueueClassPointer == NULL) {
	 fScalerQueueClassPointer = new TScalerQueue;
  }
  return fScalerQueueClassPointer;
}

TScalerQueue::TScalerQueue() {	
	fScalerQueueClassPointer = this;
	fScalersInQueue = 0;
	//When the Global Q is created, start a timer to see how long we are using it.
	fStopwatch = new TStopwatch();
	fStopwatch->Start();

	fStop = false;

	Clear();

}

TScalerQueue::~TScalerQueue() {	}


void TScalerQueue::Print(Option_t *opt) const { 
   ///Print the status of the Scaler Queue
   CheckStatus();   
}

void TScalerQueue::Clear(Option_t *opt) {
   ///Clear the entire Queue, Queue counters, and timer.
	bool locked = false;
	if(!fScalerQueue.empty()) {
		while(!TScalerQueue::All.try_lock()) {
			//do nothing
		}
		locked = true;
	}

	if(fScalersInQueue != 0) {
		printf(RED "\n\tWarning, discarding %i Scalers!" RESET_COLOR "\n", fScalersInQueue); 
		while(!fScalerQueue.empty()){
			fScalerQueue.pop();
		}	
		fScalersInQueue = 0;
	}

	fScalersIn  = 0;		
	fScalersOut = 0;

	fTotalScalersIn = 0;
	fTotalScalersOut = 0;

	fStopwatch->Reset();

	if(locked)
		TScalerQueue::All.unlock();
	return;
}


void TScalerQueue::StartStatusUpdate() {
   ///The status thread runs the status update at various intervals to show the progress of the analysis tree.
	fStatusUpdateOn = true;

	std::thread statusUpdate(&TScalerQueue::StatusUpdate, this);
	statusUpdate.detach();
}

void TScalerQueue::StopStatusUpdate() {
   ///Stops the status update
	fStatusUpdateOn = false;
}



void TScalerQueue::Add(TScalerData* scalerData) {
	///Add a Scaler to the scaler Queue.
	if(scalerData == NULL) {
		return;
	}

	while(!TScalerQueue::Sorted.try_lock()) {
		//do nothing	
	}

	fScalerQueue.push(scalerData);

	fTotalScalersIn++;
	fScalersInQueue++;
	fScalersIn++;

	TScalerQueue::Sorted.unlock();

	return;
}

void TScalerQueue::Pop() {	
   //Take a scaler out of the Queue
	while(!TScalerQueue::Sorted.try_lock()) {
		//do nothing
	}	
	fScalerQueue.pop();
	fScalersInQueue--;
	fScalersOut++;
	TScalerQueue::Sorted.unlock();
}

TScalerData* TScalerQueue::PopScaler(){
   //Take a scaler out of the Queue and return a pointer to it.
	while(!TScalerQueue::Sorted.try_lock()) {
		//do nothing
	}
	if(Size()>0) {	
		TScalerData* scaler = fScalerQueue.front();
		if(scaler) {
			fScalerQueue.pop();
			fScalersInQueue--;
			fScalersOut++;
			fTotalScalersOut++;
		}
		TScalerQueue::Sorted.unlock();
		return scaler;
	} else {
		TScalerQueue::Sorted.unlock();
		return 0;
	}
}

int TScalerQueue::Size() const {
   //Returns the number of scalers in the Queue
	return fScalersInQueue;
}

void TScalerQueue::CheckStatus() const {
   //Checks the status of the Queue. This is called by the Print() function.
	while(!TScalerQueue::All.try_lock()) {
		//do nothing
	}
	
	printf( BLUE   "# Scalers currently in Q     = %d" RESET_COLOR "\n",Size());
	printf( BLUE   "# Total Scalers put in Q     = %d" RESET_COLOR "\n",fTotalScalersIn);
	printf( DGREEN "# Total Scalers taken from Q = %d" RESET_COLOR "\n",fTotalScalersOut);

	TScalerQueue::All.unlock();
	return;
}

void TScalerQueue::StatusUpdate() {
   //Updates the status of the scaler Queue
	float time = 0;
	float scaler_rate_in = 0;
	float scaler_rate_out = 0;

	while(fStatusUpdateOn) {
		time = fStopwatch->RealTime();
		scaler_rate_in = fScalersIn/time; 
		scaler_rate_out = fScalersOut/time;
		while(!TScalerQueue::All.try_lock()){
			//do nothing
		}

		printf(BLUE "\n\tscalers rate in  = %.2f/sec, nqueue = %d\n" RESET_COLOR,scaler_rate_in, Size());
		printf(DGREEN "\tscalers rate out = %.2f/sec\n" RESET_COLOR,scaler_rate_out);
		TScalerQueue::All.unlock();
		ResetRateCounter();
		fStopwatch->Start();
	}
}

void TScalerQueue::ResetRateCounter() {
   //Resets the number of scalers in and scalers out counter. This is useful for checking to see if the rate
   //of scaler parsing is changing.
	while(!TScalerQueue::All.try_lock()) {
		//do nothing
	}

	fScalersIn	= 0;		
	fScalersOut	= 0;
	TScalerQueue::All.unlock();
}



