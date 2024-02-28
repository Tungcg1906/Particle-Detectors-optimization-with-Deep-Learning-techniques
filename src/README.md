# Description
## Code for reformatting large Ntuple (.root CERN) file that contain the hadronic calorimeter simulation. 

- [x]  Read the data format.
- [x]  Compute cells → cublets.
- [x]  Loop on 1000 cublets → save the data.
    - [ ]  Loop on 1000 cells → compute the number of photons.
        - [ ]  Compute the number of sensors.
            - [ ]  Loop on 100 sensors → compute the number of sensors, time.

## Instructions to build and run project:
0. Create a directory for the project, e.g. /lustre/cmswork/xnguyen/  
1. Copy this file to that directory.
2. Create subdirectories:
   - /lustre/cmswork/xnguyen/Outputs     --> contains dumps from runs of the program
   - /user/cmswork/xnguyen/data          --> contains root files in output
3. Compile with: ```> g++ -Ofast -g Ntuple_4.C Edep.C part_info.C `root-config --cflags` -o Ntuple_4 -L$(root-config --libdir) -Wl,-rpath,$(root-config --libdir) -lCore -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lMultiProc -lROOTVecOps -pthread -lm -ldl```
4. Run with: `>./Ntuple_4` or `>nohup ./Ntuple_4 &` to make the program runs into background.
