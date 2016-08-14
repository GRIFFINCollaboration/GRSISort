#ifndef _TCOMPILEDHISTOGRAMS_H_
#define _TCOMPILEDHISTOGRAMS_H_

#ifndef __CINT__
#include <mutex>
#endif
#include <memory>
#include <string>

#include "TObject.h"
#include "TList.h"

#include "DynamicLibrary.h"
#include "TRuntimeObjects.h"

#include "TFragment.h"

class TFile;

class TCompiledHistograms : public TObject {
public:
  TCompiledHistograms();
  TCompiledHistograms(std::string libname);

  void Load(std::string libname);
  void Fill(TFragment& fragment);
  void Reload();

  std::string GetLibraryName() const { return libname; }

  void SetDefaultDirectory(TDirectory* dir);
  TDirectory* GetDefaultDirectory() { return default_directory; }

  void ClearHistograms();

  TList* GetObjects()   { return &objects;   }
  TList* GetGates()     { return &gates;     }

  void AddCutFile(TFile* cut_file);

  void Write();


private:
  void swap_lib(TCompiledHistograms& other);
  time_t get_timestamp();
  bool file_exists();

  std::string libname;
#ifndef __CINT__
  std::shared_ptr<DynamicLibrary> library;
  std::mutex mutex;
#endif
  void (*func)(TRuntimeObjects&);
  time_t last_modified;
  time_t last_checked;

  int check_every;

  TList objects;
  TList gates;
  std::vector<TFile*> cut_files;

  TDirectory* default_directory;

  TRuntimeObjects obj;

  ClassDef(TCompiledHistograms, 0);
};

#endif /* _TCOMPILEDHISTOGRAMS_H_ */
