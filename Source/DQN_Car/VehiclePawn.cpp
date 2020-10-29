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
	SensorForward = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SensorForward"));
	SensorForward->SetupAttachment(RootComponent);

	SensorLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SensorLeft"));
	SensorLeft->SetupAttachment(RootComponent);

	SensorRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SensorRight"));
	SensorRight->SetupAttachment(RootComponent);

	SensorLeftSide = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SensorLeftSide"));
	SensorLeftSide->SetupAttachment(RootComponent);

	SensorRightSide = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SensorRightSide"));
	SensorRightSide->SetupAttachment(RootComponent);


	CurrentState = torch::zeros({ 1, UCarGI::NumStates }, Device);
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
	FHitResult outHitForward, outHitLeft, outHitRight, outHitLeftSide, outHitRightSide;
	TraceByProfile(outHitForward,   SensorForward->GetComponentLocation(),   SensorForward->GetForwardVector(), debugLine, FColor::Green);
	TraceByProfile(outHitLeft,      SensorLeft->GetComponentLocation(),      SensorLeft->GetForwardVector(), debugLine, FColor::Red);
	TraceByProfile(outHitRight,     SensorRight->GetComponentLocation(),     SensorRight->GetForwardVector(), debugLine, FColor::Blue);
	TraceByProfile(outHitLeftSide,  SensorLeftSide->GetComponentLocation(),  SensorLeftSide->GetForwardVector(), debugLine, FColor::Magenta);
	TraceByProfile(outHitRightSide, SensorRightSide->GetComponentLocation(), SensorRightSide->GetForwardVector(), debugLine, FColor::Cyan);

	FString debugMsg = FString::Printf(TEXT(	// display sensor values on screen
		"LS = %.2f    L  = %.2f    F  = %.2f    R  = %.2f    RS = %.2f"
	),
		outHitLeftSide.Time,
		outHitLeft.Time,
		outHitForward.Time,
		outHitRight.Time,
		outHitRightSide.Time
	);
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(1, 0.0, FColor::Magenta, debugMsg);

	// Get current state
	CurrentState = torch::tensor({ {	// 1x5 tensor
		outHitLeftSide.Time,
		outHitLeft.Time,
		outHitForward.Time,
		outHitRight.Time,
		outHitRightSide.Time,
	} }, Device);

	// Get reward
	if (Done)
	{
		CurrentReward = torch::tensor(-200.0f);
	}

	else if (outHitLeftSide.Time < 0.2f || outHitLeft.Time < 0.2f || outHitForward.Time < 0.2f 
			|| outHitRight.Time < 0.2f || outHitRightSide.Time < 0.2f)
		CurrentReward = torch::tensor(-30.0f);

	else
		CurrentReward = torch::tensor(0.5f);

	if (HitGate)
	{
		CurrentReward += torch::tensor(10.0f);
		HitGate = false;
	}
}

void AVehiclePawn::TakeAction(torch::Tensor& action)
{
	if (action[0].item<int>() == 0)
	{
		ApplyThrottle(1);
		//UE_LOG(LogTemp, Warning, TEXT("Throttle %d"), action[0].item<int>());
	}
	else if (action[0].item<int>() == 1)
	{
		ApplyThrottle(1);
		ApplySteering(-1);
		//UE_LOG(LogTemp, Warning, TEXT("Steer Left %d"), action[0].item<int>());
	}
	else if (action[0].item<int>() == 2)
	{
		ApplyThrottle(1);
		ApplySteering(1);
		//UE_LOG(LogTemp, Warning, TEXT("Steer Right %d"), action[0].item<int>());
	}

	// for next state and reward
	TraceDistance(false);
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
	/*if (isHit_F)
	{
		if (outHitForward.bBlockingHit)
		{

		}
	}*/
}

void AVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(NAME_ThrottleInput, this, &AVehiclePawn::ApplyThrottle);
	PlayerInputComponent->BindAxis(NAME_SteerInput, this, &AVehiclePawn::ApplySteering);
}
