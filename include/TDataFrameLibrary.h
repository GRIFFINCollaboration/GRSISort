#ifndef TDATAFRAMELIBRARY_H
#define TDATAFRAMELIBRARY_H

#include <string>

#include "TList.h"

#include "TSingleton.h"
#include "TGRSIHelper.h"

class TDataFrameLibrary : public TSingleton<TDataFrameLibrary> {
public:
	friend class TSingleton<TDataFrameLibrary>;

	~TDataFrameLibrary();

	void Load();                                 ///< if necessary loads shared object library and sets/initializes all other functions

	TGRSIHelper* CreateHelper(TList* list)  { Load(); return fCreateHelper(list); }  ///< function to open library specific data parser
	void DestroyHelper(TGRSIHelper* parser) { Load(); fDestroyHelper(parser); }  ///< function to destroy library specific data parser

private:
	TDataFrameLibrary() { fHandle = nullptr; }
	void Compile(std::string& path, const size_t& dot); ///< compile user code into shared object library, dot is the positions of the last dot (guaranteed to be after the last slash!)

	void* fHandle;                                      ///< handle for shared object library

	TGRSIHelper*    (*fCreateHelper)(TList*);
	void            (*fDestroyHelper)(TGRSIHelper*);

	/// \cond CLASSIMP
	ClassDefOverride(TDataFrameLibrary, 1); // parser library class
	/// \endcond
};

#endif
