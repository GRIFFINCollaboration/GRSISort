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
   // to copy the hits without creating a memory leak we need to check
   // if the right-hand side has more hits than this 
   // if so, we need to delete the hits pointed to by the right-hand side
   auto& hits = static_cast<TDetector&>(rhs).fHits;
   if(hits.size() > fHits.size()) {
      for(size_t i = fHits.size(); i < hits.size(); ++i) {
         delete hits[i];
      }
      hits.resize(fHits.size());
   } else if(hits.size() < fHits.size()) {
      // right-hand side has less hits, that means there is at least one we can use to determine the type
      // we need to use IsA()->New() to make a new hit of whatever derived type this actually is
      hits.resize(fHits.size(), static_cast<TDetectorHit*>(fHits[0]->IsA()->New()));
   }
   // we have now ensured that the size of the two vectors is the same, so we can copy the contents of the hits
   for(size_t i = 0; i < fHits.size(); ++i) {
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
