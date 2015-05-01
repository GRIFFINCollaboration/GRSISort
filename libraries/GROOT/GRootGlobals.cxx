
#include "GRootGlobals.h"

#include <cstdio>
#include <string>

#include <TAxis.h>

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
  GRootObjectManager::AddObject(mat,0,0);
  GRootObjectManager::AddObject(temphist,mat,0);
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
  GRootObjectManager::AddObject(mat,0,0);
  GRootObjectManager::AddObject(temphist,mat,0);
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
  GRootObjectManager::AddObject(mat,0,0);
  GRootObjectManager::AddObject(temphist,mat,0);
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
  GRootObjectManager::AddObject(mat,0,0);
  GRootObjectManager::AddObject(temphist,mat,0);
  //Add to manager;
  return temphist;
}









void Help()     { printf("This is helpful information.\n"); }
void Commands() { printf("this is a list of useful commands.\n");}

