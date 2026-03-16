#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TH2F.h"
#include "TCanvas.h"

int DrawPID_all(
        TString recoFile = "sim_digi_8_1nNDSteel.target.root", 
        TString testFile = "sim_digi_1test.target.root",
        TString simFile = "../sim_digi_8_1nNDSteel.root", 
        TString simtestFile = "../sim_digi_1test.root"
        )
{
    TFile* reco = new TFile(recoFile, "read");
    TFile* test = new TFile(testFile, "read");
    TFile* sim = new TFile(simFile, "read");
    TFile* simtest = new TFile(simtestFile, "read");
    TTree* recoTree = (TTree*)reco->Get("er");
    TTree* testTree = (TTree*)test->Get("er");
    TTree* simTree = (TTree*)sim->Get("er");
    TTree* simtestTree = (TTree*)simtest->Get("er");
    TH2F* hReco = new TH2F(
            "hReco", 
            "Reconstruction;Edep Thick station, MeV; Edep Thin station, MeV",
            400,
            0,
            200,
            200,
            0,
            100
            );
    TH2F* hTest = new TH2F(
            "hTest", 
            "Reconstruction_Test;Edep All stations, MeV; Edep One station, MeV",
            400,
            0,
            200,
            200,
            0,
            100
            );
    TH2F* hSim = new TH2F(
            "hSim", 
            "Simulation;Edep Thick station, MeV; Edep Thin station, MeV",
            400,
            0,
            200,
            400,
            0,
            100
            );
    TH2F* hSimtest = new TH2F(
            "hSim", 
            "Simulation;Edep Thick station, MeV; Edep Thin station, MeV",
            400,
            0,
            200,
            400,
            0,
            100
            );
    TCanvas* aCanvas = new TCanvas("aCanvas", "aCanvas", 1400, 1400);
    aCanvas->Divide(2, 2);
    aCanvas->cd(1);
    recoTree->Draw(
"TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080.\
fEdepInThinStation:\
TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080.\
fEdepInThickStation>>hReco",
"",
"colz"
);
    aCanvas->cd(2);
    testTree->Draw(
"TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080.\
fEdepInThinStation:\
TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080.\
fEdepInThickStation>>hTest",
"",
"colz"
);
    aCanvas->cd(3);
    simTree->Draw(
"TelescopeDigi_Telescope_he8_SingleSi_SSD20_X.fEdep:TelescopeDigi_Telescope_he8_SingleSi_SSD20_1_Y.fEdep+\
TelescopeDigi_Telescope_he8_SingleSi_SSD20_2_Y.fEdep+\
TelescopeDigi_Telescope_he8_SingleSi_SSD20_3_Y.fEdep+\
TelescopeDigi_Telescope_he8_SingleSi_SSD20_4_Y.fEdep+\
TelescopeDigi_Telescope_he8_SingleSi_SSD20_5_Y.fEdep>>hSim", 
"", 
"colz");
    aCanvas->cd(4);
    simtestTree->Draw(
"TelescopeDigi_Telescope_he8_SingleSi_SSD20_X.fEdep:TelescopeDigi_Telescope_he8_SingleSi_SSD20_1_Y.fEdep+\
TelescopeDigi_Telescope_he8_SingleSi_SSD20_2_Y.fEdep+\
TelescopeDigi_Telescope_he8_SingleSi_SSD20_3_Y.fEdep+\
TelescopeDigi_Telescope_he8_SingleSi_SSD20_4_Y.fEdep+\
TelescopeDigi_Telescope_he8_SingleSi_SSD20_5_Y.fEdep>>hSimtest", 
"", 
"colz"
);
    aCanvas->SaveAs("./Image/pid_all.png");

    return 0;
}
