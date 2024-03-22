/********************************************************************************
 *              Copyright (C) Joint Institute for Nuclear Research              *
 *                                                                              *
 *              This software is distributed under the terms of the             *
 *         GNU Lesser General Public Licence version 3 (LGPL) version 3,        *
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/

#ifndef ERNDParticle_H
#define ERNDParticle_H

#include "TLorentzVector.h"

class ERNDParticle: public TObject {
  public:
	ERNDParticle() = default;
	ERNDParticle(const TLorentzVector& lv, const TLorentzVector& lv_n1, const TLorentzVector& lv_n2, float tof, float tof_n1, float tof_n2);
    TLorentzVector LV() const { return fLV; }
    TLorentzVector LV_n1() const { return fLV_n1; }
    TLorentzVector LV_n2() const { return fLV_n2; }
    float ToF() const { return fToF; }
    float ToF_n1() const { return fToF_n1; }
    float ToF_n2() const { return fToF_n2; }
  protected:
    TLorentzVector fLV;
    TLorentzVector fLV_n1;
    TLorentzVector fLV_n2;
    float fToF = -1.;
    float fToF_n1 = -1.;
    float fToF_n2 = -1.;
	ClassDef(ERNDParticle,1);
};

#endif
