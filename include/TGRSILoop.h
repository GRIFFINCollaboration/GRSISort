#ifndef TGRSILOOP_H
#define TGRSILOOP_H

/** \addtogroup Sorting
 *  @{
 */

#include <cstdio>
#include <string>
#include <fstream>

#if !defined (__CINT__) && !defined (__CLING__)
#define _GLIBCXX_USE_NANOSLEEP 1
#include <thread>
#include <mutex>
#endif

#include "TObject.h"

#include "TGRSIint.h"
#include "TMidasFile.h"
#include "TMidasEvent.h"
#include "TXMLOdb.h"
#include "TChannel.h"
#include "Globals.h"

#include "TDataParser.h"

class TGRSILoop : public TObject {
  public:
    static TGRSILoop* Get();
    virtual ~TGRSILoop();

  private:
    static TGRSILoop* fTGRSILoop;
    static bool fSuppressError;
    TGRSILoop();

  public:
    void Initialize();
    void BeginRun(int transition, int runnumber, int time);
    void EndRun(int transition, int runnumber, int time);
    void Finalize();

  private:
    bool fTestMode;
    bool fOffline;   

    bool fIamTigress;
    bool fIamGriffin;

    TXMLOdb* fOdb;

    int fFragsReadFromMidas;
    int fFragsSentToTree;
    int fBadFragsSentToTree;
    int fDeadtimeScalersSentToTree;
    int fRateScalersSentToTree;

#if !defined (__CINT__) && !defined (__CLING__)
    std::thread* fMidasThread;
    std::thread* fFillTreeThread;
    std::thread* fFillScalerThread;

    static std::mutex fMutex;
#endif

  public:
    bool fMidasThreadRunning;
    bool fFillTreeThreadRunning;
    bool fFillScalerThreadRunning;

    void SetSuppressError(bool temp = true) { fSuppressError = temp; } 
    bool IsOnline()   { return !fOffline;  }
    bool IsOffline()  { return fOffline;   }

    bool SortMidas();
    void ProcessMidasFile(TMidasFile*);
    void FillFragmentTree(TMidasFile*);
    void FillScalerTree();
    bool ProcessMidasEvent(TMidasEvent*,TMidasFile* mFile=0);
    bool ProcessTIGRESS(uint32_t* ptr,int& dSize,TMidasEvent* mEvent=0,TMidasFile* mFile=0); 
    bool ProcessGRIFFIN(uint32_t* ptr,int& dSize,TDataParser::EBank,TMidasEvent* mEvent=0,TMidasFile* mFile=0); 
    bool Process8PI(uint32_t stream,uint32_t* ptr,int& dSize,TMidasEvent* mEvent=0,TMidasFile* mFile=0);
    bool ProcessEPICS(float* ptr,int& dSize,TMidasEvent* mEvent=0,TMidasFile* mFile=0);
    //bool ProcessEPICS(double* ptr,int& dSize,TMidasEvent* mEvent=0,TMidasFile* mFile=0);

    void SetFileOdb(char* data,int size);
    void SetTIGOdb();
    void SetGRIFFOdb();

    int GetFragsSentToTree()    { return fFragsSentToTree; }
    int GetFragsReadFromMidas() { return fFragsReadFromMidas; }

    static bool GetSuppressError() { return fSuppressError; } 

    void Clear(Option_t* opt="");
    void Print(Option_t* opt="") const;

    /// \cond CLASSIMP
    ClassDef(TGRSILoop,0)
      /// \endcond
};
/*! @} */
#endif
