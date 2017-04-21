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
		TDataParserException(TDataParser::EDataParserState, int);
		~TDataParserException();

		const char* what() const noexcept;

		int GetFailedWord()               { return fFailedWord; }
		TDataParser::EDataParserState GetParserState() { return fParserState; }

	private:
		TDataParser::EDataParserState fParserState;
		int fFailedWord;
		std::string fMessage;
};
/*! @} */
#endif
