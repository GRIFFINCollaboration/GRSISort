#include "TCSM.h"
#include "TMath.h"
#define RECOVERHITS 1
#define SUMHITS 0

/// \cond CLASSIMP
ClassImp(TCSM)
/// \endcond

int TCSM::fCfdBuildDiff = 5;

TCSM::TCSM() {
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	//InitializeSRIMInputs();
	fAlmostEqualWindow = .2;
}

TCSM::~TCSM() {
}

void TCSM::AddFragment(TFragment* frag, TChannel* chan) {
	///This function just stores the fragments and mnemonics in vectors, separated by detector number and type (horizontal/vertical strip or pad).
	///The hits themselves are built in the BuildHits function because the way we build them depends on the number of hits.

	//first index: detector number, second index: 0 = deltaE, 1 = E; third index: 0 = horizontal, 1 = vertical; fourth index: fragments
	int type = -1;
	if(chan->GetMnemonic()->arraysubposition.compare(0,1,"D") == 0) {
		type = 0;
	} else if(chan->GetMnemonic()->arraysubposition.compare(0,1,"E") == 0) {
		type = 1;
	}
	int orientation = -1;
	if(chan->GetMnemonic()->collectedcharge.compare(0,1,"N") == 0) {
		//N =  Horizontal Strips. aka "front"
		orientation = 0;
	} else if(chan->GetMnemonic()->collectedcharge.compare(0,1,"P") == 0) {
		//P = Vertical Strips. aka "back"
		orientation = 1;
	}

	if(type < 0 || orientation < 0) {
		return;
	}

	//if this is the first time we got this detector number we make a new vector (of a vector) of fragments
	if(fFragments.find(chan->GetMnemonic()->arrayposition) == fFragments.end()) {
		fFragments[chan->GetMnemonic()->arrayposition].resize(2,std::vector<std::vector<std::pair<TFragment, MNEMONIC> > >(2));
	}

	fFragments[chan->GetMnemonic()->arrayposition][type][orientation].push_back(std::make_pair(*frag,*(chan->GetMnemonic())));
}

void TCSM::BuildHits() {
	///This function takes the fragments that were stored in the successive AddFragment calls and builds hits out of them
	std::vector<std::vector<TCSMHit> > hits(2);

	//first index: detector number, second index: 0 = deltaE, 1 = E; third index: 0 = horizontal, 1 = vertical
	//loop over all found detectors
	for(auto detIt = fFragments.begin(); detIt != fFragments.end(); ++detIt) {
		//loop over all types (only detectors 1/2 should have both D and E, detectors 3/4 should only have D)
		for(size_t i = 0; i < detIt->second.size(); ++i) {
			BuildVH(detIt->second.at(i), hits[i]);
		}
	} 
	BuilddEE(hits,fCsmHits);
}


void TCSM::Clear(Option_t *option) {
	fCsmHits.clear();
}

void TCSM::Print(Option_t *option) const {
	printf("not yet written...\n");
}


