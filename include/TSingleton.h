#ifndef TSINGLETON_H
#define TSINGLETON_H

#include <iostream>

#include "TObject.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TList.h"
#include "TKey.h"

#include "Globals.h"

/** \addtogroup Sorting
 *  *  @{
 *   */

///////////////////////////////////////////////////////////////
///
/// This class is intended as a base class for singletons,
/// especially those that are written to file.
/// The Get() function is written such that it reads the class
/// from file if needed. This is the case if it hasn't been 
/// read yet, or if the gDirectory has been changed.
/// This means in a loop over different input files, Get() will
/// always return the info of the current file.
///
///////////////////////////////////////////////////////////////

template <class T>
class TSingleton : public TObject
{
public:
	static T* Get(bool verbose = false)
	{
		// if we don't have an instance yet or changed into another directory
		// we want to read from the current directory
		if(fSingleton == nullptr || fDir != gDirectory) {
			if((gDirectory->GetFile()) != nullptr) {
				TList* list = gDirectory->GetFile()->GetListOfKeys();
				TIter  iter(list);
				if(verbose) std::cout<<"Reading "<<T::Class()->GetName()<<R"( from file ")"<<CYAN<<gDirectory->GetFile()->GetName()<<RESET_COLOR<<R"(")"<<std::endl;
				while(TKey* key = static_cast<TKey*>(iter.Next())) {
					if(strcmp(key->GetClassName(), T::Class()->GetName()) != 0) {
						continue;
					}
					// we found the object in the file, so we use it as our singleton
					// this automatically deletes the old singleton if we just switched files
					Set(static_cast<T*>(key->ReadObj()));
					fDir = gDirectory; // in either case (read from file or created new), gDirectory is the current directory
					break; // we will find the newest key first, so we want to break here and not try and read other instaces of the same class
				}
			}
			if(fSingleton == nullptr) {
				fSingleton = new T;
				fDir = gDirectory; // in either case (read from file or created new), gDirectory is the current directory
			}
		}
		return fSingleton;
	}

	static T* GetAll()
	{
		// get the singleton itself
		Get();
		// check if we can get the run number and sub-run number from the directory we read this from
		std::string fileName = fDir->GetName();
		std::string filebase;
		// we expect the root-file name format to be (<dir>/)<base><5 digit run number>_<3 digit subrun number>.root
		// so the length needs to be at least 9 (assuming single character for base, run, and subrun number)
		if(fileName.length() < 9) {
			return fSingleton;
		}
		size_t underscore = fileName.find_last_of('_');
		size_t dot = fileName.find_last_of('.');
		// check that the dot is length minus 5 (which also ensures it was found) and
		// that the underscore is dot minus 4 (again ensuring it was found)
		if(dot != fileName.length()-5 || underscore != dot-4) {
			return fSingleton;
		}
		filebase = fileName.substr(0,underscore-5);
		int runNumber = atoi(fileName.substr(underscore-5).c_str());
		int subrunNumber = atoi(fileName.substr(dot-3).c_str());
		if(runNumber > 0 || subrunNumber > 0) {
			TFile* prevSubRun;
			if(subrunNumber > 0) {
				prevSubRun = new TFile(Form("%s%05d_%03d.root",filebase.c_str(),runNumber,subrunNumber-1));
			} else {
				// search for last subrun of previous run
				int subrun = 0;
				prevSubRun = new TFile(Form("%s%05d_%03d.root",filebase.c_str(),runNumber-1,subrun++));
				TFile* tmpFile = nullptr;
				while(prevSubRun != nullptr && prevSubRun->IsOpen()) {
					if(tmpFile != nullptr) {
						tmpFile->Close();
						tmpFile = prevSubRun;
					}
					prevSubRun = new TFile(Form("%s%05d_%03d.root",filebase.c_str(),runNumber-1,subrun++));
				}
				// this works if we found a subrun (i.e. the last read is not good, but the one stored in tmp is)
				// of if we didn't find one (i.e. tmpFile is a nullptr which we check for)
				prevSubRun = tmpFile;
			}
			if(prevSubRun != nullptr && prevSubRun->IsOpen()) {
				T* prevSingleton = static_cast<T*>(prevSubRun->Get(fSingleton->GetName()));
				if(prevSingleton != nullptr) {
					fSingleton->Add(prevSingleton);
					std::cout<<"Found previous "<<fSingleton->GetName()<<" data from "<<prevSubRun->GetName()<<std::endl;
				} else {
					std::cout<<"Failed to find previous "<<fSingleton->GetName()<<" data from "<<prevSubRun->GetName()<<std::endl;
					// try to find object without leading T
					prevSingleton = static_cast<T*>(prevSubRun->Get(&(fSingleton->GetName()[1])));
					if(prevSingleton != nullptr) {
						fSingleton->Add(prevSingleton);
						std::cout<<"Found previous "<<&(fSingleton->GetName()[1])<<" data from "<<prevSubRun->GetName()<<std::endl;
					} else {
						std::cout<<"Failed to find previous "<<&(fSingleton->GetName()[1])<<" data from "<<prevSubRun->GetName()<<std::endl;
					}
				}
				prevSubRun->Close();
				fDir->cd();
			} else {
					std::cout<<"Failed to find previous file "<<prevSubRun->GetName()<<" not adding data to "<<fSingleton->GetName()<<std::endl;
			}
		}
		return fSingleton;
	}

