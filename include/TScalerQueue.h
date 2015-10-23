#ifndef TTIGSCALERQUEUE_H
#define TTIGSCALERQUEUE_H

#include <stdio.h>
#include <queue>
#include <map>

#ifndef __CINT__
#define _GLIBCXX_USE_NANOSLEEP 1
#include <thread>
#include <mutex>
#endif

#include "TObjArray.h"
#include "TStopwatch.h"

#include "TScaler.h"

class TDeadtimeScalerQueue : public TObject {
	
	public:
		static TDeadtimeScalerQueue* Get(); //Returns the Queue
		virtual ~TDeadtimeScalerQueue();

      int ScalersInQueue() { return fScalersInQueue;   }

	private:
      TDeadtimeScalerQueue();
		static TDeadtimeScalerQueue* fDeadtimeScalerQueueClassPointer; //Pointer to the scaler Q singleton

		std::queue<TScalerData*> fDeadtimeScalerQueue; //The scaler Queue itself
		int fScalersInQueue;	//The current number of scalers in the Q

		void StatusUpdate();
		bool fStatusUpdateOn; //flag that determines whether the Q status should be read out

		bool fStop;
		
		int fScalersIn; 		
		int fScalersOut;

		TStopwatch* fStopwatch; //The stop watch used for timing in the status
		void ResetRateCounter();

		unsigned int fTotalScalersIn;
		unsigned int fTotalScalersOut;	

		static std::map<int,int> fScalerIdMap;	


#ifndef __CINT__
#ifndef NO_MUTEX
	public:
		static std::mutex All;
		static std::mutex Sorted;
#endif
#endif

	public:
		void Add(TScalerData*);
	
		void Pop();
		TScalerData* PopScaler();

		int Size() const;

		void StartStatusUpdate();
		void StopStatusUpdate();
		void CheckStatus() const;

		unsigned int GetTotalScalersIn()  { return fTotalScalersIn;}
		unsigned int GetTotalScalersOut() {	return fTotalScalersOut;}

		bool Running() { return !fStop;}
		void Stop() { fStop = true;}

      void Print(Option_t *opt = "") const;
		void Clear(Option_t *opt = "");
};

class TRateScalerQueue : public TObject {
	
	public:
		static TRateScalerQueue* Get(); //Returns the Queue
		virtual ~TRateScalerQueue();

      int ScalersInQueue() { return fScalersInQueue;   }

	private:
      TRateScalerQueue();
		static TRateScalerQueue* fRateScalerQueueClassPointer; //Pointer to the scaler Q singleton

		std::queue<TScalerData*> fRateScalerQueue; //The scaler Queue itself
		int fScalersInQueue;	//The current number of scalers in the Q

		void StatusUpdate();
		bool fStatusUpdateOn; //flag that determines whether the Q status should be read out

		bool fStop;
		
		int fScalersIn; 		
		int fScalersOut;

		TStopwatch* fStopwatch; //The stop watch used for timing in the status
		void ResetRateCounter();

		unsigned int fTotalScalersIn;
		unsigned int fTotalScalersOut;	

		static std::map<int,int> fScalerIdMap;	


#ifndef __CINT__
#ifndef NO_MUTEX
	public:
		static std::mutex All;
		static std::mutex Sorted;
#endif
#endif

	public:
		void Add(TScalerData*);
	
		void Pop();
		TScalerData* PopScaler();

		int Size() const;

		void StartStatusUpdate();
		void StopStatusUpdate();
		void CheckStatus() const;

		unsigned int GetTotalScalersIn()  { return fTotalScalersIn;}
		unsigned int GetTotalScalersOut() {	return fTotalScalersOut;}

		bool Running() { return !fStop;}
		void Stop() { fStop = true;}

      void Print(Option_t *opt = "") const;
		void Clear(Option_t *opt = "");
};

#endif
