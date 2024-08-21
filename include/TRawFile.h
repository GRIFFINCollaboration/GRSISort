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

#include "TRawEvent.h"

/// Reader for raw files

class TRawFile : public TObject {
public:
   enum class EOpenType { kRead,
                          kWrite };

   TRawFile() = default;   ///< default constructor
   explicit TRawFile(const char*, EOpenType = EOpenType::kRead) {}
   TRawFile(const TRawFile&)                = default;
   TRawFile(TRawFile&&) noexcept            = default;
   TRawFile& operator=(const TRawFile&)     = default;
   TRawFile& operator=(TRawFile&&) noexcept = default;
   ~TRawFile()                              = default;   ///< destructor

   virtual bool Open(const char* filename) = 0;   ///< Open input file

   virtual void Close() = 0;   ///< Close input file

   using TObject::Read;
   using TObject::Write;
#ifndef __CINT__
   virtual int Read(std::shared_ptr<TRawEvent> event) = 0;   ///< Read one event from the file
#endif
   virtual void        Skip(size_t nofEvents)                    = 0;   ///< Skip nofEvents events in file
   virtual std::string Status(bool long_file_description = true) = 0;

   virtual const char* GetFilename() const { return fFilename.c_str(); }   ///< Get the name of this file

   virtual int GetRunNumber()    = 0;
   virtual int GetSubRunNumber() = 0;

   virtual size_t BytesRead() { return fBytesRead; }
   void           IncrementBytesRead(size_t val = 1) { fBytesRead += val; }
   virtual size_t FileSize() { return fFileSize; }
   void           FileSize(size_t fileSize) { fFileSize = fileSize; }

   virtual std::string Filename() const { return fFilename; }   ///< Get the name of this file
   virtual void        Filename(const char* val) { fFilename = val; }

   size_t BufferSize() const { return fReadBuffer.size(); }
   char*  BufferData() { return fReadBuffer.data(); }
   void   ClearBuffer() { fReadBuffer.clear(); }
   void   ResizeBuffer(size_t newSize) { fReadBuffer.resize(newSize); }

#ifndef __CINT__
   virtual std::shared_ptr<TRawEvent> GetOdbEvent()
   {
      return nullptr;
   }
   virtual std::shared_ptr<TRawEvent> NewEvent() = 0;
#endif

private:
   std::string fFilename;   ///< name of the currently open file

   std::vector<char> fReadBuffer;

   size_t fBytesRead{0};
   size_t fFileSize{0};

   /// \cond CLASSIMP
   ClassDefOverride(TRawFile, 0)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif   // TRawFile.h
