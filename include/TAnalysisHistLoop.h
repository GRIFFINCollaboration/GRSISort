#ifndef _TANALYSISHISTOGRAMLOOP_H_
#define _TANALYSISHISTOGRAMLOOP_H_

#include <string>

#include "StoppableThread.h"
#include "TCompiledHistograms.h"
#include "ThreadsafeQueue.h"
#include "TUnpackedEvent.h"

class TFile;

class TAnalysisHistLoop : public StoppableThread {
public:
  static TAnalysisHistLoop* Get(std::string name="");

  ~TAnalysisHistLoop();

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TUnpackedEvent*> >& InputQueue() { return input_queue; }
  std::shared_ptr<ThreadsafeQueue<TUnpackedEvent*> >& OutputQueue() { return output_queue; }
#endif

  void SetOutputFilename(const std::string& name);
  std::string GetOutputFilename() const;

  void LoadLibrary(std::string library);
  std::string GetLibraryName() const;
  void ClearHistograms();

  void AddCutFile(TFile* cut_file);

  void Write();

  virtual void ClearQueue();

  TList* GetObjects();
  TList* GetGates();

  size_t GetItemsPopped()  { return output_queue->ItemsPopped(); }
  size_t GetItemsPushed()  { return output_queue->ItemsPushed(); }
  size_t GetItemsCurrent() { return output_queue->Size(); }
  size_t GetRate()         { return 0; }

protected:
  bool Iteration();

private:
  TAnalysisHistLoop(std::string name);

  TCompiledHistograms compiled_histograms;

  void OpenFile();
  void CloseFile();

  TFile* output_file;
  std::string output_filename;

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TUnpackedEvent*> > input_queue;
  std::shared_ptr<ThreadsafeQueue<TUnpackedEvent*> > output_queue;
#endif

  ClassDef(TAnalysisHistLoop,0);
};

#endif /* _THISTOGRAMLOOP_H_ */
