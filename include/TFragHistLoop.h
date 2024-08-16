#ifndef TFRAGHISTOGRAMLOOP_H
#define TFRAGHISTOGRAMLOOP_H

/** \addtogroup Loops
 *  @{
 */

////////////////////////////////////////////////////////////////////////////////
///
/// \class TFragHistLoop
///
/// This loop takes fragments and fills histograms based on
/// 'histos/MakeFragmentHistograms.cxx'
///
////////////////////////////////////////////////////////////////////////////////

#include <string>

#include "StoppableThread.h"
#include "TCompiledHistograms.h"
#include "ThreadsafeQueue.h"

class TFile;

class TFragHistLoop : public StoppableThread {
public:
   static TFragHistLoop* Get(std::string name = "");

	TFragHistLoop(const TFragHistLoop&) = delete;
	TFragHistLoop(TFragHistLoop&&) noexcept = delete;
	TFragHistLoop& operator=(const TFragHistLoop&) = delete;
	TFragHistLoop& operator=(TFragHistLoop&&) noexcept = delete;
   ~TFragHistLoop();

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>>& InputQueue()
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
   explicit TFragHistLoop(std::string name);

   TCompiledHistograms fCompiledHistograms;

   void OpenFile();
   void CloseFile();

   TFile*      fOutputFile;
   std::string fOutputFilename;

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>> fInputQueue;
#endif

   /// \cond CLASSIMP
   ClassDefOverride(TFragHistLoop, 0) // NOLINT
   /// \endcond
};

/*! @} */
#endif /* _THISTOGRAMLOOP_H_ */
