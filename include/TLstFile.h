#ifndef TLSTFILE_H
#define TLSTFILE_H

/** \addtogroup Sorting
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TLstFile
///
/// This Class is used to read and write LST files in the
/// root framework.
///
/////////////////////////////////////////////////////////////////

#include <string>

#ifdef __APPLE__
#include <_types/_uint32_t.h>
#else
#include <cstdint>
#endif

#include "TRawFile.h"

#include "TLstEvent.h"

/// Reader for MIDAS .mid files

class TLstFile : public TRawFile {
public:
   enum EOpenType { kRead, kWrite };

   TLstFile(); ///< default constructor
   TLstFile(const char* filename, EOpenType open_type = kRead);
   ~TLstFile() override; ///< destructor

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
   std::shared_ptr<TRawEvent> NewEvent() override { return std::make_shared<TLstEvent>(); }
#endif

protected:
   /// \cond CLASSIMP
   ClassDefOverride(TLstFile, 0) // Used to open and write Midas Files
   /// \endcond
};
/*! @} */
#endif // TLstFile.h
