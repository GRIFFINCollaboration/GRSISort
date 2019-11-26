#ifndef TGRSIMAP_H
#define TGRSIMAP_H

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <initializer_list>

/** \addtogroup Sorting
 *  * @{
 *  */

////////////////////////////////////////////////////////////
///
/// \class TGRSIMap<T>
///
/// This class re-implements std::map with more explicit
/// expections replacing out-of-range exceptions.
///
/// all constructors were copied from http://www.cplusplus.com/reference/map/map/map/ (c++11 tab)
/// and from bits/stl_map.h of gcc 4.9.1
///
////////////////////////////////////////////////////////////

template <typename key_type>
class TGRSIMapException;

template <typename key_type, typename mapped_type, typename key_compare = std::less<key_type>,
			typename allocator_type = std::allocator<std::pair<const key_type, mapped_type> > >
class TGRSIMap : public std::map<key_type, mapped_type, key_compare, allocator_type>
{
public:
	TGRSIMap(const key_compare& comp = key_compare(), const allocator_type& alloc = allocator_type()) : std::map<key_type, mapped_type, key_compare, allocator_type>(comp, alloc) {}
	TGRSIMap(const allocator_type& alloc) : std::map<key_type, mapped_type, key_compare, allocator_type>(alloc) {}
	template <class InputIterator>
		TGRSIMap(InputIterator first, InputIterator last,
				const key_compare& comp = key_compare(),
				const allocator_type& alloc = allocator_type()) : std::map<key_type, mapped_type, key_compare, allocator_type>(first, last, comp, alloc) {}
	TGRSIMap(const TGRSIMap& x) : std::map<key_type, mapped_type, key_compare, allocator_type>(x) {}
	TGRSIMap(const TGRSIMap& x, const allocator_type& alloc) : std::map<key_type, mapped_type, key_compare, allocator_type>(x, alloc) {}
	TGRSIMap(TGRSIMap&& x) : std::map<key_type, mapped_type, key_compare, allocator_type>(x) {}
	TGRSIMap(TGRSIMap&& x, const allocator_type& alloc) : std::map<key_type, mapped_type, key_compare, allocator_type>(x, alloc) {}
	~TGRSIMap() {}

	void Print() {
		for(auto it : *this) {
			std::cout<<it.first<<" - "<<it.second<<std::endl;
		}
	}

	mapped_type& at(const key_type& k) { 
		try {
			return std::map<key_type, mapped_type, key_compare, allocator_type>::at(k);
		} catch(std::exception& e) {
			throw TGRSIMapException<key_type>(k, *this);
		}
	}
	const mapped_type& at(const key_type& k) const {
		try {
			return std::map<key_type, mapped_type, key_compare, allocator_type>::at(k);
		} catch(std::exception& e) {
			throw TGRSIMapException<key_type>(k, *this);
		}
	}
};

template <typename key_type>
class TGRSIMapException : public std::exception
{
public:
	template<typename mapped_type, typename key_compare, typename allocator_type>
	TGRSIMapException(const key_type key, const TGRSIMap<key_type, mapped_type, key_compare, allocator_type>& map)
		: std::exception(), fKey(key)
		{
			for(auto it : map) {
				fKeys.push_back(it.first);
			}
		}

	std::string detail() {
		std::ostringstream str;
		str<<"Key "<<fKey<<" not found in '";
		for(auto key = fKeys.begin(); key != fKeys.end(); ++key) {
			str<<*key;
			if(std::next(key) != fKeys.end()) {
				str<<", ";
			}
		}
		str<<"'";
		return str.str();
	}

private:
	key_type fKey;
	std::vector<key_type> fKeys;
};

/*! @} */
#endif
