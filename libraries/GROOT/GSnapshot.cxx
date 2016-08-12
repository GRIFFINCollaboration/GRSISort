#include "GSnapshot.h"

#include "TCanvas.h"
#include "TDatime.h"
#include "TH2.h"
#include "TList.h"
#include "TSystem.h"
#include "TEnv.h"

GSnapshot& GSnapshot::Get() {
  static GSnapshot snapshot;
  return snapshot;
}

GSnapshot::GSnapshot(const char* snapshot_dir) {
  if(snapshot_dir) {
    fSnapshotDir = snapshot_dir;
  } else if(gEnv->Defined("GRUT.SnapshotDir")) {
    fSnapshotDir = gEnv->GetValue("GRUT.SnapshotDir","");
  } else {
    fSnapshotDir = Form("%s/snapshot",getenv("GRUTSYS"));
  }

  // True from AccessPathName() means file does not exist.
  bool dir_exists = !gSystem->AccessPathName(fSnapshotDir.c_str());

  if(dir_exists) {
    long id, size, flags, modtime;
    gSystem->GetPathInfo(fSnapshotDir.c_str(), &id, &size, &flags, &modtime);
    bool output_dir_is_dir = flags & 2;
    fCanWriteHere = output_dir_is_dir;
  } else {
    gSystem->mkdir(fSnapshotDir.c_str(), true);
    fCanWriteHere = true;
  }
}

void GSnapshot::Snapshot(TCanvas *can) {
  if(!can && gPad)
    can = gPad->GetCanvas();
  if(!fCanWriteHere || !can) {
    return;
  }

  //bool needs_png = false;
  //for(auto obj : *can->GetListOfPrimitives()) {
  //  if(obj->InheritsFrom(TH2::Class())) {
  //    needs_png = true;
  //    break;
  //  }
  //}
  //const char* ext = needs_png ? "png" : "pdf";

  std::string ext = gEnv->GetValue("GRUT.SnapshotExt","");
  if(ext.length()==0) {
    ext = "pdf";         //TODO, make this smarter again...
  }

  int date,time;
  TDatime::GetDateTime(TDatime().Get(), date, time);

  can->SaveAs(Form("%s/%s_%d_%d.%s",
		   fSnapshotDir.c_str(), can->GetName(),
		   date, time, ext.c_str()));
}
