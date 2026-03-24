#ifndef TEVENTBUILDINGLOOP_H
#define TEVENTBUILDINGLOOP_H

/** \addtogroup Loops
 *  @{
 */

////////////////////////////////////////////////////////////////////////////////
///
/// \class TEventBuildingLoop
///
/// This loop builds events (vectors of fragments) based on timestamps and a
/// build windows.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef __CINT__
#include <memory>
#include <functional>
#include <set>
#include <cstdint>
#endif

#include "StoppableThread.h"
#include "ThreadsafeQueue.h"
#include "TFragment.h"

class TEventBuildingLoop : public StoppableThread {
public:
   enum class EBuildMode : std::uint8_t { kDefault,
                                          kTime,
                                          kTimestamp,
                                          kTriggerId,
                                          kSkip };

   static TEventBuildingLoop* Get(std::string name = "", EBuildMode mode = EBuildMode::kTimestamp, uint64_t buildWindow = 2000);
   TEventBuildingLoop(const TEventBuildingLoop&)                = delete;
   TEventBuildingLoop(TEventBuildingLoop&&) noexcept            = delete;
   TEventBuildingLoop& operator=(const TEventBuildingLoop&)     = delete;
   TEventBuildingLoop& operator=(TEventBuildingLoop&&) noexcept = delete;
   ~TEventBuildingLoop();

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TFragment>>>& InputQueue()
   {
      return fInputQueue;
   }
   std::shared_ptr<ThreadsafeQueue<std::vector<std::shared_ptr<TFragment>>>>& OutputQueue()
   {
      return fOutputQueue;
   }
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TFragment>>>& OutOfOrderQueue() { return fOutOfOrderQueue; }
#endif

   bool Iteration() override;

   void ClearQueue() override;

   size_t GetItemsPushed() override { return fOutputQueue->ItemsPushed(); }
   size_t GetItemsPopped() override { return fOutputQueue->ItemsPopped(); }
   size_t GetItemsCurrent() override { return fOutputQueue->Size(); }
   size_t GetRate() override { return 0; }

   void     SetBuildWindow(uint64_t val) { fBuildWindow = val; }
   uint64_t GetBuildWindow() const { return fBuildWindow; }

   void         SetSortDepth(unsigned int val) { fSortingDepth = val; }
   unsigned int GetSortDepth() const { return fSortingDepth; }

   std::string EndStatus() override;

private:
   TEventBuildingLoop(std::string name, EBuildMode mode, uint64_t buildWindow);

#ifndef __CINT__
   bool CheckBuildCondition(const std::shared_ptr<TFragment>&);
   bool CheckTimeCondition(const std::shared_ptr<TFragment>&);
   bool CheckTimestampCondition(const std::shared_ptr<TFragment>&);
   bool CheckTriggerIdCondition(const std::shared_ptr<TFragment>&);

   void CheckWrapAround(const std::shared_ptr<TFragment>&);

   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TFragment>>>              fInputQueue;
   std::shared_ptr<ThreadsafeQueue<std::vector<std::shared_ptr<TFragment>>>> fOutputQueue;
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TFragment>>>              fOutOfOrderQueue;
#endif

   EBuildMode   fBuildMode;
   unsigned int fSortingDepth;
   uint64_t     fBuildWindow;
   bool         fPreviousSortingDepthError;
   bool         fSkipInputSort;

   std::string fOffsetFile;               ///< name of file with the offset (see below)
   Long64_t    fDaqTimeStampOffset{0};    ///< offset between DAQ timestamp (time since 1970 in s) and the timestamp of the first fragment of the run. Read from a file created by sorting the first subrun
   Long64_t    fCycleLength{0};           ///< total length of cycle in us as defined by ODB
   Long64_t    fTapeMoveLength{0};        ///< length of the tape move in us (code 0xc008)
   Long64_t    fBackgroundLength{0};      ///< length of the background in us (code 0xc002)
   Long64_t    fImplantDaqOnLength{0};    ///< length of the implant(s) in us (DAQ is on ) (code 0xc001)
   Long64_t    fImplantDaqOffLength{0};   ///< length of the implant(s) in us (DAQ is off) (code 0x8001)
   Long64_t    fDecayDaqOffLength{0};     ///< length of the decay with DAQ off in us (code 0x8004)

   std::map<UInt_t, Long64_t> fLastTimeStamp; ///< map of the last time stamp for each address

#ifndef __CINT__
   std::vector<std::shared_ptr<TFragment>> fNextEvent;

   std::multiset<std::pair<std::shared_ptr<TFragment>, Long64_t>,
                 std::function<bool(std::pair<std::shared_ptr<TFragment>, Long64_t>, std::pair<std::shared_ptr<TFragment>, Long64_t>)>>
      fOrdered;
#endif

   /// \cond CLASSIMP
   ClassDefOverride(TEventBuildingLoop, 0)   // NOLINT(readability-else-after-return)
   /// \endcond
};

/*! @} */
#endif /* _TBUILDINGLOOP_H_ */
