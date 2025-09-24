# MorphoHaptics: An Open-Source Tool for Visuohaptic Exploration of Morphological Image Datasets
This repository contains experimental data for the ACM KUI '24 full paper "MorphoHaptics: An Open-Source Tool for Visuohaptic Exploration of Morphological Image Datasets"

# Abstract 
Although digital methods have significantly advanced morphology, practitioners are still challenged to understand and process tomographic specimen data. As automated processing of fossil data remains insufficient, morphologists still engage in intensive manual work to prepare digital fossils for research objectives. We present an open-source tool that enables morphologists to explore tomographic data similarly to the physical workflows that traditional fossil preparators experience in the field. We assessed the usability of our prototype for virtual fossil preparation and its accompanying tasks in the digital preparation workflow. Our findings indicate that integrating haptics into the virtual preparation workflow enhances the understanding of the morphology and material properties of working specimens. Our design's visuohaptic sculpting of fossil volumes was deemed straightforward and an improvement over current tomographic data processing methods.

## Full Paper
The paper is available in the [ACM Digital Library](https://doi.org/10.1145/3719236.3719271).

## Licenses
The contained "source code" of this work is made available under the terms of [GNU GPLv3](./LICENSE).

## Source Code
This C++ contribution was created in Visual Studio 2022 and leverages the Chai3D Framework, included in the repository.

## Instructions

Use CMake 3.16 or above to Generate the VS 2022 Project:

```
cmake -S . -B . -G "Visual Studio 17 2022" -A x64
```

Build:

```
cmake --build . --config Release
```
Open MorphoHaptics.sln in Visual Studio 2022 and build it. The software was tested for haptic rendering using a 3DSystems Touch and a Novint Falcon on Windows 10. 

 <!--
## Licenses and Citation

- [results.csv](results.csv), available in [Creative Commons Public Domain Dedication (CC-0)](https://creativecommons.org/share-your-work/public-domain/cc0/), represented the experimental results from consented anonymous participants and was collected by Lucas Siqueira Rodrigues.
- The [data.csv](data.csv) itself, available in [Creative Commons Public Domain Dedication (CC-0)](https://creativecommons.org/share-your-work/public-domain/cc0/), combines the experimental results from consented anonymous participants and was collected by Lucas Siqueira Rodrigues in the current projected surface experiment and the [previous VR study](https://github.com/lsrodri/VHMatch).
- [logistic_regression_error_rates.py](logistic_regression_error_rates.py) Performs logistic regression on error rates on data.csv.
- [mannwhitneyu_response_time.py](mannwhitneyu_response_time.py) performs Mann-Whitney U tests for response times on data.csv.
- [data_analysis_interaction.py](data_analysis_interaction.py) examines interactions between the display environments and sensory modality conditions.
-->
<!-- - The [NASA-TLX.csv](NASA-TLX.csv) itself, available in [Creative Commons Public Domain Dedication (CC-0)](https://creativecommons.org/share-your-work/public-domain/cc0/), represented the workload self-assessment results from consented anonymous participants and was collected by Lucas Rodrigues. -->
<!-- - The [MatchToSampleExperiment](MatchToSampleExperiment) itself, available in [Creative Commons Public Domain Dedication (CC-0)](https://creativecommons.org/share-your-work/public-domain/cc0/), represented the open-source Unity project that was created by Lucas Rodrigues and used for data collection. -->

<!-- The contained "source code" (i.e., Python scripts and Jupyter Notebooks) of this work is made available under the terms of [GNU GPLv3](./LICENSE). They are fully available also in the [Open Science Framework](https://). -->

## Reference
```
@inproceedings{10.1145/3719236.3719271,
author = {Siqueira Rodrigues, Lucas and Kosch, Thomas and Nyakatura, John and Zachow, Stefan and Israel, Johann Habakuk},
title = {MorphoHaptics: An Open-Source Tool for Visuohaptic Exploration of Morphological Image Datasets},
year = {2025},
isbn = {9798400710322},
publisher = {Association for Computing Machinery},
address = {New York, NY, USA},
url = {https://doi.org/10.1145/3719236.3719271},
doi = {10.1145/3719236.3719271},
booktitle = {Proceedings of the 21st International Conference on Culture and Computer Science: From Humanism to Digital Humanities},
articleno = {4},
numpages = {10},
keywords = {Haptics, Visuohaptic Integration, Feedback, Data Analysis, Data Exploration, Human-Computer Interaction},
location = {
},
series = {KUI '24}
}
```

## Acknowledgements
The author acknowledges the support of the Cluster of Excellence »Matters of Activity. Image Space Material« funded by the Deutsche Forschungsgemeinschaft (DFG, German Research Foundation) under Germany's Excellence Strategy – EXC 2025 – 390648296.

Copyright &copy; 2024. [Cluster of Excellence Matters of Activity](https://www.matters-of-activity.de/). All rights reserved.

<!--
## Citing the Paper and Application

Below are the BibTex entries to cite the paper and data set.

```
@misc{siqueira_rodrigues_data_2d_2023,
	title = {Data from "Assessing the Effects of Sensory Modality Conditions on Object Retention across Virtual Reality and Projected Surface Display Environments"},
	url = {https://github.com/lsrodri/VHMatch2D},
	publisher = {https://github.com/lsrodri/VHMatch2D},
	author = {Siqueira Rodrigues, Lucas and Schmidt, Timo T. and Nyakatura, John and Zachow, Stefan and Israel, Johann Habakuk and Kosch, Thomas},
	year = {2023},
}
```
-->
