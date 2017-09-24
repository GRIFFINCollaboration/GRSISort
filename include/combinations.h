#ifndef _COMBINATIONS_H_
#define _COMBINATIONS_H_

class combinations {
public:
   class iterator {
   public:
      iterator(std::vector<double>& points, size_t n, bool at_beginning)
         : fPoints(points), fPoints_used(fPoints.size()), fPast_end(false)
      {
         if(at_beginning) {
            std::fill(fPoints_used.begin(), fPoints_used.begin() + n, true);
         } else {
            fPast_end = true;
         }
      }

      std::vector<double> operator*() const
      {
         std::vector<double> values;
         for(size_t i = 0; i < fPoints_used.size(); i++) {
            if(fPoints_used[i]) {
               values.push_back(fPoints[i]);
            }
         }
         return values;
      }

      iterator& operator++()
      {
         fPast_end = !std::prev_permutation(fPoints_used.begin(), fPoints_used.end());
         return *this;
      }

      iterator operator++(int)
      {
         iterator temp = *this;
         ++(*this);
         return temp;
      }

      bool operator==(const iterator& other) const
      {
         if(&fPoints != &other.fPoints || fPoints_used.size() != other.fPoints_used.size()) {
            return false;
         }
         if(fPast_end && other.fPast_end) {
            return true;
         }

         for(size_t i = 0; i < fPoints_used.size(); i++) {
            if(fPoints_used[i] != other.fPoints_used[i]) {
               return false;
            }
         }

         return true;
      }

      bool operator!=(const iterator& other) const { return !(*this == other); }

   private:
      std::vector<double>& fPoints;
      std::vector<bool>    fPoints_used;
      bool                 fPast_end;
   };

   combinations(std::vector<double>& points, size_t n) : fPoints(points), fN(n) {}

   iterator begin() const { return iterator(fPoints, fN, true); }

   iterator end() const { return iterator(fPoints, fN, false); }

private:
   std::vector<double>& fPoints;
   size_t               fN;
};

#endif
