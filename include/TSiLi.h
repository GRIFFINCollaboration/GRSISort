#ifndef TSILI_H
#define TSILI_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <iostream>

#include "TGRSIDetector.h"
#include "TSiLiHit.h"
#include "TGRSIRunInfo.h"

class TSiLi: public TGRSIDetector  {
	public:
		TSiLi();
		TSiLi(const TSiLi&);
		virtual ~TSiLi();

#ifndef __CINT__
      void AddFragment(std::shared_ptr<TFragment>, TChannel*); //!<!
#endif

		TSiLi& operator=(const TSiLi&);  // 

		void Copy(TObject&) const;
		void Clear(Option_t *opt="");   
		void Print(Option_t *opt="") const;

		Short_t GetMultiplicity() const { return fSiLiHits.size(); }
		TGRSIDetectorHit* GetHit(const Int_t& idx =0);
		TSiLiHit* GetSiLiHit(const Int_t& idx = 0);
		
		static TVector3 GetPosition(int segment);

	private:
		std::vector<TSiLiHit> fSiLiHits;

/// \cond CLASSIMP
		ClassDef(TSiLi,3);
/// \endcond
};
/*! @} */
#endif
