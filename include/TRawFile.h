#ifndef TRAWFILE_H
#define TRAWFILE_H

/** \addtogroup Sorting
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TRawFile
///
/// This Class is used to read and write raw files in the
/// root framework.
///
/////////////////////////////////////////////////////////////////

#include <string>

#ifdef __APPLE__
#include <_types/_uint32_t.h>
#else
#include <cstdint>
#endif

#include "TObject.h"

#include "TMidasEvent.h"
#include "TLstEvent.h"

/// Reader for MIDAS .mid files

class TRawFile : public TObject {
public:
   enum class EOpenType { kRead, kWrite };

   TRawFile() {} ///< default constructor
   TRawFile(const char*, EOpenType = EOpenType::kRead) {}
   ~TRawFile() override = default; ///< destructor

   virtual bool Open(const char* filename) = 0; ///< Open input file

   virtual void Close() = 0; ///< Close input file

   using TObject::Read;
   using TObject::Write;
#ifndef __CINT__
   virtual int Read(std::shared_ptr<TRawEvent> event) = 0; ///< Read one event from the file
#endif
   virtual std::string Status(bool long_file_description = true) = 0;

   virtual const char* GetFilename() const { return fFilename.c_str(); } ///< Get the name of this file

   virtual int GetRunNumber()    = 0;
   virtual int GetSubRunNumber() = 0;

   virtual size_t GetBytesRead() { return fBytesRead; }
   virtual size_t GetFileSize() { return fFileSize; }

#ifndef __CINT__
   virtual std::shared_ptr<TRawEvent> NewEvent() = 0;
#endif

protected:
   std::string fFilename; ///< name of the currently open file

   std::vector<char> fReadBuffer;

   size_t fBytesRead{0};
   size_t fFileSize{0};

   /// \cond CLASSIMP
   ClassDefOverride(TRawFile, 0) // Used to open and write Midas Files
   /// \endcond
};
/*! @} */
#endif // TRawFile.h