TVector3 TCSM::GetPosition(int detector,char pos, int horizontalstrip, int verticalstrip, double X, double Y, double Z) {
	//horizontal strips collect N charge!
	//vertical strips collect P charge!
	TVector3 Pos;
	double SideX = 68;
	double SideZ = -4.8834;
	double dEX = 54.9721;
	double dEZ = 42.948977;
	double EX = 58.062412;
	double EZ = 48.09198;
	double detTheta = 31. * (TMath::Pi()/180.);
	double x = 0.0,y = 0.0,z = 0.0;

	if(detector==3&&pos=='D') {
		//Right Side
		verticalstrip=15-verticalstrip;
		x = SideX;
		z = SideZ + (50./32.)*(2*verticalstrip+1);
	} else if(detector==4&&pos=='D') {
		//Left Side
		x = -SideX;
		z = SideZ + (50./32.)*(2*verticalstrip+1);
	} else if(detector==1&&pos=='D') {
		//Right dE
		verticalstrip=15-verticalstrip;
		x = dEX - (50./32.)*cos(detTheta)*(2*verticalstrip+1);
		z = dEZ + (50./32.)*sin(detTheta)*(2*verticalstrip+1);
	} else if(detector==2&&pos=='D') {
		//Left dE
		x = -dEX + (50./32.)*cos(detTheta)*(2*verticalstrip+1);
		z = dEZ + (50./32.)*sin(detTheta)*(2*verticalstrip+1);
	} else if(detector==1&&pos=='E') {
		//Right E
		x = EX - (50./32.)*cos(detTheta)*(2*verticalstrip+1);
		z = EZ + (50./32.)*sin(detTheta)*(2*verticalstrip+1);
	} else if(detector==2&&pos=='E') {
		//Left E
		verticalstrip=15-verticalstrip;
		x = -EX + (50./32.)*cos(detTheta)*(2*verticalstrip+1);
		z = EZ + (50./32.)*sin(detTheta)*(2*verticalstrip+1);
	} else {
		printf("***Error, unrecognized detector and position combo!***\n");
	}

	y = (50./32.)*(2*horizontalstrip+1) - (50/16.)*8;
	Pos.SetX(x + X);
	Pos.SetY(y + Y);
	Pos.SetZ(z+ Z);

	return Pos;
}

void TCSM::BuildVH(std::vector<std::vector<std::pair<TFragment, MNEMONIC> > >& strips,std::vector<TCSMHit>& hitVector) {
	///Build hits from horizontal (index = 0) and vertical (index = 1) strips into the hitVector
	if(strips[0].size() == 0 && strips[1].size() == 0) {
		return;
	} else if(strips[0].size() == 1 && strips[1].size() == 0) {
		RecoverHit('H',strips[0][0],hitVector);
	} else if(strips[0].size() == 0 && strips[1].size() == 1) {
		RecoverHit('V',strips[1][0],hitVector);
	} else if(strips[0].size() == 1 && strips[1].size() == 1) {    
		hitVector.push_back(MakeHit(strips[0][0],strips[1][0]));
	} else if(strips[1].size() == 1 && strips[0].size() == 2) {
		int he1 = strips[0][0].first.GetEnergy();
		int he2 = strips[0][1].first.GetEnergy();
		int ve1 = strips[1][0].first.GetEnergy();
		if(AlmostEqual(ve1,he1+he2) && SUMHITS) {
			hitVector.push_back(MakeHit(strips[0],strips[1]));
		} else if(AlmostEqual(ve1,he1)) {
			hitVector.push_back(MakeHit(strips[0][0],strips[1][0]));
			RecoverHit('H',strips[0][1],hitVector);
		} else if(AlmostEqual(ve1,he2)) {
			hitVector.push_back(MakeHit(strips[0][1],strips[1][0]));
			RecoverHit('H',strips[0][0],hitVector);
		}
	} else if(strips[1].size() == 2 && strips[0].size() == 1) {
		int he1 = strips[0][0].first.GetEnergy();
		int ve1 = strips[1][0].first.GetEnergy();
		int ve2 = strips[1][1].first.GetEnergy();
		if(AlmostEqual(ve1+ve2,he1) && SUMHITS) {
			hitVector.push_back(MakeHit(strips[0],strips[1]));
		} else if(AlmostEqual(ve1,he1)) {
			hitVector.push_back(MakeHit(strips[0][0],strips[1][0]));
			RecoverHit('V',strips[1][1],hitVector);
		} else if(AlmostEqual(ve2,he1)) {
			hitVector.push_back(MakeHit(strips[0][0],strips[1][1]));
			RecoverHit('V',strips[1][0],hitVector);
		}
	} else if(strips[1].size() == 2 && strips[0].size() == 2) {    
		int he1 = strips[0][0].first.GetEnergy();
		int he2 = strips[0][1].first.GetEnergy();
		int ve1 = strips[1][0].first.GetEnergy();
		int ve2 = strips[1][1].first.GetEnergy();
		if( (AlmostEqual(ve1,he1) && AlmostEqual(ve2,he2)) || (AlmostEqual(ve1,he2) && AlmostEqual(ve2,he1))) {
			//I can build both 1,1 and 2,2 or 1,2 and 2,1
			if(std::abs(ve1-he1)+std::abs(ve2-he2) <= std::abs(ve1-he2)+std::abs(ve2-he1)) {
				//1,1 and 2,2 mimimizes difference
				hitVector.push_back(MakeHit(strips[0][0],strips[1][0]));
				hitVector.push_back(MakeHit(strips[0][1],strips[1][1]));
			} else if(std::abs(ve1-he1)+std::abs(ve2-he2) > std::abs(ve1-he2)+std::abs(ve2-he1)) {
				//1,2 and 2,1 mimimizes difference
				hitVector.push_back(MakeHit(strips[0][0],strips[1][1]));
				hitVector.push_back(MakeHit(strips[0][1],strips[1][0]));
			}
		} else if( AlmostEqual(ve1,he1) ) {
			hitVector.push_back(MakeHit(strips[0][0],strips[1][0]));
		} else if( AlmostEqual(ve2,he1) ) {
			hitVector.push_back(MakeHit(strips[0][1],strips[1][0]));
		} else if( AlmostEqual(ve1,he2) ) {
			hitVector.push_back(MakeHit(strips[0][0],strips[1][1]));
		} else if( AlmostEqual(ve2,he2) ) {
			hitVector.push_back(MakeHit(strips[0][1],strips[1][1]));
		}
	}
}

