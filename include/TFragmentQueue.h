#ifndef TTIGFRAGMENTQUEUE_H
#define TTIGFRAGMENTQUEUE_H

#include <stdio.h>
#include <queue>
#include <map>

#if !defined (__CINT__) && !defined (__CLING__)
#define _GLIBCXX_USE_NANOSLEEP 1
#include <thread>
#include <mutex>
#endif

#include "TObjArray.h"
#include "TStopwatch.h"

#include "TFragment.h"

class TFragmentQueue : public TObject {
	
	public:
		static TFragmentQueue *GetQueue(std::string quename = "GOOD"); //Returns the Queue with the name "quename"
		virtual ~TFragmentQueue();

      int FragsInQueue() { return fFragsInQueue;   }

	private:
		static TFragmentQueue *fFragmentQueueClassPointer; //Pointer to the fragment Q multiton
      static std::map<std::string,TFragmentQueue*> *fFragmentMap;
      TFragmentQueue();
	

		std::queue<TFragment*> fFragmentQueue; //The fragment Queue
		int fFragsInQueue;	//The current number of fragments in the Q

		void StatusUpdate();
		bool fStatusUpdateOn; //flag that determines whether the Q status should be read out

		bool fStop;
		
		int fragments_in; 		
		int fragments_out;

		TStopwatch *sw; //The stop watch used for timing in the status
		void ResetRateCounter();

		unsigned int fTotalFragsIn;
		unsigned int fTotalFragsOut;	

		static std::map<int,int> fragment_id_map;	


#if !defined (__CINT__) && !defined (__CLING__)
#ifndef NO_MUTEX
	public:
		static std::mutex All;
		static std::mutex Sorted;
#endif
#endif

	public:
		void Add(TFragment*);
		//void Add(TGrifFragment*);


		//TFragment *GetQueue();	
	
		void Pop();
		TFragment *PopFragment();

		int Size() const;

		void StartStatusUpdate();
		void StopStatusUpdate();
		void CheckStatus() const;

		unsigned int GetTotalFragsIn() { return fTotalFragsIn;}
		unsigned int GetTotalFragsOut()	{	return fTotalFragsOut;}

		bool Running() { return !fStop;}
		void Stop() { fStop = true;}

      void Print(Option_t *opt = "") const;
		void Clear(Option_t *opt = "");
		
		ClassDef(TFragmentQueue,0); //The Class used to hold fragments when building events
};






#endif
