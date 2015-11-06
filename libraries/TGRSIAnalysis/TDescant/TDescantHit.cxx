#include "TDescantHit.h"

#include <iostream>
#include <algorithm>

#include "Globals.h"
#include "TDescant.h"

/// \cond CLASSIMP
ClassImp(TDescantHit)
/// \endcond

TDescantHit::TDescantHit()	{	
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
}

TDescantHit::~TDescantHit() { }

TDescantHit::TDescantHit(const TDescantHit &rhs) : TGRSIDetectorHit() {
	Clear();
	rhs.Copy(*this);
}

void TDescantHit::Copy(TObject &rhs) const {
	TGRSIDetectorHit::Copy(rhs);
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	static_cast<TDescantHit&>(rhs).fFilter  = fFilter;
	static_cast<TDescantHit&>(rhs).fZc      = fZc;
	static_cast<TDescantHit&>(rhs).fCcShort = fCcShort;
	static_cast<TDescantHit&>(rhs).fCcLong  = fCcLong;
	static_cast<TDescantHit&>(rhs).fPsd     = fPsd;
}

TVector3 TDescantHit::GetChannelPosition(double dist) const {
	///This should not be called by the user. Instead use 
	///TGRSIDetectorHit::GetPosition
	return TDescant::GetPosition(GetDetector());
}

bool TDescantHit::InFilter(Int_t wantedfilter) {
	/// check if the desired filter is in wanted filter;
	/// return the answer;
	return true;
}

void TDescantHit::Clear(Option_t *opt)	{
	fFilter  = 0;
	fPsd     = -1;
	fZc      = 0;
	fCcShort = 0;
	fCcLong  = 0;
	TGRSIDetectorHit::Clear();
}

void TDescantHit::Print(Option_t *opt) const	{
	printf("Descant Detector: %i\n",GetDetector());
	printf("Descant hit energy: %.2f\n",GetEnergy());
	printf("Descant hit time:   %.f\n",GetTime());
}

bool TDescantHit::AnalyzeWaveform() {
	bool error = false;

	std::vector<Short_t>* waveform = GetWaveform();
	std::vector<Int_t> baselineCorrections(8, 0);
	std::vector<Short_t> smoothedWaveform;

	// all timing algorithms use interpolation with this many steps between two samples (all times are stored as integers)
	int interpolationSteps = 256;
	int delay = 8;
	double attenuation = 24./64.;
	int halfSmoothingWindow = 0; //2*halfsmoothingwindow + 1 = number of samples in moving window.

	// baseline algorithm: correct each adc with average of first two samples in that adc
	for(size_t i = 0; i < 8 && i < waveform->size(); ++i) {
		baselineCorrections[i] = (*waveform)[i];
	}
	for(size_t i = 8; i < 16 && i < waveform->size(); ++i) {
		baselineCorrections[i-8] = ((baselineCorrections[i-8] + (*waveform)[i]) + ((baselineCorrections[i-8] + (*waveform)[i]) > 0 ? 1 : -1)) >> 1;
	}
	for(size_t i = 0; i < waveform->size(); ++i) {
		(*waveform)[i] -= baselineCorrections[i%8];
	}

	this->SetCfd(CalculateCfd(attenuation, delay, halfSmoothingWindow, interpolationSteps));

	// PSD
	// time to zero-crossing algorithm: time when sum reaches n% of the total sum minus the cfd time
	double fraction = 0.85;

	fPsd = CalculatePsd(fraction, interpolationSteps);

	if(fPsd < 0) {
		error = true;
	}

	return !error;
}

Int_t TDescantHit::CalculateCfd(double attenuation, unsigned int delay, int halfSmoothingWindow, int interpolationSteps) {
	std::vector<Short_t> monitor;

	return CalculateCfdAndMonitor(attenuation, delay, halfSmoothingWindow, interpolationSteps, monitor);
}