	static void Set(T* val)
	{
		if(fSingleton != val) {
			delete fSingleton;
			fSingleton = val;
		}
	}

	static T* AddCurrent()
	{
		// if we don't have an instance yet, we just use Get
		if(fSingleton == nullptr) {
			return Get();
		}
		// if we do have an instance and changed into another directory
		// we want to add the object read from the current directory
		if(fSingleton != nullptr && fDir != gDirectory) {
			if((gDirectory->GetFile()) != nullptr) {
				TList* list = gDirectory->GetFile()->GetListOfKeys();
				TIter  iter(list);
				std::cout<<R"(Reading from file ")"<<CYAN<<gDirectory->GetFile()->GetName()<<RESET_COLOR<<R"(": )"<<std::flush;
				while(TKey* key = static_cast<TKey*>(iter.Next())) {
					if(strcmp(key->GetClassName(), T::Class()->GetName()) != 0) {
						continue;
					}
					// we found the object in the file, so we use it as our singleton
					// this automatically deletes the old singleton if we just switched files
					std::cout<<"adding "<<T::Class()->GetName()<<" "<<std::flush;
					T* tmpSingleton = static_cast<T*>(key->ReadObj());
					fSingleton->Add(tmpSingleton);
					fDir = gDirectory; // update the directory to gDirectory so we don't read from this file again
				}
				std::cout<<std::endl;
			}
		}
		return fSingleton;
	}

	static void PrintDirectory()
	{
		std::cout<<"Read singleton "<<fSingleton<<" from "<<(fDir!=nullptr?fDir->GetName():"N/A")<<std::endl;
	}

protected:
	TSingleton();
	//TSingleton(TSingleton const &) = delete;
	// note, we can't delete this, because that wouldn't allow us to use the Set function above!
	//TSingleton& operator=(TSingleton const &) = delete;
	~TSingleton();

private:
	static T* fSingleton;
	static TDirectory* fDir;

	/// \cond CLASSIMP
	ClassDef(TSingleton, 1)
	/// \endcond
};

/// \cond CLASSIMP
templateClassImp(TSingleton)
/// \endcond

template<class T>
T* TSingleton<T>::fSingleton = nullptr;

template<class T>
TDirectory* TSingleton<T>::fDir = nullptr;

template<class T>
TSingleton<T>::TSingleton()
{
}

template<class T>
TSingleton<T>::~TSingleton()
{
	//if(fSingleton != this) delete fSingleton;
}

template<class T>
void TSingleton<T>::Streamer(TBuffer& R__b) 
{
	/// Stream an object of class T.
	if(R__b.IsReading()) {
		//R__b.ReadClassBuffer(T::Class(), &(Get()));
		//Set(static_cast<T*>(this));
	} else {
		//R__b.WriteClassBuffer(T::Class(), &(Get()));
	}
}

template<class T>
TClass* TSingleton<T>::Class()
{
	return T::Class();
}
/*! @} */
#endif
