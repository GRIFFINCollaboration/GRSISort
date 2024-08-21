#ifndef GSNAPSHOT_H
#define GSNAPSHOT_H

#include <string>

#include "TObject.h"

class TCanvas;

class GSnapshot {
public:
   static GSnapshot& Get();

   explicit GSnapshot(const char* snapshot_dir = nullptr);
   GSnapshot(const GSnapshot&)                = default;
   GSnapshot(GSnapshot&&) noexcept            = default;
   GSnapshot& operator=(const GSnapshot&)     = default;
   GSnapshot& operator=(GSnapshot&&) noexcept = default;
   ~GSnapshot()                               = default;

   void Snapshot(TCanvas* can = nullptr);

private:
   std::string fSnapshotDir;
   bool        fCanWriteHere;
};

extern GSnapshot gSnapshot;

#endif
