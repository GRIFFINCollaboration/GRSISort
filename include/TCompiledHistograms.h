#ifndef TCOMPILEDHISTOGRAMS_H
#define TCOMPILEDHISTOGRAMS_H

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
   TCompiledHistograms(std::string inputLib, std::string funcName);

   void Load(const std::string& libName, const std::string& funcName);
#ifndef __CINT__
   void Fill(std::shared_ptr<const TFragment> frag);
   void Fill(std::shared_ptr<TUnpackedEvent> detectors);
#endif
   void Reload();

   std::string GetLibraryName() const { return fLibName; }

   void        SetDefaultDirectory(TDirectory* dir);
   TDirectory* GetDefaultDirectory() { return fDefaultDirectory; }

   void ClearHistograms();

   TList* GetObjects() { return &fObjects; }
   TList* GetGates() { return &fGates; }

   void AddCutFile(TFile* cut_file);

   Int_t Write(const char* name = nullptr, Int_t option = 0, Int_t bufsize = 0) override;

private:
   void   swap_lib(TCompiledHistograms& other);
   time_t get_timestamp();
   bool   file_exists();

   std::string fLibName;
   std::string fFuncName;
#ifndef __CINT__
   std::shared_ptr<DynamicLibrary> fLibrary{nullptr};
   std::mutex                      fMutex;
#endif
   void (*fFunc)(TRuntimeObjects&);
   time_t fLastModified{0};
   time_t fLastChecked{0};

   int fCheckEvery{5};

   TList               fObjects;
   TList               fGates;
   std::vector<TFile*> fCutFiles;

   TDirectory* fDefaultDirectory{nullptr};

   TRuntimeObjects fObj;

	// \cond CLASSIMP
   ClassDefOverride(TCompiledHistograms, 0) // NOLINT
	// \endcond
};

#endif /* _TCOMPILEDHISTOGRAMS_H_ */
