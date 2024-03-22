#include "ERNDParticle.h"

ERNDParticle::ERNDParticle(const TLorentzVector& lv, const TLorentzVector& lv_n1, const TLorentzVector& lv_n2, float tof, float tof_n1, float tof_n2) 
    : fLV(lv), fLV_n1(lv_n1), fLV_n2(lv_n2), fToF(tof), fToF_n1(tof_n1), fToF_n2(tof_n2)
{}
