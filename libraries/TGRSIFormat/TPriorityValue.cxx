#include "TPriorityValue.h"

template<class T>
TPriorityValue<T>::TPriorityValue() {
	fPriority = EPriority::kDefault;
}

template<class T>
TPriorityValue<T>::TPriorityValue(T value, EPriority priority) {
	if(priority > EPriority::kDefault) {
		fValue = value;
		fPriority = priority;
	}
}

template<class T>
TPriorityValue<T>::TPriorityValue(const TPriorityValue& rhs) {
	fPriority = EPriority::kDefault;
	*this = rhs;
}

template<class T>
TPriorityValue<T>::~TPriorityValue() {
}

template<class T>
TPriorityValue<T>& TPriorityValue<T>::operator =(const TPriorityValue<T>& rhs) {
	if(rhs.fPriority > fPriority) {
		fValue = rhs.fValue;
		fPriority = rhs.fPriority;
	}
}

template<class T>
std::ostream& operator<<(std::ostream& out, const TPriorityValue<T>& val)
{
	out<<val.fValue;
	return out;
}

