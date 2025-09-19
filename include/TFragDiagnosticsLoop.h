#ifndef TFRAGDIAGNOSTICSLOOP_H
#define TFRAGDIAGNOSTICSLOOP_H

/** \addtogroup Loops
 *  @{
 */

////////////////////////////////////////////////////////////////////////////////
///
/// \class TFragDiagnosticsLoop
///
/// This loop writes fragments to a root-file.
///
////////////////////////////////////////////////////////////////////////////////

#include "TTree.h"

#include "StoppableThread.h"
#include "ThreadsafeQueue.h"
#include "TFragment.h"
#include "TBadFragment.h"
#include "TEpicsFrag.h"
#include "TH2.h"

class TFragDiagnosticsLoop : public StoppableThread {
public:
   static TFragDiagnosticsLoop* Get(std::string name = "", std::string fOutputFilename = "");

   TFragDiagnosticsLoop(const TFragDiagnosticsLoop&)                = delete;
   TFragDiagnosticsLoop(TFragDiagnosticsLoop&&) noexcept            = delete;
   TFragDiagnosticsLoop& operator=(const TFragDiagnosticsLoop&)     = delete;
   TFragDiagnosticsLoop& operator=(TFragDiagnosticsLoop&&) noexcept = delete;
   ~TFragDiagnosticsLoop();

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>>& InputQueue() { return fInputQueue; }
#endif

   void ClearQueue() override;

   void Write();

   // there is no output queue for this loop, so we assume that all items handled (= all good fragments written)
   // are also the number of items popped and that we have no current items
   size_t GetItemsPushed() override { return ItemsPopped(); }
   size_t GetItemsPopped() override { return ItemsPopped(); }
   size_t GetItemsCurrent() override { return 0; }
   size_t GetRate() override { return 0; }

   std::string EndStatus() override;

protected:
   bool Iteration() override;

private:
   TFragDiagnosticsLoop(std::string name, const std::string& fOutputFilename);
#ifndef __CINT__
   void Process(const std::shared_ptr<const TFragment>& event);
#endif
   bool CreateHistograms();

   TFile* fOutputFile{nullptr};

   TFragment*    fEventAddress{nullptr};
   TBadFragment* fBadEventAddress{nullptr};
   TEpicsFrag*   fScalerAddress{nullptr};

   //--------------- parameters for dealing with the roll-over of the AcceptedChannelId ----------------------//
   uint64_t                     fAcceptedMax{0x1 << 14};   // = 2^14 = this is the maximum number that the AcceptedChannelId can be
   std::map<unsigned int, int>  fNofRollovers;             // this is how many roll-overs we have had
   std::map<unsigned int, bool> fRolling;                  // array that tells us if we're rolling over in that channel
   std::map<unsigned int, int>  fRollnum;                  // array that tells us how many times we've had accepted ID over the threshold
   unsigned int                 fRollingThreshold{1000};
   int                          fRollnumThreshold{20};   // if we have this many numbers above the threshold, turn rolling on or off

   // these are all 2 to store the last and second to last instances of these variables (per channel for the first three)
   std::map<unsigned int, std::array<int64_t, 2>> fChannelIds;
   std::map<unsigned int, std::array<int64_t, 2>> fAcceptedChannelIds;
   std::map<unsigned int, std::array<int64_t, 2>> fTimeStamps;
   std::array<int, 2>                             fNetworkPacketNumber{{0, 0}};
   std::array<int64_t, 2>                         fNetworkPacketTimeStamp{{0, 0}};

   // time binning of histograms
   int fRunLength{0};

   // histograms
   TH2D* fAccepted{nullptr};
   TH1D* fLostNetworkPackets{nullptr};
   TH2D* fLostChannelIds{nullptr};
   TH2D* fLostAcceptedIds{nullptr};
   TH2D* fLostChannelIdsTime{nullptr};
   TH2D* fLostAcceptedIdsTime{nullptr};

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>> fInputQueue;
#endif

   /// \cond CLASSIMP
   ClassDefOverride(TFragDiagnosticsLoop, 0)   // NOLINT(readability-else-after-return)
   /// \endcond
};

/*! @} */
#endif /* _TFRAGDIAGNOSTICSLOOP_H_ */
