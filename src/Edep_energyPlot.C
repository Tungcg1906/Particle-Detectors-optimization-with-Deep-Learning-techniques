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
double sum_energy[10000] = {0.};

// Declare function
void plotEnergy(const char* filename);

template <typename T>
std::map<int, vector<UInt_t> > map_indices(T event, int entries);

void scatterPlot(double *data, int dataSize);
////////////////////////////////////////////////////////////////////////////////////////////////////////
void plotEnergy(const char* filename) {
 
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

            //sum energies depending on the cell idx.
            //Note: it is necessary to define the index within the range [0,9999] (xy plane projection)		
			      layer_cell_idx =  int(event->cell_no) % 9999;
			      sum_energy[layer_cell_idx] += event->edep;
        	}	  
    	} // end of for loop

	    ievent++;
  	} while (ievent < max_event);

    scatterPlot(sum_energy, 10000);

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

void scatterPlot(double *data, int dataSize) {

    TCanvas *canvas = new TCanvas("canvas", "Scatter Plot", 800, 800);
    TH2F *histogram = new TH2F("histogram", "Scatter Plot", 100, 0, 100, 100, 0, 100);

    for (int i = 0; i < dataSize; ++i) {
        int x = i % 99; 
        int y = 99 - std::floor(i/99); 
        int color = data[i]/max_event; //average absorbed energy

        histogram->SetBinContent(x + 1, y + 1, color);
    }

    histogram->SetMarkerStyle(21);
    histogram->SetMarkerSize(2);

    histogram->Draw("COLZ");
    canvas->SaveAs("Edep_energyDistribution.png");
    std::cout << "Result of the analysis has been saved to Edep_energyDistribution.png" << std::endl;

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
    // Check if the file path is provided as a command-line argument
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filePath>" << std::endl;
        return 1; // Return a non-zero error code
    }

    // Use the file path provided as a command-line argument
    std::string filePath = argv[1];

    // Call the plotEnergy function with the file path converted to const char*
    plotEnergy(filePath.c_str());

    return 0;
}
