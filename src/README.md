# Description
## Code for reformatting large Ntuple (.root CERN) file that contain the hadronic calorimeter simulation. 
### Progress
- [x]  Read the data format.
- [x]  Compute cells → cublets.
- [x]  Loop on 1000 cublets → save the data.
    - [ ]  Loop on 1000 cells → compute the number of photons.
        - [ ]  Compute the number of sensors.
            - [ ]  Loop on 100 sensors → compute the number of sensors, time.

### Dataset structure

 ![Data](https://github.com/Tungcg1906/Particle-Detectors-optimization-with-Deep-Learning-techniques/blob/main/images/data-struct.png)
 
Each file contains two trees:

| part_info (MC truth of particle hits)  | E_dep (the energy stored by the hit in each cell) |
| ------------- | ------------- |
| Cut: particle momentum >2MeV  | Cut: energy deposit > 1keV  |
| event_id: int to label all hits belonging to the same event  | event_id: same as before, int ID to filter single events  |
| pdg_id: PDG encoder for particle species  |  |
| Track_id: int label for the track the hit belongs to  | |
| parent_id: int label for reconstructing where hit was originated from  |   |
| pos_x,pos_y,pos_z: hit position  | Cell_no: int ID corresponding to which cell is collecting the deposit  |
| Mom: particle momentum  |   |
| edepo: energy deposited in the hit  | edep: energy deposited in cell  |


### Instructions to build and run project:
0. Create a directory for the project, e.g. /lustre/cmswork/xnguyen/  
1. Copy this file to that directory.
2. Create subdirectories:
   - /lustre/cmswork/xnguyen/Outputs     --> contains dumps from runs of the program
   - /user/cmswork/xnguyen/data          --> contains root files in output
3. Compile with: ```> g++ -Ofast -g Ntuple_4.C Edep.C part_info.C `root-config --cflags` -o Ntuple_4 -L$(root-config --libdir) -Wl,-rpath,$(root-config --libdir) -lCore -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lMultiProc -lROOTVecOps -pthread -lm -ldl```
4. Run with: `>./Ntuple_4` or `>nohup ./Ntuple_4 &` to make the program runs into background.
