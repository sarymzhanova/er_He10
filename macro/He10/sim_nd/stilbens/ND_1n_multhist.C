#if !defined(__CLING__)

//standard ROOT includes
#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TClonesArray.h"
#include "TCut.h"

//ExpertRoot includes
#include "ER10Heto8HeEventHeader.h"
#include "ERNDDigi.h"

#endif

#include "../reco/CreateCutGamma.C"

TFile *f_simV,*f_simS,*f_recoV,*f_recoS,*f_outV,*f_outS;
TTree *tree_simV, *tree_recoV;
TTree *tree_simS, *tree_recoS;

//declarations of functions
Bool_t stilbenNcut(Int_t N_stilben,ER10Heto8HeEventHeader *b_EventHeader);

void stilbenCoord();

Double_t x[5][9], y[5], z[9];	//координаты центра стильбенов в плоскости (x,y)
Int_t Nstil=0;

void OpenFilesAndTrees(TString inFilelabV, TString inFilelabS,
						TString inFileRecoV, TString inFileRecoS,
						TString outFileStilbensV, TString outFileStilbensS) {
	
	f_simV = new TFile(inFilelabV,"READ");
	tree_simV = (TTree*)f_simV -> Get("er");
	f_recoV = new TFile(inFileRecoV,"READ");
	tree_recoV = (TTree*)f_recoV -> Get("er");
	f_outV = new TFile(outFileStilbensV,"RECREATE");
	tree_simV->AddFriend(tree_recoV);
	
	f_simS = new TFile(inFilelabS,"READ");
	tree_simS = (TTree*)f_simS -> Get("er");
	f_recoS = new TFile(inFileRecoS,"READ");
	tree_recoS = (TTree*)f_recoS -> Get("er");
	f_outS = new TFile(outFileStilbensS,"RECREATE");
	tree_simS->AddFriend(tree_recoS);

}