TCSMHit TCSM::MakeHit(std::pair<TFragment, MNEMONIC>& h, std::pair<TFragment, MNEMONIC>& v) {
	TCSMHit csmHit;

	if(h.second.arrayposition != v.second.arrayposition) {
		std::cerr<<"\tSomething is wrong, Horizontal and Vertical detector numbers don't match."<<std::endl;
	}
	if(h.second.arraysubposition.c_str()[0] != v.second.arraysubposition.c_str()[0]) {
		std::cerr<<"\tSomething is wrong, Horizontal and Vertical positions don't match."<<std::endl;
	}


	if(h.second.arraysubposition[0] == 'D') {
		csmHit.SetDetectorNumber(h.second.arrayposition);
		csmHit.SetDHorizontalCharge(h.first.GetCharge());
		csmHit.SetDVerticalCharge(v.first.GetCharge());
		csmHit.SetDHorizontalStrip(h.second.segment);
		csmHit.SetDVerticalStrip(v.second.segment);
		csmHit.SetDHorizontalCFD(h.first.GetCfd());
		csmHit.SetDVerticalCFD(v.first.GetCfd());
		csmHit.SetDHorizontalTime(h.first.GetTimeStamp());
		csmHit.SetDVerticalTime(v.first.GetTimeStamp());
		csmHit.SetDHorizontalEnergy(h.first.GetEnergy());
		csmHit.SetDVerticalEnergy(v.first.GetEnergy());
		csmHit.SetDPosition(TCSM::GetPosition(h.second.arrayposition,
					h.second.arraysubposition[0],
					h.second.segment,
					v.second.segment));
	} else if(h.second.arraysubposition.c_str()[0] == 'E') {
		csmHit.SetDetectorNumber(h.second.arrayposition);
		csmHit.SetEHorizontalCharge(h.first.GetCharge());
		csmHit.SetEVerticalCharge(v.first.GetCharge());
		csmHit.SetEHorizontalStrip(h.second.segment);
		csmHit.SetEVerticalStrip(v.second.segment);
		csmHit.SetEHorizontalCFD(h.first.GetCfd());
		csmHit.SetEVerticalCFD(v.first.GetCfd());
		csmHit.SetEHorizontalTime(h.first.GetTimeStamp());
		csmHit.SetEVerticalTime(v.first.GetTimeStamp());
		csmHit.SetEHorizontalEnergy(h.first.GetEnergy());
		csmHit.SetEVerticalEnergy(v.first.GetEnergy());
		csmHit.SetEPosition(TCSM::GetPosition(h.second.arrayposition,
					h.second.arraysubposition[0],
					h.second.segment,
					v.second.segment));
	}

	return csmHit;
}

