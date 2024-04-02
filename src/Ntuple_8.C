////////////////////////////////////////////////////////////////
// Program reformatting large Ntuple .root file
// To compile: g++ -Ofast -g Ntuple_8.C Edep.C part_info.C `root-config --cflags` -o Ntuple_8 -L$(root-config --libdir) -Wl,-rpath,$(root-config --libdir) -lCore -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lMultiProc -lROOTVecOps -pthread -lm -ldl
// Example: kaon_3.root
// for 5 events -> ~152 secs to finish (+ map time)
/////////////////////////////////////////////////////////////////
#include <iostream>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <TFile.h>
#include <TTree.h>
#include <string>
#include <sstream>
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

#include "src/Edep.h"
#include "src/part_info.h"

using namespace std;

static int n_cells = 1000000; // 1000 cublets made of 1000 cells each

// Define vectors 
// Compressed vectors
int    TeventID;
int    Tcells_in_event;

std::vector<int>    Tcublet_idx;
std::vector<int>    Tcell_idx;
std::vector<double> Te_in_cell;

std::vector<int>    Tpdg;
std::vector<double> Tmax_mom;
std::vector<int>    Tphoton;

// Declare function
void processRootFile(const char* filename, TTree* outputTree, const int max_event,int map_mode=0, int verbose=1);
template <typename T>
std::map<int, vector<UInt_t> > map_indices(T& event, int entries, int mode=0, std::string filename="");

