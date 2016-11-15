#ifndef GSNAPSHOT_H_
#define GSNAPSHOT_H_

#include <string>

#include "TObject.h"

class TCanvas;

class GSnapshot {
 public:
  static GSnapshot& Get();

  GSnapshot(const char* snapshot_dir = NULL);
  ~GSnapshot() { }

  void Snapshot(TCanvas *canvas=NULL);

 private:
  std::string fSnapshotDir;
  bool fCanWriteHere;
};

extern GSnapshot gSnapshot;

#endif
