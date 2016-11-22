#ifndef _TCOMPILEDHISTOGRAMS_H_
#define _TCOMPILEDHISTOGRAMS_H_

#ifndef __CINT__
#include <mutex>
#include <memory>
#endif
#include <string>

#include "TObject.h"
#include "TList.h"

#include "DynamicLibrary.h"
#include "TRuntimeObjects.h"

#include "TFragment.h"
#include "TUnpackedEvent.h"

class TFile;

class TCompiledHistograms : public TObject {
public:
  TCompiledHistograms();
  TCompiledHistograms(std::string libname, std::string func_name);

  void Load(std::string libname, std::string func_name);
#ifndef __CINT__
  void Fill(std::shared_ptr<const TFragment> fragment);
  void Fill(std::shared_ptr<TUnpackedEvent> unpacked);
#endif
  void Reload();

  std::string GetLibraryName() const { return fLibname; }

  void SetDefaultDirectory(TDirectory* dir);
  TDirectory* GetDefaultDirectory() { return fDefault_directory; }

  void ClearHistograms();

  TList* GetObjects()   { return &fObjects; }
  TList* GetGates()     { return &fGates;   }

  void AddCutFile(TFile* cut_file);

  Int_t Write(const char* name=0, Int_t option=0, Int_t bufsize=0);


private:
  void swap_lib(TCompiledHistograms& other);
  time_t get_timestamp();
  bool file_exists();

  std::string fLibname;
  std::string fFunc_name;
#ifndef __CINT__
  std::shared_ptr<DynamicLibrary> fLibrary;
  std::mutex fMutex;
#endif
  void (*fFunc)(TRuntimeObjects&);
  time_t fLast_modified;
  time_t fLast_checked;

  int fCheck_every;

  TList fObjects;
  TList fGates;
  std::vector<TFile*> fCut_files;

  TDirectory* fDefault_directory;

  TRuntimeObjects fObj;

  ClassDef(TCompiledHistograms, 0);
};

#endif /* _TCOMPILEDHISTOGRAMS_H_ */