void ND_1n_multhist(){
	CreateCutGamma();

	// OpenFilesAndTrees("../sim_digi_8_1nNDSteel.root", "../sim_digi_8_1nNDVac.root",
	// 					"../reco/reco_sim_digi_8_1nNDSteel.root", "../reco/reco_sim_digi_8_1nNDVac.root",
	// 					"Ncoord_sim_digi_8_1nNDSteel.root", "Ncoord_sim_digi_8_1nNDVac.root");

	OpenFilesAndTrees("../sim_digi_8_1nNDAl.root", "../sim_digi_8_1nNDVac.root",
						"../reco/reco_sim_digi_8_1nNDAl.root", "../reco/reco_sim_digi_8_1nNDVac.root",
						"Ncoord_sim_digi_8_1nNDAl.root", "Ncoord_sim_digi_8_1nNDVac.root");

	
	auto arr_nddigiS = new TClonesArray("ERNDDigi",1000);
	auto arr_ndtrackS = new TClonesArray("ERNDTrack",1000);
	auto arr_telpar_8hedS = new TClonesArray("ERTelescopeParticle",1000);
	auto arr_telpar_pS = new TClonesArray("ERTelescopeParticle",1000);
	
	auto arr_nddigiV = new TClonesArray("ERNDDigi",1000);
	auto arr_ndtrackV = new TClonesArray("ERNDTrack",1000);
	auto arr_telpar_8hedV = new TClonesArray("ERTelescopeParticle",1000);
	auto arr_telpar_pV = new TClonesArray("ERTelescopeParticle",1000);
	
	ER10Heto8HeEventHeader *b_EventHeaderS = new ER10Heto8HeEventHeader();
	ER10Heto8HeEventHeader *b_EventHeaderV = new ER10Heto8HeEventHeader();
	
	tree_simS->SetBranchAddress("NDDigi",&arr_nddigiS);
	tree_simS->SetBranchAddress("MCEventHeader.",&b_EventHeaderS);
	tree_simS->SetBranchAddress("TelescopeParticle_Telescope_proton_DoubleSi_R_XY_1000010010",&arr_telpar_pS);
	tree_simS->SetBranchAddress("TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080",&arr_telpar_8hedS);
	
	tree_simV->SetBranchAddress("NDDigi",&arr_nddigiV);
	tree_simV->SetBranchAddress("MCEventHeader.",&b_EventHeaderV);
	tree_simV->SetBranchAddress("TelescopeParticle_Telescope_proton_DoubleSi_R_XY_1000010010",&arr_telpar_pV);
	tree_simV->SetBranchAddress("TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080",&arr_telpar_8hedV);
	
	Bool_t circle;
	Int_t Nstilben,triggerS,triggerV,trigger_gamma=-1;
	Int_t mult_digi,mult_th,mult_stil,mult_nogamma,mult_stil_nogamma,mult_th_nogamma,mult_stl_th_nogamma,channel;
	Int_t mult_nINcone=0;
	Int_t x_ch,y_ch;
	Float_t Z_stilben;
	TString cut_reactionS,cut_reactionV;
	
	stilbenCoord();
// 	Nstilben = 9;
 	Nstilben = 13;
 	Float_t Eth = 0.11;	//min Edep 110 keV
// 	Float_t Eth = 0.3;	//появляется разница в количестве событий с отбором гамм и гаммы + порог
	Float_t Edep,LY;
	
	TString name;
	name.Form("Common multiplicity, steel shell");
	TH1I hmultS("hmultS",name.Data(),10,0,10);
	name.Form("Common multiplicity, vacuum shell");
	TH1I hmultV("hmultV",name.Data(),10,0,10);
	name.Form("Common multiplicity, steel shell, no gamma");
	TH1I hmultS_nogamma("hmultS_nogamma",name.Data(),10,0,10);
	name.Form("Common multiplicity, vacuum shell, no gamma");
	TH1I hmultV_nogamma("hmultV_nogamma",name.Data(),10,0,10);
	name.Form("Common multiplicity with threshold Edep>%.3f MeV, steel shell",Eth);
	TH1I hmultS_th("hmultS_th",name.Data(),10,0,10);
	name.Form("Common multiplicity with threshold Edep>%.3f MeV, vacuum shell",Eth);
	TH1I hmultV_th("hmultV_th",name.Data(),10,0,10);
	
	name.Form("Common multiplicity with threshold Edep>%.3f MeV, no gamma, steel shell",Eth);
	TH1I hmultS_th_nogamma("hmultS_th_nogamma",name.Data(),10,0,10);
	name.Form("Common multiplicity with threshold Edep>%.3f MeV, no gamma, vacuum shell",Eth);
	TH1I hmultV_th_nogamma("hmultV_th_nogamma",name.Data(),10,0,10);
	
//	name.Form("Multiplicity in %i stilben, steel shell",Nstilben);
//	TH1I hmultS_stilben("hmultS_stl",name.Data(),10,0,10);
//	name.Form("Multiplicity in %i stilben, vacuum shell",Nstilben);
//	TH1I hmultV_stilben("hmultV_stl",name.Data(),10,0,10);
//	name.Form("Multiplicity in %i stilben, steel shell, no gamma",Nstilben);
//	TH1I hmultS_stilben_nogamma("hmultS_stl_no gamma",name.Data(),10,0,10);
//	name.Form("Multiplicity in %i stilben, vacuum shell, no gamma",Nstilben);
//	TH1I hmultV_stilben_nogamma("hmultV_stl_no gamma",name.Data(),10,0,10);

//=================================================================================================================//
	name.Form("Multiplicity in %i stilben, steel shell",Nstilben);
	TH1I hmultS_stilben("hmultS_stl",name.Data(),44,0,44);
	name.Form("Multiplicity in %i stilben, vacuum shell",Nstilben);
	TH1I hmultV_stilben("hmultV_stl",name.Data(),44,0,44);
	name.Form("Multiplicity in %i stilben, circle, steel shell",Nstilben);
	TH1I hmultS_stilben13("hmultS_stl13",name.Data(),44,0,44);
	name.Form("Multiplicity in %i stilben, circle, vacuum shell",Nstilben);
	TH1I hmultV_stilben13("hmultV_stl13",name.Data(),44,0,44);
	name.Form("Multiplicity in %i stilben circle, other, steel shell",Nstilben);
	TH1I hmultS_stilbenOther("hmultS_stlOther",name.Data(),44,0,44);
	name.Form("Multiplicity in %i stilben circle, other, vacuum shell",Nstilben);
	TH1I hmultV_stilbenOther("hmultV_stlOther",name.Data(),44,0,44);
	
	
	name.Form("Multiplicity in %i stilben, steel shell, no gamma",Nstilben);
	TH1I hmultS_stilben_nogamma("hmultS_stl_no gamma",name.Data(),44,0,44);
	name.Form("Multiplicity in %i stilben, vacuum shell, no gamma",Nstilben);
	TH1I hmultV_stilben_nogamma("hmultV_stl_no gamma",name.Data(),44,0,44);
	name.Form("Multiplicity in %i stilben, circle, steel shell, no gamma",Nstilben);
	TH1I hmultS_stilben13_nogamma("hmultS_stl13_no gamma",name.Data(),44,0,44);
	name.Form("Multiplicity in %i stilben, circle, vacuum shell, no gamma",Nstilben);
	TH1I hmultV_stilben13_nogamma("hmultV_stl13_no gamma",name.Data(),44,0,44);
	name.Form("Multiplicity in %i stilben circle, other, steel shell, no gamma",Nstilben);
	TH1I hmultS_stilbenOther_nogamma("hmultS_stlOther_no gamma",name.Data(),44,0,44);
	name.Form("Multiplicity in %i stilben circle, other, vacuum shell, no gamma",Nstilben);
	TH1I hmultV_stilbenOther_nogamma("hmultV_stlOther_no gamma",name.Data(),44,0,44);
	
	name.Form("Multiplicity in %i stilben with threshold Edep>%.3f MeV, steel shell",Nstilben,Eth);
	TH1I hmultS_th_stilben("hmultS_th_stl",name.Data(),44,0,44);
	name.Form("Multiplicity in %i stilben with threshold Edep>%.3f MeV, vacuum shell",Nstilben,Eth);
	TH1I hmultV_th_stilben("hmultV_th_stl",name.Data(),44,0,44);
	name.Form("Multiplicity in %i stilben with threshold Edep>%.3f MeV, circle, steel shell",Nstilben,Eth);
	TH1I hmultS_th_stilben13("hmultS_th_stl13",name.Data(),44,0,44);
	name.Form("Multiplicity in %i stilben with threshold Edep>%.3f MeV, circle, vacuum shell",Nstilben,Eth);
	TH1I hmultV_th_stilben13("hmultV_th_stl13",name.Data(),44,0,44);
	name.Form("Multiplicity in %i stilben with threshold Edep>%.3f MeV, circle, other, steel shell",Nstilben,Eth);
	TH1I hmultS_th_stilbenOther("hmultS_th_stlOther",name.Data(),44,0,44);
	name.Form("Multiplicity in %i stilben with threshold Edep>%.3f MeV, circle, other, vacuum shell",Nstilben,Eth);
	TH1I hmultV_th_stilbenOther("hmultV_th_stlOther",name.Data(),44,0,44);
	
	name.Form("Multiplicity in %i stilben with threshold Edep>%.3f MeV, no gamma, steel shell",Nstilben,Eth);
	TH1I hmultS_th_nogamma_stilben("hmultS_th_nogamma_stl",name.Data(),44,0,44);
	name.Form("Multiplicity in %i stilben with threshold Edep>%.3f MeV, no gamma, vacuum shell",Nstilben,Eth);
	TH1I hmultV_th_nogamma_stilben("hmultV_th_nogamma_stl",name.Data(),44,0,44);
	name.Form("Multiplicity in %i stilben with threshold Edep>%.3f MeV, no gamma, circle, steel shell",Nstilben,Eth);
	TH1I hmultS_th_nogamma_stilben13("hmultS_th_nogamma_stl13",name.Data(),44,0,44);
	name.Form("Multiplicity in %i stilben with threshold Edep>%.3f MeV, no gamma, circle, vacuum shell",Nstilben,Eth);
	TH1I hmultV_th_nogamma_stilben13("hmultV_th_nogamma_stl13",name.Data(),44,0,44);
	name.Form("Multiplicity in %i stilben with threshold Edep>%.3f MeV, no gamma, circle, other, steel shell",Nstilben,Eth);
	TH1I hmultS_th_nogamma_stilbenOther("hmultS_th_nogamma_stlOther",name.Data(),44,0,44);
	name.Form("Multiplicity in %i stilben with threshold Edep>%.3f MeV, no gamma, circle, other, vacuum shell",Nstilben,Eth);
	TH1I hmultV_th_nogamma_stilbenOther("hmultV_th_nogamma_stlOther",name.Data(),44,0,44);
	
	
	name.Form("Channels corresponding to geometric area of %i stilben, vacuum shell",Nstilben);
	TH1I hChannelV_stilben("hChannelV_stl",name.Data(),44,0,44);
	name.Form("Channels corresponding to geometric area of %i stilben, vacuum shell, mult 1",Nstilben);
	TH1I hChannelV_stilben_mult1("hChannelV_stl_mult1",name.Data(),44,0,44);
	name.Form("Channels corresponding to geometric area of %i stilben, vacuum shell, mult 2",Nstilben);
	TH1I hChannelV_stilben_mult2("hChannelV_stl_mult2",name.Data(),44,0,44);
	name.Form("Channels corresponding to geometric area of %i stilben, steel shell",Nstilben);
	TH1I hChannelS_stilben("hChannelS_stl",name.Data(),44,0,44);
	name.Form("Channels corresponding to geometric area of %i stilben, steel shell, mult 1",Nstilben);
	TH1I hChannelS_stilben_mult1("hChannelS_stl_mult1",name.Data(),44,0,44);
	name.Form("Channels corresponding to geometric area of %i stilben, steel shell, mult 2",Nstilben);
	TH1I hChannelS_stilben_mult2("hChannelS_stl_mult2",name.Data(),44,0,44);
	
	name.Form("Channels corresponding to geometric area of %i stilben, vacuum shell, Eth<%f",Nstilben,Eth);
	TH1I hChannelV_th_stilben("hChannelV_th_stl",name.Data(),44,0,44);
	name.Form("Channels corresponding to geometric area of %i stilben, vacuum shell, Eth<%f, mult 1",Nstilben,Eth);
	TH1I hChannelV_th_stilben_mult1("hChannelV_th_stl_mult1",name.Data(),44,0,44);
	name.Form("Channels corresponding to geometric area of %i stilben, vacuum shell, Eth<%f, mult 2",Nstilben,Eth);
	TH1I hChannelV_th_stilben_mult2("hChannelV_th_stl_mult2",name.Data(),44,0,44);
	name.Form("Channels corresponding to geometric area of %i stilben, steel shell, Eth<%f",Nstilben,Eth);
	TH1I hChannelS_th_stilben("hChannelS_th_stl",name.Data(),44,0,44);
	name.Form("Channels corresponding to geometric area of %i stilben, steel shell, Eth<%f, mult 1",Nstilben,Eth);
	TH1I hChannelS_th_stilben_mult1("hChannelS_th_stl_mult1",name.Data(),44,0,44);
	name.Form("Channels corresponding to geometric area of %i stilben, steel shell, Eth<%f, mult 2",Nstilben,Eth);
	TH1I hChannelS_th_stilben_mult2("hChannelS_th_stl_mult2",name.Data(),44,0,44);
	
	name.Form("Channels corresponding to geometric area of %i stilben, vacuum shell, no gamma",Nstilben);
	TH1I hChannelV_nogamma_stilben("hChannelV_nogamma_stl",name.Data(),44,0,44);
	name.Form("Channels corresponding to geometric area of %i stilben, vacuum shell, no gamma, mult 1",Nstilben);
	TH1I hChannelV_nogamma_stilben_mult1("hChannelV_nogamma_stl_mult1",name.Data(),44,0,44);
	name.Form("Channels corresponding to geometric area of %i stilben, vacuum shell, no gamma, mult 2",Nstilben);
	TH1I hChannelV_nogamma_stilben_mult2("hChannelV_nogamma_stl_mult2",name.Data(),44,0,44);
	name.Form("Channels corresponding to geometric area of %i stilben, no gamma, steel shell",Nstilben);
	TH1I hChannelS_nogamma_stilben("hChannelS_nogamma_stl",name.Data(),44,0,44);
	name.Form("Channels corresponding to geometric area of %i stilben, no gamma, steel shell, mult 1",Nstilben);
	TH1I hChannelS_nogamma_stilben_mult1("hChannelS_nogamma_stl_mult1",name.Data(),44,0,44);
	name.Form("Channels corresponding to geometric area of %i stilben, steel shell, no gamma, mult 2",Nstilben);
	TH1I hChannelS_nogamma_stilben_mult2("hChannelS_nogamma_stl_mult2",name.Data(),44,0,44);
	
	name.Form("Channels corresponding to geometric area of %i stilben, vacuum shell, Eth<%f, no gamma",Nstilben,Eth);
	TH1I hChannelV_th_nogamma_stilben("hChannelV_th_nogamma_stl",name.Data(),44,0,44);
	name.Form("Channels corresponding to geometric area of %i stilben, vacuum shell, Eth<%f, no gamma, mult 1",Nstilben,Eth);
	TH1I hChannelV_th_nogamma_stilben_mult1("hChannelV_th_nogamma_stl_mult1",name.Data(),44,0,44);
	name.Form("Channels corresponding to geometric area of %i stilben, vacuum shell, Eth<%f, no gamma, mult 2",Nstilben,Eth);
	TH1I hChannelV_th_nogamma_stilben_mult2("hChannelV_th_nogamma_stl_mult2",name.Data(),44,0,44);
	name.Form("Channels corresponding to geometric area of %i stilben, steel shell, Eth<%f, no gamma",Nstilben,Eth);
	TH1I hChannelS_th_nogamma_stilben("hChannelS_th_nogamma_stl",name.Data(),44,0,44);
	name.Form("Channels corresponding to geometric area of %i stilben, steel shell, Eth<%f, no gamma, mult 1",Nstilben,Eth);
	TH1I hChannelS_th_nogamma_stilben_mult1("hChannelS_th_nogamma_stl_mult1",name.Data(),44,0,44);
	name.Form("Channels corresponding to geometric area of %i stilben, steel shell, Eth<%f, no gamma, mult 2",Nstilben,Eth);
	TH1I hChannelS_th_nogamma_stilben_mult2("hChannelS_th_nogamma_stl_mult2",name.Data(),44,0,44);
	
	name.Form("Channels corresponding to geometric area of %i stilben, vacuum shell",Nstilben);
	TH2I hChannelV2D("hChannelV2D",name.Data(),9,0,9,5,0,5);
	name.Form("Channels corresponding to geometric area of %i stilben, steel shell",Nstilben);
	TH2I hChannelS2D("hChannelS2D",name.Data(),9,0,9,5,0,5);
	
	
	
	name.Form("LightYield vs Edep, vacuum, reaction cut");
	TH2F hLY_vacuum("hLY_vacuum",name.Data(),200,0,20,110,0,11);
	name.Form("LightYield vs Edep, steel, reaction cut");
	TH2F hLY_steel("hLY_steel",name.Data(),200,0,20,110,0,11);
	name.Form("LightYield vs Edep, vacuum, without gamma");
	TH2F hLY_vac_nogamma("hLY_vac_nogamma",name.Data(),200,0,20,110,0,11);
	name.Form("LightYield vs Edep, steel, without gamma");
	TH2F hLY_steel_nogamma("hLY_steel_nogamma",name.Data(),200,0,20,110,0,11);
	
	
	
	
	Int_t mult_digi13=0, mult_digiOther=0, mult_stil13=0, mult_stilOther=0, mult_stil13_nogamma=0, mult_stilOther_nogamma=0,mult_stl13_th_nogamma=0,mult_stlOther_th_nogamma=0;
	
	Int_t NumberV=0,NumberS=0;
//	for(Int_t i=0;i<10;i++){
	for(Int_t i=0;i<tree_simV->GetEntries();i++){
		if(i%10000==0)printf("== %i =============================================\n",i);
		tree_simV->GetEntry(i);
		tree_simS->GetEntry(i);
		triggerS = (Int_t)b_EventHeaderS->GetTrigger();
		triggerV = (Int_t)b_EventHeaderV->GetTrigger();
		//stilbenNcut(13,b_EventHeaderV);
		
		
		
		//общий кат на множественность и реакцию для вакуумной оболочки
		if(arr_nddigiV->GetEntriesFast()>0 && triggerV>0 && arr_telpar_8hedV->GetEntriesFast()==1 && arr_telpar_pV->GetEntriesFast()==1){
			Nstil++;
			mult_digi = arr_nddigiV->GetEntriesFast();
			hmultV.Fill(mult_digi);
			
			mult_th=0;
			circle = stilbenNcut(Nstilben,b_EventHeaderV);
			mult_stil=0;
			mult_nogamma=0;
			mult_stil_nogamma=0;
			mult_th_nogamma=0;
			mult_stl_th_nogamma=0;
			
			mult_digi13=0;
			mult_digiOther=0;
			mult_stil13=0;
			mult_stilOther=0;
			mult_stil13_nogamma=0;
			mult_stilOther_nogamma=0;
			mult_stl13_th_nogamma=0;
			mult_stlOther_th_nogamma=0;
			
			for(Int_t k=0;k<mult_digi;k++){
				Edep = ((ERNDDigi*)arr_nddigiV->At(k))->EdepN1();
				LY = ((ERNDDigi*)arr_nddigiV->At(k))->LightYield();
				channel = ((ERNDDigi*)arr_nddigiV->At(k))->Channel();
				if(Edep>Eth) mult_th++;
				if(Edep>Eth && circle) mult_stil++;
				if(circle) {
					hChannelV_stilben.Fill(channel);
					x_ch = channel%9;
					y_ch = channel/9;
					hChannelV2D.Fill(x_ch,y_ch);
				}
				if(circle&&mult_digi==1) hChannelV_stilben_mult1.Fill(channel);
				if(circle&&mult_digi==2) hChannelV_stilben_mult2.Fill(channel);
				
				if(circle&&Edep>Eth) hChannelV_th_stilben.Fill(channel);
				if(circle&&Edep>Eth&&mult_digi==1) hChannelV_th_stilben_mult1.Fill(channel);
				if(circle&&Edep>Eth&&mult_digi==2) hChannelV_th_stilben_mult2.Fill(channel);
				
				
				
				
				hLY_vacuum.Fill(Edep,LY);
				if(cut_gamma->IsInside(Edep,LY)) trigger_gamma=1;
				else trigger_gamma=-1;
				if(trigger_gamma<0){
					hLY_vac_nogamma.Fill(Edep,LY);
					mult_nogamma++;	
				}
				
				if(circle&&trigger_gamma<0) hChannelV_nogamma_stilben.Fill(channel);
				if(circle&&trigger_gamma<0&&mult_digi==1) hChannelV_nogamma_stilben_mult1.Fill(channel);
				if(circle&&trigger_gamma<0&&mult_digi==2) hChannelV_nogamma_stilben_mult2.Fill(channel);
				
				if(circle&&trigger_gamma<0&&Edep>Eth) hChannelV_th_nogamma_stilben.Fill(channel);
				if(circle&&trigger_gamma<0&&Edep>Eth&&mult_digi==1) hChannelV_th_nogamma_stilben_mult1.Fill(channel);
				if(circle&&trigger_gamma<0&&Edep>Eth&&mult_digi==2) hChannelV_th_nogamma_stilben_mult2.Fill(channel);
				
				if(trigger_gamma<0 && circle) mult_stil_nogamma++;
				if(Edep>Eth && trigger_gamma<0) mult_th_nogamma++;
				if(Edep>Eth && circle && trigger_gamma<0) mult_stl_th_nogamma++;
				
				if(circle&&channel ==13) mult_digi13++;
				if(circle&&channel !=13) mult_digiOther++;
				if(Edep>Eth && circle&&channel ==13) mult_stil13++;
				if(Edep>Eth && circle&&channel !=13) mult_stilOther++;
				if(trigger_gamma<0 && circle&&channel ==13) mult_stil13_nogamma++;
				if(trigger_gamma<0 && circle&&channel !=13) mult_stilOther_nogamma++;
				if(Edep>Eth && trigger_gamma<0 && circle&&channel ==13) mult_stl13_th_nogamma++;
				if(Edep>Eth && trigger_gamma<0 && circle&&channel !=13) mult_stilOther_nogamma++;
			}
			hmultV_th.Fill(mult_th);
			hmultV_nogamma.Fill(mult_nogamma);	
			hmultV_th_nogamma.Fill(mult_th_nogamma);		
			
			if(circle) hmultV_stilben.Fill(mult_digi);
			 hmultV_stilben13.Fill(mult_digi13);
			 hmultV_stilbenOther.Fill(mult_digiOther);
			 
			hmultV_th_stilben.Fill(mult_stil);
			hmultV_th_stilben13.Fill(mult_stil13);
			hmultV_th_stilbenOther.Fill(mult_stilOther);
			
			hmultV_stilben_nogamma.Fill(mult_stil_nogamma);	
			hmultV_stilben13_nogamma.Fill(mult_stil13_nogamma);	
			hmultV_stilbenOther_nogamma.Fill(mult_stilOther_nogamma);	
			
			hmultV_th_nogamma_stilben.Fill(mult_stl_th_nogamma);	
			hmultV_th_nogamma_stilben13.Fill(mult_stl13_th_nogamma);	
			hmultV_th_nogamma_stilbenOther.Fill(mult_stlOther_th_nogamma);	
			
			//if(circle) hChannelV_stilben.Fill(mult_digi);	
			//if(circle&&mult_digi==1) hChannelV_stilben_mult1.Fill(mult_digi);
			//if(circle&&mult_digi==2) hChannelV_stilben_mult2.Fill(mult_digi);
		}
		
		if(arr_nddigiS->GetEntriesFast()>0 && triggerS>0 && arr_telpar_8hedS->GetEntriesFast()==1 && arr_telpar_pS->GetEntriesFast()==1){
			Nstil++;
			mult_digi = arr_nddigiS->GetEntriesFast();
			hmultS.Fill(mult_digi);
			
			mult_th=0;
			circle = stilbenNcut(Nstilben,b_EventHeaderS);
			mult_stil=0;
			mult_nogamma=0;
			mult_stil_nogamma=0;
			mult_th_nogamma=0;
			mult_stl_th_nogamma=0;
			for(Int_t k=0;k<mult_digi;k++){
				Edep = ((ERNDDigi*)arr_nddigiS->At(k))->EdepN1();
				LY = ((ERNDDigi*)arr_nddigiS->At(k))->LightYield();
				channel = ((ERNDDigi*)arr_nddigiS->At(k))->Channel();
				if(Edep>Eth) mult_th++;
				if(Edep>Eth && circle) mult_stil++;
				if(circle) {
					hChannelS_stilben.Fill(channel);
					x_ch = channel%9;
					y_ch = channel/9;
					hChannelS2D.Fill(x_ch,y_ch);
				}
				
				if(circle&&Edep>Eth) hChannelS_th_stilben.Fill(channel);
				if(circle&&Edep>Eth&&mult_digi==1) hChannelS_th_stilben_mult1.Fill(channel);
				if(circle&&Edep>Eth&&mult_digi==2) hChannelS_th_stilben_mult2.Fill(channel);
				
				if(circle&&mult_digi==1) hChannelS_stilben_mult1.Fill(channel);
				if(circle&&mult_digi==2) hChannelS_stilben_mult2.Fill(channel);
				hLY_steel.Fill(Edep,LY);
				if(cut_gamma->IsInside(Edep,LY)) trigger_gamma=1;
				else trigger_gamma=-1;
				if(trigger_gamma<0){
					hLY_steel_nogamma.Fill(Edep,LY);
					mult_nogamma++;					
				}
				
				if(circle&&trigger_gamma<0) hChannelS_nogamma_stilben.Fill(channel);
				if(circle&&trigger_gamma<0&&mult_digi==1) hChannelS_nogamma_stilben_mult1.Fill(channel);
				if(circle&&trigger_gamma<0&&mult_digi==2) hChannelS_nogamma_stilben_mult2.Fill(channel);
				
				if(circle&&trigger_gamma<0&&Edep>Eth) hChannelS_th_nogamma_stilben.Fill(channel);
				if(circle&&trigger_gamma<0&&Edep>Eth&&mult_digi==1) hChannelS_th_nogamma_stilben_mult1.Fill(channel);
				if(circle&&trigger_gamma<0&&Edep>Eth&&mult_digi==2) hChannelS_th_nogamma_stilben_mult2.Fill(channel);
				
				if(trigger_gamma<0 && circle) mult_stil_nogamma++;
				if(Edep>Eth && trigger_gamma<0) mult_th_nogamma++;
				if(Edep>Eth && circle && trigger_gamma<0) mult_stl_th_nogamma++;
			}
			hmultS_th.Fill(mult_th);
			hmultS_nogamma.Fill(mult_nogamma);	
			hmultS_stilben_nogamma.Fill(mult_stil_nogamma);	
			hmultS_th_nogamma.Fill(mult_th_nogamma);
			hmultS_th_nogamma_stilben.Fill(mult_stl_th_nogamma);	
			
			if(circle) hmultS_stilben.Fill(mult_digi);
			hmultS_th_stilben.Fill(mult_stil);
		}
		if(arr_nddigiS->GetEntriesFast()>0 && triggerV>0 && arr_telpar_8hedV->GetEntriesFast()==1 && arr_telpar_pV->GetEntriesFast()==1) NumberV++;
		if(arr_nddigiS->GetEntriesFast()>0 && triggerS>0 && arr_telpar_8hedS->GetEntriesFast()==1 && arr_telpar_pS->GetEntriesFast()==1) NumberS++;
//		if(triggerV>0 && arr_telpar_8hedV->GetEntriesFast()==1 && arr_telpar_pV->GetEntriesFast()==1) NumberV++;
//		if(triggerS>0 && arr_telpar_8hedS->GetEntriesFast()==1 && arr_telpar_pS->GetEntriesFast()==1) NumberS++;
//		if(circle && triggerV>0 && arr_telpar_8hedV->GetEntriesFast()==1 && arr_telpar_pV->GetEntriesFast()==1) NumberV++;
//		if(circle && triggerS>0 && arr_telpar_8hedS->GetEntriesFast()==1 && arr_telpar_pS->GetEntriesFast()==1) NumberS++;	
	}
	printf("++ NumberV = %i ++++++++++++++++++\n++ NumberS = %i ++++++++++++++++++\n",NumberV,NumberS);
	hmultV.Write();
	hmultS.Write();
	//hmultV.Draw();
	hmultV.Print();
	f_outS->Write();
	// printf("Out file %s was created\n",outFile.Data());
}
//-------------------------------------------------------------------------------------------------------------------------
Bool_t stilbenNcut(Int_t N_stilben, ER10Heto8HeEventHeader *b_EventHeader){
	Int_t row=-1,column=-1;
	for(Int_t i=0;i<5;i++){
		for(Int_t k=0;k<9;k++){
			if(N_stilben == k+9*i) {
				row = i;
				column = k;
				break;
			}//		
		}
	}
	//printf("=============\nrow: %i\tcolumn: %i\n=============\n",row,column);
	
	TCut cut;
	TString cut_str;

	cut_str.Form("TMath::Power(MCEventHeader.fn1.fP.Unit().fY / MCEventHeader.fn1.fP.Unit().fZ * NDPoint.fZ + MCEventHeader.fReactionPos.fY -(%f),2)+TMath::Power(MCEventHeader.fn1.fP.Unit().fX / MCEventHeader.fn1.fP.Unit().fZ * NDPoint.fZ + MCEventHeader.fReactionPos.fX -(%f),2)<4.8*4.8",y[row],x[row][column]);
	
	cut = cut_str;
		
	Bool_t circle = 0;
	TLorentzVector lvN1;
	TVector3 vReacPos;
	Double_t Ux, Uy, Uz, X_reaction, Y_reaction, R;

	lvN1 = (TLorentzVector)b_EventHeader->GetN1();
	Ux = lvN1.Vect().Unit().Px();
	Uy = lvN1.Vect().Unit().Py();
	Uz = lvN1.Vect().Unit().Pz();
	vReacPos = (TVector3)b_EventHeader->GetReactionPos();
	X_reaction = vReacPos.X();
	Y_reaction = vReacPos.Y();
	
	R = TMath::Power(Uy/Uz*z[column]+Y_reaction-y[row],2) + TMath::Power(Ux/Uz*z[column]+X_reaction-x[row][column],2);
	
	//printf("=====radius %f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n",R,Ux,Uy,Uz,X_reaction,Y_reaction,x[row][column],y[row],z[column]);
	//if (R<4.8*4.8) printf("=====radius %f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n",R,Ux,Uy,Uz,X_reaction,Y_reaction,x[row][column],y[row],z[column]);
	if (R<4.8*4.8) circle=1;
	
	
	return circle;
}
//-------------------------------------------------------------------------------------------------------------------------
void stilbenCoord(){
//==================== Draw neutron coordinates ============================================	
	/*	
	x[4][0]	x[4][1]	x[4][2]	x[4][3]	x[4][4]	x[4][5]	x[4][6]	x[4][7]	x[4][8]
	x[3][0]	x[3][1]	x[3][2]	x[3][3]	x[3][4]	x[3][5]	x[3][6]	x[3][7]	x[3][8]
	x[2][0]	x[2][1]	x[2][2]	x[2][3]	x[2][4]	x[2][5]	x[2][6]	x[2][7]	x[2][8]
	x[1][0]	x[1][1]	x[1][2]	x[1][3]	x[1][4]	x[1][5]	x[1][6]	x[1][7]	x[1][8]
	x[0][0]	x[0][1]	x[0][2]	x[0][3]	x[0][4]	x[0][5]	x[0][6]	x[0][7]	x[0][8]
	*/
	
	
	Double_t R = 4.8;
	Double_t h = TMath::Sqrt(3)*R;
	
	for (Int_t i=0;i<5;i++) {
		y[i] = -2*h+i*h;
		printf("====== row %i ======\n",i);
		if(i%2==0){
 			for(Int_t k=0;k<9;k++){
				x[i][k] = -8*R+2*k*R;	//x[5][9] = x[i][k]
				printf("%f\t%f\tcolumn %i\n",x[i][k],y[i],k); 
			}
		}	 
		else{
			for(Int_t k=0;k<9;k++){
				x[i][k] = -7*R+2*k*R;
				printf("%f\t%f\tcolumn %i\n",x[i][k],y[i],k);
			}
		}
 	 }
 	 
 	 for(Int_t i=0;i<9;i++){
 	 	if(i==0 || i==8) z[i]=786.5;
 	 	if(i==1 || i==7) z[i]=790.0;
 	 	if(i==2 || i==6) z[i]=792.5;
 	 	if(i==3 || i==5) z[i]=793.99;
 	 	if(i==4) 	 z[i]=794.5;
 	 	
 	 }

}
