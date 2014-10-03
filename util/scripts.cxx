// put the includes here, even though this probably won't quite work properly when compiled
#include <vector>

#include <TFile.h>
#include <TGraph.h>
#include <TH1I.h>
#include <TString.h>
#include <TTree.h>

// almost any quick analysis will end up using the tree and a fragment
TTree *tree = NULL;
TFragment *frag = new TFragment;

// construct a TGraph from an arbitrary STL vector, useful for ROOT plotting
template< typename T >
TGraph* makeGraph(const vector<T> &v, const char *name = "vectorGraph")
{
  TObject *obj = gROOT->FindObjectAny(name);
  if (obj) {
    obj->Delete();
  }
  TGraph *graph = new TGraph(v.size());
  graph->SetNameTitle(name, name);

  for (Int_t i=0; i < v.size(); i++) {
    graph->SetPoint(i, i, v.at(i));
  }

  return graph;
}

// construct a TH1 from an arbitrary STL vector, useful for ROOT plotting
template<typename T, typename U>
U* makeHistoCommon(const vector<T> &v, const char *hisName = "vectorHis")
{
  TObject *obj = gROOT->FindObjectAny(hisName);
  if (obj) {
    obj->Delete();
  }
  U *his = new U(hisName, hisName, v.size(), 0, v.size());
  
  for (Int_t i=0; i < v.size(); i++) {
    his->SetBinContent(i+1, v.at(i));
  }

  return his;
}

// specific function for integer vectors
TH1* makeHisto(const vector<int> &v, const char *hisName = "vectorHis")
{
  return makeHistoCommon<int,TH1I>(v, hisName);
}

// specific function for double vectors
TH1* makeHisto(const vector<double> &v, const char *hisName = "vectorHis")
{
  return makeHistoCommon<int,TH1D>(v, hisName);
}

// specific function for short vectors
TH1* makeHisto(const vector<short> &v, const char *hisName = "vectorHis")
{
  return makeHistoCommon<short,TH1S>(v, hisName);
}

// draw the next waveform that we find associated with an event
void DrawNext(void)
{
  static Int_t evno=0;

  if (tree == NULL) {
    tree = (TTree*)gROOT->FindObject("FragmentTree");
  }
  tree->SetBranchAddress("TFragment", &frag);
  
  do {
    tree->GetEntry(evno++);
  } while (frag->wavebuffer.empty());
  
  cout << "Event number " << evno << endl;
  frag->Print();

  //printf("wavebuffer.size() = %i\n",wavebuffer.size()); 
  TH1 *his = makeHisto(frag->wavebuffer);
  his->Draw();
}

// plot two TTree Draw commands from different files f1, f2 on the same canvas
//   two different selections can be used (selection1, selection2)
//   hisrange like "(4000,0,4000)" can give arbitrary binning
void CompareDraw(TFile *f1, TFile *f2, const char *varexp, const char *hisrange, const char *selection, const char *selection2)
{
  TString expr;

  tree = (TTree*)f1->FindObjectAny("FragmentTree");
  expr = TString(varexp) + ">>compdrw1" + TString(hisrange);
  tree->Draw(expr, selection, "goff");
  compdrw1->SetLineColor(1);

  tree = (TTree*)f2->FindObjectAny("FragmentTree");
  expr = TString(varexp) + ">>compdrw2" + TString(hisrange);
  tree->Draw(expr, selection2, "goff");
  compdrw2->SetLineColor(2);

  if (compdrw1->GetMaximum() > compdrw2->GetMaximum()) {
    compdrw1->Draw();
    compdrw2->Draw("SAME");
  } else {
    compdrw2->Draw();
    compdrw1->Draw("SAME");
  }
}

// compare one selection from two different files
void CompareDraw(TFile *f1, TFile *f2, const char *varexp, const char *hisrange, const char *selection)
{
  CompareDraw(f1,f2,varexp,hisrange,selection,selection);
}

// compare two selections from one file
void CompareDraw(TFile *f1, const char *varexp, const char *hisrange, const char *selection, const char *selection2)
{
  CompareDraw(f1,f1,varexp,hisrange,selection,selection2);
}

void scripts()
{
  // empty function in case someone does a ".x"
}

// do th linking for the template functions if we are in an interactive session
#ifdef __CINT__
#pragma link C++ function makeGraph<int>(const vector<int> &, const char *)
#pragma link C++ function makeGraph<double>(const vector<double> &, const char *)
#pragma link C++ function makeHistoCommon<int, TH1I>(const vector<int> &, const char *)
#pragma link C++ function makeHistoCommon<double, TH1D>(const vector<double> &, const char *)
#pragma link C++ function makeHistoCommon<short, TH1S>(const vector<short> &, const char *)
#endif
