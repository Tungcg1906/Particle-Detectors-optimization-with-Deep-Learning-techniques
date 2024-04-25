/////////////////////////////////////////////////////////////////
// Program validating the data after processed thru the code Ntuple
// Execute: g++ -o energy_delta_edep energy_delta_edep.cc `root-config --cflags --libs`
// Run: ./energy_delta_edep outputFile.root 5 1       
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
#include <algorithm>
#include <chrono>
#include <dirent.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <TLegend.h>
#include <utility>
#include <vector>
using namespace std;

// Declare variables to hold branch values
int Tentries;

vector<int> *Ttrack;
vector<double> *Tparent_id;
vector<double> *Tpdg;
vector<double> *Tedep; 
vector<double> *Tdeltae;

// Particle types of interest
std::vector<int> particles_of_interest = {22, 11, -11, 2112, 2212, 211, -211, 111, 321, -321};
std::vector<string> labels = {"#gamma", "e-", "e+", "n", "p", "#pi+", "#pi-", "#pi0", "K+", "K-"};
int num_part = particles_of_interest.size();

TH1F *energyHistograms[10]; // Array of histograms for each particle type

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void processParticles(const char *filename, const int event, const int set_log)
{
    // Get the tree "outputTree" and set branch addresses
    TFile *inputFile = TFile::Open(filename);
    TTree *Tree = dynamic_cast<TTree *>(inputFile->Get("outputTree"));

    // Set branch addresses
    Tree->SetBranchAddress("Tcells_in_event", &Tentries);
    Tree->SetBranchAddress("Tpdg", &Tpdg);
    Tree->SetBranchAddress("Ttrack", &Ttrack);
    Tree->SetBranchAddress("Tparent_id", &Tparent_id);
    Tree->SetBranchAddress("Tedep", &Tedep); 
    Tree->SetBranchAddress("Tdeltae", &Tdeltae); 
    // Loop over entries in the tree
    Long64_t nEntries = Tree->GetEntries();

    for (Long64_t i = 0; i < nEntries; i++)
    {
        if (i == event)
        {
            Tree->GetEntry(i);

            for (size_t j = 0; j < Tentries; j++)
            {
                Long64_t pdg = (*Tpdg)[j];
                Long64_t parent = (*Tparent_id)[j];
                double E = (*Tdeltae)[j] + (*Tedep)[j]; 

                // Check if particle is one of the particles of interest
                auto it = find(particles_of_interest.begin(), particles_of_interest.end(), pdg);
                if (it != particles_of_interest.end())
                {
                    // Find the index of the particle type
                    int index = distance(particles_of_interest.begin(), it);

                    // Fill the energy into the histogram corresponding to the particle type
                    energyHistograms[index]->Fill(E);
                }
            }
        }
    }

    ////////////////////////////
    //////// GRAPHICS //////////
    ////////////////////////////
    
    // Calculate the maximum count among all histograms
    double maxCount = 0;
    for (int i = 0; i < num_part; ++i) {
        if (energyHistograms[i]->GetMaximum() > maxCount) {
            maxCount = energyHistograms[i]->GetMaximum();
        }
    }

    // Create and configure canvas
    TCanvas *canvas = new TCanvas("canvas", "Energy Histograms", 1000, 1000);

    // Adjust the y-axis range based on the maximum count
    double yMax = maxCount * 2; // Margin for better visualization
    energyHistograms[0]->SetMaximum(yMax);

    // Draw histograms
    for (int i = 0; i < num_part; ++i)
    {
        energyHistograms[i]->SetLineColor(i + 1); // Set line color for each histogram
        energyHistograms[i]->Draw("HIST SAME");
    }

    // Set histogram titles and labels
    for (int i = 0; i < num_part; ++i)
    {
        energyHistograms[i]->SetTitle("Energy Distribution (Delta E + Edep)");
        energyHistograms[i]->GetXaxis()->SetTitle("Energy");
        gPad->Modified();
        energyHistograms[i]->GetXaxis()->SetRange(-1e3,50);
        gPad->Update(); 
    }

    // Add legend
    TLegend *legend = new TLegend(0.1, 0.7, 0.3, 0.9);
    for (int i = 0; i < num_part; ++i)
    {
        legend->AddEntry(energyHistograms[i], labels[i].c_str(), "l");
    }
    legend->Draw();
    // Update canvas and draw
    canvas->Update();
    canvas->SetLogy();
    //canvas->SetLogx();
    canvas->Draw();
    
    // Save canvas as image
    std::string name = "energy_histograms.png";
    canvas->SaveAs(name.c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

    // Create histograms for each particle type
    for (int i = 0; i < num_part; ++i)
    {
        energyHistograms[i] = new TH1F(Form("energy_histograms%d", particles_of_interest[i]), "", 50, -1e3, 50);
        energyHistograms[i]->SetStats(0);
    }

    processParticles(filePath.c_str(), event, set_log);

    // Cleanup
    for (int i = 0; i < num_part; ++i)
    {
        delete energyHistograms[i];
    }

    return 0;
}
