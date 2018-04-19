#include "TParallelFileMerger.h"

#include <iostream>

bool NeedInitialMerge(TDirectory* dir)
{
   if(dir == nullptr) return false;

   TIter nextkey(dir->GetListOfKeys());
   TKey* key;
   while((key = static_cast<TKey*>(nextkey())) != nullptr) {
      TClass* cl = TClass::GetClass(key->GetClassName());
      if(cl->InheritsFrom(TDirectory::Class())) {
         TDirectory* subDir = static_cast<TDirectory*>(dir->GetList()->FindObject(key->GetName()));
         if(subDir == nullptr) {
            subDir = static_cast<TDirectory*>(key->ReadObj());
         }
         if(NeedInitialMerge(subDir)) {
				//std::cout<<"need initial merge for subdir "<<subDir->GetName()<<std::endl;
            return true;
         }
      } else {
         if(cl->GetResetAfterMerge() != nullptr) {
				//std::cout<<"need initial merge for key "<<key->GetName()<<std::endl;
            return true;
         }
      }
   }

   return false;
}

void DeleteObject(TDirectory* dir, bool withReset)
{
   if(dir == nullptr) return;

   TIter nextkey(dir->GetListOfKeys());
   TKey* key; 
   while((key = static_cast<TKey*>(nextkey())) != nullptr) {
      TClass* cl = TClass::GetClass(key->GetClassName());
      if(cl->InheritsFrom(TDirectory::Class())) {
         TDirectory* subDir = static_cast<TDirectory*>(dir->GetList()->FindObject(key->GetName()));
         if(subDir == nullptr) {
            subDir = static_cast<TDirectory*>(key->ReadObj());
         }
         DeleteObject(subDir, withReset);
      } else {
         bool toDelete = false;
         if(withReset) {
            toDelete = (cl->GetResetAfterMerge() != nullptr);//if a reset-after-merge function exists, delete
         } else {
            toDelete = (cl->GetResetAfterMerge() == nullptr);//if a reset-after-merge function does not exits, delete ?
         }
         if(toDelete) {
				//std::cout<<"withReset "<<withReset<<", toDelete "<<toDelete<<", deleting key "<<key<<": "<<key->GetName()<<std::endl;
            //key->Delete();
            dir->GetListOfKeys()->Remove(key);
            delete key;
         }
      }
   }
}

void MigrateKey(TDirectory* destination, TDirectory* source)
{
   if(destination == nullptr || source == nullptr) return;
	//std::cout<<"MigrateKey from "<<destination->GetName()<<" to "<<source->GetName()<<std::endl;

   TIter nextkey(source->GetListOfKeys());
   TKey* key;
   while((key = static_cast<TKey*>(nextkey())) != nullptr) {
      TClass* cl = TClass::GetClass(key->GetClassName());
      if(cl->InheritsFrom(TDirectory::Class())) {
         TDirectory* sourceSubDir = static_cast<TDirectory*>(source->GetList()->FindObject(key->GetName()));
         if(sourceSubDir == nullptr) {
            sourceSubDir = static_cast<TDirectory*>(key->ReadObj());
         }
         TDirectory* destinationSubDir = static_cast<TDirectory*>(destination->GetList()->FindObject(key->GetName()));
         if(destinationSubDir == nullptr) {
            destinationSubDir = static_cast<TDirectory*>(key->ReadObj());
         }
			//std::cout<<"migrating sub directories"<<std::endl;
         MigrateKey(destination, source); //or just destination, source ???
      } else {
         TKey* oldKey = destination->GetKey(key->GetName());
         if(oldKey != nullptr) {
            oldKey->Delete();
            delete oldKey;
         }
			//std::cout<<"deleted old key, now creating new key "<<key->GetName()<<" in "<<destination->GetName()<<std::endl;
         TKey* newKey = new TKey(destination, *key, 0); // a priori the file are from the same client ...
         destination->GetFile()->SumBuffer(newKey->GetObjlen());
         newKey->WriteFile(0);
         if(destination->GetFile()->TestBit(TFile::kWriteError)) {
            return;
         }
      }
   }
   destination->SaveSelf();
}

