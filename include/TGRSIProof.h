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
#include <time.h>
#include <fstream>
#include <streambuf>

#include "Rtypes.h"
#include "TProof.h"
#include "TSystemFile.h"
#include "TSystemDirectory.h"
#include "TString.h"
#include "TList.h"
#include "TROOT.h"

#include <unistd.h>
const char * const nulFileName = "/dev/null";
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

class TGRSIProof : public TProof	{
  public:
   TGRSIProof() : TProof() {} 
  // TGRSIProof(const TObject& rhs) : TProof(rhs) {} 
   virtual ~TGRSIProof(){}
   
   static TGRSIProof* Open(const char* worker = ""){
      TGRSIProof *p = static_cast<TGRSIProof*>(TProof::Open(worker));
      p->LoadLibsIntoProof();
      return p;
   }
   void LoadLibsIntoProof(){
      //This function Loops through all libraries in GRSISort, and brute-force loads them into proof
      //This is done because order of loading matters. We suppress the cout output because proof->Exec
      //barfs all of the output. To me there is no obvious way to load every library on each of the PROOF slaves
   
      const char* pPath = getenv("GRSISYS");

      //First set the include path on each slave
      this->Exec(Form("gInterpreter->AddIncludePath(\"%s/include\")",pPath));
      std::cout << "Loading Libraries" << std::endl;
      //This block builds a list of all of the files that end in .so in the lib directory
      TSystemDirectory dir(Form("%s/lib",pPath),Form("%s/lib",pPath));
      TList *files = dir.GetListOfFiles();
      std::vector<TString> files_copy;
      if(files) {
         TSystemFile *file;
         TString fname;
         TIter next(files);
         while((file=dynamic_cast<TSystemFile*>(next()))) {
            fname = file->GetName();
            if(!file->IsDirectory() && fname.EndsWith(".so")) {
               files_copy.push_back(fname);
            }
         }
      }

      //This block quietly loops through the libraries a total of nLibraries times to brute force load all dependencies
      //Now load all libraries brute force until it works.
   	std::stringstream buffer;
	   std::streambuf* std_out = std::cout.rdbuf(buffer.rdbuf());
      for(size_t i = 0; i<files_copy.size(); ++i){
         for(auto it = files_copy.begin(); it != files_copy.end(); ++it){
            this->Exec(Form("gSystem->Load(\"%s/lib/%s\");",pPath,it->Data()));
         }
      }
	   std::cout.rdbuf(std_out);
   }

   /// \cond CLASSIMP
   ClassDef(TGRSIProof,1);  // Event Fragments
   /// \endcond
};
/*! @} */
#endif // TGRSIProof_H
