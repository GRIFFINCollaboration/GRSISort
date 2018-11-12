#ifndef TPARSERLIBRARY_H
#define TPARSERLIBRARY_H

#include <string>

#include "TSingleton.h"
#include "TRawFile.h"
#include "TDataParser.h"

class TParserLibrary : public TSingleton<TParserLibrary> {
public:
	friend class TSingleton<TParserLibrary>;

	~TParserLibrary();

	void Load();                                 ///< if necessary loads shared object library and sets/initializes all other functions

	TRawFile* CreateRawFile(const std::string& file) { Load(); return fCreateRawFile(file); } ///< function to open library specific raw data file
	void DestroyRawFile(TRawFile* file)              { Load(); fDestroyRawFile(file); }       ///< function to destroy library specific raw data file

	TDataParser* CreateDataParser()                  { Load(); return fCreateDataParser(); }  ///< function to open library specific data parser
	void DestroyDataParser(TDataParser* parser)      { Load(); fDestroyDataParser(parser); }  ///< function to destroy library specific data parser

	std::string LibraryVersion()                     { Load(); return fLibraryVersion(); }    ///< returns version of shared object library loaded

private:
	TParserLibrary() { fHandle = nullptr; }

	void* fHandle;                                      ///< handle for shared object library

	void         (*fInitLibrary)();
	std::string  (*fLibraryVersion)();

	TRawFile*    (*fCreateRawFile)(const std::string&);
	void         (*fDestroyRawFile)(TRawFile*);

	TDataParser* (*fCreateDataParser)();
	void         (*fDestroyDataParser)(TDataParser*);

	/// \cond CLASSIMP
	ClassDefOverride(TParserLibrary, 1); // parser library class
	/// \endcond
};

#endif
