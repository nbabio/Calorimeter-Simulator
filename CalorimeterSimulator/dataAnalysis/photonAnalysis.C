//----------------------------------------------------------------------//
// Optical photon analysis: compares the photons created in the fiber   //
// cores ("photons" tree) with the ones leaving the fibers              //
// ("exitPhotons" tree). Photons are matched by (eventNumber, trackID). //
//                                                                      //
// Usage: root -l -b -q 'photonAnalysis.C("output.root", "plots")'      //
//----------------------------------------------------------------------//
#include <map>
#include <utility>
#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TLine.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TMath.h"
#include "TEfficiency.h"

void photonAnalysis(const char *fileName = "output.root", const char *outDir = "photonPlots",
                    double nCore = 1.59, double nClad = 1.49) { // Refractive indices of the core and cladding are hardcoded

    gStyle->SetOptStat(0);
    gSystem->mkdir(outDir, kTRUE);

    TFile *f = TFile::Open(fileName);
    TTree *tc = (TTree*) f->Get("photons");
    TTree *te = (TTree*) f->Get("exitPhotons");

    //Created photons
    int cEvt, cDet, cLayer, cTrack;
    double cDirz, cLocalz, cLambda, cTime;
    tc->SetBranchAddress("eventNumber", &cEvt);
    tc->SetBranchAddress("det", &cDet);
    tc->SetBranchAddress("layer", &cLayer);
    tc->SetBranchAddress("trackID", &cTrack);
    tc->SetBranchAddress("dirz", &cDirz);
    tc->SetBranchAddress("localz", &cLocalz);
    tc->SetBranchAddress("wavelength", &cLambda);
    tc->SetBranchAddress("time", &cTime);

    //Exiting photons
    int eEvt, eTrack, eSide, eVol;
    double eLocalz, eLambda, eTime, eLength, eDirOutz;
    te->SetBranchAddress("eventNumber", &eEvt);
    te->SetBranchAddress("trackID", &eTrack);
    te->SetBranchAddress("side", &eSide);
    te->SetBranchAddress("exitVolume", &eVol);
    te->SetBranchAddress("localz", &eLocalz);
    te->SetBranchAddress("wavelength", &eLambda);
    te->SetBranchAddress("time", &eTime);
    te->SetBranchAddress("trackLength", &eLength);
    te->SetBranchAddress("dirOutz", &eDirOutz);

    //Index of the created photons by (event, trackID)
    std::map<std::pair<int,int>, Long64_t> created;
    for(Long64_t i = 0; i < tc->GetEntries(); i++) {
        tc->GetEntry(i);
        created[{cEvt, cTrack}] = i;
    }

    //Histograms
    TH1D *hCosAll  = new TH1D("hCosAll",  ";|cos#theta| at creation (w.r.t. fiber axis);Photons", 50, 0, 1);
    TH1D *hCosExit = new TH1D("hCosExit", ";|cos#theta| at creation (w.r.t. fiber axis);Photons", 50, 0, 1);
    TH1D *hLamAll  = new TH1D("hLamAll",  ";#lambda [nm];Normalized", 60, 360, 520);
    TH1D *hLamExit = new TH1D("hLamExit", ";#lambda [nm];Normalized", 60, 360, 520);
    TH1D *hDt      = new TH1D("hDt",      ";t_{exit} - t_{creation} [ns];Photons", 100, 0, 10);
    TH2D *hDtL     = new TH2D("hDtL",     ";Track length [cm];t_{exit} - t_{creation} [ns]", 100, 30, 130, 100, 0, 10);
    TH2D *hPathCos = new TH2D("hPathCos", ";1/|cos#theta| at creation;Track length / |#Deltaz|", 100, 1, 1.5, 100, 1, 1.5);
    TH1D *hTimeP   = new TH1D("hTimeP",   ";Arrival time [ns];Photons", 100, 0, 40);
    TH1D *hTimeM   = new TH1D("hTimeM",   ";Arrival time [ns];Photons", 100, 0, 40);
    TH1D *hOut     = new TH1D("hOut",     ";Exit angle in air #theta_{out} [deg];Photons", 90, 0, 90);
    //Efficiency per (det, layer): bin = 10*det + layer
    TH1D *hDLAll   = new TH1D("hDLAll",  "", 40, -0.5, 39.5);
    TH1D *hDLExit  = new TH1D("hDLExit", "", 40, -0.5, 39.5);

    for(Long64_t i = 0; i < tc->GetEntries(); i++) {
        tc->GetEntry(i);
        hCosAll->Fill(TMath::Abs(cDirz));
        hLamAll->Fill(cLambda);
        hDLAll->Fill(10*cDet + cLayer);
    }

    Long64_t nMatched = 0, nCore0 = 0;
    for(Long64_t i = 0; i < te->GetEntries(); i++) {
        te->GetEntry(i);
        hLamExit->Fill(eLambda);
        hOut->Fill(TMath::ACos(TMath::Abs(eDirOutz)) * TMath::RadToDeg());
        (eSide > 0 ? hTimeP : hTimeM)->Fill(eTime);
        if(eVol == 0) nCore0++;

        auto it = created.find({eEvt, eTrack});
        if(it == created.end()) continue;
        nMatched++;
        tc->GetEntry(it->second);
        double absCos = TMath::Abs(cDirz);
        hCosExit->Fill(absCos);
        hDLExit->Fill(10*cDet + cLayer);
        hDt->Fill(eTime - cTime);
        hDtL->Fill(eLength, eTime - cTime);
        double dz = TMath::Abs(eLocalz - cLocalz);
        if(dz > 0 && absCos > 0) hPathCos->Fill(1.0/absCos, eLength/dz);
    }

    //Summary
    double nC = tc->GetEntries(), nE = te->GetEntries();
    double cosCrit = nClad/nCore;
    printf("\nCreated photons:        %.0f\n", nC);
    printf("Exiting photons:        %.0f (matched to a created photon: %lld)\n", nE, nMatched);
    printf("  from core / cladding: %lld / %lld\n", nCore0, (Long64_t)nE - nCore0);
    printf("  side +1 / side -1:    %.0f / %.0f\n", hTimeP->GetEntries(), hTimeM->GetEntries());
    printf("Trapping efficiency:    %.2f %% (%.2f %% per end)\n", 100*nE/nC, 50*nE/nC);
    printf("Meridional estimate:    %.2f %% per end, (1 - n_clad/n_core)/2\n", 50*(1 - cosCrit));
    printf("Critical angle:         %.1f deg (cos = %.3f)\n", TMath::ACos(cosCrit)*TMath::RadToDeg(), cosCrit);
    printf("Mean wavelength:        created %.1f nm, exit %.1f nm\n", hLamAll->GetMean(), hLamExit->GetMean());
    printf("Mean propagation time:  %.2f ns\n", hDt->GetMean());
    if(hDtL->GetEntries() > 2) {
        hDtL->Fit("pol1", "Q0");
        double slope = hDtL->GetFunction("pol1")->GetParameter(1); //ns/cm
        printf("Effective speed:        %.2f cm/ns (c/n_core = %.2f cm/ns)\n\n", 1.0/slope, 29.98/nCore);
    }

    TFile *out = new TFile(Form("%s/photonAnalysis.root", outDir), "RECREATE");

    //Efficiencies (built before styling the histograms)
    TEfficiency *eff = new TEfficiency(*hCosExit, *hCosAll);
    eff->SetName("acceptance");
    eff->SetTitle(";|cos#theta| at creation;Fraction that exits the fiber");
    TEfficiency *effDL = new TEfficiency(*hDLExit, *hDLAll);
    effDL->SetName("effDetLayer");
    effDL->SetTitle(";10 #times det + layer;Trapping efficiency");
    TEfficiency *effLam = new TEfficiency(*hLamExit, *hLamAll);
    effLam->SetName("transmissionLambda");
    effLam->SetTitle(";#lambda [nm];Fraction that exits the fiber");
    for(TH1 *h : {(TH1*)hCosAll, (TH1*)hLamAll, (TH1*)hTimeM}) { h->SetLineColor(kBlue); h->SetLineWidth(2); }
    for(TH1 *h : {(TH1*)hCosExit, (TH1*)hLamExit, (TH1*)hTimeP}) { h->SetLineColor(kRed); h->SetLineWidth(2); }

    //1) Angular acceptance
    TCanvas *c1 = new TCanvas("c1", "", 1200, 500);
    c1->Divide(2, 1);
    c1->cd(1);
    hCosAll->SetMinimum(0.5); gPad->SetLogy();
    hCosAll->Draw("hist"); hCosExit->Draw("hist same");
    TLegend *l1 = new TLegend(0.15, 0.75, 0.5, 0.88);
    l1->AddEntry(hCosAll, "Created", "l"); l1->AddEntry(hCosExit, "Exiting", "l"); l1->Draw();
    c1->cd(2);
    eff->Draw("AP");
    gPad->Update();
    TLine *lc = new TLine(cosCrit, 0, cosCrit, 1.05);
    lc->SetLineStyle(2); lc->SetLineColor(kGray+2); lc->Draw();
    c1->SaveAs(Form("%s/acceptance.png", outDir));

    //2) Efficiency per detector/layer
    TCanvas *c2 = new TCanvas("c2", "", 800, 500);
    effDL->Draw("AP");
    c2->SaveAs(Form("%s/efficiency_det_layer.png", outDir));

    //3) Wavelength spectra
    TCanvas *c3 = new TCanvas("c3", "", 1200, 500);
    c3->Divide(2, 1);
    c3->cd(1);
    TH1D *hLamAllN = (TH1D*) hLamAll->Clone("hLamAllN");
    TH1D *hLamExitN = (TH1D*) hLamExit->Clone("hLamExitN");
    hLamAllN->Scale(1.0/hLamAllN->Integral()); hLamExitN->Scale(1.0/hLamExitN->Integral());
    hLamAllN->SetMaximum(1.2*TMath::Max(hLamAllN->GetMaximum(), hLamExitN->GetMaximum()));
    hLamAllN->Draw("hist"); hLamExitN->Draw("hist same");
    l1->Draw();
    c3->cd(2);
    effLam->Draw("AP");
    c3->SaveAs(Form("%s/wavelength.png", outDir));

    //4) Timing and optical path
    TCanvas *c4 = new TCanvas("c4", "", 1800, 500);
    c4->Divide(3, 1);
    c4->cd(1); hDt->SetLineWidth(2); hDt->Draw("hist");
    c4->cd(2); hDtL->Draw("colz");
    c4->cd(3); hPathCos->Draw("colz");
    c4->SaveAs(Form("%s/timing_path.png", outDir));

    //5) Arrival time per side and exit angle in air
    TCanvas *c5 = new TCanvas("c5", "", 1200, 500);
    c5->Divide(2, 1);
    c5->cd(1);
    hTimeP->SetMaximum(1.2*TMath::Max(hTimeP->GetMaximum(), hTimeM->GetMaximum()));
    hTimeP->Draw("hist"); hTimeM->Draw("hist same");
    TLegend *l5 = new TLegend(0.6, 0.75, 0.88, 0.88);
    l5->AddEntry(hTimeP, "Side +1", "l"); l5->AddEntry(hTimeM, "Side -1", "l"); l5->Draw();
    c5->cd(2); hOut->Draw("hist");
    c5->SaveAs(Form("%s/arrival_exitAngle.png", outDir));

    out->cd();
    for(TObject *o : {(TObject*)hCosAll, (TObject*)hCosExit, (TObject*)hLamAll, (TObject*)hLamExit,
                      (TObject*)hDt, (TObject*)hDtL, (TObject*)hPathCos, (TObject*)hTimeP, (TObject*)hTimeM,
                      (TObject*)hOut, (TObject*)eff, (TObject*)effDL, (TObject*)effLam})
        o->Write();
    out->Close();
    f->Close();
}
