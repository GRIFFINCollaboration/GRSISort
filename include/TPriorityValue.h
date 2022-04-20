#ifndef TPRIORITYVALUE_H
#define TPRIORITYVALUE_H

#include <ostream>
#include <string>

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

// template specialization requires full duplicate of original class, take care to update all specializations when changing this!
template<class T>
class TPriorityValue {
public:
	TPriorityValue() {
		fPriority = EPriority::kDefault;
	}

	TPriorityValue(T value, EPriority priority = EPriority::kDefault) {
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
			fValue = rhs.fValue;
			fPriority = rhs.fPriority;
		}
		return *this;
	}

	// comparison operators
	bool operator ==(const TPriorityValue<T>& rhs) {
		return fValue == rhs.fValue;
	}
	bool operator !=(const TPriorityValue<T>& rhs) {
		return fValue != rhs.fValue; // could also be !(this == rhs)
	}
	bool operator < (const TPriorityValue<T>& rhs) {
		return fValue < rhs.fValue;
	}
	bool operator > (const TPriorityValue<T>& rhs) {
		return fValue > rhs.fValue; // could also be rhs < this
	}
	bool operator <=(const TPriorityValue<T>& rhs) {
		return fValue <= rhs.fValue; // could also be !(this>rhs)
	}
	bool operator >=(const TPriorityValue<T>& rhs) {
		return fValue >= rhs.fValue; // could also be !(this<rhs)
	}

	// comparison operators for base class
	bool operator ==(const T& rhs) const {
		return fValue == rhs;
	}
	bool operator !=(const T& rhs) const {
		return fValue != rhs; // could also be !(this == rhs)
	}
	bool operator < (const T& rhs) const {
		return fValue <  rhs;
	}
	bool operator > (const T& rhs) const {
		return fValue >  rhs; // could also be rhs < this
	}
	bool operator <=(const T& rhs) const {
		return fValue <= rhs; // could also be !(this>rhs)
	}
	bool operator >=(const T& rhs) const {
		return fValue >= rhs; // could also be !(this<rhs)
	}

	// explicit conversion
	explicit operator T() const { return fValue; }
	
	// streaming operator
	template<class U>
	friend std::ostream& operator<<(std::ostream&, const TPriorityValue<U>&);

private:
	T fValue;
	EPriority fPriority;
};

// vector specialization
template<class T>
class TPriorityValue<std::vector<T> > {
	// these are the vector specific functions, the rest is just copy-paste of the original class
public:
	// we only have specific (const) functions to access the vectors data
	const T& at(size_t n) const { return fValue.at(n); }
	typename std::vector<T>::const_iterator back() const { return fValue.back(); }
	typename std::vector<T>::const_iterator begin() const { return fValue.begin(); }
	typename std::vector<T>::const_iterator cbegin() const { return fValue.cbegin(); }
	typename std::vector<T>::const_iterator cend() const { return fValue.cend(); }
	typename std::vector<T>::const_reverse_iterator crbegin() const { return fValue.crbegin(); }
	typename std::vector<T>::const_reverse_iterator crend() const { return fValue.crend(); }
	bool empty() const { return fValue.empty(); }
	typename std::vector<T>::const_iterator end() const { return fValue.end(); }
	typename std::vector<T>::const_iterator front() const { return fValue.front(); }
	size_t max_size() const { return fValue.max_size(); }
	const T& operator[](size_t n) const { return fValue[n]; }
	typename std::vector<T>::const_reverse_iterator rbegin() const { return fValue.rbegin(); }
	typename std::vector<T>::const_reverse_iterator rend() const { return fValue.rend(); }
	void reserve(size_t n) { fValue.reserve(n); }
	void shrink_to_fit() { fValue.shrink_to_fit(); }
	size_t size() const { return fValue.size(); }
	
