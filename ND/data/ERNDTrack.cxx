/********************************************************************************
 *              Copyright (C) Joint Institute for Nuclear Research              *
 *                                                                              *
 *              This software is distributed under the terms of the             *
 *         GNU Lesser General Public Licence version 3 (LGPL) version 3,        *
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/
#include "ERNDTrack.h"

#include "FairLogger.h"

ERNDTrack::ERNDTrack(const TVector3& detectorVertex, Bool_t isN1, Bool_t isN2, const TVector3& targetVertex,
                     float edep, float edep_n1, float edep_n2, float time, float time_n1, float time_n2, float tac)
    : fTargetVertex(targetVertex)
    , fDetectorVertex(detectorVertex), fIsN1(isN1),fIsN2(isN2)
    , fEdep(edep),fEdep_n1(edep_n1),fEdep_n2(edep_n2)
    , fTime(time),fTime_n1(time_n1),fTime_n2(time_n2)
    , fTAC(tac)
{
}	

TVector3 ERNDTrack::Direction() const {
	TVector3 direction = fDetectorVertex - fTargetVertex;
	direction.SetMag(1.);
	return direction;
}

ClassImp(ERNDTrack)
