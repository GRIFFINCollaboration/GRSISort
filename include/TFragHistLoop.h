#ifndef _TFRAGHISTOGRAMLOOP_H_
#define _TFRAGHISTOGRAMLOOP_H_

#include <string>

#include "StoppableThread.h"
#include "TCompiledHistograms.h"
#include "ThreadsafeQueue.h"

class TFile;

class TFragHistLoop : public StoppableThread {
public:
  static TFragHistLoop* Get(std::string name="");

  ~TFragHistLoop();

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TFragment*> >& InputQueue() { return input_queue; }
  std::shared_ptr<ThreadsafeQueue<TFragment*> >& OutputQueue() { return output_queue; }
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
  TFragHistLoop(std::string name);

  TCompiledHistograms compiled_histograms;

  void OpenFile();
  void CloseFile();

  TFile* output_file;
  std::string output_filename;

#ifndef __CINT__
  std::shared_ptr<ThreadsafeQueue<TFragment*> > input_queue;
  std::shared_ptr<ThreadsafeQueue<TFragment*> > output_queue;
#endif

  ClassDef(TFragHistLoop,0);
};

#endif /* _THISTOGRAMLOOP_H_ */
