/////////////////////////////////////////////////////////////////
// Program validating the data after processed thru the code Ntuple
// 
//
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


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////        
void processRootFile(const char* filename, TTree* outputTree, TH2F* outputHist) {
  // Get the tree "Edep" and set branch addresses     
  TFile* inputFile = TFile::Open(filename); 
  TTree* Tree = dynamic_cast<TTree*>(inputFile->Get("outputTree"));

  int entries = Tree->GetEntries();

  // Set branch addresses
  Tree->SetBranchAddress("cell_idx", &cell_idx);
  Tree->SetBranchAddress("Tedep", &Tedep);

  // Loop over entries in the tree
  Long64_t nEntries = Tree->GetEntries();
  for (Long64_t i = 0; i < nEntries; i++) {
    Tree->GetEntry(i);
    for (size_t j = 0; j < Tedep->size(); j++) {
      double E = (*Tedep)[j]; // Access each value in Tedep vector
      int z_idx = (*cell_idx)[j] / 10000;
      int y_idx = ((*cell_idx)[j] - z_idx * 10000) / 100;
      int x_idx = (*cell_idx)[j] - z_idx * 10000 - y_idx * 100; 
      outputHist->Fill(x_idx, y_idx, E);
    }
  }

  // Close the file
  inputFile->Close();
}// end function


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////        
int test() {
  const char* filename = "data/cublet_kaon/cublet_kaon_3.root";

  TCanvas* canvas = new TCanvas("canvas", "Scatter Plot", 800, 600);
  TH2F *histogram = new TH2F("histogram", "Scatter Plot", 200, 0, 100, 200, 0, 100);

  processRootFile(filename, nullptr, histogram);

  histogram->Draw("COLZ");
  histogram->SetMarkerStyle(21);
  histogram->SetMarkerSize(2);
  histogram->SetMarkerColor(kRed);
  canvas->SetLogz();
  canvas->Update();
  canvas->Draw();
  canvas->SaveAs("outputTree_Edep.png");
  return 0;
}// end test function



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main() {
    test();
    return 0;
}  


