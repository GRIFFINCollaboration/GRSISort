#ifndef TDATAPARSEREXCEPTION_H
#define TDATAPARSEREXCEPTION_H
/** \addtogroup Sorting
 *  @{
 */

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

#include <exception>
#include <string>

#include "TDataParser.h"

class TDataParserException : public std::exception {
public:
   TDataParserException(TDataParser::EDataParserState state, int failedWord, bool multipleErrors);
   ~TDataParserException() override;

   const char* what() const noexcept override;

   int                           GetFailedWord() { return fFailedWord; }
   TDataParser::EDataParserState GetParserState() { return fParserState; }
   bool                          GetMultipleErrors() { return fMultipleErrors; }

private:
   TDataParser::EDataParserState fParserState;
   int                           fFailedWord;
   bool                          fMultipleErrors;
   std::string                   fMessage;
};
/*! @} */
#endif
