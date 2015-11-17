#ifndef GROOTOBJECTMANAGER__H
#define GROOTOBJECTMANAGER__H

/** \addtogroup GROOT
 *  @{
 */

#include <string>
#include <map>

#include "TObject.h"
#include "TList.h"
#include "TVirtualPad.h"

#include "TDirectoryFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TGraph.h"

#include "GCanvas.h"

class GPadObj : public TNamed {
  public:
	GPadObj(TObject*obj,Int_t pnum=0,Option_t *opt="");
	virtual ~GPadObj();

  private:
   //PadObj(); 
	TObject*   fObject;
	TObject*   fParent;
	int        fPadNumber;
	GCanvas*   fCanvas;
	//std::vector<std::string> fSource;
	std::string  fOption;
	
/// \cond CLASSIMP
   ClassDef(GPadObj,0)
/// \endcond
};

class GMemObj : public TNamed {
  public:
   GMemObj(TObject *obj, TObject *par=0,TFile *file=0,Option_t *opt="");
   GMemObj(const GMemObj &obj) : TNamed() { ((GMemObj&)obj).Copy(*this); }
   virtual ~GMemObj();

   void Copy(TObject &object) const;

   TObject *GetObject() { return fThis;   }
   TObject *GetParent() { return fParent; }
   TFile   *GetFile()   { return fFile;   }
   void     SetParent(TObject *parent) { fParent = parent; }
   void     SetFile(TFile *file)       { fFile   = file;   }

   const char *GetObjOption() { return fOption.c_str(); }
   const char *GetObjName()   { return fObjName.c_str(); }

  private:
   TObject *fThis;
   TObject *fParent;
   TFile   *fFile;
   std::string fOption;
   std::string fObjName;

/// \cond CLASSIMP
  ClassDef(GMemObj,0)  
/// \endcond
};

class GRootObjectManager {
  public:
	static GRootObjectManager *Instance();
	virtual ~GRootObjectManager();
	
	static void AddCanvas(GCanvas*);
	static void AddCanvas(TCanvas*);
	static void RemoveCanvas(GCanvas*);
	static void RemoveCanvas(TCanvas* = 0);

	static void AddObject(TObject *obj,TObject *par=0,TFile *file=0,Option_t *opt="");
	static void RemoveObject(TObject*);
	//static void AddObject(const char *name,TObject *par=0,TFile *file=0,Option_t *opt="");
	static void RemoveObject(const char *name);
  
	static bool SetParent(TObject *object,TObject *parent);
	static bool SetFile(TObject *object,TFile *file);

	TH1    *GetNext1D(TObject *object =0);
	TH1    *GetLast1D(TObject *object =0);
	TH2    *GetNext2D(TObject *object =0);
	TH2    *GetLast2D(TObject *object =0);
	TGraph *GetNextGraph(TObject *object=0);
	TGraph *GetLastGraph(TObject *object=0);

	GMemObj *FindMemObject(TObject *object ) { return ((GMemObj*)fObjectsMap->FindObject(object)); }
	GMemObj *FindMemObject(const char *name) { return ((GMemObj*)fObjectsMap->FindObject(Form("%s_%s",name,"memobj"))); }


	static void Update(Option_t *opt = "MemClean");
	static TList *GetObjectsList() { return fObjectsMap; }
	void Print();


  private:
	GRootObjectManager();
	static GRootObjectManager *fGRootObjectManager;

	static std::map<TCanvas*,std::vector<GPadObj> > fCanvasMap;
	static TList *fCanvasList;

	//static std::map<std::string,GMemObj > fObjectsMap;
	static TList *fObjectsMap;

	//static std::map<TObject*,std::vector<GMemObj*> > f1DHistsMap;
	//static std::map<TObject*,std::vector<GMemObj*> > f2DHistsMap;
	//static std::map<TObject*,std::vector<GMemObj*> > f3DHistsMap;
	//static std::map<TObject*,std::vector<GMemObj*> > fGraphsMap;
	//static std::map<TObject*,std::vector<GMemObj*> > fCutsMap;
	//static std::map<TObject*,std::vector<GMemObj*> > fMiscMap;

	//void Cleanup();
	static void ExtractObjects(TCollection*);
	static void ExtractObjectsFromFile(TDirectoryFile *file);

/// \cond CLASSIMP
	ClassDef(GRootObjectManager,0)
/// \endcond
};
/*! @} */
#endif
