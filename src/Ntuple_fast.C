/////////////////////////////////////////////////////////////////
// Program reformatting large Ntuple .root file
// Enrico's edition
// Vectors are declared locally inside the funciton
// To compile: g++ -Ofast -g Ntuple_fast.C Edep.C part_info.C `root-config --cflags` -o Ntuple_fast -L$(root-config --libdir) -Wl,-rpath,$(root-config --libdir) -lCore -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lMultiProc -lROOTVecOps -pthread -lm -ldl
// Example: kaon_3.root
// for 5 events -> 276 secs to finish
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

static int max_event = 100;
template <typename T>
std::map<int, vector<UInt_t> > map_indices(T event, int entries);
// Declare function
void processRootFile(const char* filename, TTree* outputTree);
void fill_n_tuple(std::vector<int>& eventID, std::vector<int>& cell_idx, std::vector<int>& cells_in_cublet, 
		  std::vector<int>& cublet_idx, std::vector<double>& e_in_cell, std::vector<double>& Te_in_cell,
		  std::vector<int>& Tn_in_cell, std::vector<int>& Tcublet_idx, std::vector<int>& Tcells_in_cuble, std::vector<int>& TeventID,
		  std::vector<int>& picell, std::vector<int>& pipdg, std::vector<double>& pimom, std::vector<int>& picublet_idx, std::vector<int>& picells_in_cublet,
		  std::vector<int>& Tpdg_id, std::vector<double>& Tmom, std::vector<int>& Tphoton,
		  TTree* outputTree);
void cublet_info(int cell_idx, std::vector<int>& cublet_idx, std::vector<int>& cells_in_cublet);
void cublet_part_info(int picell, std::vector<int>& picublet_id, std::vector<int>& picell_id);

