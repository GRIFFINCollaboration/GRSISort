#ifndef TGRSIROOTIO_H
#define TGRSIROOTIO_H

#include <cstdio>

#include "TTree.h"
#include "TChain.h"
#include "TFile.h"
#include "TProofLite.h"

#include "TFragmentSelector.h"

#include "Globals.h"
#include "TChannel.h"
#include "TNucleus.h"
#include "TKinematics.h"
#include "TEpicsFrag.h"

class TGRSIRootIO : public TObject {

   public:
      static TGRSIRootIO *Get();
      ~TGRSIRootIO();
   
   private:
      static TGRSIRootIO *fTGRSIRootIO;
      TGRSIRootIO();

   private:
      //TTree *fTChannelTree;
      TTree *fFragmentTree;
      TTree *fBadFragmentTree;
      TTree *fEpicsTree;
      TFile *foutfile;
      int fTimesFillCalled;
      int fTimesBadFillCalled;
      int fEPICSTimesFillCalled;

      std::vector<TFile*> finfiles;

      TFragment  *fBufferFrag;
      TFragment  *fBadBufferFrag;
      TEpicsFrag *fEXBufferFrag;
      TChannel   *fBufferChannel;

   public:
      void SetUpRootOutFile(int,int);
      void CloseRootOutFile(); 
      int GetRunNumber(std::string);
      int GetSubRunNumber(std::string);

      void LoadRootFile(TFile*);
      

      TFile *GetRootOutFile()  { return foutfile;   }  

//      void SetUpChannelTree();
//      TTree *GetChannelTree()  { return fTChannelTree;  }
//      void FillChannelTree(TChannel*);
//      void FinalizeChannelTree();

      void SetUpFragmentTree();
      TTree *GetFragmentTree()  { return fFragmentTree;  }
      void FillFragmentTree(TFragment*);
      void FinalizeFragmentTree();

      void SetUpBadFragmentTree();
      TTree *GetBadFragmentTree()  { return fBadFragmentTree;  }
      void FillBadFragmentTree(TFragment*);
      void FinalizeBadFragmentTree();

      void SetUpEpicsTree();
      TTree *GetEpicsTree()  { return fEpicsTree;  }
      void FillEpicsTree(TEpicsFrag*);
      void FinalizeEpicsTree();

      void MakeUserHistsFromFragmentTree();
      void WriteRunStats();
		//void StartMakeAnalysisTree();
		//void MakeAnalysisTree(std::vector<std::string>);//int argc, char **argv);

   ClassDef(TGRSIRootIO,0)

};

#endif 