TCSMHit TCSM::MakeHit(std::vector<std::pair<TFragment, MNEMONIC> >& hhV,std::vector<std::pair<TFragment, MNEMONIC> >& vvV) {
	TCSMHit csmHit;

	if(hhV.size() == 0 || vvV.size() == 0) {
		std::cerr<<"\tSomething is wrong, empty vector in MakeHit"<<std::endl;
	}

	//-------------------- horizontal strips
	int DetNumH = hhV[0].second.arrayposition;
	char DetPosH = hhV[0].second.arraysubposition[0];
	int ChargeH = hhV[0].first.GetCharge();
	double EnergyH = hhV[0].first.GetEnergy();
	int biggestH = 0;

	//get accumulative charge/energy and find the strip with the highest charge (why not energy?)
	for(size_t i = 1; i < hhV.size(); ++i) {
		if(hhV[i].first.GetCharge() > hhV[biggestH].first.GetCharge()) {
			biggestH = i;
		}

		if(hhV[i].second.arrayposition != DetNumH) {
			std::cerr<<"\tSomething is wrong, Horizontal detector numbers don't match in vector loop."<<std::endl;
		}
		if(hhV[i].second.arraysubposition[0] != DetPosH) {
			std::cerr<<"\tSomething is wrong, Horizontal detector positions don't match in vector loop."<<std::endl;
		}
		ChargeH += hhV[i].first.GetCharge();
		EnergyH += hhV[i].first.GetEnergy();
	}

	int StripH = hhV[biggestH].second.segment;
	int ConFraH = hhV[biggestH].first.GetCfd();
	double TimeH = hhV[biggestH].first.GetTimeStamp();

	//-------------------- vertical strips
	int DetNumV = vvV[0].second.arrayposition;
	char DetPosV = vvV[0].second.arraysubposition[0];
	int ChargeV = vvV[0].first.GetCharge();
	double EnergyV = vvV[0].first.GetEnergy();
	int biggestV = 0;

	//get accumulative charge/energy and find the strip with the highest charge (why not energy?)
	for(size_t i = 1; i < vvV.size(); ++i) {
		if(vvV[i].first.GetCharge() > vvV[biggestV].first.GetCharge()) {
			biggestV = i;
		}

		if(vvV[i].second.arrayposition != DetNumV) {
			std::cerr<<"\tSomething is wrong, Vertical detector numbers don't match in vector loop."<<std::endl;
		}
		if(vvV[i].second.arraysubposition[0] != DetPosV) {
			std::cerr<<"\tSomething is wrong, Vertical detector positions don't match in vector loop."<<std::endl;
		}
		ChargeV += vvV[i].first.GetCharge();
		EnergyV += vvV[i].first.GetEnergy();
	}

	int StripV = vvV[biggestV].second.segment;
	int ConFraV = vvV[biggestV].first.GetCfd();
	double TimeV = vvV[biggestV].first.GetTimeStamp();

	if(DetNumH != DetNumV) {
		std::cerr<<"\tSomething is wrong, Horizontal and Vertical detector numbers don't match in vector."<<std::endl;
	}
	if(DetPosH != DetPosV) {
		std::cerr<<"\tSomething is wrong, Horizontal and Vertical positions don't match in vector."<<std::endl;
	}

	if(DetPosH == 'D') {
		csmHit.SetDetectorNumber(DetNumH);
		csmHit.SetDHorizontalCharge(ChargeH);
		csmHit.SetDVerticalCharge(ChargeV);
		csmHit.SetDHorizontalStrip(StripH);
		csmHit.SetDVerticalStrip(StripV);
		csmHit.SetDHorizontalCFD(ConFraH);
		csmHit.SetDVerticalCFD(ConFraV);
		csmHit.SetDHorizontalTime(TimeH);
		csmHit.SetDVerticalTime(TimeV);
		csmHit.SetDHorizontalEnergy(EnergyH);
		csmHit.SetDVerticalEnergy(EnergyV);
		csmHit.SetDPosition(TCSM::GetPosition(DetNumH,
					DetPosH,
					StripH,
					StripV));
	} else if(DetPosH == 'E') {
		csmHit.SetDetectorNumber(DetNumH);
		csmHit.SetEHorizontalCharge(ChargeH);
		csmHit.SetEVerticalCharge(ChargeV);
		csmHit.SetEHorizontalStrip(StripH);
		csmHit.SetEVerticalStrip(StripV);
		csmHit.SetEHorizontalCFD(ConFraH);
		csmHit.SetEVerticalCFD(ConFraV);
		csmHit.SetEHorizontalTime(TimeH);
		csmHit.SetEVerticalTime(TimeV);
		csmHit.SetEHorizontalEnergy(EnergyH);
		csmHit.SetEVerticalEnergy(EnergyV);
		csmHit.SetEPosition(TCSM::GetPosition(DetNumH,
					DetPosH,
					StripH,
					StripV));
	}

	return(csmHit);
}