Int_t TDescantHit::CalculateCfdAndMonitor(double attenuation, unsigned int delay, int halfSmoothingWindow, int interpolationSteps, std::vector<Short_t> &monitor) {
	Short_t monitormax = 0;

	bool armed = false;

	Int_t cfd = 0;
	std::vector<Short_t>* waveform = GetWaveform();
	std::vector<Short_t> smoothedWaveform;

	if(waveform->size() > delay+1) {
		if(halfSmoothingWindow > 0) {
			smoothedWaveform = TDescantHit::CalculateSmoothedWaveform(halfSmoothingWindow);
		} else {
			smoothedWaveform = *waveform;
		}

		monitor.resize(smoothedWaveform.size()-delay);
		monitor[0] = attenuation*smoothedWaveform[delay]-smoothedWaveform[0];
		if(monitor[0] > monitormax) {
			armed = true;
			monitormax = monitor[0];
		}

		for(size_t i = delay + 1; i < smoothedWaveform.size(); ++i) {
			monitor[i-delay] = attenuation*smoothedWaveform[i]-smoothedWaveform[i-delay];
			if(monitor[i-delay] > monitormax) {
				armed=true;
				monitormax = monitor[i-delay];
			} else {
				if(armed == true && monitor[i-delay] < 0) {
					armed = false;
					if(monitor[i-delay-1] - monitor[i-delay] != 0) {
						//Linear interpolation.
						cfd = (i-delay)*interpolationSteps + (monitor[i-delay-1]*interpolationSteps)/(monitor[i-delay-1]-monitor[i-delay]);
					} else {
						//Should be impossible, since monitor[i-delay-1] => 0 and monitor[i-delay] > 0
						cfd = 0;
					}
				}
			}
		}
	} else {
		monitor.resize(0);
	}

	return cfd;
}

std::vector<Short_t> TDescantHit::CalculateSmoothedWaveform(unsigned int halfSmoothingWindow) {
	std::vector<Short_t>* waveform = GetWaveform();
	std::vector<Short_t> smoothedWaveform(std::max((size_t)0, waveform->size()-2*halfSmoothingWindow), 0);

	for(size_t i = halfSmoothingWindow; i < waveform->size() - halfSmoothingWindow; ++i) {
		for(int j = -static_cast<int>(halfSmoothingWindow); j <= static_cast<int>(halfSmoothingWindow); ++j) {
			smoothedWaveform[i-halfSmoothingWindow] += (*waveform)[i+j];
		}
	}

	return smoothedWaveform;
}

std::vector<Short_t> TDescantHit::CalculateCfdMonitor(double attenuation, int delay, int halfSmoothingWindow) {
	std::vector<Short_t>* waveform = GetWaveform();
	std::vector<Short_t> monitor(std::max(static_cast<size_t>(0), waveform->size()-delay), 0);
	std::vector<Short_t> smoothedWaveform;

	if(halfSmoothingWindow > 0) {
		smoothedWaveform = TDescantHit::CalculateSmoothedWaveform(halfSmoothingWindow);
	} else {
		smoothedWaveform = *waveform;
	}

	for(size_t i = delay; i < waveform->size(); ++i) {
		monitor[i-delay] = attenuation*smoothedWaveform[i]-smoothedWaveform[i-delay];
	}

	return monitor;
}

std::vector<Int_t> TDescantHit::CalculatePartialSum() {
	std::vector<Short_t>* waveform = GetWaveform();
	std::vector<Int_t> partialSums(waveform->size(), 0);

	if(waveform->size() > 0) {
		partialSums[0] = waveform->at(0);
		for(size_t i = 1; i < waveform->size(); ++i) {
			partialSums[i] = partialSums[i-1] + (*waveform)[i];
		}
	}
	return partialSums;
}

Int_t TDescantHit::CalculatePsd(double fraction, int interpolationSteps) {
	std::vector<Int_t> partialSums;

	return CalculatePsdAndPartialSums(fraction, interpolationSteps, partialSums);
}

Int_t TDescantHit::CalculatePsdAndPartialSums(double fraction, int interpolationSteps, std::vector<Int_t>& partialSums) {
	Int_t psd;
	std::vector<Short_t> *waveform = GetWaveform();
	partialSums = CalculatePartialSum();
	if(partialSums.empty()) {
		return -1;
	}
	int totalSum = partialSums.back();

	fPsd = -1;
	if(partialSums[0] < fraction*totalSum) {
		for(size_t i = 1; i < partialSums.size(); ++i) {
			if(partialSums[i] >= fraction*totalSum) {
				fPsd = i*interpolationSteps - ((partialSums[i]-fraction*totalSum)*interpolationSteps)/(*waveform)[i];
				break;
			}
		}
	}

	return psd;
}
