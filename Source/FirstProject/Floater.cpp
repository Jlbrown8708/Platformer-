// Fill out your copyright notice in the Description page of Project Settings.


//#include "Floater.h"
//#include "Components/StaticMeshComponent.h"
//
//// Sets default values
//AFloater::AFloater()
//{
// 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
//	PrimaryActorTick.bCanEverTick = true;
//
//
//	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CustomStaticMesh"));
//
//	InitialLocation = FVector(0.0f);
//	PlacedLocation = FVector(0.0f);
//	WorldOrigin = FVector(0.0f, 0.0f, 0.0f);
//	InitialDirection = FVector(0.0f, 0.0f, 0.0f);
//	InitialForce = FVector(2000000.0f, 0.0f, 0.0f);
//	InitialTorque = FVector(2000000.0f, 0.0f, 0.0f);
//	bInitializeFloaterLocation = false;
//	bShouldFloat = false;
//
//	RunningTime = 0.0f;
//
//}
//
//// Called when the game starts or when spawned
//void AFloater::BeginPlay()
//{
//	Super::BeginPlay();
//
//	float InitialX = FMath::FRandRange(-500.f, 500.f);
//	float InitialY = FMath::FRandRange(-500.f, 500.f);
//	float InitialZ = FMath::FRandRange(0.f, 500.f);
//
//	InitialLocation.X = InitialX;
//	InitialLocation.Y = InitialY;
//	InitialLocation.Z = InitialZ;
//
//	
//	//InitialLocation *= 20.f;
//	
//	PlacedLocation = GetActorLocation();
//
//	if (bInitializeFloaterLocation)
//	{
//		SetActorLocation(InitialLocation);
//	}
//	
//	BaseZLocation = PlacedLocation.Z;
//
//	
//	StaticMesh->AddForce(InitialForce);
//	StaticMesh->AddTorque(InitialTorque);
//}
//
//// Called every frame
//void AFloater::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//
//	if (bShouldFloat)
//	{
//		
//		FVector NewLocation = GetActorLocation();
//		NewLocation.Z = BaseZLocation + (100.f) * FMath::Sin(RunningTime);
//		
//		RunningTime += DeltaTime;
//		
//		
//		
//		
//		
//		
//		/*FHitResult HitResult;
//		AddActorLocalOffset(InitialDirection, true, &HitResult);
//
//		FVector HitLocation = HitResult.Location;*/
//
//		}
//
//	FRotator Rotation = FRotator(0.0f, 0.0f, 0.0f);
//
//	AddActorLocalRotation(Rotation);
//
//}
//
