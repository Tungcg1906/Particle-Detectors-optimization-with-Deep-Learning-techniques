/////////////////////////////////////////////////////////////////
// Program reformatting large Ntuple .root file
// First edition
//
/////////////////////////////////////////////////////////////////
#include <iostream>
#include <chrono>
#include <set>
#include <algorithm>
#include <TFile.h>
#include <TTree.h>
#include <string>
#include <vector>
#include <utility>  
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TROOT.h>
#include <TChain.h>
#include <fstream>
#include <sstream>
//#include <filesystem>
#include <sys/types.h>
#include <dirent.h>
#include "TStopwatch.h"

#include "Edep.h"
#include "part_info.h"

using namespace std;

// Declare function
void processRootFile(const char* filename, TTree* outputTree);
void fill_n_tuple(std::vector<int>& cell_idx, std::vector<int>& cells_in_cublet, std::vector<int>& cublet_idx, TTree* outputTree);
void cublet_info(int cell_idx, std::vector<int>& cublet_idx, std::vector<int>& cells_in_cublet);

void processRootFile(const char* filename, TTree* outputTree) {
 
  // Start measuring time
  auto start_time = std::chrono::high_resolution_clock::now();

  std::cout << "Processing file: " << filename << std::endl;

  // Define vectors
  std::vector<int> cublet_idx;
  std::vector<int> cells_in_cublet;
  std::vector<double> e_in_cell;
  std::vector<int> cell_idx;
  
  // Get the tree "Edep" and set branch addresses     
  TFile* inputFile = TFile::Open(filename);  
  TTree* edepTree = dynamic_cast<TTree*>(inputFile->Get("Edep"));
  Edep* event = new Edep(edepTree);
  int entries = edepTree->GetEntries();

  // Set branches for output tree
  outputTree->Branch("cublet_idx", &cublet_idx);
  outputTree->Branch("cells_in_cublet", &cells_in_cublet);
  outputTree->Branch("cell_idx", &cell_idx);

  bool branchesCreated = false;
  int ievent = 0;
  for (int j = 0; j < entries; ++j) {
    event->GetEntry(j);

    if (!branchesCreated) {
      // Create branches for "event_id" and "edep" dynamically based on the current event
      outputTree->Branch("event_id", &event->event_id, "event_id/I");
      outputTree->Branch("edep", &event->edep);
      branchesCreated = true;
    }     

    if (event->event_id == ievent) {
      // Add the current entry to vectors
      e_in_cell.push_back(event->edep);
      cell_idx.push_back(event->cell_no);
    }
    else {
      // Fill the new Ntuples
      fill_n_tuple(cell_idx, cells_in_cublet, cublet_idx, outputTree);
      // std::cout << "event: " << event->event_id << ", energy: " << event->edep << std::endl;
      // std::cout << "cublet index: " << cublet_idx[ievent] << ", cells in cublet: " << cells_in_cublet[ievent] << std::endl;
      
      // Reset the iteration
      ++ievent;
      // ievent = event->event_id;

      // Clear the vectors
      e_in_cell.clear();
      cell_idx.clear();
            
      // Add the current entry to vectors
      e_in_cell.push_back(event->edep);
      cell_idx.push_back(event->cell_no); 
    }
  }

  // Stop measuring time
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::minutes>(end_time - start_time);
  std::cout << "Finished processing file. Time taken: " << duration.count() << "mins " << std::endl;
}

void fill_n_tuple(std::vector<int>& cell_idx, std::vector<int>& cells_in_cublet, std::vector<int>& cublet_idx, TTree* outputTree){
  // Clear vectors before filling with new data
  cublet_idx.clear(); 
  cells_in_cublet.clear();
  
  // Convert cell to cublet
  for (int i = 0; i < cell_idx.size(); ++i) {
    cublet_info(cell_idx[i], cublet_idx, cells_in_cublet);
  }

  // Use an unordered_set for faster lookup
  std::unordered_set<int> cubletSet(cublet_idx.begin(), cublet_idx.end());

  // Loop over the indices from 0 to 999
  for (int i = 0; i < 1000; ++i) {

    // std::cout << "Checking cublet index: " << i << std::endl;

    // Check if the cublet index exists in the cubletSet
    if (cubletSet.find(i) != cubletSet.end()) {
      // Store cublet_id and cells_in_cublet
      outputTree->Fill();
      // std::cout << "cublet index: " << cublet_idx[i] << ", cells in cublet: " << cells_in_cublet[i] << std::endl; 
    }
  }
}

void cublet_info(int cell_idx, std::vector<int>& cublet_idx, std::vector<int>& cells_in_cublet){
  // Calculate cublet_idx
  int z_idx = cell_idx / 10000;
  int y_idx = (cell_idx - z_idx * 10000) / 100;
  int x_idx = cell_idx - z_idx * 10000 - y_idx * 100;
  int cublet_x = x_idx / 10;
  int cublet_y = y_idx / 10;
  int cublet_z = z_idx / 10;
  int cublet_idx_info = cublet_x + cublet_y * 10 + cublet_z * 100;

  // Calculate the cell_in_cublet
  int x_in_cublet = x_idx - cublet_x * 10;
  int y_in_cublet = y_idx - cublet_y * 10;
  int z_in_cublet = z_idx - cublet_z * 10;
  int cells_in_cublet_info = x_in_cublet + y_in_cublet * 10 + z_in_cublet * 100;

  // Store the vectors
  cublet_idx.push_back(cublet_idx_info);
  cells_in_cublet.push_back(cells_in_cublet_info);  
}

int main(int argc, char* argv[]) {
  const char* inputFolder = "/lustre/cmswork/xnguyen/data/";
  const char* outputFolder = "/lustre/cmswork/xnguyen/data/cublet_";
  //const char* folders[] = {"neutron"};
  const char* folders[] = {"kaon", "neutron", "pion", "proton"};
  
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