void TCSM::BuilddEE(std::vector<std::vector<TCSMHit> >& hitVec,std::vector<TCSMHit>& builtHits) {
	std::vector<TCSMHit> d1;
	std::vector<TCSMHit> d2;
	std::vector<TCSMHit> e1;
	std::vector<TCSMHit> e2;

	for(size_t i = 0; i < hitVec[0].size(); i++) {
		if(hitVec[0][i].GetDetectorNumber() == 3 || hitVec[0][i].GetDetectorNumber() == 4) { //I am in side detectors
			//I will never have a pair in the side detector, so go ahead and send it through.
			builtHits.push_back(hitVec[0][i]);
		} else if(hitVec[0][i].GetDetectorNumber() == 1) {
			d1.push_back(hitVec[0][i]);
		} else if(hitVec[0][i].GetDetectorNumber() == 2) {
			d2.push_back(hitVec[0][i]);
		} else {
			std::cerr<<"  Caution, in BuilddEE detector number in D vector is out of bounds."<<std::endl;
		}
	}

	for(size_t i = 0; i < hitVec[1].size(); ++i) {
		if(hitVec[1][i].GetDetectorNumber() == 1) {
			e1.push_back(hitVec[1][i]);
		} else if(hitVec[1][i].GetDetectorNumber() == 2) {
			e2.push_back(hitVec[1][i]);
		} else {
			std::cerr<<"  Caution, in BuilddEE detector number in E vector is out of bounds."<<std::endl;
		}
	}

	MakedEE(d1,e1,builtHits);
	MakedEE(d2,e2,builtHits);
}

