/////////////////////////////////////////////////////////////////
// Program validating the data after processed thru the code Ntuple
// Frist edition
// Execute the code: root -l output_energyPlot.C
// Run the code: ./output_energyPlot.C
/////////////////////////////////////////////////////////////////
#include <iostream>
#include <chrono>
#include <algorithm>
#include <TFile.h>
#include <TTree.h>
#include <string>
#include <vector>
#include <utility> 
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TROOT.h>
#include <TChain.h>
#include <sys/types.h>
#include <dirent.h>
using namespace std;

// Declare variables to hold branch values
vector<int>* cell_idx = nullptr;
vector<double>* Tedep = nullptr;
vector<double>* e_in_cell = nullptr;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////        
void processRootFile(const char* filename, TTree* outputTree, TH2F* outputHist1, TH2F* outputHist2) {
  // Get the tree "Edep" and set branch addresses     
  TFile* inputFile = TFile::Open(filename); 
  TTree* Tree = dynamic_cast<TTree*>(inputFile->Get("outputTree"));

  int entries = Tree->GetEntries();

  // Set branch addresses
  Tree->SetBranchAddress("cell_idx", &cell_idx);
  Tree->SetBranchAddress("Tedep", &Tedep);
  Tree->SetBranchAddress("e_in_cell", &e_in_cell);

  // Loop over entries in the tree
  Long64_t nEntries = Tree->GetEntries();
  for (Long64_t i = 0; i < nEntries; i++) {
    Tree->GetEntry(i);
    for (size_t j = 0; j < Tedep->size(); j++) {
      double E_tot = (*Tedep)[j]; // Access for total energy
      int z_idx = (*cell_idx)[j] / 10000;
      int y_idx = ((*cell_idx)[j] - z_idx * 10000) / 100;
      int x_idx = (*cell_idx)[j] - z_idx * 10000 - y_idx * 100; 
      outputHist1->Fill(x_idx, y_idx, E_tot);
    }// end for loop j
    for (size_t k = 0; k < e_in_cell->size(); k++){
      double E = (*e_in_cell)[k]; // Access for energy
      int z_idx = (*cell_idx)[k] / 10000;
      int y_idx = ((*cell_idx)[k] - z_idx * 10000) / 100;
      int x_idx = (*cell_idx)[k] - z_idx * 10000 - y_idx * 100;
      outputHist2->Fill(x_idx, y_idx, E);
    }// end for loop k
  }// end for loop i

  // Close the file
  inputFile->Close();
}// end function


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////        
int test() {
  const char* filename = "data/cublet_kaon/cublet_kaon_3.root";

  TCanvas* canvas = new TCanvas("canvas", "Histograms", 1000, 600);

  // Creating first histogram
  TH2F *histogram1 = new TH2F("histogram1", "Total Energy Distribution", 100, 0, 100, 100, 0, 100);
  histogram1->GetXaxis()->SetTitle("X");
  histogram1->GetYaxis()->SetTitle("Y");
  histogram1->SetMarkerStyle(21);

  // Creating second histogram
  TH2F *histogram2 = new TH2F("histogram2", "Energy Distribution", 100, 0, 100, 100, 0, 100);
  histogram2->GetXaxis()->SetTitle("X");
  histogram2->GetYaxis()->SetTitle("Y");
  histogram2->SetMarkerStyle(21);

  processRootFile(filename, nullptr, histogram1, histogram2);

  canvas->Divide(2, 1);
  canvas->cd(1);
  histogram1->Draw("COLZ");
  canvas->cd(2);
  histogram2->Draw("COLZ");

  canvas->cd(1)->SetLogz();
  canvas->cd(2)->SetLogz();
  canvas->Update();
  canvas->Draw();
  canvas->SaveAs("outputHistograms.png");
  return 0;
}// end test function




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main() {
    test();
    return 0;
}  
