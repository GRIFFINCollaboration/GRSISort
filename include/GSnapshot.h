#ifndef GSNAPSHOT_H_
#define GSNAPSHOT_H_

#include <string>

#include "TObject.h"

class TCanvas;

class GSnapshot {
public:
   static GSnapshot& Get();

   GSnapshot(const char* snapshot_dir = nullptr);
   ~GSnapshot() = default;

   void Snapshot(TCanvas* can = nullptr);

private:
   std::string fSnapshotDir;
   bool        fCanWriteHere;
};

extern GSnapshot gSnapshot;

#endif
