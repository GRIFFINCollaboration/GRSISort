#ifndef TDATAFRAMELIBRARY_H
#define TDATAFRAMELIBRARY_H
#include "RVersion.h"
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 14, 0)

#include <string>

#include "TList.h"

#include "TSingleton.h"
#include "TGRSIHelper.h"

class TDataFrameLibrary : public TSingleton<TDataFrameLibrary> {
public:
   friend class TSingleton<TDataFrameLibrary>;

   ~TDataFrameLibrary();

   void Load();   ///< if necessary loads shared object library and sets/initializes all other functions

   TGRSIHelper* CreateHelper(TList* list)
   {
      /// function to open library specific data parser
      Load();
      return fCreateHelper(list);
   }
   void DestroyHelper(TGRSIHelper* parser)
   {
      /// function to destroy library specific data parser
      Load();
      fDestroyHelper(parser);
   }

private:
   TDataFrameLibrary() { fHandle = nullptr; }
   /// compile user code into shared object library, dot is the positions of the last dot (guaranteed to be after the last slash!)
   /// slash is the position of the last slash (could be npos)
   void Compile(std::string& path, const size_t& dot, const size_t& slash);

   void* fHandle;   ///< handle for shared object library

   TGRSIHelper* (*fCreateHelper)(TList*);
   void (*fDestroyHelper)(TGRSIHelper*);

   /// \cond CLASSIMP
   ClassDefOverride(TDataFrameLibrary, 1);   // parser library class
   /// \endcond
};

#endif
#endif
