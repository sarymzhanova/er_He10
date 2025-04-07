#if !defined(__CLING__)

//standard ROOT includes
#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TClonesArray.h"
#include "TCut.h"
#include "TCanvas.h"

#include "FairAnaSelector.h"

//ExpertRoot includes
#include "ER10Heto8HeEventHeader.h"
#include "ERNDDigi.h"
#include "ERNDParticle.h"

#endif

TFile *f_sim;
TTree *tree_sim;
TClonesArray *arr_mctrack, *arr_Sipoint;
Bool_t ifDefineIon = 0;

void OpenFilesAndTrees(TString inFilelab){
    f_sim = new TFile(inFilelab,"READ");
    tree_sim = (TTree*)f_sim -> Get("er");
}

void ReadBranch(){
    arr_mctrack = new TClonesArray("ERMCTrack",1000);
    arr_Sipoint = new TClonesArray("ERPoint",1000);
    tree_sim->SetBranchAddress("MCTrack",&arr_mctrack);
    tree_sim->SetBranchAddress("TelescopePoint_Telescope_Si_SingleSi_beamCalc_X",&arr_Sipoint);
}

Double_t DefineMass(TString filename){
    Int_t Z = 1, A = 1, Q = 1;
    TString ionName = "proton";
    if(filename.Contains("proton")){
        Z = 1;
        A = 1;
        Q = 1;
        ionName = "proton";
        ifDefineIon = 1;
    }
    else if(filename.Contains("3He")){
        Z = 2;
        A = 3;
        Q = 2;
        ionName = "3He";
        ifDefineIon = 1;
    }
    else if(filename.Contains("4He")){
        Z = 2;
        A = 4;
        Q = 2;
        ionName = "4He";
        ifDefineIon = 1;        
    }
    else if(filename.Contains("9C")){
        Z = 6;
        A = 9;
        Q = 6;
        ionName = "9C";
        ifDefineIon = 1;
    }
    else{
        printf("\n!!!No ion name in file!!!\n!!!Proton is set as ion!!!\n ");
        printf("Ion will be defined further from the data\n");
        ifDefineIon = 0;
    }
    printf("\nIon is %s, Z=%i, A=%i,Q=%i\n\n",ionName.Data(),Z,A,Q);
    
    ERIonMixGenerator* generator = new ERIonMixGenerator(ionName, Z, A, Q, 1);
    FairIon *ion = generator->Ion();
    Double_t mass;
    mass  = 1000*ion->GetMass();
    printf("Mass is %f MeV\n",mass);
    return mass;
}
Double_t DefineMass(Int_t PID){
    TString pidstr;
    pidstr.Form("%i",PID);
    TString Zstr = pidstr;
    Zstr.Remove(6,4);
    Zstr.Remove(0,3);                
    Int_t Z = Zstr.Atoi();

    TString Astr = pidstr;
    Astr.Remove(9,1);
    Astr.Remove(0,6);
    printf("A=<%s>\n",Astr.Data());
    Int_t A = Astr.Atoi();

    Int_t Q;
    TString ionName;
    if(Z==1){
        Q = 1;
        if(A == 1) ionName = "proton";
        ifDefineIon = 1;
    }
    else if(Z==2){
        if(A == 3) ionName = "3He";
        if(A == 4) ionName = "4He";
        Q = 2;        
        ifDefineIon = 1;
    }
    else if(Z==6){
        Q = 6;
        if(A == 9) ionName = "9C";
        ifDefineIon = 1;
    }

    ifDefineIon = 1;
    // FairPrimaryGenerator* primGen = new FairPrimaryGenerator();
    // TDatabasePDG *tdb = TDatabasePDG::Instance();
    // ion_mass = TDatabasePDG::Instance()->GetParticle(PID)->Mass();

    ERIonMixGenerator* generator = new ERIonMixGenerator(ionName, Z, A, Q, 1);
    FairIon *ion = generator->Ion();
    Double_t mass;
    mass  = 1000*ion->GetMass();
    
    printf("Ion is defined. PID = %i, Z = %i, A = %i, mass = %f MeV\n",PID,Z,A,mass);

    return mass;
}

