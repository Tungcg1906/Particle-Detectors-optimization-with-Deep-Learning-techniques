/////////////////////////////////////////////////////////////////
// Program plotting energy disribution of Edep tree
// First edition
//
/////////////////////////////////////////////////////////////////
#include <iostream>
#include <fstream>
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
#include <cmath>

#include "Edep.h"
#include "part_info.h"

using namespace std;

static int max_event = 100;
TH2F *histoCell = new TH2F("hist", "Scatter Plot", 100, 0, 100, 100, 0, 100);
TH2F *histoCublet = new TH2F("histo", "Scatter Plot", 10, 0, 10, 10, 0, 10);

// Declare function
void plotEnergyCell(const char* filename);
void plotEnergyCublet(const char* filename);

template <typename T>
std::map<int, vector<UInt_t> > map_indices(T event, int entries);

////////////////////////////////////////////////////////////////////////////////////////////////////////
void plotEnergyCell(const char* filename) {
 
	// Start measuring time
  	auto start_time = std::chrono::high_resolution_clock::now();
  	std::cout << "Processing file: " << filename << std::endl;
	
  	// Get the tree "Edep" and set branch addresses     
  	TFile* inputFile = TFile::Open(filename);  

  	TTree* edepTree = dynamic_cast<TTree*>(inputFile->Get("Edep"));
  	Edep*  event = new Edep(edepTree);
  	int entries = edepTree->GetEntries();

  	std::map<int, std::vector<UInt_t> > m_idx_edep   = map_indices(event, entries);
  	std::cout << "Edep index map complete" << std::endl;

  	int ievent = 0;
  	do {
    	 
	    // check if event index is present
    	if (m_idx_edep.find(ievent) == m_idx_edep.end()) {
     		 continue;
    	}
	
	    int layer_cell_idx = 0;
    	for (int j = 0; j < m_idx_edep[ievent].size(); j+=2) 
      {
      		for (int k = m_idx_edep[ievent][j]; k < m_idx_edep[ievent][j+1]+1; k++) 
          {
			
			      event->GetEntry(k);

            //fill histogram with energies 
            //Note: it is necessary to define the index within the range [0,9999] (xy plane projection)		
			      layer_cell_idx =  int(event->cell_no) % 10000;
            
            int x = layer_cell_idx % 100; 
            int y = layer_cell_idx/100.; 
            double E = (event->edep)/max_event; //average absorbed energy

			      histoCell->Fill(x, y, E);
        	}	  
    	} // end of for loop

	    ievent++;
  	} while (ievent < max_event);

    TCanvas *canvas = new TCanvas("canvas", "Scatter Plot", 800, 600);

    histoCell->Draw("COLZ");
    histoCell->SetMarkerStyle(21);
    histoCell->SetMarkerSize(2);
    histoCell->SetMarkerColor(kRed);
    
    canvas->SetLogz();
    canvas->Update();
    canvas->Draw();
    canvas->SaveAs("Edep_energyDistribution.png");

  	// Stop measuring time
  	auto end_time = std::chrono::high_resolution_clock::now();
  	auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
  	std::cout << "Finished processing file. Time taken: " << duration.count() << " secs. " << std::endl;
}// end function

////////////////////////////////////////////////////////////////////////////////////////////////////////
void plotEnergyCublet(const char* filename) {
 
	// Start measuring time
  	auto start_time = std::chrono::high_resolution_clock::now();
  	std::cout << "Processing file: " << filename << std::endl;
	
  	// Get the tree "Edep" and set branch addresses     
  	TFile* inputFile = TFile::Open(filename);  

  	TTree* edepTree = dynamic_cast<TTree*>(inputFile->Get("Edep"));
  	Edep*  event = new Edep(edepTree);
  	int entries = edepTree->GetEntries();

  	std::map<int, std::vector<UInt_t> > m_idx_edep   = map_indices(event, entries);
  	std::cout << "Edep index map complete" << std::endl;

  	int ievent = 0;
  	do {
    	 
	    // check if event index is present
    	if (m_idx_edep.find(ievent) == m_idx_edep.end()) {
     		 continue;
    	}
	
	    int layer_cell_idx = 0;
    	for (int j = 0; j < m_idx_edep[ievent].size(); j+=2) 
      {
      		for (int k = m_idx_edep[ievent][j]; k < m_idx_edep[ievent][j+1]+1; k++) 
          {
			
			      event->GetEntry(k);
            int cell_idx = int(event->cell_no);

            // Calculate cublet_idx
            int z_idx = cell_idx / 10000;
            int y_idx = (cell_idx - z_idx * 10000) / 100;
            int x_idx = cell_idx - z_idx * 10000 - y_idx * 100;

            int cublet_idx = x_idx / 10 + (y_idx / 10) * 10 + (z_idx / 10) * 100;

            int layer_cublet_idx = cublet_idx%100;

            int x = layer_cublet_idx % 10; 
            int y = layer_cublet_idx/10;

            double E = (event->edep)/max_event; //average absorbed energy

			      histoCublet->Fill(x, y, E);
        	}	  
    	} // end of for loop

	    ievent++;
  	} while (ievent < max_event);

    TCanvas *canvas = new TCanvas("canvas", "Scatter Plot", 600, 600);

    histoCublet->Draw("COLZ");
    histoCublet->SetMarkerStyle(21);
    histoCublet->SetMarkerSize(2);
    histoCublet->SetMarkerColor(kRed);
    
    canvas->SetLogz();
    canvas->Update();
    canvas->Draw();
    canvas->SaveAs("Edep_energyDistribution_Cublet.png");

  	// Stop measuring time
  	auto end_time = std::chrono::high_resolution_clock::now();
  	auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
  	std::cout << "Finished processing file. Time taken: " << duration.count() << " secs. " << std::endl;
}// end function

//////////////////
template <typename T>
std::map<int, vector<UInt_t> > map_indices(T event, int entries) {

  std::map<int, std::vector<UInt_t> > map_idx = {};
  int prev_id = -1;
  for (int j = 0; j < entries; j++) {
    event->GetEntry(j);
    int curr_id = event->event_id;
    // check if new evt index
    if (map_idx.find(curr_id) == map_idx.end()) {
      std::vector<UInt_t> v_temp = {};
      map_idx[curr_id] = v_temp;
    }
    // check if change point
    if (prev_id != curr_id) {
      map_idx[curr_id].push_back(j);
      if (prev_id != -1) {
        map_idx[prev_id].push_back(j-1);
      }
      prev_id = curr_id;
    }
  }
  map_idx[prev_id].push_back(entries-1);
  
  return(map_idx);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
    // Check if the file path is provided as a command-line argument
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <string filePath>" << " <bool cub_repr>"<<std::endl;
        return 1; // Return a non-zero error code
    }

    // Use the file path provided as a command-line argument
    std::string filePath = argv[1];
    bool cub_repr = argv[2];
    
    if (cub_repr == true)
    {
      std::cout << "Choosen representation: cubelets" << std::endl;
      plotEnergyCublet(filePath.c_str());
    }
    else 
    {
      std::cout << "Choosen representation: cells" << std::endl;
      plotEnergyCell(filePath.c_str());
    }

    return 0;
}
