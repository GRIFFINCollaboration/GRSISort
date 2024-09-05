#ifndef TTIGSCALERQUEUE_H
#define TTIGSCALERQUEUE_H

/** \addtogroup Sorting
 *  @{
 */

#include <cstdio>
#include <queue>
#include <map>

#if !defined(__CINT__) && !defined(__CLING__)
// NOLINTBEGIN(readability-identifier-naming, cppcoreguidelines-macro-usage)
#define _GLIBCXX_USE_NANOSLEEP 1
// NOLINTEND(readability-identifier-naming, cppcoreguidelines-macro-usage)
#include <thread>
#include <mutex>
#endif

#include "TObjArray.h"
#include "TStopwatch.h"

#include "TScaler.h"

class TDeadtimeScalerQueue : public TObject {
public:
   static TDeadtimeScalerQueue* Get();   // Returns the Queue
   TDeadtimeScalerQueue(const TDeadtimeScalerQueue&)                = default;
   TDeadtimeScalerQueue(TDeadtimeScalerQueue&&) noexcept            = default;
   TDeadtimeScalerQueue& operator=(const TDeadtimeScalerQueue&)     = default;
   TDeadtimeScalerQueue& operator=(TDeadtimeScalerQueue&&) noexcept = default;
   ~TDeadtimeScalerQueue()                                          = default;

private:
   TDeadtimeScalerQueue();
   static TDeadtimeScalerQueue* fDeadtimeScalerQueueClassPointer;   // Pointer to the scaler Q singleton

   std::queue<TScalerData*> fDeadtimeScalerQueue;   // The scaler Queue itself
   int                      fScalersInQueue{0};     // The current number of scalers in the Q

   void StatusUpdate();
   bool fStatusUpdateOn{false};   // flag that determines whether the Q status should be read out

   bool fStop{false};

   int fScalersIn{0};
   int fScalersOut{0};

   TStopwatch* fStopwatch{nullptr};   // The stop watch used for timing in the status
   void        ResetRateCounter();

   unsigned int fTotalScalersIn{0};
   unsigned int fTotalScalersOut{0};

   static std::map<int, int> fScalerIdMap;

public:
#if !defined(__CINT__) && !defined(__CLING__)
#ifndef NO_MUTEX
   static std::mutex All;      // NOLINT(readability-identifier-naming)
   static std::mutex Sorted;   // NOLINT(readability-identifier-naming)
#endif
#endif

   void Add(TScalerData*);

   void         Pop() override;
   TScalerData* PopScaler();

   int Size() const;

   void StartStatusUpdate();
   void StopStatusUpdate();
   void CheckStatus() const;

   unsigned int GetTotalScalersIn() const { return fTotalScalersIn; }
   unsigned int GetTotalScalersOut() const { return fTotalScalersOut; }

   bool Running() const { return !fStop; }
   void Stop() { fStop = true; }

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;
};

class TRateScalerQueue : public TObject {
public:
   static TRateScalerQueue* Get();   // Returns the Queue
   TRateScalerQueue(const TRateScalerQueue&)                = default;
   TRateScalerQueue(TRateScalerQueue&&) noexcept            = default;
   TRateScalerQueue& operator=(const TRateScalerQueue&)     = default;
   TRateScalerQueue& operator=(TRateScalerQueue&&) noexcept = default;
   ~TRateScalerQueue()                                      = default;

private:
   TRateScalerQueue();
   static TRateScalerQueue* fRateScalerQueueClassPointer;   // Pointer to the scaler Q singleton

   std::queue<TScalerData*> fRateScalerQueue;     // The scaler Queue itself
   int                      fScalersInQueue{0};   // The current number of scalers in the Q

   void StatusUpdate();
   bool fStatusUpdateOn{false};   // flag that determines whether the Q status should be read out

   bool fStop{false};

   int fScalersIn{0};
   int fScalersOut{0};

   TStopwatch* fStopwatch{nullptr};   // The stop watch used for timing in the status
   void        ResetRateCounter();

   unsigned int fTotalScalersIn{0};
   unsigned int fTotalScalersOut{0};

   static std::map<int, int> fScalerIdMap;

public:
#if !defined(__CINT__) && !defined(__CLING__)
#ifndef NO_MUTEX
   static std::mutex All;      // NOLINT(readability-identifier-naming)
   static std::mutex Sorted;   // NOLINT(readability-identifier-naming)
#endif
#endif

   void Add(TScalerData*);

   void         Pop() override;
   TScalerData* PopScaler();

   int Size() const;

   void StartStatusUpdate();
   void StopStatusUpdate();
   void CheckStatus() const;

   unsigned int GetTotalScalersIn() const { return fTotalScalersIn; }
   unsigned int GetTotalScalersOut() const { return fTotalScalersOut; }

   bool Running() const { return !fStop; }
   void Stop() { fStop = true; }

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;
};
/*! @} */
#endif