void TCSM::MakedEE(std::vector<TCSMHit>& DHitVec,std::vector<TCSMHit>& EHitVec,std::vector<TCSMHit>& BuiltHits) {
	if(DHitVec.size()==0 && EHitVec.size()==0)
		return;
	else if(DHitVec.size()==1 && EHitVec.size()==0)
		BuiltHits.push_back(DHitVec.at(0));
	else if(DHitVec.size()==0 && EHitVec.size()==1)
		BuiltHits.push_back(EHitVec.at(0));
	else if(DHitVec.size()==1 && EHitVec.size()==1)
		BuiltHits.push_back(CombineHits(DHitVec.at(0),EHitVec.at(0)));
	else if(DHitVec.size()==2 && EHitVec.size()==0) {
		BuiltHits.push_back(DHitVec.at(0));
		BuiltHits.push_back(DHitVec.at(1));
	} else if(DHitVec.size()==0 && EHitVec.size()==2) {
		BuiltHits.push_back(EHitVec.at(0));
		BuiltHits.push_back(EHitVec.at(1));
	} else if(DHitVec.size()==2 && EHitVec.size()==1) {
		double dt1 = DHitVec.at(0).GetDPosition().Theta();
		double dt2 = DHitVec.at(1).GetDPosition().Theta();
		double et = EHitVec.at(0).GetEPosition().Theta();

		if( std::abs(dt1-et) <= std::abs(dt2-et) ) {
			BuiltHits.push_back(CombineHits(DHitVec.at(0),EHitVec.at(0)));
			//BuiltHits.back().Print();
			BuiltHits.push_back(DHitVec.at(1));
			//BuiltHits.back().Print();
		} else {
			BuiltHits.push_back(CombineHits(DHitVec.at(1),EHitVec.at(0)));
			//BuiltHits.back().Print();
			BuiltHits.push_back(DHitVec.at(0));
			//BuiltHits.back().Print();
		}
	} else if(DHitVec.size()==1 && EHitVec.size()==2) {
		double dt = DHitVec.at(0).GetDPosition().Theta();
		double et1 = EHitVec.at(0).GetEPosition().Theta();
		double et2 = EHitVec.at(0).GetEPosition().Theta();

		if( std::abs(dt-et1) <= std::abs(dt-et2) ) {
			BuiltHits.push_back(CombineHits(DHitVec.at(0),EHitVec.at(0)));
			//BuiltHits.back().Print();
			BuiltHits.push_back(EHitVec.at(1));
			//BuiltHits.back().Print();
		} else {
			BuiltHits.push_back(CombineHits(DHitVec.at(0),EHitVec.at(1)));
			//BuiltHits.back().Print();
			BuiltHits.push_back(EHitVec.at(0));
			//BuiltHits.back().Print();
		}
	} else if(DHitVec.size()==2 && EHitVec.size()==2) {
		double dt1 = DHitVec.at(0).GetDPosition().Theta();
		double dt2 = DHitVec.at(1).GetDPosition().Theta();
		double et1 = EHitVec.at(0).GetEPosition().Theta();
		double et2 = EHitVec.at(1).GetEPosition().Theta();

		if( std::abs(dt1-et1)+std::abs(dt2-et2) <= std::abs(dt1-et2)+std::abs(dt2-et1) ) {
			BuiltHits.push_back(CombineHits(DHitVec.at(0),EHitVec.at(0)));
			//BuiltHits.back().Print();
			BuiltHits.push_back(CombineHits(DHitVec.at(1),EHitVec.at(1)));
			//BuiltHits.back().Print();
		} else {
			BuiltHits.push_back(CombineHits(DHitVec.at(0),EHitVec.at(1)));
			//BuiltHits.back().Print();
			BuiltHits.push_back(CombineHits(DHitVec.at(1),EHitVec.at(0)));
			//BuiltHits.back().Print();
		}
	} else {
		std::cout<<"D Size: "<<DHitVec.size()<<" E Size: "<<EHitVec.size()<<std::endl;
	}
}

