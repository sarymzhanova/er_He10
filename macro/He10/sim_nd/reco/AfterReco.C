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
void DrawEdE(TString outFilename);
void DrawParentTrackID(TString outFilename);
void DrawParentPDG(TString outFilename);
void DrawEdepVsLY(TString outFilename);

TFile *f_sim,*f_reco,*f_out;
TTree *tree_sim, *tree_reco, *tree_out;

TClonesArray *arr_reco_8he,*arr_reco_p,*arr_reco_track8he,*arr_reco_trackp;
TClonesArray *arr_reco_bdt,*arr_reco_bdp;
TClonesArray *arr_ndtrack,*arr_ndparticle;
TClonesArray *arr_ndpoint,*arr_nddigi,*arr_nddigi_clr;
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
	arr_ndpoint = new TClonesArray("ERNDPoint",1000);
	arr_nddigi = new TClonesArray("ERNDDigi",1000);
	arr_nddigi_clr = new TClonesArray("ERNDDigi",1000);
	
	b_EventHeader = new ER10Heto8HeEventHeader();	

	tree_sim->SetBranchAddress("MCEventHeader.",&b_EventHeader);
	tree_sim->SetBranchAddress("TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080",&arr_reco_8he);
	tree_sim->SetBranchAddress("TelescopeParticle_Telescope_proton_DoubleSi_R_XY_1000010010",&arr_reco_p);
//	tree_sim->SetBranchAddress("TelescopeParticle_Telescope_proton_DoubleSi_R_XY_2212",&arr_reco_p);
	tree_sim->SetBranchAddress("BeamDetTrack",&arr_reco_bdt);
	tree_sim->SetBranchAddress("BeamDetParticle.",&arr_reco_bdp);
	tree_sim->SetBranchAddress("TelescopeTrack_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y",&arr_reco_track8he);
	tree_sim->SetBranchAddress("TelescopeTrack_Telescope_proton_DoubleSi_R_XY",&arr_reco_trackp);
	tree_sim->SetBranchAddress("NDTrack",&arr_ndtrack);
	tree_sim->SetBranchAddress("NDParticle",&arr_ndparticle);
	tree_sim->SetBranchAddress("NDPoint",&arr_ndpoint);
	tree_sim->SetBranchAddress("NDDigi",&arr_nddigi);
	tree_sim->SetBranchAddress("NDDigi",&arr_nddigi_clr);

}

void InitBranchesToWrite() {

	//tree_out->Branch("MCEventHeader.",&b_EventHeader);
	tree_out->Branch("TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080",&arr_reco_8he);
	tree_out->Branch("TelescopeParticle_Telescope_proton_DoubleSi_R_XY_1000010010",&arr_reco_p);
//	tree_out->Branch("TelescopeParticle_Telescope_proton_DoubleSi_R_XY_2212",&arr_reco_p);
	tree_out->Branch("TelescopeTrack_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y",&arr_reco_track8he);
	tree_out->Branch("TelescopeTrack_Telescope_proton_DoubleSi_R_XY",&arr_reco_trackp);
	tree_out->Branch("BeamDetTrack",&arr_reco_bdt);
	tree_out->Branch("BeamDetParticle.",&arr_reco_bdp);
	tree_out->Branch("NDTrack",&arr_ndtrack);
	tree_out->Branch("NDParticle",&arr_ndparticle);
	tree_out->Branch("NDDigi_clr",&arr_nddigi_clr);
	tree_out->Branch("NDDigi",&arr_nddigi);

}

