
#include "GRootGlobals.h"

#include <cstdio>
#include <string>

#include <TAxis.h>
#include <TDirectory.h>
#include <TFile.h>

#include <GRootObjectManager.h>

TH1D *ProjectionX(TH2* mat,int lowbin,int highbin) {
  if(!mat)
     return 0;
  std::string hname = mat->GetName();
  if((lowbin==0)&&highbin==-1) {
     hname.append("X_total");
  } else if((lowbin!=0)&&(highbin==-1)) {
     hname.append(Form("X_0-%.0f",mat->GetXaxis()->GetXmax()));
  } else {
     if(lowbin>highbin) {
       printf("ProjectX: low value greater than highvalue\n");
       return 0;
     }
     hname.append(Form("X_%.0f-%.0f",mat->GetXaxis()->GetBinLowEdge(lowbin),
                                    mat->GetXaxis()->GetBinUpEdge(highbin)));
  }
  TH1D *temphist = mat->ProjectionX(hname.c_str(),lowbin,highbin);
  //Add to manager;
  temphist->SetNameTitle(hname.c_str(),hname.c_str());
  GRootObjectManager::AddObject(mat,0,0);
  GRootObjectManager::AddObject(temphist,mat,0,"ProjX");
  return temphist;
}

TH1D *ProjectionY(TH2* mat,int lowbin,int highbin) {
  if(!mat)
     return 0;
  std::string hname = mat->GetName();
  if((lowbin==0)&&highbin==-1) {
     hname.append("Y__total");
  } else if((lowbin!=0)&&(highbin==-1)) {
     hname.append(Form("Y_0-%.0f",mat->GetYaxis()->GetXmax()));
  } else {
     if(lowbin>highbin) {
       printf("ProjectX: low value greater than highvalue\n");
       return 0;
     }
     hname.append(Form("Y_%.0f-%.0f",mat->GetYaxis()->GetBinLowEdge(lowbin),
                                    mat->GetYaxis()->GetBinUpEdge(highbin)));
  }
  TH1D *temphist = mat->ProjectionY(hname.c_str(),lowbin,highbin);
  temphist->SetNameTitle(hname.c_str(),hname.c_str());
  GRootObjectManager::AddObject(mat,0,0);
  GRootObjectManager::AddObject(temphist,mat,0,"ProjY");
  //Add to manager;
  return temphist;
}


TH1D *ProjectionX(TH2* mat,double lowvalue,double highvalue) {
  if(!mat)
     return 0;
  std::string hname = mat->GetName();
  int lowbin, highbin;
  if((lowvalue<0.1)&&highvalue<0) {
     hname.append("X__total");
     lowbin =0; highbin = -1;
  } else if((lowvalue>0.1)&&(highvalue<-1)) {
     hname.append(Form("X_0-%.0f",highvalue));
     lowbin=0; highbin = mat->GetXaxis()->FindBin(highvalue);
  } else {
     if(lowvalue>highvalue) {
       printf("ProjectX: low value greater than highvalue\n");
       return 0;
     }
     hname.append(Form("X_%.0f-%.0f",lowvalue,highvalue));
     lowbin=mat->GetYaxis()->FindBin(lowvalue); highbin=mat->GetYaxis()->FindBin(highvalue);
  }
  TH1D *temphist = mat->ProjectionX(hname.c_str(),lowbin,highbin);
  temphist->SetNameTitle(hname.c_str(),hname.c_str());
  GRootObjectManager::AddObject(mat,0,0);
  GRootObjectManager::AddObject(temphist,mat,0,"ProjX");
  //Add to manager;
  return temphist;
}


TH1D *ProjectionY(TH2* mat,double lowvalue,double highvalue) {
  if(!mat)
     return 0;
  std::string hname = mat->GetName();
  int lowbin, highbin;
  if((lowvalue<0.1)&&highvalue<0) {
     hname.append("Y__total");
     lowbin =0; highbin = -1;
  } else if((lowvalue>0.1)&&(highvalue<-1)) {
     hname.append(Form("Y_0-%.0f",highvalue));
     lowbin=0; highbin = mat->GetYaxis()->FindBin(highvalue);
  } else {
     if(lowvalue>highvalue) {
       printf("ProjectY: low value greater than highvalue\n");
       return 0;
     }
     hname.append(Form("Y_%.0f-%.0f",lowvalue,highvalue));
     lowbin=mat->GetYaxis()->FindBin(lowvalue); highbin=mat->GetYaxis()->FindBin(highvalue);
  }
  TH1D *temphist = mat->ProjectionY(hname.c_str(),lowbin,highbin);
  temphist->SetNameTitle(hname.c_str(),hname.c_str());
  GRootObjectManager::AddObject(mat,0,0);
  GRootObjectManager::AddObject(temphist,mat,0,"ProjY");
  //Add to manager;
  return temphist;
}


void SaveAll(const char *fname,Option_t *opt) {
  TString sname(fname);
  if(!sname.Contains(".root")) {
    printf("Try SaveAll(filename.root) instead;  Warning: if filename.root already exists, it will be replaced!\n");
    return;
  }
  TDirectory *cur_dir = gDirectory;
  TFile f(sname.Data(),opt); 

  TList *list = GRootObjectManager::GetObjectsList();
  list->Sort();
  TIter iter(list);
  while(TObject  *obj = iter.Next()) {
    if(!obj->InheritsFrom("GMemObj"))
       continue;
    GMemObj *mobj = (GMemObj*)obj;
    if(mobj->GetObject())
       mobj->GetObject()->Write();
  }
  f.Close();
  cur_dir->cd();
  return;
}



void Help()     { printf("This is helpful information.\n"); }
void Commands() { printf("this is a list of useful commands.\n");}

