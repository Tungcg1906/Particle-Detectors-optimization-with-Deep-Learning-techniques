////////////////////////////////////////////////////////////////
// Program reformatting large Ntuple .root file
// To compile: g++ -Ofast -g Ntuple_9.C part_info.C `root-config --cflags` -o Ntuple_9 -L$(root-config --libdir) -Wl,-rpath,$(root-config --libdir) -lCore -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lMultiProc -lROOTVecOps -pthread -lm -ldl
// To run the code: ./Ntuple_9 -f hadron.root -o outputFile.root -e 5
// Example: hadron.root
// for 5 events ->  secs to finish (+ map time)
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

#include "part_info.h"

using namespace std;

// Define vectors 
// Compressed vectors
int    TeventID;
int    Tentries;

std::vector<int>    Tpdg;
std::vector<int>    Ttrack;
std::vector<int>    Tparent_id;
std::vector<double> Tmom;
std::vector<double> Tedep;
std::vector<double> Tdeltae;
std::vector<double> Tglob_t;
std::vector<int>    Tcublet_idx;
std::vector<int>    Tcell_idx;

// Declare function
void processRootFile(const char* filename, TTree* outputTree, const int max_event,int map_mode=0, int verbose=1);
template <typename T>
std::map<int, vector<UInt_t> > map_indices(T& event, int entries, int mode=0, std::string filename="");

////////////////////////////////////////////////////////////////////////////////////////////////////////
void processRootFile(const char* filename, TTree* outputTree, const int max_event, int map_mode, int verbose) {
 
  // Start measuring time
  auto start_time = std::chrono::high_resolution_clock::now();
  if (verbose > 0) std::cout << "Processing file: " << filename << "\nmax_event: "<< max_event << std::endl;

  // Get the tree "Edep" and set branch addresses     
  TFile* inputFile = TFile::Open(filename);  
  TTree* partInfoTree = dynamic_cast<TTree*>(inputFile->Get("part_info"));
  part_info* pievent = new part_info(partInfoTree);
  int pientries = partInfoTree->GetEntries();

  // Set branches for output tree
  outputTree->Branch("Tevent_id",         &TeventID);
  outputTree->Branch("Tcells_in_event",   &Tentries);
  outputTree->Branch("Tpdg",              &Tpdg);
  outputTree->Branch("Ttrack",            &Ttrack);
  outputTree->Branch("Tparent_id",        &Tparent_id);
  outputTree->Branch("Tmax_mom",          &Tmom);
  outputTree->Branch("Tedep",             &Tedep);
  outputTree->Branch("Tdeltae",           &Tdeltae);
  outputTree->Branch("Tglob_t",           &Tglob_t);
  outputTree->Branch("Tcublet_idx",       &Tcublet_idx);
  outputTree->Branch("Tcell_idx",         &Tcell_idx);


  // Map the entries of trees
  std::string map_name(filename);
  map_name .erase(map_name.size()-5);
  std::map<int, std::vector<UInt_t> > m_idx_pi = map_indices(pievent, pientries, map_mode, map_name+"_piMap.txt" );
  if (verbose > 0) std::cout << "Second index map complete" << std::endl;
  
  int ievent = 0;
  int epsilon = 1;
  bool data = false;

  // loop over all events
  do {
    // check if event index is present
    if (m_idx_pi.find(ievent) == m_idx_pi.end()) {
      ievent++;
      continue;
    }

    // set default values
    TeventID = ievent;
    Tentries = 0;

    Tpdg.clear();
    Ttrack.clear();
    Tparent_id.clear();
    Tmom.clear();
    Tedep.clear();
    Tdeltae.clear();
    Tglob_t.clear();
    Tcublet_idx.clear();
    Tcell_idx.clear();

    // loop over part_info to get primary particle identity, max momentum and number of photons
    for (int j = 0; j < m_idx_pi[ievent].size(); j += 2) {
      for (int k = m_idx_pi[ievent][j]; k < m_idx_pi[ievent][j + 1] + 1; k++) {
        
        pievent->GetEntry(k);

        int pdg_id    = pievent->part_id;
        int track     = pievent->track_id;
        int parent    = pievent->parent_id;
        double pi_mom = pievent->mom;
        double E      = pievent->edepo;
        double DeltaE = pievent->deltae;
        double g_time = pievent->global_t;
        int cub_idx   = pievent->cublet_idx;
        int ce_idx    = pievent->cell_in_cub;

        Tpdg.push_back(pdg_id);
        Ttrack.push_back(track);
        Tparent_id.push_back(parent);
        Tmom.push_back(pi_mom);
        Tedep.push_back(E);
        Tdeltae.push_back(DeltaE);
        Tglob_t.push_back(g_time);
        Tcublet_idx.push_back(cub_idx);
        Tcell_idx.push_back(ce_idx);

        Tentries += 1;
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
        std::cerr << "Error: Unable to open the map file for writing." << std::endl;
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
      std::cerr << "Error: Unable to open the map file for reading." << std::endl;
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

  int map_mode = 0;
  int verbose = 1;
  int max_event = 0;
  std::string filePath = "";
	std::string outputFilePath = "outputFile.root";
  
  for (int i = 1; i < argc; i++) {
    std::string flag(argv[i]);

    if (flag.find("map_mode") != string::npos) {
      map_mode = std::stoi(flag.substr(11));
    }
    else if (flag=="-m") {
      i += 1;
      map_mode = std::stoi(argv[i]);
    }

    else if (flag.find("verbose") != string::npos) {
      verbose = std::stoi(flag.substr(10));
    }
    else if (flag=="-v") {
      i += 1;
      verbose = std::stoi(argv[i]);
    }

    else if (flag.find("maxEvent") != string::npos) {
      max_event = std::stoi(flag.substr(11));
    }
    else if (flag=="-e") {
      i += 1;
      max_event = std::stoi(argv[i]);
    }

    else if (flag.find("filePath") != string::npos) {
      filePath = flag.substr(11);
    }
    else if (flag=="-f") {
      i += 1;
      filePath = argv[i];
    }

    else if (flag.find("output") != string::npos) {
      outputFilePath = flag.substr(9);
    }
    else if (flag=="-o") {
      i += 1;
      outputFilePath = argv[i];
    }
  }	

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
	std::cout << "File processing completed." << std::endl;

  return 0;
}
