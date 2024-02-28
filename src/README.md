# Code for reformat Ntuple root file
Reformatting Ntuples dataset (.root CERN):

- [x]  Read the data format.
- [x]  Compute cells → cublets.
- [x]  Loop on 1000 cublets → save the data.
    - [ ]  Loop on 1000 cells → compute the number of photons.
        - [ ]  Compute the number of sensors.
            - [ ]  Loop on 100 sensors → compute the number of sensors, time.
