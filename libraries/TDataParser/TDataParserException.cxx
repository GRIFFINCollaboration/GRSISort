#include "TDataParserException.h"
#include "TDataParser.h"

TDataParserException::TDataParserException(TDataParser::EDataParserState state, int failedWord, bool multipleErrors)
   : fParserState(state), fFailedWord(failedWord), fMultipleErrors(multipleErrors)
{
   /// default constructor for TDataParserException, stores the data parser state and the word the parser failed on
   /// and creates a message based on them that can be accessed via TDataParserException::what()
   std::ostringstream stream;
   stream<<"TDataParser failed ";
   if(fMultipleErrors) {
      stream<<"on multiple words, first was ";
   } else {
      stream<<"only on ";
   }
   stream<<fFailedWord<<". word"<<std::endl;

   fMessage = stream.str();
}

TDataParserException::~TDataParserException()
{
   /// default destructor
}

const char* TDataParserException::what() const noexcept
{
   /// return message string built in default constructor
   return fMessage.c_str();
}
