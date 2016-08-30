#ifndef _TUNPACKEDEVENT_H_
#define _TUNPACKEDEVENT_H_

#ifndef __CINT__
#include <type_traits>
#endif

#include "TClass.h"

#include "TDetector.h"

class TFragment;

class TUnpackedEvent {
public:
  TUnpackedEvent();
  ~TUnpackedEvent();

  template<typename T>
  T* GetDetector(bool make_if_not_found = false);

  TDetector* GetDetector(TClass* cls, bool make_if_not_found = false);

  std::vector<TDetector*>& GetDetectors() { return detectors; }
  void AddDetector(TDetector* det) { detectors.push_back(det); }
  void AddRawData(TFragment* frag);
  void ClearRawData();

  void Build();

  int Size() { return detectors.size(); }

private:
  void BuildHits();

  std::vector<TFragment*> fragments;
  std::vector<TDetector*> detectors;
};

#ifndef __CINT__
template<typename T>
T* TUnpackedEvent::GetDetector(bool make_if_not_found) {
  static_assert(std::is_base_of<TDetector, T>::value,
                "T must be a subclass of TDetector");
  for(auto det : detectors) {
    T* output = dynamic_cast<T*>(det);
    if(output){
      return output;
    }
  }

  if(make_if_not_found) {
    T* output = new T;
    detectors.push_back(output);
    return output;
  } else {
    return NULL;
  }
}
#endif

#endif /* _TUNPACKEDEVENT_H_ */
