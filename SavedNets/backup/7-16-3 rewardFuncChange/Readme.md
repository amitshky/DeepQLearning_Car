# hyperparameters
Start = 0.99f;\
End   = 0.01f;\
Decay = 1e-4f;\
Gamma = 0.95f;\
LearningRate = 1e-3f;

Capacity  = 256 * 1024;\
BatchSize  = 64;\
NumStates  = 7;\
NumActions = 3;\
UpdateStep = 10 * 3600;

HiddenLayer1 = 16\
HiddenLayer2 = 16\
HiddenLayer3 = 8


optimizer = rmsprop

## reward function:
```
if (Done)
	CurrentReward = torch::tensor(-200.0f);

else if (  outHitF.Time   < 0.2f
		|| outHitL.Time   < 0.2f 
		|| outHitR.Time   < 0.2f
		|| outHitLS.Time  < 0.2f 
		|| outHitRS.Time  < 0.2f 
		|| outHitLS1.Time < 0.2f 
		|| outHitRS1.Time < 0.2f)
	CurrentReward = torch::tensor(-70.0f);

else
	CurrentReward = torch::tensor(0.0f);

if (!Done && HitGate)
{
	CurrentReward += torch::tensor(100.0f);
	HitGate = false;
}
```