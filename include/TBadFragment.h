#ifndef TBADFRAGMENT_H
#define TBADFRAGMENT_H

/** \addtogroup Sorting
 *  @{
 */

#include "TFragment.h"

/////////////////////////////////////////////////////////////////
///
/// \class TBadFragment
///
/// This Class contains all of the information in an event
/// fragment that wasn't parsed correctly plus which word the
/// parsing failed on and the data
///
/// \author Vinzenz Bildstein
/// \date 20. April 2017
/////////////////////////////////////////////////////////////////

class TBadFragment : public TFragment {
public:
   TBadFragment();
   TBadFragment(TFragment& fragment, uint32_t* data, int size, int failedWord, bool multipleErrors);
   TBadFragment(const TBadFragment&);
   ~TBadFragment() override;

   std::vector<uint32_t> GetData() const { return fData; }
   int                   GetFailedWord() const { return fFailedWord; }
   bool                  GetMultipleErrors() const { return fMultipleErrors; }

   void Print(Option_t* opt = "") const override;

private:
   std::vector<uint32_t> fData;
   int                   fFailedWord{};
   bool                  fMultipleErrors{};

   /// \cond CLASSIMP
   ClassDefOverride(TBadFragment, 1);
   /// \endcond
};
/*! @} */
#endif
