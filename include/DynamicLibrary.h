#ifndef DYNAMICLIBRARY_H
#define DYNAMICLIBRARY_H

#include <string>

/// Loads a Shared Object library
/**
   The be moved, but not copied.
   When the object is destroyed, the library is closed.
   The caller is responsible for keeping the object alive
     while symbols from the object are being used.
 */
class DynamicLibrary {
public:
   /// Loads a shared object library.
   /**
      @param libname_param Path to the library to be loaded.
      @param unique_name Generates a uniquely named symlink to the file, which is then opened.

      dlopen/dlclose maintains a reference count,
        caching shared objects in case a library is loaded multiple times.
      However, if a library is going to change as the program runs,
        then this caching works against us.
      In this case, "unique_name" should be set to true.
    */
   explicit DynamicLibrary(std::string libname_param, bool unique_name = false);

   /// Destructs the shared object library
   /**
      Closes the shared object library.
      All symbols from the shared object library become invalid.
      The caller is responsible for keeping the DynamicLibrary alive,
        while symbols from the library are in use.
    */
   ~DynamicLibrary();

   /// Move constructor
   DynamicLibrary(DynamicLibrary&& other) noexcept ;

   /// Move assignment operator
   DynamicLibrary& operator=(DynamicLibrary&& other) noexcept ;

#ifdef __CINT__
private:
   DynamicLibrary(const DynamicLibrary&);
   DynamicLibrary& operator=(const DynamicLibrary&);

public:
#else
   DynamicLibrary(const DynamicLibrary&)            = delete;
   DynamicLibrary& operator=(const DynamicLibrary&) = delete;
#endif

   /// Extracts a symbol from the shared library.
   void* GetSymbol(const char* symbol);

private:
   void swap(DynamicLibrary& other);

   void*       fLibrary;
   std::string fLibName;
   std::string fTempName;
};

#endif /* _DYNAMICLIBRARY_H_ */