void AfterReco(){
	
	CreateCut();		//cut_8he - события лежат на гистограмме dE-E, соответствующей событиям 8Не
	CreateCutGamma();	//cut_gamma - вырезать события, соответствующие гамма-квантам на гистограмме NDDigi.LightYield(y) : NDDigi.Edep (x)

	OpenFilesAndTrees("../sim_digi_8_1nNDVac.root", 
						"sim_digi_8_1nNDVac.target.root",
						"reco_sim_digi_8_1nNDVac.root");
	
	InitBranchesToRead();
	InitBranchesToWrite();	
	
	
	Int_t trigger_reaction=0;
	Int_t ndpoint_pdg, parent_trackid,parent_pdg;
	Float_t ndpoint_Eloss, EdepN1, EdepN2;
	Float_t tofN1, tofN2, ToT;
	
	ERNDDigi *nddigi = new ERNDDigi();
	
	TH2F hEdE("EdE","EdE of He8 decay",100,0,70,100,0,160);
	TH1F hTrackID("trackID","ID track of parent",100,-1,100);
	TH1F hParentPDG("parentPDG","PDG of parent",3001,-1,3000);
	
	for(Int_t i=0;i<tree_sim->GetEntries();i++){
		tree_sim->GetEntry(i);	
		tree_reco->GetEntry(i);	
		trigger_reaction = (Int_t)b_EventHeader->GetTrigger();
		ToT = (Int_t)b_EventHeader->GetReactionTime();	//time on target

		if(arr_nddigi->GetEntriesFast()==2){
			for(Int_t k=0;k<2;k++){
				EdepN1 = ((ERNDDigi*)arr_nddigi->At(k))->EdepN1();
				EdepN2 = ((ERNDDigi*)arr_nddigi->At(k))->EdepN2();
				tofN1 = ((ERNDDigi*)arr_nddigi->At(k))->TimeN1();
				tofN1 -= ToT;	//subtract time on target to obtain time of flight
				tofN2 = ((ERNDDigi*)arr_nddigi->At(k))->TimeN2();
				tofN2 -= ToT;
			}
		}

		if(arr_reco_8he->GetEntries()>0 && arr_reco_p->GetEntries()>0 ){	//&& arr_nddigi->GetEntriesFast()==2
			
			he8_E = ((ERTelescopeParticle*)arr_reco_8he->At(0))->GetEdepInThickStation();
			he8_dE = ((ERTelescopeParticle*)arr_reco_8he->At(0))->GetEdepInThinStation();
			//printf("%i\tdE %f\tE %f\n",i,he8_dE,he8_E);
			if(cut_8he->IsInside(he8_dE, he8_E)) {			
				trigger_8he = 1;
				hEdE.Fill(he8_dE,he8_E);
				//printf("%i ==========================================\n",i);
				for(Int_t iNDpoint=0;iNDpoint < arr_ndpoint->GetEntries();iNDpoint++){
					ndpoint_pdg = ((ERNDPoint*)arr_ndpoint->At(0))->GetPDG();
					ndpoint_Eloss = ((ERNDPoint*)arr_ndpoint->At(0))->GetEnergyLoss();
					parent_trackid = ((ERNDPoint*)arr_ndpoint->At(0))->GetParentTrackID();
					parent_pdg = ((ERNDPoint*)arr_ndpoint->At(0))->GetParentPDGID();
					hTrackID.Fill(parent_trackid);
					hParentPDG.Fill(parent_pdg);
					
						
				}

			}
				
			
			if(trigger_8he != 1) Clear();
		}
		else Clear();
		
		tree_out->Fill();
		Clear();
	}
	
	hEdE.Write();
	hTrackID.Write();
	hParentPDG.Write();
	tree_out->Write();

	hEdE.Draw();
	if(hEdE.Integral()==0) printf("=======================================================\n  Graphical cut is not valid, Integral of EdE is zero\n=======================================================\n");
//	hEdE.Write();		       
	f_out->Close();
	f_sim->Close();
	f_reco->Close();
	

	
	// DrawEdE(inFileReco);
	// DrawEdE(outFile);
	// DrawEdepVsLY(outFile);
	//DrawParentTrackID(outFile);
	//DrawParentPDG(outFile);
}
//========================================================================================================================== 



void Clear(){
	arr_reco_8he->Clear();
	arr_reco_p->Clear();
	arr_reco_track8he->Clear();
	arr_reco_trackp->Clear();
	arr_reco_bdt->Clear();
	arr_reco_bdp->Clear();
	arr_nddigi->Clear();
	arr_nddigi_clr->Clear();
	arr_ndtrack->Clear();
	arr_ndparticle->Clear();
	he8_E = -1;
	he8_dE = -1;
	trigger_8he = 0;
}

void DrawEdE(TString outFilename){
	TFile *f_reco = new TFile(outFilename,"READ");
	TH2F *hEdE = (TH2F*)f_reco -> Get("EdE");
	TTree *tree_reco = (TTree*)f_reco -> Get("er");
	
	TCanvas *c1 = new TCanvas(outFilename,outFilename,1000,500);
	tree_reco->Draw("TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080.fEdepInThickStation : TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080.fEdepInThinStation >> hEdE_raw(140,0,140,180,0,180)");	//
	cut_8he->SetLineColor(2);
	cut_8he->SetLineWidth(2);
	cut_8he->Draw("same");
	
	f_reco->Close();
}

void DrawParentTrackID(TString outFilename){
	TFile *f_reco = new TFile(outFilename,"READ");
	TH1F *hTrackID = (TH1F*)f_reco -> Get("trackID");
	TString name = outFilename;
	name.Prepend("trackID_");
	TCanvas *c1 = new TCanvas(name,name,1000,500);
	hTrackID->Draw();
	f_reco->Close();
}

void DrawParentPDG(TString outFilename){
	TFile *f_reco = new TFile(outFilename,"READ");
	TH1F *hParentPDG = (TH1F*)f_reco -> Get("parentPDG");
	TString name = outFilename;
	name.Prepend("parentPDG_");
	TCanvas *c2 = new TCanvas(name,name,1000,500);
	hParentPDG->Draw();
	f_reco->Close();
}

void DrawEdepVsLY(TString outFilename){
	TFile *f_reco = new TFile(outFilename,"READ");
	TTree *t_reco = (TTree*)f_reco -> Get("er");
	TString name = outFilename;
	name.Prepend("EdepVsLY_");
	TCanvas *c3 = new TCanvas(name,name,1000,500);
	c3->Divide(2,1);
	c3->cd(1);
	t_reco->Draw("NDDigi.fLightYield:NDDigi.fEdep","","colz");
	c3->cd(2);
	t_reco->Draw("NDDigi_clr.fLightYield:NDDigi_clr.fEdep","","colz");
}
