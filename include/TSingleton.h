#ifndef TSINGLETON_H
#define TSINGLETON_H

#include <iostream>

#include "TObject.h"

/** \addtogroup Sorting
 *  *  @{
 *   */

/////////////////////////////////////////////////////////////////
/////
///// \class TGRSIRunInfo
/////
/////////////////////////////////////////////////////////////////

template <class T>
class TSingleton : public TObject
{
public:
	static T& Get()
	{
		static T singleton;
		return singleton;
	}
	static void Set(T val)
	{
		Get() = val;
	}

	TSingleton() { fStreamerInstance = 0; }
	//TSingleton(TSingleton const &) = delete;
	// note, we can't delete this, because that wouldn't allow us to use the Set function above!
	//TSingleton& operator=(TSingleton const &) = delete;
	~TSingleton() {}

private:
	int fStreamerInstance;

	/// \cond CLASSIMP
	ClassDef(TSingleton, 1)
	/// \endcond
};

templateClassImp(TSingleton)

template<class T>
void TSingleton<T>::Streamer(TBuffer& R__b) 
{
	/// Stream an object of class T.
	++fStreamerInstance;
	std::cout<<"Calling "<<(R__b.IsReading() ? "reading" : "writing")<<" streamer for TSingleton<"<<T::Class()->GetName()<<">"<<std::endl;
	if(fStreamerInstance == 1) {
		Get().Streamer(R__b);
	}
	if(R__b.IsReading()) {
		//std::cout<<"ReadClassBuffer("<<T::Class()<<", "<<&(Get())<<") "<<this<<std::endl;
		//R__b.ReadClassBuffer(T::Class(), &(Get()));
		std::cout<<"this "<<this<<", &Get() "<<&(Get());
		//Set(*static_cast<T*>(this));
		std::cout<<"=> &Get() "<<&(Get())<<std::endl;
	} else {
		//std::cout<<"WriteClassBuffer("<<T::Class()<<", "<<&(Get())<<") "<<this<<std::endl;
		//R__b.WriteClassBuffer(T::Class(), &(Get()));
		//std::cout<<"done"<<std::endl<<std::endl;
	}
}

template<class T>
TClass* TSingleton<T>::Class()
{
	return T::Class();
}
/*! @} */
#endif
