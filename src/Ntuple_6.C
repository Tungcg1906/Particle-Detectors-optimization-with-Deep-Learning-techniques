/////////////////////////////////////////////////////////////////
// Program reformatting large Ntuple .root file
// Sixth edition
// Arrays are declared globally
// Example: kaon_3.root
// for 5 events -> ??? secs to finish
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

#include "Edep.h"
#include "part_info.h"

using namespace std;

static int max_event = 5;

const int MAX_SIZE = 1000000; 

// Arrays for Edep
double e_in_cell[MAX_SIZE];
int cell_idx[MAX_SIZE];
int eventID[MAX_SIZE];

// Arrays for part_info
int pipdg[MAX_SIZE];
int picell[MAX_SIZE];
double pimom[MAX_SIZE];

// Arrays for created vectors
int cublet_idx[MAX_SIZE];
int cells_in_cublet[MAX_SIZE];
int picublet_idx[MAX_SIZE];
int picells_in_cublet[MAX_SIZE];

// Arrays for compressed vectors
double Te_in_cell[MAX_SIZE];
int Tcublet_idx[MAX_SIZE];
int Tcells_in_cublet[MAX_SIZE];
int TeventID[MAX_SIZE];
int Tn_in_cell[MAX_SIZE];
int Tpdg_id[MAX_SIZE];
double Tmom[MAX_SIZE];
int Tphoton[MAX_SIZE];


// Declare function
void processRootFile(const char* filename, TTree* outputTree);
void fill_n_tuple(TTree* outputTree);
void cublet_info(int cell_idx);
void cublet_part_info(int picell);

