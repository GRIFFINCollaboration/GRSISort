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

   void Load();   ///< if necessary loads shared object library and sets/initializes all other functions

   TRawFile* CreateRawFile(const std::string& file)
   {
      /// function to open library specific raw data file
      Load();
      return fCreateRawFile(file);
   }
   void DestroyRawFile(TRawFile* file)
   {
      /// function to destroy library specific raw data file
      Load();
      fDestroyRawFile(file);
   }

   TDataParser* CreateDataParser()
   {
      /// function to open library specific data parser
      Load();
      return fCreateDataParser();
   }
   void DestroyDataParser(TDataParser* parser)
   {
      /// function to destroy library specific data parser
      Load();
      fDestroyDataParser(parser);
   }

   std::string LibraryVersion()
   {
      /// returns version of shared object library loaded
      Load();
      return fLibraryVersion();
   }

private:
   TParserLibrary() = default;
	TParserLibrary(const TParserLibrary&) = default;
	TParserLibrary(TParserLibrary&&) noexcept = default;
	TParserLibrary& operator=(const TParserLibrary&) = default;
	TParserLibrary& operator=(TParserLibrary&&) noexcept = default;

   void* fHandle{nullptr};   ///< handle for shared object library

   void (*fInitLibrary)();
   std::string (*fLibraryVersion)();

   TRawFile* (*fCreateRawFile)(const std::string&);
   void (*fDestroyRawFile)(TRawFile*);

   TDataParser* (*fCreateDataParser)();
   void (*fDestroyDataParser)(TDataParser*);

   /// \cond CLASSIMP
   ClassDefOverride(TParserLibrary, 1) // NOLINT
   /// \endcond
};

#endif