void read_BeamCalc(TString filename = "BeamCalc_proton_500AMeV.root"){

    // filename = "BeamCalc_8He_500AMeV.root";
    OpenFilesAndTrees(filename);
    ReadBranch();

    Int_t mult_mctrack,mult_Sipoint;
    ERTelescope *Sipoint;
    ERMCTrack *mctrack;

    Float_t dEloss,Pin,Pxin,Pyin,Pzin;
    Float_t Eloss_Si_beam=0,Eloss_Si_tot=0,Ekin;
    Float_t ThetaX,ThetaY;
    TVector3 vPin;
    Int_t motherID,PID;

    TH1F *hEkin = new TH1F("hEkin","",100,490,500);
    TH1F *hPtheta = new TH1F("hPtheta","",100,-0.02,0.25);
    TH1F *hEloss_Si_beam = new TH1F("hEloss_Si_beam","",100,-1,3);
    TH1F *hEloss_Si_tot = new TH1F("hEloss_Si_tot","",100,-1,25);

    
    Double_t ion_mass;
    ion_mass = DefineMass(filename);
    

    // for(Int_t i=0;i<10;i++){
    for(Int_t i=0;i<tree_sim->GetEntries();i++){
        tree_sim->GetEntry(i);
        mult_mctrack = arr_mctrack->GetEntriesFast();
        mult_Sipoint = arr_Sipoint->GetEntriesFast();

        for(Int_t k=0;k<mult_Sipoint;k++){
            dEloss = ((ERPoint*)arr_Sipoint->At(k))->GetEnergyLoss();
            motherID = ((ERPoint*)arr_Sipoint->At(k))->GetMot0TrackID();
            PID = ((ERPoint*)arr_Sipoint->At(k))->GetPID();
            // printf("%i\t%f\t%i\t%i\n",k,dEloss,PID,motherID);
            if(motherID==-1 && ifDefineIon == 0){   //means primary particle, which wasn't defined from file                
                // // TDatabasePDG *tdb = TDatabasePDG::Instance();
                // // ion_mass = TDatabasePDG::Instance()->GetParticle(PID)->Mass();
                ion_mass = DefineMass(PID);
            }
            Pin = 1000*((ERPoint*)arr_Sipoint->At(k))->GetPIn();
            Pxin = 1000*((ERPoint*)arr_Sipoint->At(k))->GetPxIn();
            Pyin = 1000*((ERPoint*)arr_Sipoint->At(k))->GetPyIn();
            Pzin = 1000*((ERPoint*)arr_Sipoint->At(k))->GetPzIn();
            vPin.SetXYZ(Pxin,Pyin,Pzin);

            Eloss_Si_tot+=dEloss;

            if(motherID==-1){
                Ekin = TMath::Sqrt(ion_mass*ion_mass+Pin*Pin) - ion_mass;
                // 
                ThetaX = TMath::ATan(Pxin/Pzin);
                ThetaY = TMath::ATan(Pyin/Pzin);
                // printf("ThetaX\t%f\tThetaY\t%f\tTheta\t%f\n",ThetaX,ThetaY,vPin.Theta());
                Eloss_Si_beam+=dEloss;

                hEkin->Fill(Ekin);
                hPtheta->Fill(vPin.Theta());

                // printf("%i\tEloss beam %f\tdEloss\t%f\n",i,Eloss_Si_beam,dEloss);
                
            }
        }
        hEloss_Si_beam->Fill(Eloss_Si_beam);
        hEloss_Si_tot->Fill(Eloss_Si_tot);

        Eloss_Si_beam=0;
        Eloss_Si_tot=0;
    }
    cout << endl << endl;
    cout << "Macro finished succesfully." << endl;

    // TCanvas c1;
    // hEkin->Draw();

    // TCanvas c2;
    // hPtheta->Draw();
}