	// copy-paste of original class (with 'T ' replace by 'std::vector<T> ', 'T>' by 'std::vector<T> >', 'T& ' by 'std::vector<T>& ', and 'T* ' by 'std::vector<T>* '):
	// minus the boolean conversion operator
public:
	TPriorityValue() {
		fPriority = EPriority::kDefault;
	}

	TPriorityValue(std::vector<T> value, EPriority priority) {
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
	void Set(std::vector<T> val, EPriority priority) {
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
	void Reset(std::vector<T> val) {
		fValue = val;
		ResetPriority();
	}

	void ResetPriority() {
		fPriority = EPriority::kDefault;
	}

	// getters
	const std::vector<T>&  Value() const    { return fValue; }
	EPriority              Priority() const { return fPriority; }
	std::vector<T>*        Address()        { return &fValue; }
	const std::vector<T>*  Address() const  { return &fValue; }

	// assignment and move assignment operators
	TPriorityValue<std::vector<T> >& operator =(const TPriorityValue<std::vector<T> >& rhs) {
		if(rhs.fPriority != EPriority::kDefault && rhs.fPriority >= fPriority) {
			fValue = rhs.fValue;
			fPriority = rhs.fPriority;
		}
		return *this;
	}

	TPriorityValue<std::vector<T> >& operator =(TPriorityValue<std::vector<T> >&& rhs) {
		if(rhs.fPriority >= fPriority) {
			fValue = std::move(rhs.fValue);
			fPriority = std::move(rhs.fPriority);
		}
		return *this;
	}

	// comparison operators
	bool operator ==(const TPriorityValue<std::vector<T> >& rhs) {
		return fValue == rhs.fValue;
	}
	bool operator !=(const TPriorityValue<std::vector<T> >& rhs) {
		return fValue != rhs.fValue; // could also be !(this == rhs)
	}
	bool operator < (const TPriorityValue<std::vector<T> >& rhs) {
		return fValue < rhs.fValue;
	}
	bool operator > (const TPriorityValue<std::vector<T> >& rhs) {
		return fValue > rhs.fValue; // could also be rhs < this
	}
	bool operator <=(const TPriorityValue<std::vector<T> >& rhs) {
		return fValue <= rhs.fValue; // could also be !(this>rhs)
	}
	bool operator >=(const TPriorityValue<std::vector<T> >& rhs) {
		return fValue >= rhs.fValue; // could also be !(this<rhs)
	}

	// comparison operators for base class
	bool operator ==(const std::vector<T>& rhs) const {
		return fValue == rhs;
	}
	bool operator !=(const std::vector<T>& rhs) const {
		return fValue != rhs; // could also be !(this == rhs)
	}
	bool operator < (const std::vector<T>& rhs) const {
		return fValue <  rhs;
	}
	bool operator > (const std::vector<T>& rhs) const {
		return fValue >  rhs; // could also be rhs < this
	}
	bool operator <=(const std::vector<T>& rhs) const {
		return fValue <= rhs; // could also be !(this>rhs)
	}
	bool operator >=(const std::vector<T>& rhs) const {
		return fValue >= rhs; // could also be !(this<rhs)
	}

	// explicit conversion
	explicit operator std::vector<T>() const { return fValue; }
	
	// streaming operator
	template<class U>
	friend std::ostream& operator<<(std::ostream&, const TPriorityValue<U>&);

private:
	std::vector<T> fValue;
	EPriority fPriority;
};

// string specialization
template<>
class TPriorityValue<std::string> {
	// these are the string specific functions, the rest is just copy-paste of the original class
public:
	const char& at (size_t pos) const { return fValue.at(pos); }
	const char& back() const { return fValue.back(); }
	std::string::const_iterator begin() const { return fValue.begin(); }
	size_t capacity() const { return fValue.capacity(); }
	std::string::const_iterator cbegin() const { return fValue.cbegin(); }
	std::string::const_iterator cend() const { return fValue.cend(); }
	int compare (const std::string& str) const noexcept { return fValue.compare(str); }
	int compare (size_t pos, size_t len, const std::string& str) const { return fValue.compare(pos, len, str); }
	int compare (size_t pos, size_t len, const std::string& str, size_t subpos, size_t sublen) const { return fValue.compare(pos, len, str, subpos, sublen); }
	int compare (const char* s) const { return fValue.compare(s); }
	int compare (size_t pos, size_t len, const char* s) const { return fValue.compare(pos, len, s); }
	int compare (size_t pos, size_t len, const char* s, size_t n) const { return fValue.compare(pos, len, s, n); }
	size_t copy (char* s, size_t len, size_t pos = 0) const { return fValue.copy(s, len, pos); }
	std::string::const_reverse_iterator crbegin() const { return fValue.crbegin(); }
	std::string::const_reverse_iterator crend() const { return fValue.crend(); }
	const char* c_str() const noexcept { return fValue.c_str(); }
	const char* data() const noexcept { return fValue.data(); }
	bool empty() const { return fValue.empty(); }
	std::string::const_iterator end() const { return fValue.end(); }
	//find
	size_t find (const std::string& str, size_t pos = 0) const noexcept { return fValue.find(str, pos); }
	size_t find (const char* s, size_t pos = 0) const { return fValue.find(s, pos); }
	size_t find (const char* s, size_t pos, size_t n) const { return fValue.find(s, pos, n); }
	size_t find (char c, size_t pos = 0) const noexcept { return fValue.find(c, pos); }
	size_t find_first_not_of (const std::string& str, size_t pos = 0) const noexcept { return fValue.find_first_not_of(str, pos); }
	size_t find_first_not_of (const char* s, size_t pos = 0) const { return fValue.find_first_not_of(s, pos); }
	size_t find_first_not_of (const char* s, size_t pos, size_t n) const { return fValue.find_first_not_of(s, pos, n); }
	size_t find_first_not_of (char c, size_t pos = 0) const noexcept { return fValue.find_first_not_of(c, pos); }
	size_t find_first_of (const std::string& str, size_t pos = 0) const noexcept { return fValue.find_first_of(str, pos); }
	size_t find_first_of (const char* s, size_t pos = 0) const { return fValue.find_first_of(s, pos); }
	size_t find_first_of (const char* s, size_t pos, size_t n) const { return fValue.find_first_of(s, pos, n); }
	size_t find_first_of (char c, size_t pos = 0) const noexcept { return fValue.find_first_of(c, pos); }
	size_t find_last_not_of (const std::string& str, size_t pos = std::string::npos) const noexcept { return fValue.find_last_not_of(str, pos); }
	size_t find_last_not_of (const char* s, size_t pos = std::string::npos) const { return fValue.find_last_not_of(s, pos); }
	size_t find_last_not_of (const char* s, size_t pos, size_t n) const { return fValue.find_last_not_of(s, pos, n); }
	size_t find_last_not_of (char c, size_t pos = std::string::npos) const noexcept { return fValue.find_last_not_of(c, pos); }
	size_t find_last_of (const std::string& str, size_t pos = std::string::npos) const noexcept { return fValue.find_last_of(str, pos); }
	size_t find_last_of (const char* s, size_t pos = std::string::npos) const { return fValue.find_last_of(s, pos); }
	size_t find_last_of (const char* s, size_t pos, size_t n) const { return fValue.find_last_of(s, pos, n); }
	size_t find_last_of (char c, size_t pos = std::string::npos) const noexcept { return fValue.find_last_of(c, pos); }
	const char& front() const { return fValue.front(); }
	size_t length() const noexcept { return fValue.length(); }
	size_t max_size() const noexcept { return fValue.max_size(); }
	const char& operator[] (size_t pos) const { return fValue[pos]; }
	std::string::const_reverse_iterator rbegin() const noexcept { return fValue.rbegin(); }
	std::string::const_reverse_iterator rend() const noexcept { return fValue.rend(); }
	void reserve (size_t n = 0) { fValue.reserve(n); }
	size_t rfind (const std::string& str, size_t pos = std::string::npos) const noexcept { return fValue.rfind(str, pos); }
	size_t rfind (const char* s, size_t pos = std::string::npos) const { return fValue.rfind(s, pos); }
	size_t rfind (const char* s, size_t pos, size_t n) const { return fValue.rfind(s, pos, n); }
	size_t rfind (char c, size_t pos = std::string::npos) const noexcept { return fValue.rfind(c, pos); }
	void shrink_to_fit() { fValue.shrink_to_fit(); }
	size_t size() const noexcept { return fValue.size(); }
	std::string substr (size_t pos = 0, size_t len = std::string::npos) const { return fValue.substr(pos, len); }

	// copy-paste of original class (with 'T ' replace by 'std::string ', 'T>' by 'std::string>', 'T& ' by 'std::string& ', and 'T* ' by 'std::string* '):
	// minus the boolean conversion operator
public:
	TPriorityValue() {
		fPriority = EPriority::kDefault;
	}

	TPriorityValue(std::string value, EPriority priority) {
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
	void Set(std::string val, EPriority priority) {
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
	void Reset(std::string val) {
		fValue = val;
		ResetPriority();
	}

	void ResetPriority() {
		fPriority = EPriority::kDefault;
	}

	// getters
	const std::string&  Value() const    { return fValue; }
	EPriority           Priority() const { return fPriority; }
	std::string*        Address()        { return &fValue; }
	const std::string*  Address() const  { return &fValue; }

	// assignment and move assignment operators
	TPriorityValue<std::string>& operator =(const TPriorityValue<std::string>& rhs) {
		if(rhs.fPriority != EPriority::kDefault && rhs.fPriority >= fPriority) {
			fValue = rhs.fValue;
			fPriority = rhs.fPriority;
		}
		return *this;
	}

	TPriorityValue<std::string>& operator =(TPriorityValue<std::string>&& rhs) {
		if(rhs.fPriority >= fPriority) {
			fValue = std::move(rhs.fValue);
			fPriority = std::move(rhs.fPriority);
		}
		return *this;
	}

	// comparison operators
	bool operator ==(const TPriorityValue<std::string>& rhs) {
		return fValue == rhs.fValue;
	}
	bool operator !=(const TPriorityValue<std::string>& rhs) {
		return fValue != rhs.fValue; // could also be !(this == rhs)
	}
	bool operator < (const TPriorityValue<std::string>& rhs) {
		return fValue < rhs.fValue;
	}
	bool operator > (const TPriorityValue<std::string>& rhs) {
		return fValue > rhs.fValue; // could also be rhs < this
	}
	bool operator <=(const TPriorityValue<std::string>& rhs) {
		return fValue <= rhs.fValue; // could also be !(this>rhs)
	}
	bool operator >=(const TPriorityValue<std::string>& rhs) {
		return fValue >= rhs.fValue; // could also be !(this<rhs)
	}

	// comparison operators for base class
	bool operator ==(const std::string& rhs) const {
		return fValue == rhs;
	}
	bool operator !=(const std::string& rhs) const {
		return fValue != rhs; // could also be !(this == rhs)
	}
	bool operator < (const std::string& rhs) const {
		return fValue <  rhs;
	}
	bool operator > (const std::string& rhs) const {
		return fValue >  rhs; // could also be rhs < this
	}
	bool operator <=(const std::string& rhs) const {
		return fValue <= rhs; // could also be !(this>rhs)
	}
	bool operator >=(const std::string& rhs) const {
		return fValue >= rhs; // could also be !(this<rhs)
	}

	// explicit conversion
	explicit operator std::string() const { return fValue; }
	
	// streaming operator
	template<class U>
		friend std::ostream& operator<<(std::ostream&, const TPriorityValue<U>&);

private:
	std::string fValue;
	EPriority fPriority;
};

// streaming operator
	template<class T>
std::ostream& operator<<(std::ostream& out, const TPriorityValue<T>& val)
{
	out<<val.fValue;
	return out;
}

/*! @} */
#endif
