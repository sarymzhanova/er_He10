#if !defined(__CLING__)

//standard ROOT includes
#include "TClonesArray.h"
#include "TFile.h"
#include "TTree.h"
#include "TH2.h"
#include "TCanvas.h"

//ExpertRoot includes
#include "ER10Heto8HeEventHeader.h"
#include "ERNDDigi.h"
#include "ERTelescopeParticle.h"
#include "ERNDPoint.h"

#endif

#include "./CreateCut.C"
#include "./CreateCutGamma.C"

void Clear();

TFile *f_sim,*f_reco,*f_out;
TTree *tree_sim, *tree_reco, *tree_out;

TClonesArray *arr_reco_8he,*arr_reco_p,*arr_reco_track8he,*arr_reco_trackp;
TClonesArray *arr_reco_bdt,*arr_reco_bdp;
TClonesArray *arr_ndtrack,*arr_ndparticle;
// TClonesArray *arr_ndpoint,*arr_nddigi,*arr_nddigi_clr;
ER10Heto8HeEventHeader *b_EventHeader;

Float_t he8_E = -1, he8_dE = -1;
Float_t gamma_Edep = -1, gamma_LY = -1;
Int_t trigger_8he = 0;
Int_t trigger_gamma = 0;

void OpenFilesAndTrees(TString inFileLab, TString inFileReco, TString outFile) {
	
	f_sim = new TFile(inFileLab,"READ");
	tree_sim = (TTree*)f_sim -> Get("er");
	f_reco = new TFile(inFileReco,"READ");
	tree_reco = (TTree*)f_reco -> Get("er");
	tree_sim->AddFriend(tree_reco);
	f_out = new TFile(outFile,"RECREATE");
	tree_out = new TTree("er","");

}

void InitBranchesToRead() {

	arr_reco_8he = new TClonesArray("ERTelescopeParticle",1000);
	arr_reco_p = new TClonesArray("ERTelescopeParticle",1000);
	arr_reco_track8he = new TClonesArray("ERTelescopeTrack",1000);
	arr_reco_trackp = new TClonesArray("ERTelescopeTrack",1000);
	arr_reco_bdt = new TClonesArray("ERBeamDetTrack",1000);
	arr_reco_bdp = new TClonesArray("ERBeamDetParticle",1000);
	arr_ndparticle = new TClonesArray("ERNDParticle",1000);
	arr_ndtrack = new TClonesArray("ERNDTrack",1000);
		
	b_EventHeader = new ER10Heto8HeEventHeader();	

	tree_sim->SetBranchAddress("MCEventHeader.",&b_EventHeader);
	tree_sim->SetBranchAddress("TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080",&arr_reco_8he);
	tree_sim->SetBranchAddress("TelescopeParticle_Telescope_proton_DoubleSi_R_XY_1000010010",&arr_reco_p);
	tree_sim->SetBranchAddress("BeamDetTrack",&arr_reco_bdt);
	tree_sim->SetBranchAddress("BeamDetParticle.",&arr_reco_bdp);
	tree_sim->SetBranchAddress("TelescopeTrack_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y",&arr_reco_track8he);
	tree_sim->SetBranchAddress("TelescopeTrack_Telescope_proton_DoubleSi_R_XY",&arr_reco_trackp);
	tree_sim->SetBranchAddress("NDTrack",&arr_ndtrack);
	tree_sim->SetBranchAddress("NDParticle",&arr_ndparticle);
	
}

void InitBranchesToWrite() {

	tree_out->Branch("TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080",&arr_reco_8he);
	tree_out->Branch("TelescopeParticle_Telescope_proton_DoubleSi_R_XY_1000010010",&arr_reco_p);
	tree_out->Branch("TelescopeTrack_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y",&arr_reco_track8he);
	tree_out->Branch("TelescopeTrack_Telescope_proton_DoubleSi_R_XY",&arr_reco_trackp);
	tree_out->Branch("BeamDetTrack",&arr_reco_bdt);
	tree_out->Branch("BeamDetParticle.",&arr_reco_bdp);
	tree_out->Branch("NDTrack",&arr_ndtrack);
	tree_out->Branch("NDParticle",&arr_ndparticle);

}

void AfterReco(){
	
	CreateCut();		//cut_8he - события лежат на гистограмме dE-E, соответствующей событиям 8Не
	CreateCutGamma();	//cut_gamma - вырезать события, соответствующие гамма-квантам на гистограмме NDDigi.LightYield(y) : NDDigi.Edep (x)

	// OpenFilesAndTrees("../sim_digi_8_1nNDVac.root", 
	// 					"sim_digi_8_1nNDVac.target.root",
	// 					"reco_sim_digi_8_1nNDVac.root");
	OpenFilesAndTrees("../sim_digi_8_1nNDSteel.root", 
						"sim_digi_8_1nNDSteel.target.root",
						"reco_sim_digi_8_1nNDSteel.root");
	// OpenFilesAndTrees("../sim_digi_8_1nNDAl.root", 
	// 					"sim_digi_8_1nNDAl.target.root",
	// 					"reco_sim_digi_8_1nNDAl.root");										
	
	InitBranchesToRead();
	InitBranchesToWrite();	
	
	
	Int_t trigger_reaction=0;
	
	TH2F hEdE("EdE","EdE of He8 decay",100,0,70,100,0,160);
	
	for(Int_t i=0;i<tree_sim->GetEntries();i++){
		tree_sim->GetEntry(i);	
		tree_reco->GetEntry(i);	
		trigger_reaction = (Int_t)b_EventHeader->GetTrigger();

		//both telescopes (forward square and backward annual) were fired
		if(arr_reco_8he->GetEntries() >0 && arr_reco_p->GetEntries() >0 ){
		// if(trigger_reaction && arr_reco_8he->GetEntries() == 1 && arr_reco_p->GetEntries() == 1 ){
		// if(arr_reco_8he->GetEntries() == 1 && arr_reco_p->GetEntries() == 1 ){			
			he8_E = ((ERTelescopeParticle*)arr_reco_8he->At(0))->GetEdepInThickStation();
			he8_dE = ((ERTelescopeParticle*)arr_reco_8he->At(0))->GetEdepInThinStation();
			if(cut_8he->IsInside(he8_dE, he8_E)) {			
				trigger_8he = 1;
				hEdE.Fill(he8_dE,he8_E);
			}
			
			if(trigger_8he != 1) Clear();
		}
		else Clear();
		
		tree_out->Fill();
		Clear();
	}

	hEdE.Write();
	tree_out->Write();

	f_out->Close();
	f_sim->Close();
	f_reco->Close();
	Info("AfterReco", "Script finished");
	
}
//========================================================================================================================== 

void Clear(){
	arr_reco_8he->Clear();
	arr_reco_p->Clear();
	arr_reco_track8he->Clear();
	arr_reco_trackp->Clear();
	arr_reco_bdt->Clear();
	arr_reco_bdp->Clear();
	arr_ndtrack->Clear();
	arr_ndparticle->Clear();
	he8_E = -1;
	he8_dE = -1;
	trigger_8he = 0;
}