void TCSM::OldBuilddEE(std::vector<TCSMHit> &DHitVec,std::vector<TCSMHit> &EHitVec,std::vector<TCSMHit> &BuiltHits) {
	bool printbit =0;
	if(DHitVec.size()==0&&EHitVec.size()==0)//Why am I even here?!
		return;

	std::vector<bool> EUsed (EHitVec.size(),false);
	std::vector<bool> DUsed (DHitVec.size(),false);

	for(size_t diter=0;diter<DHitVec.size();diter++) {
		if(DUsed.at(diter))
			continue;

		for(size_t eiter=0;eiter<EHitVec.size();eiter++) {
			if(EUsed.at(eiter))
				continue;

			if(DHitVec.at(diter).GetDetectorNumber()==EHitVec.at(eiter).GetDetectorNumber()) {//Hits are in the same stack
				if( AlmostEqual(DHitVec.at(diter).GetDPosition().Theta(),EHitVec.at(eiter).GetEPosition().Theta())) {//Same-ish Theta
					//&& AlmostEqual(DHitVec.at(diter).GetDPosition().Phi(),EHitVec.at(eiter).GetEPosition().Phi()) )//Same-ish Phi
					BuiltHits.push_back(CombineHits(DHitVec.at(diter),EHitVec.at(eiter)));
					DUsed.at(diter) = true;
					EUsed.at(eiter) = true;
					break;
				}
			}
		}
	}

	//This loop adds uncorrelated events in the telescope together.  This may be bad, but let's see.
	for(size_t i=0;i<DHitVec.size();i++) {
		if(!DUsed.at(i)) {
			for(size_t j=0;j<EHitVec.size();j++) {
				if(!EUsed.at(j)) {
					if(EHitVec.at(j).GetDetectorNumber()==DHitVec.at(i).GetDetectorNumber()) {
						BuiltHits.push_back(CombineHits(DHitVec.at(i),EHitVec.at(j)));
						DUsed.at(i) = true;
						EUsed.at(j) = true;
						break;
					}
				}
			}
		}
	}


	//Send through the stragglers.  This is very permissive, but we trust BuildVH to take care of the riff-raff
	for(size_t i=0;i<DHitVec.size();i++) {
		if(!DUsed.at(i)) {
			BuiltHits.push_back(DHitVec.at(i));
		}
	}
	for(size_t j=0;j<EHitVec.size();j++) {
		if(!EUsed.at(j)) {
			BuiltHits.push_back(EHitVec.at(j));
		}
	}

	if(printbit) {
		for(size_t k =0; k<BuiltHits.size();k++) {
			std::cout<<DGREEN;
			BuiltHits.at(k).Print();
			std::cout<<RESET_COLOR<<std::endl;
		}
	}
}

void TCSM::RecoverHit(char orientation, std::pair<TFragment, MNEMONIC>& hit, std::vector<TCSMHit>& hits) {
	if(!RECOVERHITS) {
		return;
	}

	TCSMHit csmHit;

	int detno = hit.second.arrayposition;
	char pos = hit.second.arraysubposition[0];

	switch(detno) {
		case 1:
			return;
		case 2:
			if(pos == 'D' && orientation=='V') {//Recover 2DN09, channel 1040
				csmHit.SetDetectorNumber(detno);
				csmHit.SetDHorizontalCharge(hit.first.GetCharge());
				csmHit.SetDVerticalCharge(hit.first.GetCharge());
				csmHit.SetDHorizontalStrip(9);
				csmHit.SetDVerticalStrip(hit.second.segment);
				csmHit.SetDHorizontalCFD(hit.first.GetCfd());
				csmHit.SetDVerticalCFD(hit.first.GetCfd());
				csmHit.SetDHorizontalTime(hit.first.GetTimeStamp());
				csmHit.SetDVerticalTime(hit.first.GetTimeStamp());
				csmHit.SetDHorizontalEnergy(hit.first.GetEnergy());
				csmHit.SetDVerticalEnergy(hit.first.GetEnergy());
				csmHit.SetDPosition(TCSM::GetPosition(detno,
							pos,
							9,
							hit.second.segment));
			}
			break;
		case 3:
			if(pos=='E') {
				std::cerr<<"3E in RecoverHit"<<std::endl;

				return;
			} else if(orientation=='H') {//Recover 3DP11, channel 1145
				csmHit.SetDetectorNumber(detno);
				csmHit.SetDHorizontalCharge(hit.first.GetCharge());
				csmHit.SetDVerticalCharge(hit.first.GetCharge());
				csmHit.SetDHorizontalStrip(hit.second.segment);
				csmHit.SetDVerticalStrip(11);
				csmHit.SetDHorizontalCFD(hit.first.GetCfd());
				csmHit.SetDVerticalCFD(hit.first.GetCfd());
				csmHit.SetDHorizontalTime(hit.first.GetTimeStamp());
				csmHit.SetDVerticalTime(hit.first.GetTimeStamp());
				csmHit.SetDHorizontalEnergy(hit.first.GetEnergy());
				csmHit.SetDVerticalEnergy(hit.first.GetEnergy());
				csmHit.SetDPosition(TCSM::GetPosition(detno,
							pos,
							hit.second.segment,
							11));
			}
			break;
		case 4:
			if(pos=='E') {
				std::cerr<<"4E in RecoverHit"<<std::endl;
				return;
			} else if(orientation=='H') {//Recover 4DP15, channel 1181
				csmHit.SetDetectorNumber(detno);
				csmHit.SetDHorizontalCharge(hit.first.GetCharge());
				csmHit.SetDVerticalCharge(hit.first.GetCharge());
				csmHit.SetDHorizontalStrip(hit.second.segment);
				csmHit.SetDVerticalStrip(15);
				csmHit.SetDHorizontalCFD(hit.first.GetCfd());
				csmHit.SetDVerticalCFD(hit.first.GetCfd());
				csmHit.SetDHorizontalTime(hit.first.GetTimeStamp());
				csmHit.SetDVerticalTime(hit.first.GetTimeStamp());
				csmHit.SetDHorizontalEnergy(hit.first.GetEnergy());
				csmHit.SetDVerticalEnergy(hit.first.GetEnergy());
				csmHit.SetDPosition(TCSM::GetPosition(detno,
							pos,
							hit.second.segment,
							15));
			}
			break;
		default:
			std::cerr<<"Something is wrong.  The detector number in recover hit is out of bounds."<<std::endl;
			return;
	}

	if(!csmHit.IsEmpty()) {
		hits.push_back(csmHit);
	}
}

