#ifndef COMBINATIONS_H
#define COMBINATIONS_H
#include <vector>
#include <algorithm>

class combinations {
public:
   class iterator {
   public:
      iterator(std::vector<double>& points, size_t n, bool at_beginning)
         : fPoints(points), fPointsUsed(fPoints.size())
      {
         if(at_beginning) {
            std::fill(fPointsUsed.begin(), fPointsUsed.begin() + n, true);
         } else {
            fPastEnd = true;
         }
      }

      std::vector<double> operator*() const
      {
         std::vector<double> values;
         for(size_t i = 0; i < fPointsUsed.size(); i++) {
            if(fPointsUsed[i]) {
               values.push_back(fPoints[i]);
            }
         }
         return values;
      }

      iterator& operator++()
      {
         fPastEnd = !std::prev_permutation(fPointsUsed.begin(), fPointsUsed.end());
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
         if(&fPoints != &other.fPoints || fPointsUsed.size() != other.fPointsUsed.size()) {
            return false;
         }
         if(fPastEnd && other.fPastEnd) {
            return true;
         }

         for(size_t i = 0; i < fPointsUsed.size(); i++) {
            if(fPointsUsed[i] != other.fPointsUsed[i]) {
               return false;
            }
         }

         return true;
      }

      bool operator!=(const iterator& other) const { return !(*this == other); }

   private:
      std::vector<double>& fPoints;
      std::vector<bool>    fPointsUsed;
      bool                 fPastEnd{false};
   };

   combinations(std::vector<double>& points, size_t n) : fPoints(points), fN(n) {}

   iterator begin() const { return {fPoints, fN, true}; }

   iterator end() const { return {fPoints, fN, false}; }

private:
   std::vector<double>& fPoints;
   size_t               fN;
};

#endif
