#ifndef TGRSIROOTIO_H
#define TGRSIROOTIO_H

#include <cstdio>

#include <TTree.h>
#include <TChain.h>
#include <TFile.h>
#include <TProofLite.h>

#include "TFragmentSelector.h"

#include "Globals.h"
#include "TChannel.h"


class TGRSIRootIO : public TObject {

   public:
      static TGRSIRootIO *Get();
      ~TGRSIRootIO();
   
   private:
      static TGRSIRootIO *fTGRSIRootIO;
      TGRSIRootIO();

   private:
      TTree *fTChannelTree;
      TTree *fFragmentTree;
      TFile *foutfile;
      int fTimesFillCalled;


      TFragment *fBufferFrag;
      TChannel *fBufferChannel;

   public:
      void SetUpRootOutFile(int,int);
      void CloseRootOutFile(); 
      int GetRunNumber(std::string);
      int GetSubRunNumber(std::string);


      TFile *GetRootOutFile()  { return foutfile;   }  

//      void SetUpChannelTree();
//      TTree *GetChannelTree()  { return fTChannelTree;  }
//      void FillChannelTree(TChannel*);
//      void FinalizeChannelTree();

      void SetUpFragmentTree();
      TTree *GetFragmentTree()  { return fFragmentTree;  }
      void FillFragmentTree(TFragment*);
      void FinalizeFragmentTree();


      void MakeUserHistsFromFragmentTree();


   ClassDef(TGRSIRootIO,0)

};

#endif 