TCSMHit TCSM::CombineHits(TCSMHit d_hit,TCSMHit e_hit) {
	if(d_hit.GetDetectorNumber()!=e_hit.GetDetectorNumber())
		std::cerr<<"Something is wrong.  In combine hits, the detector numbers don't match"<<std::endl;

	d_hit.SetEHorizontalStrip(e_hit.GetEHorizontalStrip());
	d_hit.SetEVerticalStrip(e_hit.GetEVerticalStrip());

	d_hit.SetEHorizontalCharge(e_hit.GetEHorizontalCharge());
	d_hit.SetEVerticalCharge(e_hit.GetEVerticalCharge());

	d_hit.SetEHorizontalEnergy(e_hit.GetEHorizontalEnergy());
	d_hit.SetEVerticalEnergy(e_hit.GetEVerticalEnergy());

	d_hit.SetEHorizontalCFD(e_hit.GetEHorizontalCFD());
	d_hit.SetEVerticalCFD(e_hit.GetEVerticalCFD());

	d_hit.SetEHorizontalTime(e_hit.GetEHorizontalTime());
	d_hit.SetEVerticalTime(e_hit.GetEVerticalTime());

	d_hit.SetEPosition(e_hit.GetEPosition());

	//d_hit.Print();
	return(d_hit);

}

bool TCSM::AlmostEqual(int val1, int val2) {
	double diff = double(std::abs(val1 - val2));
	double ave = (val1+val2)/2.;
	double frac = diff/ave;
	return frac < fAlmostEqualWindow;
}

bool TCSM::AlmostEqual(double val1, double val2) {
	double frac = std::fabs(val1 - val2)/((val1+val2)/2.);
	return frac < fAlmostEqualWindow;
}

TCSMHit* TCSM::GetCSMHit(const int& i) {
	try {
		return &fCsmHits.at(i);   
	} catch (const std::out_of_range& oor) {
		std::cerr << ClassName() << " is out of range: " << oor.what() << std::endl;
	}
	return 0;
}
