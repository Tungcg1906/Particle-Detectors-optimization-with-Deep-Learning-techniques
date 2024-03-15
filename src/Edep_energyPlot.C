/////////////////////////////////////////////////////////////////
// Program reformatting large Ntuple .root file
// Fifth edition
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

#include "Edep.h"
#include "part_info.h"

using namespace std;

static int max_event = 100;

// Define vectors                                                                                                                                                                                         
// Initial vectors from Edep                                                                                                                                                                              
std::vector<double> e_in_cell;
std::vector<int> cell_idx;
std::vector<int> eventID;



double sum_energy[10000] = {0.};

// Declare function
void processRootFile(const char* filename, TTree* outputTree);

void summed_energy(std::vector<int> idx, std::vector<double> en);

template <typename T>
std::map<int, vector<UInt_t> > map_indices(T event, int entries);

////////////////////////////////////////////////////////////////////////////////////////////////////////
void processRootFile(const char* filename, TTree* outputTree) {
 
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

    	e_in_cell.clear();
    	cell_idx.clear();
    	eventID.clear();
	
	int layer_cell_idx = 0;
    	for (int j = 0; j < m_idx_edep[ievent].size(); j+=2) {
      		for (int k = m_idx_edep[ievent][j]; k < m_idx_edep[ievent][j+1]+1; k++) {
			
			event->GetEntry(k);
            		
			layer_cell_idx =  int(event->cell_no) % 9999;
			sum_energy[layer_cell_idx] += event->edep;
        	}	  
    	} // end of for loop

	ievent++;
  	} while (ievent < max_event);
 	
  	// Stop measuring time
  	auto end_time = std::chrono::high_resolution_clock::now();
  	auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
  	std::cout << "Finished processing file. Time taken: " << duration.count() << " secs. " << std::endl;
}// end function

///////////
void summed_energy(std::vector<int> idx, std::vector<double> en)
{
	int layer_cell_idx = 0;
	for (int i = 0; i < idx.size(); i++)
	{	
		layer_cell_idx =  idx[i] % 9999;
		sum_energy[layer_cell_idx] += en[i];		
	}	
}

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
  const char* inputFolder = "/lustre/cmswork/adevita/data/";
  const char* outputFolder = "/lustre/cmswork/adevita/data/cublet_";
  //const char* folders[] = {"kaon", "neutron", "pion", "proton"};
  const char* folders[] = {"kaon"};

  for (const char* folder : folders) {
    std::string folderPath = std::string(inputFolder) + folder + "/";
    std::string outputFolderPath = std::string(outputFolder) + folder + "/";
    DIR* dir = opendir(folderPath.c_str());
    if (dir) {
      struct dirent* entry;
      while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_REG && std::string(entry->d_name).find(".root") != std::string::npos) {
	  std::string filePath = folderPath + entry->d_name;
	  std::string outputFilePath = outputFolderPath + "cublet_" + std::string(entry->d_name);

	  // Open the output file for writing
          TFile outputFile(outputFilePath.c_str(), "RECREATE");
          TTree* outputTree = new TTree("outputTree", "Output Tree Description");
	    
          // Process the root file
          processRootFile(filePath.c_str(), outputTree);
	  
          // Print the output file path before processing                                                                                                                                                  
	  std::cout << "Processing file: " << outputFilePath << std::endl;

          // Close the output file and clean up memory
          outputFile.Write();  // Write the TTree to the file
          outputFile.Close();
	  // delete outputTree;

          // Print completion message
	  std::cout << "File processing completed." << std::endl;
        }
      }
      closedir(dir);
    }
  }

  return 0;
}
