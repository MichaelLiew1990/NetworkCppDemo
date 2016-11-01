#include "NetworkCppDemo.h"
#include "SpawnVolume.h"
#include "Pickup.h"
#include "kismet/KismetMathLibrary.h"


ASpawnVolume::ASpawnVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	if (Role == ROLE_Authority)
	{
		WhereToSpawn = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnVolume"));
		RootComponent = WhereToSpawn;

		//set some base values for range
		SpawnDelayRangeLow = 1.0f;
		SpawnDelayRangeHigh = 3.5f;
	}
}

void ASpawnVolume::BeginPlay()
{
	Super::BeginPlay();
	
	//Delay for a bit before spawning the next pickup
	SpawnDelay = FMath::FRandRange(SpawnDelayRangeLow, SpawnDelayRangeHigh);
	GetWorldTimerManager().SetTimer(SpawnTimer, this, &ASpawnVolume::SpawnPickup, SpawnDelay, false);
}

void ASpawnVolume::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

UBoxComponent* ASpawnVolume::GetWhereToSpawn() const
{
	return WhereToSpawn;
}

FVector ASpawnVolume::GetRandomPointInVolume()
{
	if (WhereToSpawn != NULL)
	{
		FVector SpawnOrigin = WhereToSpawn->Bounds.Origin;
		FVector SpawnExtent = WhereToSpawn->Bounds.BoxExtent;
		return UKismetMathLibrary::RandomPointInBoundingBox(SpawnOrigin, SpawnExtent);
	}
	return FVector();
}

void ASpawnVolume::SpawnPickup()
{
	if (Role == ROLE_Authority && WhatToSpawn != NULL)
	{
		if (UWorld* const World = GetWorld())
		{
			//setup any required params/info
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = Instigator;

			//where shall we put the pickup?
			FVector SpawnLocation = GetRandomPointInVolume();

			//set a random rotation for the spawned pickup
			FRotator SpawnRotation;
			SpawnRotation.Yaw = FMath::FRand()*360.f;
			SpawnRotation.Pitch = FMath::FRand()*360.f;
			SpawnRotation.Roll = FMath::FRand()*360.f;

			//drop the new pickup into the world
			APickup* const SpawnPickup = World->SpawnActor<APickup>(WhatToSpawn, SpawnLocation, SpawnRotation, SpawnParams);

			//Delay for a bit before spawning the next pickup
			SpawnDelay = FMath::FRandRange(SpawnDelayRangeLow, SpawnDelayRangeHigh);
			GetWorldTimerManager().SetTimer(SpawnTimer, this, &ASpawnVolume::SpawnPickup, SpawnDelay, false);
		}
	}
}

