
#include "Measurement1D.h"
#include "TH1D.h"

#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include <stdlib.h>

using namespace std;

class DUNEMecSandbox : public Measurement1D {
public:
  std::unique_ptr<TH1D> fhist_q0;
  std::unique_ptr<TH1D> fhist_q0_unweighted;
  std::unique_ptr<TH1D> fhist_q0_MECOnly;
  std::unique_ptr<TH1D> fhist_q3;
  std::unique_ptr<TH1D> fhist_q3_MECOnly;

  std::unique_ptr<TH2D> fhist_q0q3;
  std::unique_ptr<TH2D> fhist_q0q3_MECOnly;

  //********************************************************************
  DUNEMecSandbox(nuiskey samplekey) {
    //********************************************************************

    // START boilerplate
    //
    // Setup common settings
    fSettings = LoadSampleSettings(samplekey);
    fSettings.SetAllowedTypes("FIX,FREE,SHAPE/FULL,DIAG/MASK", "FIX/FULL");
    fSettings.SetEnuRange(0.0, 100.0);
    fSettings.DefineAllowedTargets("Ar");
    fSettings.DefineAllowedSpecies("numu");

    fSettings.SetTitle("");
    fSettings.SetXTitle("p_{T} [GeV/#it{c}]");
    fSettings.SetYTitle("p_{z} [GeV/#it{c}]");
    // END boilerplate

    fhist_q0 = std::make_unique<TH1D>("fhist_q0", "", 100, 0, 2);
    fhist_q0_unweighted =
        std::make_unique<TH1D>("fhist_q0_unweighted", "", 100, 0, 2);
    fhist_q0_MECOnly =
        std::make_unique<TH1D>("fhist_q0_MECOnly", "", 100, 0, 2);
    fhist_q3 = std::make_unique<TH1D>("fhist_q3", "", 100, 0, 2);
    fhist_q3_MECOnly =
        std::make_unique<TH1D>("fhist_q3_MECOnly", "", 100, 0, 2);

    fhist_q0q3 = std::make_unique<TH2D>("fhist_q0q3", "", 100, 0, 2, 100, 0, 2);
    fhist_q0q3_MECOnly =
        std::make_unique<TH2D>("fhist_q0q3_MECOnly", "", 100, 0, 2, 100, 0, 2);

    // this is just to appease NUISANCE, it is copied to make the tracked 'MC'
    // histogram
    fDataHist = new TH1D("DUNEMecSandbox", "", 1, 0, 1);

    // more boilerplate
    FinaliseSampleSettings();
    fScaleFactor =
        ((GetEventHistogram()->Integral("width") * 1E-38) / (fNEvents + 0.)) /
        this->TotalIntegratedFlux();
    FinaliseMeasurement();
  };

  //HOW TO ADD NEW VARIABLES: 
  // to add new variables add them to this class 9all methods/initializers)
  // then make sure they are set in FillEventVariables, and then you can
  // use them to fill histograms in FillExtraHistograms
  struct MyMECBox : public MeasurementVariableBox1D {
    MyMECBox() : MeasurementVariableBox1D(), mode{0}, q0{0}, q3{0} {}

    MeasurementVariableBox *CloneSignalBox() {
      auto cl = new MyMECBox();

      cl->fX = fX;
      cl->mode = mode;
      cl->q0 = q0;
      cl->q3 = q3;
      return cl;
    }

    void Reset() {
      MeasurementVariableBox1D::Reset();
      mode = 0;
      q0 = 0;
      q3 = 0;
    }

    int mode;
    double q0, q3;
  };

  MeasurementVariableBox *CreateBox() { return new MyMECBox(); };

  //********************************************************************
  void FillEventVariables(FitEvent *event) {
    //********************************************************************

    FitParticle *neutrino = event->GetNeutrinoIn();
    FitParticle *muon = event->GetHMFSParticle(13);

    if (!muon || !neutrino) {
      return;
    }

    auto myvarbox = dynamic_cast<MyMECBox *>(GetBox());
    myvarbox->mode = event->Mode;

    TLorentzVector q = neutrino->fP - muon->fP;
    myvarbox->q0 = (q.E()) / 1.E3;
    myvarbox->q3 = (q.Vect().Mag()) / 1.E3;
    fXVar = 1;

    //set any new variables in the box here
    //myvarbox->mynewvar = <some calc>
  }

  //the weight parameter corresponds to the weight for this event for the
  // currently processing throw/comparison
  void FillExtraHistograms(MeasurementVariableBox *vars, double weight) {

    Measurement1D::FillExtraHistograms(vars, weight);

    auto myvarbox = dynamic_cast<MyMECBox *>(vars);

    fhist_q0->Fill(myvarbox->q0, weight);
    fhist_q0_unweighted->Fill(myvarbox->q0);
    fhist_q3->Fill(myvarbox->q3, weight);
    fhist_q0q3->Fill(myvarbox->q0, myvarbox->q3, weight);

    fhist_q0_MECOnly->Fill(myvarbox->q0, weight * (myvarbox->mode == 2));
    fhist_q3_MECOnly->Fill(myvarbox->q3, weight * (myvarbox->mode == 2));
    fhist_q0q3_MECOnly->Fill(myvarbox->q0, myvarbox->q3,
                             weight * (myvarbox->mode == 2));

    //fill any new histograms here
    //mynewhist->Fill(myvarbox->mynewvar, weight)
  }

  //********************************************************************
  bool isSignal(FitEvent *event) {
    //********************************************************************

    FitParticle *neutrino = event->GetNeutrinoIn();
    FitParticle *muon = event->GetHMFSParticle(13);

    return (muon && neutrino);
  }

  void Write(std::string drawOpt) {

    fhist_q0->Write();
    fhist_q0_unweighted->Write();
    fhist_q3->Write();
    fhist_q0_MECOnly->Write();
    fhist_q3_MECOnly->Write();
    fhist_q0q3->Write();
    fhist_q0q3_MECOnly->Write();
    //make sure to write out new histograms
    //mynewhist->Write();

    // we have to tidy this up in this SO if we don't want horrible crashes on
    // program tear down
    fDataHist->Write();
    fDataHist->SetDirectory(nullptr);
    fMCHist->Write();
    fMCHist->SetDirectory(nullptr);
  }

  void ResetAll() {
    Measurement1D::ResetAll();

    fhist_q0->Reset();
    fhist_q0_unweighted->Reset();
    fhist_q3->Reset();
    fhist_q0_MECOnly->Reset();
    fhist_q3_MECOnly->Reset();
    fhist_q0q3->Reset();
    fhist_q0q3_MECOnly->Reset();

    //make sure to reset out new histograms
    //mynewhist->Reset();
  }
};