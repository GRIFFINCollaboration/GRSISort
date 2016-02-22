#ifndef TGRSIROOTIO_H
#define TGRSIROOTIO_H

/** \addtogroup Sorting
 *  @{
 */

#include <cstdio>

#include "TTree.h"
#include "TChain.h"
#include "TFile.h"
#include "TProofLite.h"

#include "TFragmentSelector.h"
#include "TOldFragment.h"
#include "TNewFragment.h"

#include "Globals.h"
#include "TChannel.h"
#include "TNucleus.h"
#include "TKinematics.h"
#include "TEpicsFrag.h"
#include "TPPG.h"
#include "TScaler.h"
#include "TDiagnostics.h"

class TGRSIRootIO : public TObject {
   public:
      static TGRSIRootIO* Get();
      virtual ~TGRSIRootIO();
   
   private:
      static TGRSIRootIO* fTGRSIRootIO;
      TGRSIRootIO();

   private:
      //TTree* fTChannelTree;
      TTree* fFragmentTree;
      TTree* fBadFragmentTree;
      TTree* fEpicsTree;
      TTree* fDeadtimeScalerTree;
      TTree* fRateScalerTree;
      TPPG* fPPG;
      TDiagnostics* fDiagnostics;

      TFile* fOutFile;
      int fTimesFillCalled;
      int fTimesBadFillCalled;
      int fTimesPPGCalled;
      int fTimesDeadtimeScalerCalled;
      int fTimesRateScalerCalled;
      int fEPICSTimesFillCalled;

      std::vector<TFile*> fInFiles;

      TFragment*  fBufferFrag;
      TFragment*  fBadBufferFrag;
      TEpicsFrag* fEXBufferFrag;
      TChannel*   fBufferChannel;

		TScalerData* fDeadtimeScalerData;
		TScalerData* fRateScalerData;

		static bool fOldFragment;
		static bool fDescant;
		Int_t fDescantData[3];

   public:
      bool SetUpRootOutFile(int,int);
      void CloseRootOutFile(); 
      int GetRunNumber(std::string);
      int GetSubRunNumber(std::string);

      void LoadRootFile(TFile*);
      
      TFile* GetRootOutFile()  { return fOutFile;   }  

      void SetUpFragmentTree();
      TTree* GetFragmentTree()  { return fFragmentTree;  }
      void FillFragmentTree(TFragment*);
      void FinalizeFragmentTree();

      void SetUpBadFragmentTree();
      TTree* GetBadFragmentTree()  { return fBadFragmentTree;  }
      void FillBadFragmentTree(TFragment*);
      void FinalizeBadFragmentTree();

      void SetUpPPG();
      TPPG* GetPPG()  { return fPPG;  }
      void FillPPG(TPPGData*);
      void FinalizePPG();
      int GetTimesPPGCalled()  { return fTimesPPGCalled;  }

      void SetUpScalerTrees();
      TTree* GetDeadtimeScalerTree()  { return fDeadtimeScalerTree;  }
      TTree* GetRateScalerTree()  { return fRateScalerTree;  }
      void FillDeadtimeScalerTree(TScalerData*);
      void FillRateScalerTree(TScalerData*);
      void FinalizeScalerTrees();
      int GetTimesDeadtimeScalerCalled()  { return fTimesDeadtimeScalerCalled;  }
      int GetTimesRateScalerCalled()  { return fTimesRateScalerCalled;  }

      void SetUpEpicsTree();
      TTree* GetEpicsTree()  { return fEpicsTree;  }
      void FillEpicsTree(TEpicsFrag*);
      void FinalizeEpicsTree();

      void SetUpDiagnostics();
      TDiagnostics* GetDiagnostics();
      void FinalizeDiagnostics();

      void MakeUserHistsFromFragmentTree();

		void SetOldFragment(bool value) { fOldFragment = value; }
		bool GetOldFragment() { return fOldFragment; }

		void SetDescant(bool value) { fDescant = value; }
		bool GetDescant() { return fDescant; }

/// \cond CLASSIMP
   ClassDef(TGRSIRootIO,0)
/// \endcond
};
/*! @} */
#endif 
