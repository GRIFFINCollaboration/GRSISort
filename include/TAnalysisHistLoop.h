#ifndef TANALYSISHISTOGRAMLOOP_H
#define TANALYSISHISTOGRAMLOOP_H

/** \addtogroup Loops
 *  @{
 */

////////////////////////////////////////////////////////////////////////////////
///
/// \class TAnalysisHistLoop
///
/// This loop takes built events and fills histograms using
/// 'histos/MakeAnalysisHistograms.cxx'
///
////////////////////////////////////////////////////////////////////////////////

#include <string>

#include "StoppableThread.h"
#include "TCompiledHistograms.h"
#include "ThreadsafeQueue.h"
#include "TUnpackedEvent.h"

class TFile;

class TAnalysisHistLoop : public StoppableThread {
public:
   static TAnalysisHistLoop* Get(std::string name = "");

   TAnalysisHistLoop(const TAnalysisHistLoop&)                = delete;
   TAnalysisHistLoop(TAnalysisHistLoop&&) noexcept            = delete;
   TAnalysisHistLoop& operator=(const TAnalysisHistLoop&)     = delete;
   TAnalysisHistLoop& operator=(TAnalysisHistLoop&&) noexcept = delete;
   ~TAnalysisHistLoop();

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent>>>& InputQueue()
   {
      return fInputQueue;
   }
#endif

   void        SetOutputFilename(const std::string& name);
   std::string GetOutputFilename() const;

   void        LoadLibrary(const std::string& library);
   std::string GetLibraryName() const;
   void        ClearHistograms();

   void AddCutFile(TFile* cut_file);

   void Write();

   void ClearQueue() override;

   TList* GetObjects();
   TList* GetGates();

   size_t GetItemsPopped() override { return 0; }
   size_t GetItemsPushed() override { return 0; }
   size_t GetItemsCurrent() override { return 0; }
   size_t GetRate() override { return 0; }

protected:
   bool Iteration() override;

private:
   explicit TAnalysisHistLoop(std::string name);

   TCompiledHistograms fCompiledHistograms;

   void OpenFile();
   void CloseFile();

   TFile*      fOutputFile;
   std::string fOutputFilename;

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent>>> fInputQueue;
#endif

   /// \cond CLASSIMP
   ClassDefOverride(TAnalysisHistLoop, 0)   // NOLINT(readability-else-after-return)
   /// \endcond
};

/*! @} */
#endif /* _THISTOGRAMLOOP_H_ */
