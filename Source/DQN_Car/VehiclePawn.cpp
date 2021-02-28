// Fill out your copyright notice in the Description page of Project Settings.


#include "VehiclePawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "WheeledVehicleMovementComponent4W.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "CarGI.h"


static const FName NAME_SteerInput("Steer");
static const FName NAME_ThrottleInput("Throttle");

AVehiclePawn::AVehiclePawn()
{
	PrimaryActorTick.bCanEverTick = false;
	UWheeledVehicleMovementComponent4W* Vehicle4W = CastChecked<UWheeledVehicleMovementComponent4W>(GetVehicleMovement());

	// Adjust tire loading
	Vehicle4W->MinNormalizedTireLoad = 0.0f;
	Vehicle4W->MinNormalizedTireLoadFiltered = 0.2f;
	Vehicle4W->MaxNormalizedTireLoad = 2.0f;
	Vehicle4W->MaxNormalizedTireLoadFiltered = 2.0f;

	// Torque setup
	// how much torque we have at given rpm
	Vehicle4W->MaxEngineRPM = 5700.0f;
	Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->Reset();
	Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(0.0f, 400.0f);
	Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(1890.0f, 500.0f);
	Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(5730.0f, 400.0f);

	// Adjusting Steering
	// how much steering we have at a given speed
	Vehicle4W->SteeringCurve.GetRichCurve()->Reset();
	Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(0.0f, 1.0f);
	Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(40.0f, 0.7f);
	Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(120.0f, 0.6f);

	Vehicle4W->DifferentialSetup.DifferentialType = EVehicleDifferential4W::LimitedSlip_4W;
	Vehicle4W->DifferentialSetup.FrontRearSplit = 0.65f;	// rear wheels get more spin

	// Automatic Gear setup
	Vehicle4W->TransmissionSetup.bUseGearAutoBox = false;
	Vehicle4W->TransmissionSetup.GearSwitchTime = 0.15f;
	Vehicle4W->TransmissionSetup.GearAutoBoxLatency = 1.0f;


	// SpringArm Component for the camera
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 250.0f;
	SpringArm->bUsePawnControlRotation = false;

	// Camera Component
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->FieldOfView = 90.0f;

	// Sensor empty mesh component
	// maybe store them in an array later when refactoring
	SensorF = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SensorF"));
	SensorF->SetupAttachment(RootComponent);
	SensorF->SetRelativeTransform(FTransform(FVector(250.0f, 0.0f, 70.0f)));
	SensorF->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	SensorL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SensorL"));
	SensorL->SetupAttachment(RootComponent);
	SensorL->SetRelativeTransform(FTransform(FVector(231.0f, -70.0f, 70.0f)));
	SensorL->SetRelativeRotation(FRotator(0.0f, -30.0f, 0.0f));

	SensorR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SensorR"));
	SensorR->SetupAttachment(RootComponent);
	SensorR->SetRelativeTransform(FTransform(FVector(231.0f, 70.0f, 70.0f)));
	SensorR->SetRelativeRotation(FRotator(0.0f, 30.0f, 0.0f));

	SensorLS = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SensorLS"));
	SensorLS->SetupAttachment(RootComponent);
	SensorLS->SetRelativeTransform(FTransform(FVector(94.0f, -98.0f, 70.0f)));
	SensorLS->SetRelativeRotation(FRotator(0.0f, -60.0f, 0.0f));

	SensorRS = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SensorRS"));
	SensorRS->SetupAttachment(RootComponent);
	SensorRS->SetRelativeTransform(FTransform(FVector(94.0f, 98.0f, 70.0f)));
	SensorRS->SetRelativeRotation(FRotator(0.0f, 60.0f, 0.0f));

	SensorLS1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SensorLS1"));
	SensorLS1->SetupAttachment(RootComponent);
	SensorLS1->SetRelativeTransform(FTransform(FVector(-30.0f, -98.0f, 70.0f)));
	SensorLS1->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	SensorRS1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SensorRS1"));
	SensorRS1->SetupAttachment(RootComponent);
	SensorRS1->SetRelativeTransform(FTransform(FVector(-30.0f, 98.0f, 70.0f)));
	SensorRS1->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));


	CurrentState = torch::zeros({ UCarGI::NumStates }, Device);
	CurrentReward = torch::zeros({ 1 }, Device);
}

AVehiclePawn::~AVehiclePawn()
{

}

void AVehiclePawn::BeginPlay()
{
	Super::BeginPlay();

	USkeletalMeshComponent* meshComp = GetMesh();
	meshComp->OnComponentHit.AddDynamic(this, &AVehiclePawn::OnHit);
	meshComp->OnComponentBeginOverlap.AddDynamic(this, &AVehiclePawn::OnOverlap);

	TraceDistance();	// get state and reward
}

void AVehiclePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AVehiclePawn::TraceDistance(bool debugLine)
{
	FHitResult outHitF, outHitL, outHitR, outHitLS, outHitRS, outHitLS1, outHitRS1;
	TraceByProfile(outHitF,   SensorF->GetComponentLocation(),   SensorF->GetForwardVector(),   debugLine, FColor::Green);
	TraceByProfile(outHitL,   SensorL->GetComponentLocation(),   SensorL->GetForwardVector(),   debugLine, FColor::Green);
	TraceByProfile(outHitR,   SensorR->GetComponentLocation(),   SensorR->GetForwardVector(),   debugLine, FColor::Green);
	TraceByProfile(outHitLS,  SensorLS->GetComponentLocation(),  SensorLS->GetForwardVector(),  debugLine, FColor::Green);
	TraceByProfile(outHitRS,  SensorRS->GetComponentLocation(),  SensorRS->GetForwardVector(),  debugLine, FColor::Green);
	TraceByProfile(outHitLS1, SensorLS1->GetComponentLocation(), SensorLS1->GetForwardVector(), debugLine, FColor::Green);
	TraceByProfile(outHitRS1, SensorRS1->GetComponentLocation(), SensorRS1->GetForwardVector(), debugLine, FColor::Green);

	//FString debugMsg = FString::Printf(TEXT(	// display sensor values on screen
	//		"LS1 = %.3f    LS = %.3f    L = %.3f    F = %.3f    R = %.3f    RS = %.3f    RS1 = %.3f"
	//	), 
	//	outHitLS1.Time,
	//	outHitLS.Time,
	//	outHitL.Time,
	//	outHitF.Time,
	//	outHitR.Time,
	//	outHitRS.Time,
	//	outHitRS1.Time
	//);
	//if (GEngine)
	//	GEngine->AddOnScreenDebugMessage(1, 0.0, FColor::Magenta, debugMsg);

	// Get current state
	CurrentState = torch::tensor({
		outHitLS1.Time,
		outHitLS.Time,
		outHitL.Time,
		outHitF.Time,
		outHitR.Time,
		outHitRS.Time,
		outHitRS1.Time
	}, Device);

	// Get reward
	if (Done)
		CurrentReward = torch::tensor(-200.0f);

	else if (  outHitF.Time   < 0.1f
			|| outHitL.Time   < 0.1f 
			|| outHitR.Time   < 0.1f
			|| outHitLS.Time  < 0.1f 
			|| outHitRS.Time  < 0.1f 
			|| outHitLS1.Time < 0.1f 
			|| outHitRS1.Time < 0.1f)
		CurrentReward = torch::tensor(-20.0f);

	else
		CurrentReward = torch::tensor(0.0f);

	if (!Done && HitGate)
	{
		CurrentReward += torch::tensor(50.0f);
		HitGate = false;
	}
}

void AVehiclePawn::TakeAction(torch::Tensor& action)
{
	if (action.item<int>() == 0)
	{
		ApplyThrottle(0.8f);
		//UE_LOG(LogTemp, Warning, TEXT("Throttle %d"), action.item<int>());
	}
	else if (action.item<int>() == 1)
	{
		ApplyThrottle(0.8f);
		ApplySteering(-0.8f);
		//UE_LOG(LogTemp, Warning, TEXT("Steer Left with throttle %d"), action.item<int>());
	}
	else if (action.item<int>() == 2)
	{
		ApplyThrottle(0.8f);
		ApplySteering(0.8f);
		//UE_LOG(LogTemp, Warning, TEXT("Steer Right with throttle %d"), action.item<int>());
	}
	//else if (action.item<int>() == 3)
	//{
	//	ApplySteering(-0.8f);
	//	//UE_LOG(LogTemp, Warning, TEXT("Steer left %d"), action.item<int>());
	//}
	//else if (action.item<int>() == 4)
	//{
	//	ApplySteering(0.8f);
	//	//UE_LOG(LogTemp, Warning, TEXT("Steer Right %d"), action.item<int>());
	//}

	// for next state and reward
	TraceDistance();
}

void AVehiclePawn::ApplyThrottle(float val)
{
	//UE_LOG(LogTemp, Warning, TEXT("Throttle: %f"), val);
	GetVehicleMovementComponent()->SetThrottleInput(val);
}

void AVehiclePawn::ApplySteering(float val)
{
	//UE_LOG(LogTemp, Warning, TEXT("Steering: %f"), val);
	GetVehicleMovementComponent()->SetSteeringInput(val);
}

void AVehiclePawn::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		Done = true;
		//if (GEngine)
		//	GEngine->AddOnScreenDebugMessage(5, 0.0, FColor::Purple, FString::Printf(TEXT("I Hit: %s"), *OtherActor->GetName()));
	}
}

void AVehiclePawn::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// for reward gate
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		HitGate = true;
		//if (GEngine)
		//	GEngine->AddOnScreenDebugMessage(8, 1.0, FColor::Orange, FString::Printf(TEXT("I Hit: %s"), *OtherActor->GetClass()->GetName()));
	}
}

void AVehiclePawn::TraceByProfile(FHitResult& outHit, const FVector& start, const FVector& forward, bool drawDebug, FColor debugColor)
{
	FVector end = (forward * 1000.0f) + start;

	if (drawDebug)
		DrawDebugLine(GetWorld(), start, end, debugColor);

	FCollisionQueryParams collisionParams;

	bool isHit = GetWorld()->LineTraceSingleByProfile(outHit, start, end, "BlockAll", collisionParams);
	// uncomment if isHit is needed // example given in the comment below
	//return isHit;

	// example: 
	//if (isHit_F)
	//{
	//	if (outHitForward.bBlockingHit)
	//	{

	//	}
	//}
}

//void AVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
//{
//	Super::SetupPlayerInputComponent(PlayerInputComponent);
//
//	PlayerInputComponent->BindAxis(NAME_ThrottleInput, this, &AVehiclePawn::ApplyThrottle);
//	PlayerInputComponent->BindAxis(NAME_SteerInput, this, &AVehiclePawn::ApplySteering);
//}
