#include "TSharcHit.h"

#include "TClass.h"

#include "TSharc.h"
#include "TChannel.h"

/// \cond CLASSIMP
ClassImp(TSharcHit)
/// \endcond

TSharcHit::TSharcHit()
{
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear("ALL");
}

TSharcHit::~TSharcHit() = default;

TSharcHit::TSharcHit(const TSharcHit& rhs) : TGRSIDetectorHit()
{
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
	rhs.Copy(*this);
}

void TSharcHit::Copy(TObject& rhs) const
{
	TGRSIDetectorHit::Copy(rhs);
	static_cast<const TGRSIDetectorHit&>(fBackHit).Copy(static_cast<TObject&>(static_cast<TSharcHit&>(rhs).fBackHit));
	static_cast<const TGRSIDetectorHit&>(fPadHit).Copy(static_cast<TObject&>(static_cast<TSharcHit&>(rhs).fPadHit));

	// static_cast<TSharcHit&>(rhs).fDetectorNumber = fDetectorNumber;
	// static_cast<TSharcHit&>(rhs).fFrontStrip     = fFrontStrip;
	// static_cast<TSharcHit&>(rhs).fBackStrip      = fBackStrip;
}

void TSharcHit::Clear(Option_t* options)
{
	TGRSIDetectorHit::Clear(options); //
	fBackHit.Clear(options);          //
	fPadHit.Clear(options);           //

	// fDetectorNumber = -1;    //
	// fFrontStrip    = -1;    //
	// fBackStrip     = -1;    //
}

void TSharcHit::Print(Option_t*) const
{
	printf(DGREEN "[D/F/B] = %02i\t/%02i\t/%02i " RESET_COLOR "\n", GetDetector(), GetFrontStrip(), GetBackStrip());
	// printf("Sharc hit charge: %02f\n",GetFrontCharge());
	// printf("Sharc hit energy: %f\n",GetDeltaE());
	// printf("Sharc hit time:   %f\n",GetDeltaT());
	// printf( DGREEN "=	=	=	=	=	=	=	" RESET_COLOR "\n");
}

TVector3 TSharcHit::GetPosition(Double_t) const
{
	// return  fposition; // returned from this -> i.e front...
	// PC BENDER PLEASE LOOK AT THIS.
	//
	// this is fine, in all reality this function should not be used in sharc analysis,
	// the buildhits function now properly sets fPosition in the base class, for finer
	// position tweaks of the target, one should just use the static function in the
	// sharc mother class.   pcb.

	return TSharc::GetPosition(TGRSIDetectorHit::GetDetector(), TGRSIDetectorHit::GetSegment(), GetBack().GetSegment(),
			TSharc::GetXOffset(), TSharc::GetYOffset(), TSharc::GetZOffset());
	// return
	// TSharc::GetPosition(fDetectorNumber,fFrontStrip,fBackStrip,TSharc::GetXOffset(),TSharc::GetYOffset(),TSharc::GetZOffset());
}

TVector3 TSharcHit::GetPosition() const
{
	return GetPosition(GetDefaultDistance());
}

Double_t TSharcHit::GetTheta(double Xoff, double Yoff, double Zoff)
{
	TVector3 posOff;
	posOff.SetXYZ(Xoff, Yoff, Zoff);
	return (GetPosition() + posOff).Theta();
}

void TSharcHit::SetFront(const TFragment& frag)
{
	// frag.CopyHit(*this);
	// printf("frag->GetCfd() = %i\n",frag.GetCfd());
	// frag.Print("a");
	frag.Copy(*this);
	// printf("shit->GetCfd() = %i\n",GetCfd());
	// printf("============================================\n");
	// printf("============================================\n");
	// printf("============================================\n");
	// printf("============================================\n"); fflush(stdout);
	// SetPosition(TSharc::GetPosition(TGRSIDetector::GetDetector(),
	//                           TGRSIDetector::GetSegment(),
	//                           GetBack()->GetSegment(),
	//                           TSharc::GetXOffset(),TSharc::GetYOffset(),TSharc::GetZOffset());
}

void TSharcHit::SetBack(const TFragment& frag)
{
	// frag.CopyHit(fBackHit);
	//  fBackHit.Copy(frag);
	frag.Copy(fBackHit);
}

void TSharcHit::SetPad(const TFragment& frag)
{
	//  frag.CopyHit(fPadHit);
	//  fPadHit.Copy(frag);
	frag.Copy(fPadHit);
}
