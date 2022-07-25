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
/// This class re-implements std::map with more explicit
/// exceptions replacing out-of-range exceptions.
///
////////////////////////////////////////////////////////////

template <typename key_type>
class TGRSIMapException;

template <typename key_type, typename mapped_type, typename key_compare = std::less<key_type>,
			typename allocator_type = std::allocator<std::pair<const key_type, mapped_type> > >
class TGRSIMap
{
public:
	TGRSIMap() {}
	~TGRSIMap() {}

	void Print() {
		for(auto it : fMap) {
			std::cout<<it.first<<" - "<<it.second<<std::endl;
		}
	}

	mapped_type& at(const key_type& k) { 
		try {
			return fMap.at(k);
		} catch(std::exception& e) {
			throw TGRSIMapException<key_type>(k, fMap);
		}
	}
	const mapped_type& at(const key_type& k) const {
		try {
			return fMap.at(k);
		} catch(std::exception& e) {
			throw TGRSIMapException<key_type>(k, fMap);
		}
	}

	      mapped_type& operator[](const key_type& k)       { return fMap[k]; }
	const mapped_type& operator[](const key_type& k) const { return fMap[k]; }

	typedef std::map<key_type, mapped_type, key_compare, allocator_type> map_t;
	typename map_t::iterator begin()       { return fMap.begin(); }
	typename map_t::const_iterator begin() const { return fMap.begin(); }
	typename map_t::iterator end()       { return fMap.end(); }
	typename map_t::const_iterator end() const { return fMap.end(); }

	// capacity functions of std::map
	bool empty() const noexcept { return fMap.empty(); }
	size_t size() const noexcept { return fMap.size(); }
	size_t max_size() const noexcept { return fMap.max_size(); }
	// modifier functions of std::map
	void clear() noexcept { fMap.clear(); }
	//insert
	//insert_or_assign
	//emplace
	//emplace_hint
	//try_emplace
	void erase(typename map_t::iterator pos) { fMap.erase(pos); }
	void erase(typename map_t::iterator first, typename map_t::iterator last) { fMap.erase(first, last); }
	void swap(map_t& other) { fMap.swap(other); }
	// lookup functions of std::map
	typename map_t::size_type count(const key_type* key) const { return fMap.count(key); }
	typename map_t::iterator find(const key_type& key) { return fMap.find(key); }
	typename map_t::const_iterator find(const key_type& key) const { return fMap.find(key); }
	//equal_range
	//lower_bound
	//upper_bound

private:
	std::map<key_type, mapped_type, key_compare, allocator_type> fMap;
};

template <typename key_type>
class TGRSIMapException : public std::exception
{
public:
	template<typename mapped_type, typename key_compare, typename allocator_type>
	TGRSIMapException(const key_type key, const std::map<key_type, mapped_type, key_compare, allocator_type>& map)
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
