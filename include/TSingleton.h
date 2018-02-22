#ifndef TSINGLETON_H
#define TSINGLETON_H

#include <iostream>

#include "TObject.h"
#include "TList.h"
#include "TKey.h"

/** \addtogroup Sorting
 *  *  @{
 *   */

///////////////////////////////////////////////////////////////
///
/// \class TGRSIRunInfo
///
///////////////////////////////////////////////////////////////

template <class T>
class TSingleton : public TObject
{
public:
	static T& Get()
	{
		static T singleton;
		//if((gDirectory->GetFile()) != nullptr) {
		//	TList* list = gDirectory->GetFile()->GetListOfKeys();
		//	TIter  iter(list);
		//	std::cout<<"Reading "<<T::Class()->GetName()<<R"( from file ")"<<CYAN<<gDirectory->GetFile()->GetName()<<RESET_COLOR<<R"(")"<<std::endl;
		//	while(TKey* key = static_cast<TKey*>(iter.Next())) {
		//		if(strcmp(key->GetClassName(), T::Class()->GetName()) != 0) {
		//			continue;
		//		}

		//		Set(*static_cast<T*>(key->ReadObj()));
		//		//std::cout<<"read from file "<<this<<":"<<std::endl;
		//		//this->Print();
		//	}
		//}
		return singleton;
	}
	static void Set(T val)
	{
		Get() = val;
	}

	TSingleton() {}
	//TSingleton(TSingleton const &) = delete;
	// note, we can't delete this, because that wouldn't allow us to use the Set function above!
	//TSingleton& operator=(TSingleton const &) = delete;
	~TSingleton() {}

private:

	/// \cond CLASSIMP
	ClassDef(TSingleton, 1)
	/// \endcond
};

templateClassImp(TSingleton)

template<class T>
void TSingleton<T>::Streamer(TBuffer& R__b) 
{
	/// Stream an object of class T.
	std::cout<<__PRETTY_FUNCTION__<<std::endl;
	std::cout<<"Calling "<<(R__b.IsReading() ? "reading" : "writing")<<" streamer for TSingleton<"<<T::Class()->GetName()<<">, TBuffer size "<<R__b.BufferSize()<<std::endl;
	std::cout<<this<<".Print():"<<std::endl;
	//static_cast<T*>(this)->Print();
	//std::cout<<&Get()<<".Print():"<<std::endl;
	//Get().Print();
	if(R__b.IsReading()) {
		//std::cout<<"ReadClassBuffer("<<T::Class()->GetName()<<", "<<&(Get())<<") "<<this<<std::endl;
		////R__b.ReadClassBuffer(T::Class(), &(Get()));
		//std::cout<<"this "<<this<<", &Get() "<<&(Get());
		////Set(*static_cast<T*>(this));
		//std::cout<<"=> &Get() "<<&(Get())<<std::endl;
	} else {
		std::cout<<"WriteClassBuffer("<<T::Class()->GetName()<<", "<<&(Get())<<") "<<this<<std::endl;
		//R__b.WriteClassBuffer(T::Class(), &(Get()));
		std::cout<<"done"<<std::endl<<std::endl;
	}
	std::cout<<"Done with "<<(R__b.IsReading() ? "reading" : "writing")<<" streamer for TSingleton<"<<T::Class()->GetName()<<">, TBuffer size "<<R__b.BufferSize()<<std::endl;
	std::cout<<this<<".Print():"<<std::endl;
	//static_cast<T*>(this)->Print();
	//std::cout<<&Get()<<".Print():"<<std::endl;
	//Get().Print();
	std::cout<<"----------------------------------------"<<std::endl;
}

template<class T>
TClass* TSingleton<T>::Class()
{
	return T::Class();
}
/*! @} */
#endif