TParallelFileMerger::TParallelFileMerger(const char* filename, bool writeCache) : fFilename(filename), fNClientsContact(0), fMerger(false, true)
{
	// Default constructor.
	fMerger.SetPrintLevel(0);
	fMerger.OutputFile(filename,"RECREATE");
	if(writeCache) new TFileCacheWrite(fMerger.GetOutputFile(), 32*1024*1024);
}

TParallelFileMerger::~TParallelFileMerger()
{
	// Destructor.
	for(auto iter = fClients.begin(); iter != fClients.end(); ++iter) {
		delete iter->fFile;
	}
}

ULong_t TParallelFileMerger::Hash() const
{
	// Return hash value for this object.
	return fFilename.Hash();
}

const char* TParallelFileMerger::GetName() const
{
	// Return the name of the object which is the name of the output file.
	return fFilename;
}

bool TParallelFileMerger::InitialMerge(TFile* input)
{
	// Initial merge of the input to copy the resetable object (TTree) into the output
	// and remove them from the input file.

	fMerger.AddFile(input);

	bool result = fMerger.PartialMerge(TFileMerger::kIncremental | TFileMerger::kResetable);

	//std::cout<<"did initial (partial) merge of file "<<input<<": "<<input->GetName()<<std::endl;
	DeleteObject(input, true);
	return result;
}

Bool_t TParallelFileMerger::Merge()
{
	// Merge the current inputs into the output file.
	DeleteObject(fMerger.GetOutputFile(), false); // Remove object that can *not* be incrementally merge and will *not* be reset by the client code.
	for(unsigned int f = 0 ; f < fClients.size(); ++f) {
		fMerger.AddFile(fClients[f].fFile);
	}
	Bool_t result = fMerger.PartialMerge(TFileMerger::kAllIncremental);

	// Remove any 'resetable' object (like TTree) from the input file so that they will not
	// be re-merged.  Keep only the object that always need to be re-merged (Histograms).
	for(unsigned int f = 0 ; f < fClients.size(); ++f) {
		if(fClients[f].fFile) {
			DeleteObject(fClients[f].fFile, true);
		} else {
			// We back up the file (probably due to memory constraint)
			TFile* file = TFile::Open(fClients[f].fLocalName, "UPDATE");
			DeleteObject(file, true); // Remove object that can be incrementally merge and will be reset by the client code.
			file->Write();
			delete file;
		}
	}
	fLastMerge = TTimeStamp();
	fNClientsContact = 0;
	fClientsContact.Clear();

	return result;
}

Bool_t TParallelFileMerger::NeedFinalMerge()
{
	// Return true, if there is any data that has not been merged.
	return fClientsContact.CountBits() > 0;
}

Bool_t TParallelFileMerger::NeedMerge(Float_t clientThreshold)
{
	// Return true, if enough client have reported
	if(fClients.empty()) {
		return false;
	}

	// Calculate average and rms of the time between the last 2 contacts.
	Double_t sum = 0;
	Double_t sum2 = 0;
	for(unsigned int c = 0 ; c < fClients.size(); ++c) {
		sum += fClients[c].fTimeSincePrevContact;
		sum2 += fClients[c].fTimeSincePrevContact*fClients[c].fTimeSincePrevContact;
	}
	Double_t avg = sum/fClients.size();
	Double_t sigma = (sum2 != 0) ? TMath::Sqrt(sum2/fClients.size() - avg*avg) : 0;
	Double_t target = avg + 2*sigma;
	TTimeStamp now;
	if((now.AsDouble() - fLastMerge.AsDouble()) > target) {
		return true;
	}
	Float_t cut = clientThreshold * fClients.size();
	return fClientsContact.CountBits() > cut  || fNClientsContact > 2*cut;
}

void TParallelFileMerger::RegisterClient(UInt_t clientId, TFile* file)
{
	// Register that a client has sent a file.
	++fNClientsContact;
	fClientsContact.SetBitNumber(clientId);
	if(fClients.size() < clientId+1) {
		fClients.push_back(ClientInfo(fFilename, clientId));
	}
	fClients[clientId].Set(file);
}
