
#include "GRootGlobals.h"

#include <cstdio>
#include <string>

#include <TRint.h>
#include <Getline.h>
#include <TAxis.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TPolyMarker.h>
#include <TSpectrum.h>
#include <TText.h>
#include <TExec.h>
#include <TKey.h>
#include <TObject.h>
#include <TClass.h>

#include <GRootObjectManager.h>

TH1D *ProjectionX(TH2* mat,int lowbin,int highbin) {
  if(!mat)
     return 0;
  std::string hname = mat->GetName();
  if((lowbin==0)&&highbin==-1) {
     hname.append("X_total");
  } else if((lowbin!=0)&&(highbin==-1)) {
     hname.append(Form("X_0to%.0f",mat->GetXaxis()->GetXmax()));
  } else {
     if(lowbin>highbin) {
       printf("ProjectX: low value greater than highvalue\n");
       return 0;
     }
     hname.append(Form("X_%.0fto%.0f",mat->GetXaxis()->GetBinLowEdge(lowbin),
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
     hname.append(Form("Y_0to%.0f",mat->GetYaxis()->GetXmax()));
  } else {
     if(lowbin>highbin) {
       printf("ProjectX: low value greater than highvalue\n");
       return 0;
     }
     hname.append(Form("Y_%.0fto%.0f",mat->GetYaxis()->GetBinLowEdge(lowbin),
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

int PeakSearch(TH1* hst, double sigma, double thresh,Option_t *opt)  {
	//TSpectrum spec(20);
	//spec.Search(hst,sigma,"Qnodraw",thresh);
   TSpectrum::StaticSearch(hst,sigma,"Qnodraw",thresh);
	TPolyMarker *pm = (TPolyMarker*)hst->GetListOfFunctions()->FindObject("TPolyMarker");
	if (!pm)
	{
		printf("No TPolyMarker object in the list of functions for histogram %s\n",hst->GetName());
		return 0;
	}
	TObjArray* testarray = (TObjArray*)hst->GetListOfFunctions()->FindObject("PeakLabels");
	if (testarray)
	{
		hst->GetListOfFunctions()->Remove(testarray);
		//delete testarray;
      testarray->Delete();  //guarantees the text objects it points to are removed as well.
	}

   TObjArray* array = new TObjArray();
	array->SetName("PeakLabels");
	int n = pm->GetN();
   if(n<1)
      return 0;
	TText* text;
	double *x = pm->GetX();
	double *y = pm->GetY();
	for (int i=0;i<n;i++)
	{
		text = new TText(x[i],y[i],Form("%.1f",x[i]));
		text->SetTextSize(0.025);
		text->SetTextAngle(90);
		text->SetTextAlign(12);
		text->SetTextFont(42);
		text->SetTextColor(hst->GetLineColor());
		array->Add(text);
	}
	hst->GetListOfFunctions()->Remove(pm);
	//delete pm;
   pm->Delete();
	hst->GetListOfFunctions()->Add(array);
	return n;
}

bool ShowPeaks(TH1 **hists,unsigned int NHists)
{
   //printf("Show peaks called,  0x%08x, Nhist = %i.",hists,NHists);
	//TList* list = gFile->GetListOfKeys();  //things can go out of scope preventing this from working.
	//TIter iter(list);
	double sigma  = 2.0;
	double thresh = 0.01;
   int num_found =0;
   for(unsigned int x=0;x<NHists;x++) {
     if(TObject *obj = hists[x]->GetListOfFunctions()->FindObject("PeakLabels")) {
        //if we have any array of peak labels, we remove it; we have no 
        //idea whether the user has change the range of the histogram so we 
        //research/re-display.
        hists[x]->GetListOfFunctions()->Remove(obj); 
        ((TObjArray*)obj)->Delete();
     }
     num_found += PeakSearch(hists[x],sigma,thresh,"");  // this find and adds the peaks...
                                                            // with the peaks added to the list of functions,
                                                            // returning true to update the pad will display them.
   }
   if(num_found) {
      //printf("Show peaks, found %i, returning true.",num_found);
      return true;
   }   
   else { 
      //printf("Show peaks, found %i, returning false.",num_found);
      return false;
   }
   /*
   while(obj=iter())
	{
		TKey* key = (TKey*) obj;
		const char* buffer= key->GetClassName();
		if (strncmp(buffer,"TH1",3) == 0)
		{
			TH1* hst = (TH1*) gFile->Get(obj->GetName());
			TExec* testexec = (TExec*)hst->GetListOfFunctions()->FindObject("PeakSearch");
			if (testexec)
			{
				return false;
			}
			else
			{
				const char* name = hst->GetName();
				TExec* peaksearch = new TExec("PeakSearch",Form("PeakSearch(%s,%f)",name,thresh));
				hst->GetListOfFunctions()->Add(peaksearch);
			}
		}
	}
	return true;
   */
}

bool RemovePeaks(TH1 **hists, unsigned int Nhists)
{
   bool return_flag = false;
   for(unsigned int x=0;x<Nhists;x++) {
     if(TObject *obj = hists[x]->GetListOfFunctions()->FindObject("PeakLabels")) {
        //if we have any array of peak labels, we remove it; 
        return_flag = true;
        hists[x]->GetListOfFunctions()->Remove(obj); 
        ((TObjArray*)obj)->Delete();
     }
   }
   return return_flag;
   /*
	TList* list = gFile->GetListOfKeys();
	TIter iter(list);
	TObject *obj;
	while(obj=iter())
	{
		TKey* key = (TKey*) obj;
		const char* buffer= key->GetClassName();
		if (strncmp(buffer,"TH1",3) == 0)
		{
			TH1* hst = (TH1*) gFile->Get(obj->GetName());
			TObjArray* testarray = (TObjArray*)hst->GetListOfFunctions()->FindObject("PeakLabels");
			if (testarray)
			{
				hst->GetListOfFunctions()->Remove(testarray);
				delete testarray;
			}

			TExec* testexec = (TExec*)hst->GetListOfFunctions()->FindObject("PeakSearch");
			if (testexec)
			{
				hst->GetListOfFunctions()->Remove(testexec);
				delete testexec;
			}
		}
	}
	return true;
   */
}

TH1 *GrabHist() {
 //return the histogram from the current canvas
 TH1 *hist=0;
 if(!gPad)
    return hist;
 TIter iter(gPad->GetListOfPrimitives());
 while(TObject *obj = iter.Next()) {
   if(obj->InheritsFrom(TH1::Class())) {
     hist = (TH1*)obj;
     break;
   }
 }
 return hist;
}



void Prompt()   { Getlinem(EGetLineMode::kInit,((TRint*)gApplication)->GetPrompt()); }

void Help()     { printf("This is helpful information.\n"); }
void Commands() { printf("this is a list of useful commands.\n");}

