#ifndef TTDRFILE_H
#define TTDRFILE_H

/** \addtogroup Sorting
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TTdrFile
///
/// This Class is used to read and write TDR files in the
/// root framework.
///
/////////////////////////////////////////////////////////////////

#include <string>
#include <fstream>

#ifdef __APPLE__
#include <_types/_uint32_t.h>
#else
#include <cstdint>
#endif

#include "TRawFile.h"

#include "TTdrEvent.h"

/// Reader for MIDAS .mid files

class TTdrFile : public TRawFile {
public:
   TTdrFile(); ///< default constructor
   TTdrFile(const char* filename, TRawFile::EOpenType open_type = TRawFile::EOpenType::kRead);
   ~TTdrFile() override; ///< destructor

   bool Open(const char* filename) override; ///< Open input file

   void Close() override; ///< Close input file

   using TObject::Read;
   using TObject::Write;
#ifndef __CINT__
   int Read(std::shared_ptr<TRawEvent> lstEvent) override; ///< Read one event from the file
#endif
   std::string Status(bool long_file_description = true) override;

   int GetRunNumber() override;
   int GetSubRunNumber() override;

#ifndef __CINT__
   std::shared_ptr<TRawEvent> NewEvent() override { return std::make_shared<TTdrEvent>(); }
#endif

protected:
	std::ifstream fInputFile;
   /// \cond CLASSIMP
   ClassDefOverride(TTdrFile, 0) // Used to open and write Midas Files
   /// \endcond
};
/*! @} */
#endif // TTdrFile.h
