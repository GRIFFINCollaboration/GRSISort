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
/// \class TSingleton<T>
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
	static T* Get()
	{
		// if we don't have an instance yet or changed into another directory
		// we want to read from the current directory
		if(fSingleton == nullptr || fDir != gDirectory) {
			delete fSingleton; // in case we just changed directories
			fSingleton = nullptr;
			if((gDirectory->GetFile()) != nullptr) {
				TList* list = gDirectory->GetFile()->GetListOfKeys();
				TIter  iter(list);
				std::cout<<"Reading "<<T::Class()->GetName()<<R"( from file ")"<<CYAN<<gDirectory->GetFile()->GetName()<<RESET_COLOR<<R"(")"<<std::endl;
				while(TKey* key = static_cast<TKey*>(iter.Next())) {
					if(strcmp(key->GetClassName(), T::Class()->GetName()) != 0) {
						continue;
					}
					Set(static_cast<T*>(key->ReadObj()));
				}
			}
			if(fSingleton == nullptr) {
				fSingleton = new T;
			}
			fDir = gDirectory; // in either case (read from file or created new), gDirectory is the current directory
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

templateClassImp(TSingleton)

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
