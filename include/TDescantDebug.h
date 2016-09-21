#ifndef DESCANTDEBUG_H
#define DESCANTDEBUG_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>

#include "TObject.h"

class TDescantDebug : public TObject {
	public:
		TDescantDebug();
		~TDescantDebug();

		size_t CfdMonitorSize() { return fCfdMonitor.size(); }

		void CfdMonitor(short val) { fCfdMonitor.push_back(val); }
		void CfdMonitor(std::vector<short> val) { fCfdMonitor = val; }

		short CfdMonitor(size_t i) { return fCfdMonitor.at(i); }
		std::vector<short>& CfdMonitor() { return fCfdMonitor; }

      void Clear(Option_t *opt = "");
	private:
		std::vector<short> fCfdMonitor;
      /// \cond CLASSIMP
      ClassDef(TDescantDebug,1)
      /// \endcond
};
/*! @} */
#endif