////////////////////////////////////////////////////////////////////////////////////////////////////////
void processRootFile(const char* filename, TTree* outputTree, const int max_event, int map_mode, int verbose) {
 
  // Start measuring time
  auto start_time = std::chrono::high_resolution_clock::now();
  if (verbose > 0) std::cout << "Processing file: " << filename << "\tmax_event = "<< max_event << std::endl;

  // Get the tree "Edep" and set branch addresses     
  TFile* inputFile = TFile::Open(filename);  

  TTree* edepTree = dynamic_cast<TTree*>(inputFile->Get("Edep"));
  Edep*  event = new Edep(edepTree);
  int entries = edepTree->GetEntries();

  TTree* partInfoTree = dynamic_cast<TTree*>(inputFile->Get("part_info"));
  part_info* pievent = new part_info(partInfoTree);
  int pientries = partInfoTree->GetEntries();

  // Set branches for output tree
  outputTree->Branch("Tevent_id",         &TeventID);
  outputTree->Branch("Tcells_in_event",   &Tcells_in_event);

  outputTree->Branch("Tcublet_idx",       &Tcublet_idx);
  outputTree->Branch("Tcell_idx",         &Tcell_idx);
  outputTree->Branch("Tedep",             &Te_in_cell);

  outputTree->Branch("Tpdg",              &Tpdg);
  outputTree->Branch("Tmax_mom",          &Tmax_mom);
  outputTree->Branch("Tphoton",           &Tphoton);

  // Map the entries of trees
  std::string map_name(filename);
  map_name .erase(map_name.size()-5);
  std::map<int, std::vector<UInt_t> > m_idx_edep = map_indices(event, entries, map_mode, map_name+"_edepMap.txt");
  if (verbose > 0) std::cout << "First index map complete" << std::endl;
  std::map<int, std::vector<UInt_t> > m_idx_pi   = map_indices(pievent, pientries, map_mode, map_name+"_piMap.txt" );
  if (verbose > 0) std::cout << "Second index map complete" << std::endl;
  
  int ievent = 0;
  int epsilon = 1;
  bool data = false;

  // loop over all events
  do {
    // check if event index is present
    if (m_idx_edep.find(ievent) == m_idx_edep.end() &&
	      m_idx_pi.find(ievent)   == m_idx_pi.end()) {
      continue;
    }

    // set default values
    TeventID = ievent;
    Tcells_in_event = 0;

    Tcublet_idx.clear();
    Tcell_idx.clear();
    Te_in_cell.clear();

    Tpdg.clear();
    Tmax_mom.clear();
    Tphoton.clear();

    // loop over Edep to get energy deposition and number of cells activated per cublet
    for (int j = 0; j < m_idx_edep[ievent].size(); j += 2) {
      for (int k = m_idx_edep[ievent][j]; k < m_idx_edep[ievent][j + 1] + 1; k++) {
        event->GetEntry(k);
        int cell_idx = int(event->cell_no);
        double E = (event->edep);

        int z_idx = (cell_idx/100000);
        int x_idx = ((cell_idx%10000)%100)/10;
        int y_idx = (cell_idx%10000)/1000;
        int cublet_idx = z_idx*100 + y_idx*10 + x_idx;
        
        Tcell_idx.push_back(cell_idx);
        Tcublet_idx.push_back(cublet_idx);
        Te_in_cell.push_back(E);

        Tcells_in_event += 1;
      }
    } // end of for loop over Edep

    Tpdg.assign(Tcells_in_event,0);
    Tmax_mom.assign(Tcells_in_event,0);
    Tphoton.assign(Tcells_in_event,0);

    // loop over part_info to get primary particle identity, max momentum and number of photons
    for (int j = 0; j < m_idx_pi[ievent].size(); j += 2) {
      for (int k = m_idx_pi[ievent][j]; k < m_idx_pi[ievent][j + 1] + 1; k++) {
        
        pievent->GetEntry(k);
        double pi_mom = pievent->mom;
        int pdg_id = pievent->pdg_id;

        // Calculate cublet_idx
        double x = pievent->pos_x;
        double y = pievent->pos_y;
        double z = pievent->pos_z;

        int ix = (std::floor((x+150)/3) == 100) ? 99 : std::floor((x+150)/3);
	      int iy = (std::floor((y+150)/3) == 100) ? 99 : std::floor((y+150)/3);
	      int iz = (std::floor((z+600)/12) == 100) ? 99 : std::floor((z+600)/12);
        
        int picell_idx = ix + 100 * iy + 10000 * iz;

        int save_index = -1;
        for (int i = 0; i < Tcell_idx.size(); i++) 
        {
            if (Tcell_idx[i] == picell_idx) 
            {
              save_index = i;
              break;
            }
        }

        if (save_index == -1)
        {
            std::cerr << "Wrong cell definition!" << std::endl;
            std::cerr << "(x,y,z) = ("<< x <<" , "<< y  <<" , "<< z << ") -> "<<  "picell_idx: "<< picell_idx << std::endl;
            exit(EXIT_FAILURE);
            

        }

        int z_idx = (picell_idx/100000);
        int x_idx = ((picell_idx%10000)%100)/10;
        int y_idx = (picell_idx%10000)/1000;
        int cublet_idx = z_idx*100 + y_idx*10 + x_idx;

        if (pi_mom > Tmax_mom[save_index]) {
          Tpdg[save_index] = pdg_id;
          Tmax_mom[save_index] = pi_mom;
        }

        if (pdg_id == 22) {
          Tphoton[save_index] += 1;
        }


      }

    } // end of for loop over part_info

    outputTree->Fill();
    if (verbose > 0) std::cout << "Event: " << ievent << " filled correctly!" <<std::endl;
    
    ievent++;
  } while (ievent < max_event);
  
  // Stop measuring time
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
  if (verbose > 0) std::cout << "Finished processing file. Time taken: " << duration.count() << " secs. " << std::endl;
} // end function


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
std::map<int, vector<UInt_t> > map_indices(T& event, int entries, int mode, std::string filename) {

  std::map<int, std::vector<UInt_t> > map_idx = {};

  // mode 0 and 1: compute index map from scratch
  if (mode!=2) {
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
    
    // mode 1: print map to file for later use
    if (mode==1) {
      std::ofstream outputFile(filename);
      if (!outputFile) {
        std::cerr << "Error: Unable to open the file for writing." << std::endl;
        return(map_idx);
      }
      for (const auto& pair : map_idx) {
        outputFile << pair.first << '\t';
        for (int j = 0; j < pair.second.size(); j++) {
          outputFile << pair.second[j] << "   ";
        }
        outputFile << std::endl;
      }
      outputFile.close();
      std::cout << "Map saved successfully" << std::endl;
    }
  }

  // mode 2: read map from file
  if (mode==2) {
    std::ifstream inputFile(filename);
    if (!inputFile) {
      std::cerr << "Error: Unable to open the file for reading." << std::endl;
      return(map_idx);
    }

    std::string line;
    while (std::getline(inputFile, line)) {
      std::istringstream iss(line);
      int key;
      std::vector<UInt_t> value;
      UInt_t v;
      if (!(iss >> key)) {
        std::cerr << "Error: Unable to parse line: " << line << std::endl;
        continue;
      }
      while (iss >> v) {
          value.push_back(v);
      }
      map_idx[key] = value;
    }
    inputFile.close();
  }
  
  return(map_idx);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
  const char* inputFolder = "data/";
  const char* outputFolder = "data/cublet_";
  //const char* folders[] = {"kaon", "neutron", "pion", "proton"};
  const char* folders[] = {"kaon"};

  int map_mode = 0;
  int verbose = 1;
  int max_event = 0;

  for (int i = 1; i < argc; i++) {
    std::string flag(argv[i]);
    if (flag.find("map_mode") != string::npos) {
      map_mode = std::stoi(flag.substr(11));
    }
    else if (flag.find("verbose") != string::npos) {
      verbose = std::stoi(flag.substr(10));
    }
    else if (flag.find("maxEvent") != string::npos) {
      max_event = std::stoi(flag.substr(11));
    }
  }

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
          processRootFile(filePath.c_str(), outputTree, max_event,map_mode, verbose);
	  
          // Print the output file path before processing                                                                                                                                                  
	        std::cout << "Processing file: " << outputFilePath << std::endl;

          // Close the output file and clean up memory
          outputFile.Write(0,TObject::kWriteDelete,0);  // Write the TTree to the file
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
