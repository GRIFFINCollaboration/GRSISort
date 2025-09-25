#include "TDetector.h"
#include "TClass.h"

TDetector::TDetector(const TDetector& rhs) : TObject(rhs)
{
   /// Default Copy constructor.
   rhs.Copy(*this);
}

TDetector::TDetector(TDetector&& rhs) noexcept : TObject(rhs)
{
   /// Default Move constructor.
   rhs.Copy(*this);
}

TDetector::~TDetector()
{
   /// Default Destructor.
   for(auto* hit : fHits) {
      delete hit;
   }
}

void TDetector::Copy(TObject& rhs) const
{
   TObject::Copy(rhs);
   auto& hits = static_cast<TDetector&>(rhs).fHits;
   hits.resize(fHits.size(), nullptr);
   for(size_t i = 0; i < fHits.size(); ++i) {
      // if this is a new hit, allocate it first before copying to it
      if(hits[i] == nullptr) {
         // we need to use IsA()->New() to make a new hit of whatever derived type this actually is
         hits[i] = static_cast<TDetectorHit*>(fHits[i]->IsA()->New());
      }
      fHits[i]->Copy(*(hits[i]), true);
   }
}

void TDetector::Print(Option_t*) const
{
   /// Default print statement for TDetector.
   Print(std::cout);
}

void TDetector::Print(std::ostream& out) const
{
   /// Print detector to stream out. Iterates over hits and prints them.
   std::ostringstream str;
   str << "TDetector " << this << ":" << std::endl;
   for(auto* hit : fHits) {
      hit->Print(str);
   }
   out << str.str();
}

void TDetector::Clear(Option_t* opt)
{
   if(strcmp(opt, "a") == 0) {
      for(auto* hit : fHits) {
         delete hit;
      }
   }
   fHits.clear();
}

void TDetector::ClearTransients()
{
   for(auto* hit : fHits) {
      hit->ClearTransients();
   }
}

TDetectorHit* TDetector::GetHit(const int& index) const
{
   try {
      return fHits.at(index);
   } catch(const std::out_of_range& oor) {
      std::cerr << ClassName() << " is out of range: " << oor.what() << std::endl;
      throw grsi::exit_exception(1);
   }
   return nullptr;
}
