#ifndef TPRIORITYVALUE_H
#define TPRIORITYVALUE_H

#include "TObject.h"

/** \addtogroup Sorting
 *  @{
 */

/////////////////////////////////////////////////////////////
///
/// \class TPriorityValue
///
/// The TPriorityValue defines a template of values with 
/// priorities. The priorities are used to over-write values
/// set by the root-file by those read from an input file,
/// and those by values set manually by the user
///
/////////////////////////////////////////////////////////////

template<class T>
class TPriorityValue : public TObject {
public:
	enum class EPriority { kDefault, kRootFile, kInputFile, kUser };

	TPriorityValue();
	TPriorityValue(T, EPriority);
	TPriorityValue(const TPriorityValue&);
	~TPriorityValue();

	T         Value()    { return fValue; }
	EPriority Priority() { return fPriority; }

	TPriorityValue<T>& operator =(const TPriorityValue<T>&);
private:
	T fValue;
	EPriority fPriority;

   /// \cond CLASSIMP
   ClassDefOverride(TPriorityValue, 1) // Descant Physics structure
	/// \endcond
};

template<class T>
std::ostream& operator<<(std::ostream&, const TPriorityValue<T>&);

/*! @} */
#endif
