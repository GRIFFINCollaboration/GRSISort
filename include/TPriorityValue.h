#ifndef TPRIORITYVALUE_H
#define TPRIORITYVALUE_H

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

enum class EPriority { kDefault, kRootFile, kInputFile, kUser, kForce };

template<class T>
class TPriorityValue {
public:
	TPriorityValue() {
		fPriority = EPriority::kDefault;
	}

	TPriorityValue(T value, EPriority priority) {
		fValue = value;
		fPriority = priority;
	}

	TPriorityValue(const TPriorityValue& rhs) {
		fPriority = EPriority::kDefault;
		*this = rhs;
	}

	TPriorityValue(TPriorityValue&& rhs) {
		fPriority = EPriority::kDefault;
		*this = rhs;
	}

	~TPriorityValue() {
	}


	// setter
	void Set(T val, EPriority priority) {
		if(priority >= fPriority) {
			fValue = val;
			if(priority != EPriority::kForce) {
				fPriority = priority;
			}
		}
	}

	void SetPriority(EPriority priority) {
		// only allow the priority to be increased, not decreased
		if(priority > fPriority) {
			fPriority = priority;
		}
	}
	
	// reset functions
	void Reset(T val) {
		fValue = val;
		ResetPriority();
	}

	void ResetPriority() {
		fPriority = EPriority::kDefault;
	}

	// getters
	const T&  Value() const    { return fValue; }
	EPriority Priority() const { return fPriority; }
	T*        Address()        { return &fValue; }
	const T*  Address() const  { return &fValue; }

	// assignment and move assignment operators
	TPriorityValue<T>& operator =(const TPriorityValue<T>& rhs) {
		if(rhs.fPriority != EPriority::kDefault && rhs.fPriority >= fPriority) {
			fValue = rhs.fValue;
			fPriority = rhs.fPriority;
		}
		return *this;
	}

	TPriorityValue<T>& operator =(TPriorityValue<T>&& rhs) {
		if(rhs.fPriority >= fPriority) {
			fValue = std::move(rhs.fValue);
			fPriority = std::move(rhs.fPriority);
		}
		return *this;
	}

	template<class U>
	friend std::ostream& operator<<(std::ostream&, const TPriorityValue<U>&);

private:
	T fValue;
	EPriority fPriority;
};

template<class T>
std::ostream& operator<<(std::ostream& out, const TPriorityValue<T>& val)
{
	out<<val.fValue;
	return out;
}

/*! @} */
#endif
