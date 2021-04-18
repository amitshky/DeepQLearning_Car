# DeepQLearning_Car
Simulation based Self-Driving Car using Reinforcement Learning.\
This project was built with cuda version 10.1 and Unreal Engine 4.23.1.

## Setup
* Copy libtorch folder into Thirdparty folder
* Change "cuda_path" to the directory containing your local cuda v10.1 files
* Right click DQN_Car.uproject file and generate visual studio files 
* Copy all the .dll files from libtorch/lib to Binaries/Win64 (if you don't see this folder, you may have to build the project first)
* Open any source file containing ```#include "torch/torch.h"``` you should see a macro ```THIRD_PARTY_INCLUDES_START```, go to its definition  (should be inside WindowsPlatformCompilerPreSetup.h file) and add the following:
``` 
__pragma(warning(disable: 4273))\
__pragma(warning(disable: 4582))\
__pragma(warning(disable: 4583))\
__pragma(warning(disable: 4018))\
```
* Open DQN_Car.sln file and check to see if the startup project is "DQN_Car" and not "UE4"

## Run
* Make sure to follow all the steps of the Setup
* If you want to resume the training then set ```g_Resume = true``` in ```CarGI.cpp```
* Then build and run (press F5 for visual studio)

## ToDo
### Unreal Environment Simulation
- [x] Car Movement
- [x] Track (Default; Training)

### DQN Implementation
- [x] Replay Memory
- [x] Neural network and training interface class
- [x] Epsilon Greedy Strategy and Agent
- [x] Algorithm Implementation
