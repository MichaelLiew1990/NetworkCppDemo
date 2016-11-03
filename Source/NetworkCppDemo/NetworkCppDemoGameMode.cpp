// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "NetworkCppDemo.h"
#include "NetworkCppDemoGameMode.h"
#include "NetworkCppDemoCharacter.h"

ANetworkCppDemoGameMode::ANetworkCppDemoGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	DecayRate = 0.02f;

	PowerDrainDelay = 0.25f;
}

void ANetworkCppDemoGameMode::BeginPlay()
{
	GetWorldTimerManager().SetTimer(PowerDrainTimer, this, &ANetworkCppDemoGameMode::DrainPowerOverTime, PowerDrainDelay, true);
}

float ANetworkCppDemoGameMode::GetDecayRate()
{
	return DecayRate;
}

void ANetworkCppDemoGameMode::DrainPowerOverTime()
{
	UWorld* World = GetWorld();
	check(World);

	for (FConstControllerIterator It = World->GetControllerIterator(); It; It++) {
		if (APlayerController* PlayerController = Cast<APlayerController>(*It)) {
			if (ANetworkCppDemoCharacter* BatteryCharacter = Cast<ANetworkCppDemoCharacter>(PlayerController->GetPawn())) {
				if (BatteryCharacter->GetCurrentPower() > 0) {
					float Delta = -PowerDrainDelay*DecayRate*(BatteryCharacter->GetInitialPower());
					BatteryCharacter->UpdatePower(Delta);
				}
			}
		}
	}
}
