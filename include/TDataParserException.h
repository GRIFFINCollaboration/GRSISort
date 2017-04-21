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
		TDataParserException(TDataParserState, int);
		~TDataParserException();

		const char* what() const noexcept;

		int GetFailedWord()               { return fFailedWord; }
		TDataParserState GetParserState() { return fParserState; }

	private:
		TDataParserState fParserState;
		int fFailedWord;
		std::string fMessage;
};
/*! @} */
#endif
