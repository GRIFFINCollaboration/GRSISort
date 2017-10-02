#ifndef TTIGSCALERQUEUE_H
#define TTIGSCALERQUEUE_H

/** \addtogroup Sorting
 *  @{
 */

#include <cstdio>
#include <queue>
#include <map>

#if !defined(__CINT__) && !defined(__CLING__)
#define _GLIBCXX_USE_NANOSLEEP 1
#include <thread>
#include <mutex>
#endif

#include "TObjArray.h"
#include "TStopwatch.h"

#include "TScaler.h"

class TDeadtimeScalerQueue : public TObject {
public:
   static TDeadtimeScalerQueue* Get(); // Returns the Queue
   ~TDeadtimeScalerQueue() override;

   int ScalersInQueue() { return fScalersInQueue; }

private:
   TDeadtimeScalerQueue();
   static TDeadtimeScalerQueue* fDeadtimeScalerQueueClassPointer; // Pointer to the scaler Q singleton

   std::queue<TScalerData*> fDeadtimeScalerQueue; // The scaler Queue itself
   int                      fScalersInQueue;      // The current number of scalers in the Q

   void StatusUpdate();
   bool fStatusUpdateOn{false}; // flag that determines whether the Q status should be read out

   bool fStop;

   int fScalersIn{0};
   int fScalersOut{0};

   TStopwatch* fStopwatch; // The stop watch used for timing in the status
   void        ResetRateCounter();

   unsigned int fTotalScalersIn{0};
   unsigned int fTotalScalersOut{0};

   static std::map<int, int> fScalerIdMap;

#if !defined(__CINT__) && !defined(__CLING__)
#ifndef NO_MUTEX
public:
   static std::mutex All;
   static std::mutex Sorted;
#endif
#endif

public:
   void Add(TScalerData*);

   void         Pop() override;
   TScalerData* PopScaler();

   int Size() const;

   void StartStatusUpdate();
   void StopStatusUpdate();
   void CheckStatus() const;

   unsigned int GetTotalScalersIn() { return fTotalScalersIn; }
   unsigned int GetTotalScalersOut() { return fTotalScalersOut; }

   bool Running() { return !fStop; }
   void Stop() { fStop = true; }

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;
};

class TRateScalerQueue : public TObject {
public:
   static TRateScalerQueue* Get(); // Returns the Queue
   ~TRateScalerQueue() override;

   int ScalersInQueue() { return fScalersInQueue; }

private:
   TRateScalerQueue();
   static TRateScalerQueue* fRateScalerQueueClassPointer; // Pointer to the scaler Q singleton

   std::queue<TScalerData*> fRateScalerQueue; // The scaler Queue itself
   int                      fScalersInQueue;  // The current number of scalers in the Q

   void StatusUpdate();
   bool fStatusUpdateOn{false}; // flag that determines whether the Q status should be read out

   bool fStop;

   int fScalersIn{0};
   int fScalersOut{0};

   TStopwatch* fStopwatch; // The stop watch used for timing in the status
   void        ResetRateCounter();

   unsigned int fTotalScalersIn{0};
   unsigned int fTotalScalersOut{0};

   static std::map<int, int> fScalerIdMap;

#if !defined(__CINT__) && !defined(__CLING__)
#ifndef NO_MUTEX
public:
   static std::mutex All;
   static std::mutex Sorted;
#endif
#endif

public:
   void Add(TScalerData*);

   void         Pop() override;
   TScalerData* PopScaler();

   int Size() const;

   void StartStatusUpdate();
   void StopStatusUpdate();
   void CheckStatus() const;

   unsigned int GetTotalScalersIn() { return fTotalScalersIn; }
   unsigned int GetTotalScalersOut() { return fTotalScalersOut; }

   bool Running() { return !fStop; }
   void Stop() { fStop = true; }

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;
};
/*! @} */
#endif
