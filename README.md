# Particle Detectors optimization with Deep Learning techniques

AUC vs cell's size             |  Hadronic calorimeter
:-------------------------:|:-------------------------:
![](https://github.com/Tungcg1906/Particle-Detectors-optimization-with-Deep-Learning-techniques/blob/main/images/AUC-width.png)  |  ![](https://github.com/Tungcg1906/Particle-Detectors-optimization-with-Deep-Learning-techniques/blob/main/images/CMS-Detector.gif)

## Phase 1 

We want to answer a very basic question: can we distinguish those 4 particle species based on the pattern of energy release in an arbitrarily granular device? And if so, how does the size of the cells affect this capability?
The $\textbf{«money plot»}$ is thus one where we put on the y axis some measures of discrimination (1/2, 1/3, 1/4, 2/3, 3/4 or some combination of the six) and on the x axis the cell size, so we can find out whether there is merit in keeping the cell size below the value when information is irrecoverable.

<table>
  <tr>
    <td><img src="https://github.com/Tungcg1906/Particle-Detectors-optimization-with-Deep-Learning-techniques/blob/main/images/combined_plots.gif" alt="Combined Plots" width="400"></td>
    <td><img src="https://github.com/Tungcg1906/Particle-Detectors-optimization-with-Deep-Learning-techniques/blob/main/images/combined_plots_XY.gif" alt="Combined Plots XY" width="400"></td>
  </tr>
</table>

## Phase 2

Once we have this information, we can extend the study to consider more audacious questions – in particular, $\textbf{«can we integrate tracking and calorimetry in a single device?»}$. Note, however, that this is long-term.
To understand the above, note that collider detectors «track» charged particles in very lightweight media (gas or air-spaced silicon sensors) by recording their ionization trails, and only after this destroy them in calorimeters, measuring their ionization trails, and only after this destroy them in calorimeters, measure their energy. Adding material to a tracker has until now been anathema, as nuclear interactions $\textbf{«spoil»}$ the precise track reconstruction – but AI can make sense of them, and use it to do particle ID!
The final goal is thus to work toward a detector of the future, which integrates the two techniques into one, thanks to the more power pattern recognition capabilities we have today.


## [Execute the code](https://github.com/Tungcg1906/Particle-Detectors-optimization-with-Deep-Learning-techniques/blob/main/src/README.md)

## [Generate the data](https://github.com/Tungcg1906/GEANT4-simulation)
