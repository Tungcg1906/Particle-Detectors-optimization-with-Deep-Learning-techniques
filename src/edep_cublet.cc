/////////////////////////////////////////////////////////////////
// Program validating the data after processed thru the code Ntuple
// Execute: g++ -o edep_cublet edep_cublet.cc `root-config --cflags --libs`
// Run: ./edep_cublet outputFile.root 5 1       
// particle-parent matrix
/////////////////////////////////////////////////////////////////
#include <TCanvas.h>
#include <TChain.h>
#include <TFile.h>
#include <TH1.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TTree.h>
#include <TGraph.h> 
#include <iostream>
#include <string>
#include <TLegend.h>
#include <vector>
using namespace std;

// Declare variables to hold branch values
int Tentries;
vector<double> *Tedep; 
vector<int> *Tcublet_idx;
vector<int> *Tcell_idx;

void processParticles(const char *filename, const int event, const int set_log)
{
    // Get the tree "outputTree" and set branch addresses
    TFile *inputFile = TFile::Open(filename);
    TTree *Tree = dynamic_cast<TTree *>(inputFile->Get("outputTree"));

    // Set branch addresses
    Tree->SetBranchAddress("Tcells_in_event", &Tentries);
    Tree->SetBranchAddress("Tedep", &Tedep); 
    Tree->SetBranchAddress("Tcublet_idx", &Tcublet_idx); 
    Tree->SetBranchAddress("Tcell_idx", &Tcell_idx); 

    // Create a TGraph for the scatter plot
    TGraph *scatterPlot1 = new TGraph();
    TGraph *scatterPlot2 = new TGraph();
    TGraph *scatterPlot3 = new TGraph();

    // Loop over entries in the tree
    Long64_t nEntries = Tree->GetEntries();

    for (Long64_t i = 0; i < nEntries; i++)
    {
        if (i == event)
        {
            Tree->GetEntry(i);

            // Loop over data points
            for (size_t j = 0; j < Tentries; j++)
            {
                double cubletIdx = (*Tcublet_idx)[j];
                double cellIdx = (*Tcell_idx)[j];
                double edep = (*Tedep)[j];

                // Add data point to the scatter plot
                scatterPlot1->SetPoint(scatterPlot1->GetN(), cubletIdx, edep);
                scatterPlot2->SetPoint(scatterPlot2->GetN(), cellIdx, edep);
                scatterPlot3->SetPoint(scatterPlot3->GetN(), cubletIdx, cellIdx);
            }
        }
    }

    // Create and configure canvas
    TCanvas *canvas = new TCanvas("canvas", "Scatter Plots", 1000, 1100);
    canvas->Divide(1, 3); // Divide canvas into two pads for two plots

    // Draw the first scatter plot
    canvas->cd(1);
    scatterPlot1->SetTitle("Edep vs. Cublet Index");
    scatterPlot1->Draw("AP");
    scatterPlot1->GetXaxis()->SetTitle("cublet indices");
    scatterPlot1->GetYaxis()->SetTitle("edep");
    gPad->Modified();
    gPad->Update();
    //canvas->cd(1)->SetLogy();
    //canvas->cd(1)->SetLogx();

    // Draw the second scatter plot
    canvas->cd(2);
    scatterPlot2->SetTitle("Edep vs. Cell Index");
    scatterPlot2->Draw("AP");
    scatterPlot2->GetXaxis()->SetTitle("cell indices");
    scatterPlot2->GetYaxis()->SetTitle("edep");
    gPad->Modified();
    gPad->Update();
    //canvas->cd(2)->SetLogy();
    //canvas->cd(2)->SetLogx();

    // Draw the third scatter plot
    canvas->cd(3);
    scatterPlot3->SetTitle("Cell Index vs. Cublet Index");
    scatterPlot3->Draw("AP");
    scatterPlot3->GetXaxis()->SetTitle("cublet indices");
    scatterPlot3->GetYaxis()->SetTitle("cell indices");
    gPad->Modified();
    gPad->Update();
    //canvas->cd(3)->SetLogy();
    //canvas->cd(3)->SetLogx();

    // Update canvas and draw
    canvas->Update();
    canvas->Draw();

    // Save canvas as image
    std::string name = "edep_plots_noE_0.png";
    canvas->SaveAs(name.c_str());

    // Clean up the scatter plot objects
    delete scatterPlot1;
    delete scatterPlot2;
    delete scatterPlot3;
}
int main(int argc, char *argv[])
{
    // Check if the file path is provided as a command-line argument
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <string filePath>"
                  << " <int event>"
                  << " <bool set_log>" << std::endl;
        return 1;
    }

    // Use the file path provided as a command-line argument
    std::string filePath = argv[1];
    int event = std::stoi(argv[2]);
    int set_log = std::stoi(argv[3]);

    processParticles(filePath.c_str(), event, set_log);

    return 0;
}

