#ifndef TPARALLELFILEMERGER_H
#define TPARALLELFILEMERGER_H

#include <iostream>

#include "TSystem.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TKey.h"
#include "TTimeStamp.h"
#include "TFileMerger.h"
#include "TBits.h"
#include "TClass.h"
#include "TFileCacheWrite.h"
#include "TMath.h"

bool NeedInitialMerge(TDirectory* dir);
void DeleteObject(TDirectory* dir, bool withReset);
void MigrateKey(TDirectory* destination, TDirectory* source);

struct ClientInfo
{
   TFile*     fFile;      // This object does *not* own the file, it will be owned by the owner of the ClientInfo.
   TString    fLocalName;
   UInt_t     fContactsCount;
   TTimeStamp fLastContact;
   Double_t   fTimeSincePrevContact;

   ClientInfo() : fFile(nullptr), fLocalName(), fContactsCount(0), fTimeSincePrevContact(0) {}
   ClientInfo(const char *filename, UInt_t clientId) : fFile(nullptr), fContactsCount(0), fTimeSincePrevContact(0) {
      fLocalName.Form("%s-%d-%d",filename, clientId, gSystem->GetPid());
   }

   void Set(TFile* file)
   {
      // Register the new file as coming from this client.
      if(file != fFile) {
         // We need to keep any of the keys from the previous file that
         // are not in the new file.
         if(fFile != nullptr) {
				//std::cout<<"Migrating from "<<fFile->GetName()<<" ("<<fFile<<")"<<" to "<<file->GetName()<<" ("<<file<<")"<<std::endl;
            MigrateKey(fFile, file);
            // delete the previous memory file (if any)
            delete file;
         } else {
            fFile = file;
         }
      }
      TTimeStamp now;
      fTimeSincePrevContact = now.AsDouble() - fLastContact.AsDouble();
      fLastContact = now;
      ++fContactsCount;
   }
};

class TParallelFileMerger : public TObject
{
private:
   typedef std::vector<ClientInfo> ClientColl_t;

   TString       fFilename;
   TBits         fClientsContact;       //
   UInt_t        fNClientsContact;      //
   ClientColl_t  fClients;
   TTimeStamp    fLastMerge;
   TFileMerger   fMerger;

public:
   TParallelFileMerger(const char *filename, bool writeCache = false);

   ~TParallelFileMerger();

   ULong_t Hash() const;

   const char* GetName() const;

   bool InitialMerge(TFile *input);

   bool Merge();

   bool NeedFinalMerge();

   bool NeedMerge(Float_t clientThreshold);

   void RegisterClient(UInt_t clientId, TFile* file);

   //ClassDef(TParallelFileMerger, 0);
};

#endif
