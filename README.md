# DeepQLearning_Car
Simulation based Self-Driving Car using Reinforcement Learning.\
This project was built with cuda version 10.1 and Unreal Engine 4.23.1.


## Setup
* In DQN_Car.Build.cs file change "cuda_path" to the directory containing your local cuda v10.1 files
* Right click DQN_Car.uproject file and generate visual studio files 
* Copy all the .dll files from ThirdParty/libtorch/lib to Binaries/Win64 (if you don't see this folder, you may have to build the project first)
* Open any file containing ```#include <torch/torch.h>``` you should see a macro ```THIRD_PARTY_INCLUDES_START```, go to its definition  (should be inside WindowsPlatformCompilerPreSetup.h file) and add the following:
``` 
__pragma(warning(disable: 4273))\
__pragma(warning(disable: 4582))\
__pragma(warning(disable: 4583))\
__pragma(warning(disable: 4018))\
```
* Open DQN_Car.sln file and check to see if the startup project is "DQN_Car" and not "UE4"
* Change the "rootPath" variable in "DQN_Car_GI.cpp" to a path on your device (the path should be an absolute path).

## Run
* Make sure to follow all the steps of the Setup
* Then build and run (press F5 for visual studio)
