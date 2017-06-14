#ifndef TMIDASFILE_H
#define TMIDASFILE_H

/** \addtogroup Sorting
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TMidasFile
///
/// This Class is used to read and write MIDAS files in the
/// root framework. It reads and writes TMidasEvents.
///
/////////////////////////////////////////////////////////////////

#include <string>

#ifdef __APPLE__
#include <_types/_uint32_t.h>
#else
#include <stdint.h>
#endif

#include "TRawFile.h"

#include "TMidasEvent.h"

/// Reader for MIDAS .mid files

class TMidasFile : public TRawFile {
public:
   enum EOpenType { kRead, kWrite };

   TMidasFile(); ///< default constructor
   TMidasFile(const char* filename, EOpenType open_type = kRead);
   virtual ~TMidasFile(); ///< destructor

   bool Open(const char* filename);    ///< Open input file
   bool OutOpen(const char* filename); ///< Open output file

   void Close();    ///< Close input file
   void OutClose(); ///< Close output file

   using TObject::Read;
   using TObject::Write;
#ifndef __CINT__
   int Read(std::shared_ptr<TRawEvent> event);                         ///< Read one event from the file
   bool Write(std::shared_ptr<TMidasEvent> event, Option_t* opt = ""); ///< Write one event to the output file
#endif
   std::string Status(bool long_file_description = true);

#ifndef __CINT__
   void FillBuffer(std::shared_ptr<TMidasEvent> event, Option_t* opt = ""); // Fill buffer to write out chunks of data
#endif
   bool WriteBuffer();
   // int GetBufferSize() const { return fWriteBuffer.size(); }

   const char* GetFilename() const { return fFilename.c_str(); }   ///< Get the name of this file
   int         GetLastErrno() const { return fLastErrno; }         ///< Get error value for the last file error
   const char* GetLastError() const { return fLastError.c_str(); } ///< Get error text for the last file error

#ifndef __CINT__
   std::shared_ptr<TMidasEvent> GetFirstEvent() { return fFirstEvent; }
#endif

   int GetRunNumber();
   int GetSubRunNumber();

   void SetMaxBufferSize(int maxsize);

#ifndef __CINT__
   std::shared_ptr<TRawEvent> NewEvent() { return std::make_shared<TMidasEvent>(); }
#endif

protected:
   void ReadMoreBytes(size_t bytes);

#ifndef __CINT__
   std::shared_ptr<TMidasEvent> fFirstEvent;
#endif

   std::string fOutFilename; ///< name of the currently open file

   std::vector<char> fWriteBuffer;
   uint32_t          fCurrentBufferSize;
   uint32_t          fMaxBufferSize;

   int         fLastErrno; ///< errno from the last operation
   std::string fLastError; ///< error string from last errno
protected:
   int currentEventNumber;

   bool fDoByteSwap; ///< "true" if file has to be byteswapped

   int   fFile;      ///< open input file descriptor
   void* fGzFile;    ///< zlib compressed input file reader
   void* fPoFile;    ///< popen() input file reader
   int   fOutFile;   ///< open output file descriptor
   void* fOutGzFile; ///< zlib compressed output file reader

   /// \cond CLASSIMP
   ClassDef(TMidasFile, 0) // Used to open and write Midas Files
   /// \endcond
};
/*! @} */
#endif // TMidasFile.h
