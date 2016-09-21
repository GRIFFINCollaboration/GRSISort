#include "TDescantDebug.h"

/// \cond CLASSIMP
ClassImp(TDescantDebug)
/// \endcond

TDescantDebug::TDescantDebug() {
}

TDescantDebug::~TDescantDebug() {
}

void TDescantDebug::Clear(Option_t* opt) {
	fCfdMonitor.clear();
}

