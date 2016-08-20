#ifndef _COMBINATIONS_H_
#define _COMBINATIONS_H_

class combinations {
 public:
  class iterator {
  public:    
    iterator(std::vector<double>& points, size_t n, bool at_beginning)
      : points(points), points_used(points.size()), past_end(false) {
      if(at_beginning) {
	std::fill(points_used.begin(), points_used.begin() + n, true);
      } else {
	past_end = true;
      }
    }

    std::vector<double> operator*() const {
      std::vector<double> values;
      for(size_t i = 0; i<points_used.size(); i++) {
	if(points_used[i]) {
	  values.push_back(points[i]);
	}
      }
      return values;
    }

    iterator& operator++() {
      past_end = !std::prev_permutation(points_used.begin(), points_used.end());
      return *this;
    }

    iterator operator++(int) {
      iterator temp = *this;
      ++(*this);
      return temp;
    }

    bool operator==(const iterator& other) const {
      if(&points != &other.points ||
	 points_used.size() != other.points_used.size()) {
	return false;
      }
      if(past_end && other.past_end) {
	return true;
      }
      
      for(size_t i = 0; i<points_used.size(); i++) {
	if(points_used[i] != other.points_used[i]) {
	  return false;
	}
      }

      return true;
    }

    bool operator!=(const iterator& other) const {
      return !(*this == other);
    }
    
  private:
    std::vector<double>& points;
    std::vector<bool> points_used;
    bool past_end;
  };
  
  combinations(std::vector<double>& points, size_t n)
    : points(points), n(n) { }

  iterator begin() const {
    return iterator(points, n, true);
  }

  iterator end() const {
    return iterator(points, n, false);
  }
  
 private:
  std::vector<double>& points;
  size_t n;
};

#endif
