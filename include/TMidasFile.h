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

#include "TObject.h"

#include "TMidasEvent.h"

/// Reader for MIDAS .mid files

class TMidasFile : public TObject {
public:
  enum EOpenType {
    kRead,
    kWrite
  };

  TMidasFile(); ///< default constructor
  TMidasFile(const char* filename, EOpenType open_type = kRead);
  virtual ~TMidasFile(); ///< destructor

  bool Open(const char* filename); ///< Open input file
  bool OutOpen(const char* filename); ///< Open output file

  void Close(); ///< Close input file
  void OutClose(); ///< Close output file

  using TObject::Read;
  using TObject::Write;
  int  Read(TMidasEvent* event); ///< Read one event from the file
  int  Read(TMidasEvent& event) { return Read(&event); } ///< Read one event from the file
  bool Write(TMidasEvent* event,Option_t* opt =""); ///< Write one event to the output file
  std::string Status(bool long_file_description = true);

  void FillBuffer(TMidasEvent* event, Option_t* opt=""); //Fill buffer to write out chunks of data
  bool WriteBuffer();
  //int GetBufferSize() const { return fWriteBuffer.size(); }

  const char* GetFilename()  const { return fFilename.c_str();  } ///< Get the name of this file
  int         GetLastErrno() const { return fLastErrno; }         ///< Get error value for the last file error
  const char* GetLastError() const { return fLastError.c_str(); } ///< Get error text for the last file error

  TMidasEvent& GetFirstEvent() { return fFirstEvent; }

  int	GetRunNumber();
  int	GetSubRunNumber();

  void SetMaxBufferSize(int maxsize);

protected:
  void ReadMoreBytes(size_t bytes);

  TMidasEvent fFirstEvent;

  std::string fFilename; ///< name of the currently open file
  std::string fOutFilename; ///< name of the currently open file

  std::vector<char> fWriteBuffer;
  uint32_t fCurrentBufferSize;
  uint32_t fMaxBufferSize;

  std::vector<char> fReadBuffer;

  int         fLastErrno; ///< errno from the last operation
  std::string fLastError; ///< error string from last errno
protected:
  int currentEventNumber;
  size_t bytesRead;
  size_t filesize;


  bool fDoByteSwap; ///< "true" if file has to be byteswapped

  int         fFile; ///< open input file descriptor
  void*       fGzFile; ///< zlib compressed input file reader
  void*       fPoFile; ///< popen() input file reader
  int         fOutFile; ///< open output file descriptor
  void*       fOutGzFile; ///< zlib compressed output file reader

/// \cond CLASSIMP
	ClassDef(TMidasFile,0) //Used to open and write Midas Files
/// \endcond
};
/*! @} */
#endif // TMidasFile.h