////////////////////////////////////////////////////////////////////////////////////////////////////////
void processRootFile(const char* filename, TTree* outputTree) {
 
  // Start measuring time
  auto start_time = std::chrono::high_resolution_clock::now();
  std::cout << "Processing file: " << filename << std::endl;

  // Define vectors 
  // Initial vectors from Edep
  std::vector<double> e_in_cell;
  std::vector<int> cell_idx;
  std::vector<int> eventID;
  
  // Initial vectors from part_info
  std::vector<int> pipdg;
  std::vector<int> picell;
  std::vector<double> pimom;

  // Created vectors
  std::vector<int> cublet_idx;
  std::vector<int> cells_in_cublet;
  std::vector<int> picublet_idx;
  std::vector<int> picells_in_cublet;

  // Compressed vectors
  std::vector<double> Te_in_cell;
  std::vector<int> Tcublet_idx;
  std::vector<int> Tcells_in_cublet;
  std::vector<int> TeventID;
  std::vector<int> Tn_in_cell;
  std::vector<int> Tpdg_id;
  std::vector<double> Tmom;
  std::vector<int> Tphoton;

  // Get the tree "Edep" and set branch addresses     
  TFile* inputFile = TFile::Open(filename);  

  TTree* edepTree = dynamic_cast<TTree*>(inputFile->Get("Edep"));
  TTree* partInfoTree = dynamic_cast<TTree*>(inputFile->Get("part_info"));
  Edep*  event = new Edep(edepTree);
  part_info* pievent = new part_info(partInfoTree);
  int entries = edepTree->GetEntries();
  int pientries = partInfoTree->GetEntries();

  // Set branches for output tree
  outputTree->Branch("e_in_cell", &e_in_cell);
  outputTree->Branch("cell_idx", &cell_idx);
  outputTree->Branch("Tcublet_idx", &Tcublet_idx);
  outputTree->Branch("Tcells_in_cublet", &Tcells_in_cublet);
  outputTree->Branch("Tevent_id", &TeventID);
  outputTree->Branch("Tedep", &Te_in_cell);
  outputTree->Branch("Tpdg_id", &Tpdg_id);
  outputTree->Branch("Tmom", &Tmom);
  outputTree->Branch("Tphoton", &Tphoton);

  // Map the entries of trees
  std::map<int, std::vector<UInt_t> > m_idx_edep = map_indices(event, entries);
  std::cout << "First index map complete" << std::endl;
  std::map<int, std::vector<UInt_t> > m_idx_pi   = map_indices(pievent, pientries);
  std::cout << "Second index map complete" << std::endl;
  
  int ievent = 0;
  int epsilon = 1;
  bool data = false;
  do {
    // check if event index is present
    if (m_idx_edep.find(ievent) == m_idx_edep.end() &&
	m_idx_pi.find(ievent)   == m_idx_pi.end()) {
      continue;
    }
    e_in_cell.clear();
    cell_idx.clear();
    eventID.clear();
    pipdg.clear();
    picell.clear();
    pimom.clear();

    for (int j = 0; j < m_idx_edep[ievent].size(); j+=2) {
      for (int k = m_idx_edep[ievent][j]; k < m_idx_edep[ievent][j+1]+1; k++) {
	event->GetEntry(k);
	// Add the current entry to vectors
	e_in_cell.push_back(event->edep);
	cell_idx.push_back(event->cell_no);
	eventID.push_back(event->event_id);  
      } // end for k loop
    } // end of for loop on all data file information

    for (int j = 0; j < m_idx_pi[ievent].size(); j+=2) {
      for (int k = m_idx_pi[ievent][j]; k < m_idx_pi[ievent][j+1]+1; k++) {
	pievent->GetEntry(k);
	// Add the current entry to vectors
	pipdg.push_back(pievent->pdg_id);
	double x = pievent->pos_x;
	double y = pievent->pos_y;
	double z = pievent->pos_z;
	int ix = (x + 150 - epsilon) / 3;
	int iy = (-y + 150 - epsilon) / 3;
	int iz = (z - epsilon) / 12;
	picell.push_back(ix + 100 * iy + 10000 * iz);
	pimom.push_back(pievent->mom);
      }// end for k loop
    } // end of for loop on all data file information 
    
    // Fill the new Ntuples
    fill_n_tuple (eventID, cell_idx, cells_in_cublet, cublet_idx, e_in_cell, Te_in_cell, 
		  Tn_in_cell, Tcublet_idx, Tcells_in_cublet, TeventID, 
		  picell, pipdg, pimom, picublet_idx, picells_in_cublet, Tpdg_id, Tmom, Tphoton, outputTree);

    for (size_t i = 0; i < Tpdg_id.size(); ++i) {
      std::cout << "Tpdg_id : " << Tpdg_id[i]
                << " , Tmom : " << Tmom[i] << " , Tphoton: " << Tphoton[i] << std::endl;
    }
    
    for (size_t i = 0; i < Te_in_cell.size(); ++i) {
      std::cout << "Te_in_cell = " << Te_in_cell[i] << std::endl;
      std::cout << "Tcublet_idx = " << Tcublet_idx[i] << std::endl;
      std::cout << "Tcells_in_cublet = " << Tcells_in_cublet[i] << std::endl;
      std::cout << "TeventID = " << TeventID[i] << std::endl;
      std::cout << "Tn_in_cell = " << Tn_in_cell[i] << std::endl;
    }

    std::cout << "Size of Tpdg_id : " << Tpdg_id.size() << " , size of Tmom : " << Tmom.size()
              << " , size of Tphoton : " << Tphoton.size()
              << " , size of cublet_idx:  " <<  Tcublet_idx.size()
              << " , size of Tevent ID: " << TeventID.size()
              << " , size of Tedep: " << Te_in_cell.size()
              << " , size of cublet_idx : " << cublet_idx.size()
              << " , size of picublet_idx : " << picublet_idx.size() 
	      << " , size of cell_idx : " << cell_idx.size()
	      << " , size of picell : " << picell.size()
	      << std::endl;
    outputTree->Fill();
    std::cout << "Tree filled correctly!" <<std::endl;
    ievent++;
  } while (ievent < max_event);
  
  // Stop measuring time
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
  std::cout << "Finished processing file. Time taken: " << duration.count() << " secs. " << std::endl;
}// end function


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void fill_n_tuple(std::vector<int>& eventID, std::vector<int>& cell_idx, std::vector<int>& cells_in_cublet, 
		  std::vector<int>& cublet_idx, std::vector<double>& e_in_cell, std::vector<double>& Te_in_cell,
		  std::vector<int>& Tn_in_cell, std::vector<int>& Tcublet_idx, std::vector<int>& Tcells_in_cublet, std::vector<int>& TeventID, 
		  std::vector<int>& picell, std::vector<int>& pipdg, std::vector<double>& pimom, std::vector<int>& picublet_idx, std::vector<int>& picells_in_cublet, 
		  std::vector<int>& Tpdg_id, std::vector<double>& Tmom, std::vector<int>& Tphoton,
		  TTree* outputTree){
  // Clear vectors before filling with new data
  cublet_idx.clear(); 
  cells_in_cublet.clear();
  picublet_idx.clear();
  picells_in_cublet.clear();
  Tcublet_idx.clear();
  Tcells_in_cublet.clear();
  TeventID.clear();
  Te_in_cell.clear();
  Tn_in_cell.clear();
  Tpdg_id.clear();
  Tmom.clear();
  Tphoton.clear();

  // Convert cell to cublet
  for (int i = 0; i < cell_idx.size(); i++) {
    cublet_info(cell_idx[i], cublet_idx, cells_in_cublet);
  }

  // Fetch cublet and cell id for each of particles
  for (int i = 0; i < picell.size(); i++) {    
    cublet_part_info(picell[i], picublet_idx, picells_in_cublet);
  }//end for i loop
  
  // Collapse summing energy
  std::vector<bool> done;
  for (int i=0; i < cublet_idx.size(); i++) {
    done.push_back(false);
  }// end for i loop i < cublet_idx.size()
  for (int i = 0;  i < cublet_idx.size() - 1; i++) {  
    if (!done[i]) {
      double E = e_in_cell.at(i);
      int ne = 1;
      done[i] = true;
      for (int j = i + 1; j < cublet_idx.size(); j++){
	if (!done[j]) {
	  if (cublet_idx.at(j) == cublet_idx.at(i) && cells_in_cublet.at(j) == cells_in_cublet.at(i)){
	    E += e_in_cell.at(j);
	    ne++;
	    done[j] = true;
	  }// end if
	} // end if j is !done
      }// end for j loop
      
      int kmax = -1;
      double maxmom = 0;
      int photonNo = 0;
      
      // Fetch pdg id of highest momentum p; article with same cell location, from part_info block 
      for (int k = 0; k < picublet_idx.size(); k++) {
	if (cublet_idx.at(i) == picublet_idx.at(k) && cells_in_cublet.at(i) == picells_in_cublet.at(k)) {
	    if (pipdg.at(k) == 111){
	      photonNo++;
	    }// end if pdg == 111  
	    
	    if (maxmom < pimom.at(k)) {
	      maxmom = pimom.at(k);
	      kmax = k;
	    }// end if maxmom < pimom
	}// end if i == k
      }// end for k loop
      
      // Store the vectors
      if (kmax != -1){
	Tpdg_id.push_back(pipdg.at(kmax));
	Tmom.push_back(maxmom);
	Tcublet_idx.push_back(cublet_idx.at(i));
	Tcells_in_cublet.push_back(cells_in_cublet.at(i));
	TeventID.push_back(eventID.at(i));
	Te_in_cell.push_back(E);
	Tn_in_cell.push_back(ne);
	Tphoton.push_back(photonNo);
      }// end if kmax != 1  
    } // end if i is !done
  } // end for i loop with i < cublet_idx.size() - 1 
    return;
}// end function


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void cublet_info(int cell_idx, std::vector<int>& cublet_idx, std::vector<int>& cells_in_cublet){
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

  // Store the vectors
  cublet_idx.push_back(cublet_idx_info);
  cells_in_cublet.push_back(cells_in_cublet_info);  
  return;
}// end function


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////   
void cublet_part_info(int picell, std::vector<int>& picublet_idx, std::vector<int>& picells_in_cublet){
  // Calculate picublet_idx and picells_in_cublet 
  int z_idx = picell / 10000;
  int y_idx = (picell - z_idx * 10000) / 100;
  int x_idx = picell - z_idx * 10000 - y_idx * 100;
  int picublet_id_info = x_idx / 10 + (y_idx / 10) * 10 + (z_idx / 10) * 100;
  int picells_id_info = x_idx - (x_idx / 10) * 10 + (y_idx - (y_idx / 10) * 10) * 10 + (z_idx - (z_idx / 10) * 10) * 100;

  // Store the vectors
  picublet_idx.push_back(picublet_id_info);
  picells_in_cublet.push_back(picells_id_info);
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
