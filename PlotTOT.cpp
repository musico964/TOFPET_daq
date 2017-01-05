PlotTOT()
{
	Int_t n = 6;
	Double_t x[6] = {0.0,10.0,20.0,30.0,40.0,45.0};
	Double_t tot27[6] = {24.0,21.9,18.0,12.0,1.3,0.0};
	Double_t tot32[6] = {15.8,15.1,12.0,10.0,7.0,5.0};
	Double_t tot34[6] = {14.0,13.0,11.0,9.0,6.1,4.0};
	
	for (Int_t i=0; i<n; i++)
	{
		tot27[i] = tot27[i]*6.25;
		tot32[i] = tot32[i]*6.25;
		tot34[i] = tot34[i]*6.25;
	}
	
	TGraph *t27 = new TGraph (5, x, tot27);
	TGraph *t32 = new TGraph (n, x, tot32);
	TGraph *t34 = new TGraph (n, x, tot34);
	
	TCanvas *c1 = new TCanvas("c1","TOT for channels 27, 32, 34",200,10,600,400);
	t27->SetTitle("TOT for channels 27, 32, 34");
	t27->SetLineColor(1);
	t27->SetLineWidth(3);
	t27->SetMarkerStyle(21);
	t27->SetMarkerColor(1);
//	t27->SetMarkerSize(2);
	t32->SetLineColor(2);
	t32->SetLineWidth(3);
	t32->SetMarkerStyle(22);
	t32->SetMarkerColor(2);
//	t32->SetMarkerSize(2);
	t34->SetLineColor(4);
	t34->SetLineWidth(3);
	t34->SetMarkerStyle(23);
	t34->SetMarkerColor(4);
//	t34->SetMarkerSize(2);
	//t27->SetFillStyle(3005);
	t27->GetXaxis()->SetTitle("calib_sw");
	t27->GetYaxis()->SetTitle("TOT (ns)");
	//t27->GetXaxis()->CenterTitle();
	//t27->GetYaxis()->CenterTitle();

	t27->Draw("ACP");
	t32->Draw("CP");
	t34->Draw("CP");

	TLegend *legend = new TLegend(.75,.80,.95,.95);
	legend->AddEntry(t27,"Ch 27 chip 0","L");
	legend->AddEntry(t32,"Ch 32 chip 2","L");
	legend->AddEntry(t34,"Ch 34 chip 2","L");
//	legend->SetHeader("The Legend Title");
	legend->Draw();
}
