// Author: Ryan Dunlop    10/16

/** \addtogroup Sorting
 *  @{
 */

#ifndef TGRSIPROOF_H
#define TGRSIPROOF_H

#include "Globals.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <ctime>
#include <fstream>
#include <streambuf>

#include "Rtypes.h"
#include "TProof.h"
#include "TSystemFile.h"
#include "TSystemDirectory.h"
#include "TString.h"
#include "TList.h"
#include "TROOT.h"
#include "TError.h"

#include <unistd.h>
const char* const nulFileName = "/dev/null";
#define CROSS_DUP(fd) dup(fd)
#define CROSS_DUP2(fd, newfd) dup2(fd, newfd)

/////////////////////////////////////////////////////////////////
///
/// \class TGRSIProof
///
/// This Class allows proof to work with GRSI
///
///
/////////////////////////////////////////////////////////////////

class TGRSIProof : public TProof {
public:
   TGRSIProof() : TProof() {}
   // TGRSIProof(const TObject& rhs) : TProof(rhs) {}
   ~TGRSIProof() override = default;

   static TGRSIProof* Open(const char* worker = "")
   {
      TGRSIProof* p = dynamic_cast<TGRSIProof*>(TProof::Open(worker));
      p->LoadLibsIntoProof();
      return p;
   }
   void LoadLibsIntoProof()
   {
      // This function Loops through all libraries in GRSISort, and brute-force loads them into proof
      // This is done because order of loading matters. We suppress the cout output because proof->Exec
      // barfs all of the output. To me there is no obvious way to load every library on each of the PROOF slaves

      const char* pPath = getenv("GRSISYS");

      // First set the include path on each slave
      Exec(Form(R"(gInterpreter->AddIncludePath("%s/include"))", pPath));
      std::cout << "Loading Libraries" << std::endl;
      Exec(Form(R"(gSystem->Load("%s/lib/libGRSI.so");)", pPath));
   }

   /// \cond CLASSIMP
   ClassDefOverride(TGRSIProof, 1); // Event Fragments
   /// \endcond
};
/*! @} */
#endif // TGRSIProof_H
