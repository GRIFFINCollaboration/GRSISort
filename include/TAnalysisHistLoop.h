#ifndef _TANALYSISHISTOGRAMLOOP_H_
#define _TANALYSISHISTOGRAMLOOP_H_

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

   ~TAnalysisHistLoop();

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent>>>& InputQueue() { return fInputQueue; }
#endif

   void SetOutputFilename(const std::string& name);
   std::string GetOutputFilename() const;

   void LoadLibrary(std::string library);
   std::string GetLibraryName() const;
   void        ClearHistograms();

   void AddCutFile(TFile* cut_file);

   void Write();

   virtual void ClearQueue();

   TList* GetObjects();
   TList* GetGates();

   size_t GetItemsPopped() { return 0; }
   size_t GetItemsPushed() { return 0; }
   size_t GetItemsCurrent() { return 0; }
   size_t GetRate() { return 0; }

protected:
   bool Iteration();

private:
   TAnalysisHistLoop(std::string name);

   TCompiledHistograms fCompiledHistograms;

   void OpenFile();
   void CloseFile();

   TFile*      fOutputFile;
   std::string fOutputFilename;

#ifndef __CINT__
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent>>> fInputQueue;
#endif

   ClassDef(TAnalysisHistLoop, 0);
};

/*! @} */
#endif /* _THISTOGRAMLOOP_H_ */
