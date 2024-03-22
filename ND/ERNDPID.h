/********************************************************************************
 *              Copyright (C) Joint Institute for Nuclear Research              *
 *                                                                              *
 *              This software is distributed under the terms of the             *
 *         GNU Lesser General Public Licence version 3 (LGPL) version 3,        *
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/

#ifndef ERNDPID_H
#define ERNDPID_H

#include "TClonesArray.h"
#include "TLorentzVector.h"

#include "ERTask.h"
#include "ERNDParticle.h"
#include "ERNDSetup.h"

class ERNDPID : public ERTask {
  public:
    ERNDPID();
    virtual InitStatus Init(); 
    virtual void Exec(Option_t* opt);
    virtual void Reset();
  protected:
    //Input arrays
    TClonesArray* fNDTracks = nullptr;
    TClonesArray* fBeamDetParticle = nullptr; 
    //Output arrays
    TClonesArray *fNDParticles = nullptr;
    ERNDSetup* fSetup = nullptr;
    ERNDParticle* AddParticle(const TLorentzVector& lv, const TLorentzVector& lv_n1, const TLorentzVector& lv_n2, float tof, float tof_n1, float tof_n2);
    ClassDef(ERNDPID,1)
};

#endif
