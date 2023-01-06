#ifndef _TUNPACKEDEVENT_H_
#define _TUNPACKEDEVENT_H_

#ifndef __CINT__
#include <type_traits>
#include <memory>
#endif
#include <sstream>

#include "TClass.h"

#include "TDetector.h"

class TFragment;

class TUnpackedEvent {
public:
   TUnpackedEvent();
   ~TUnpackedEvent();

#ifndef __CINT__
   template <typename T>
   std::shared_ptr<T> GetDetector(bool make_if_not_found = false);
   std::shared_ptr<TDetector> GetDetector(TClass* cls, bool make_if_not_found = false);

   std::vector<std::shared_ptr<TDetector>>& GetDetectors() { return fDetectors; }
   void AddDetector(const std::shared_ptr<TDetector>& det) { fDetectors.push_back(det); }
   void AddRawData(const std::shared_ptr<const TFragment>& frag);
   void SetRawData(const std::vector<std::shared_ptr<const TFragment>>& fragments) { fFragments = fragments; }
#endif
   void ClearRawData();

   void Build();

   int Size() { return fDetectors.size(); }

	std::ostringstream Print();

private:
   void BuildHits();

#ifndef __CINT__
   std::vector<std::shared_ptr<const TFragment>> fFragments;
   std::vector<std::shared_ptr<TDetector>>       fDetectors;
#endif
};

#ifndef __CINT__
template <typename T>
std::shared_ptr<T> TUnpackedEvent::GetDetector(bool make_if_not_found)
{
   static_assert(std::is_base_of<TDetector, T>::value, "T must be a subclass of TDetector");
   for(const auto& det : fDetectors) {
      std::shared_ptr<T> output = std::dynamic_pointer_cast<T>(det);
      if(output) {
         return output;
      }
   }

   if(make_if_not_found) {
      std::shared_ptr<T> output = std::make_shared<T>();
      fDetectors.push_back(output);
      return output;
   } else {
      return nullptr;
   }
}
#endif

#endif /* _TUNPACKEDEVENT_H_ */