////////////////////////////////////////////////////////////////////////////////////////////////////////
void processRootFile(const char* filename, TTree* outputTree) {
 
  // Start measuring time
  auto start_time = std::chrono::high_resolution_clock::now();
  std::cout << "Processing file: " << filename << std::endl;

  // Get the tree "Edep" and set branch addresses     
  TFile* inputFile = TFile::Open(filename);  

  TTree* edepTree = dynamic_cast<TTree*>(inputFile->Get("Edep"));
  TTree* partInfoTree = dynamic_cast<TTree*>(inputFile->Get("part_info"));
  Edep*  event = new Edep(edepTree);
  part_info* pievent = new part_info(partInfoTree);
  int entries = edepTree->GetEntries();
  int pientries = partInfoTree->GetEntries();

  // Set branches for output tree
  outputTree->Branch("Tcublet_idx", &Tcublet_idx, "Tcublet_idx[MAX_SIZE]/I");
  outputTree->Branch("Tcells_in_cublet", &Tcells_in_cublet, "Tcells_in_cublet[MAX_SIZE]/I");
  outputTree->Branch("Tevent_id", &TeventID, "Tevent_id[MAX_SIZE]/I");
  outputTree->Branch("Tedep", &Te_in_cell, "Tedep[MAX_SIZE]/D");
  outputTree->Branch("Tpdg_id", &Tpdg_id, "Tpdg_id[MAX_SIZE]/I");
  outputTree->Branch("Tmom", &Tmom, "Tmom[MAX_SIZE]/D");
  outputTree->Branch("Tphoton", &Tphoton, "Tphoton[MAX_SIZE]/I");
  
  int ievent = 0;
  int epsilon = 1;
  bool data = false;
  do {
    data = false;
    
    // Clear the arrays
    delete[] e_in_cell;
    delete[] cell_idx;
    delete[] eventID;
    delete[] pipdg;
    delete[] picell;
    delete[] pimom;

    int currentIndex = 0; // Track the current index
    for (int j = 0; j < entries; j++) {
      event->GetEntry(j);
      if (event->event_id == ievent) {
	// Add the current entry to arrays
	e_in_cell[currentIndex] = event->edep;
        cell_idx[currentIndex] = event->cell_no;
        eventID[currentIndex] = event->event_id;
	currentIndex++;
	data = true;
      } // end if loop
    } // end of for loop on all data file information

    for (int j = 0; j < pientries; j++) {
      pievent->GetEntry(j);
      if (pievent->event_id == ievent) {
	// Add the current entry to vectors
	pipdg[currentIndex] = pievent->pdg_id;
	double x = pievent->pos_x;
	double y = pievent->pos_y;
	double z = pievent->pos_z;
	int ix = (x + 150 - epsilon) / 3;
	int iy = (-y + 150 - epsilon) / 3;
	int iz = (z - epsilon) / 12;
	picell[currentIndex] = ix + 100 * iy + 10000 * iz;
        pimom[currentIndex] = pievent->mom;
        currentIndex++;
        data = true;
      } // end if loop
    } // end of for loop on all data file information
    
    // Fill the new Ntuples
    fill_n_tuple (outputTree);
    
    for (size_t i = 0; i < MAX_SIZE; ++i) {
      std::cout << "Tpdg_id : " << Tpdg_id[i]
		<< " , Tmom : " << Tmom[i] << " , Tphoton: " << Tphoton[i] << std::endl;
    }
   
    outputTree->Fill();
    ievent++;
  } while (data && ievent < max_event);
  
  // Stop measuring time
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
  std::cout << "Finished processing file. Time taken: " << duration.count() << " secs. " << std::endl;
}// end function


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void fill_n_tuple(TTree* outputTree){
  // Clear arrays by resetting indices to initial values
  delete[] cublet_idx;
  delete[] cells_in_cublet;
  delete[] picublet_idx;
  delete[] picells_in_cublet;
  delete[] Tcublet_idx;
  delete[] Tcells_in_cublet;
  delete[] TeventID;
  delete[] Te_in_cell;
  delete[] Tn_in_cell;
  delete[] Tpdg_id;
  delete[] Tmom;
  delete[] Tphoton;

  // Convert cell to cublet
  for (int i = 0; i < MAX_SIZE; i++) {
    cublet_info(cell_idx[i]);
  }

  // Fetch cublet and cell id for each of particles
  for (int i = 0; i < MAX_SIZE; i++) {    
    cublet_part_info(picell[i]);
  }//end for i loop
  
  // Collapse summing energy
  std::vector<bool> done;
  for (int i=0; i < MAX_SIZE; i++) {
    done[i] = false;
  }
  for (int i = 0;  i < MAX_SIZE - 1; i++) {  
    if (!done[i]) {
      double E = e_in_cell[i];
      int ne = 1;
      done[i] = true;
      for (int j = i + 1; j < MAX_SIZE; j++){
	if (!done[j]) {
	  if (cublet_idx[j] == cublet_idx[i] && cells_in_cublet[j] == cells_in_cublet[i]){
	    E += e_in_cell[j];
	    ne++;
	    done[j] = true;
	  }// end if
	} // end if j is !done
      }// end for j loop

      int kmax = -1;
      double maxmom = 0;
      int photonNo = 0;
      // Fetch pdg id of highest momentum p; article with same cell location, from part_info block 
      for (int k = 0; k < MAX_SIZE; k++) {
	if (cublet_idx[i] == picublet_idx[k] && cells_in_cublet[i] == picells_in_cublet[k]) {
	  if (pipdg[k] == 111){
            photonNo++;
          }// end if pdg == 111  

	  if (maxmom < pimom[k]) {
	    maxmom = pimom[k];
	    kmax = k;
	  }// end if maxmom < pimom
	}// end if i == k
      }// end for k loop

      // Store the arrays
      if (kmax != -1){
	Tpdg_id[i] = pipdg[kmax];
	Tmom[i] = maxmom;
	Tcublet_idx[i] = cublet_idx[i];
	Tcells_in_cublet[i] = cells_in_cublet[i];
	TeventID[i] = eventID[i];
	Te_in_cell[i] = E;
	Tn_in_cell[i] = ne;
	Tphoton[i] = photonNo;
      }// end if kmax != 1
    } // end if i is !done
  } // end for i loop 
  return;
}// end function


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void cublet_info(int cell_idx){
  // Calculate cublet_idx
  int z_idx = cell_idx / 10000;
  int y_idx = (cell_idx - z_idx * 10000) / 100;
  int x_idx = cell_idx - z_idx * 10000 - y_idx * 100;
  //int cublet_x = x_idx / 10;
  //int cublet_y = y_idx / 10;
  //int cublet_z = z_idx / 10;
  //int cublet_idx_info = cublet_x + cublet_y * 10 + cublet_z * 100;
  int cublet_idx_info = x_idx / 10 + (y_idx / 10) * 10 + (z_idx / 10) * 100;

  // Calculate the cell_in_cublet
  //int x_in_cublet = x_idx - cublet_x * 10;
  //int y_in_cublet = y_idx - cublet_y * 10;
  //int z_in_cublet = z_idx - cublet_z * 10;
  //int cells_in_cublet_info = x_in_cublet + y_in_cublet * 10 + z_in_cublet * 100;
  int cells_in_cublet_info = x_idx - (x_idx / 10) * 10 + (y_idx - (y_idx / 10) * 10) * 10 + (z_idx - (z_idx / 10) * 10) * 100;
  int currentIndex = 0; // Track the current index     

  // Store the values in arrays
  cublet_idx[currentIndex] = cublet_idx_info;
  cells_in_cublet[currentIndex] = cells_in_cublet_info;
  currentIndex++;  
  return;
}// end function


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////   
void cublet_part_info(int picell){
  // Calculate picublet_idx and picells_in_cublet 
  int z_idx = picell / 10000;
  int y_idx = (picell - z_idx * 10000) / 100;
  int x_idx = picell - z_idx * 10000 - y_idx * 100;
  int picublet_id_info = x_idx / 10 + (y_idx / 10) * 10 + (z_idx / 10) * 100;
  int picells_id_info = x_idx - (x_idx / 10) * 10 + (y_idx - (y_idx / 10) * 10) * 10 + (z_idx - (z_idx / 10) * 10) * 100;
  int currentIndex = 0; // Track the current index     

  // Store the values in arrays
  picublet_idx[currentIndex] = picublet_id_info;
  picells_in_cublet[currentIndex] = picells_id_info;
  currentIndex++;  // Increment the size counter
  return;
}// end of function


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
  const char* inputFolder = "/lustre/cmswork/xnguyen/data/";
  const char* outputFolder = "/lustre/cmswork/xnguyen/data/cublet_";
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

          // Print completion message
	  std::cout << "File processing completed." << std::endl;
        }
      }
      closedir(dir);
    }
  }

  return 0;
}
