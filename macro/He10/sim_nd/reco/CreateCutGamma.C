TCutG *cut_gamma;

void CreateCutGamma(){
   cut_gamma = new TCutG("CUTG",12);
   cut_gamma->SetVarX("NDDigi.fEdep");
   cut_gamma->SetVarY("NDDigi.fLightYield");
   cut_gamma->SetTitle("Graph");
   cut_gamma->SetFillColor(1);
   cut_gamma->SetPoint(0,-0.1006886,0.002020071);
   cut_gamma->SetPoint(1,-0.02323593,0.3171716);
   cut_gamma->SetPoint(2,8.047332,9.246465);
   cut_gamma->SetPoint(3,13.39157,15.33939);
   cut_gamma->SetPoint(4,17.55852,17.96566);
   cut_gamma->SetPoint(5,18.47246,17.33535);
   cut_gamma->SetPoint(6,17.06282,15.49697);
   cut_gamma->SetPoint(7,14.30551,12.50303);
   cut_gamma->SetPoint(8,6.993976,5.937374);
   cut_gamma->SetPoint(9,0.11,0);
   cut_gamma->SetPoint(10,-0.1006886,0);
}
