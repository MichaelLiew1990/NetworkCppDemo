// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "NetworkCppDemo.h"
#include "NetworkCppDemoGameMode.h"
#include "NetworkCppDemoCharacter.h"
#include "NetworkCppDemoGameState.h"
#include "SpawnVolume.h"

ANetworkCppDemoGameMode::ANetworkCppDemoGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	static ConstructorHelpers::FClassFinder<AHUD> PlayerHUDClass(TEXT("/Game/Blueprints/BP_NetworkCppDemoHUD"));
	if (PlayerHUDClass.Class != NULL)
	{
		HUDClass = PlayerHUDClass.Class;
	}

	GameStateClass = ANetworkCppDemoGameState::StaticClass();

	DecayRate = 0.05f;

	PowerDrainDelay = 0.25f;

	PowerToWinMultiplier = 1.25f;

	DeadPlayerCount = 0;
}

void ANetworkCppDemoGameMode::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	check(World);
	ANetworkCppDemoGameState* MyGameState = Cast<ANetworkCppDemoGameState>(GameState);
	check(MyGameState);

	DeadPlayerCount = 0;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(World, ASpawnVolume::StaticClass(), FoundActors);
	for (auto Actor : FoundActors) {
		if (ASpawnVolume* Test = Cast<ASpawnVolume>(Actor)) {
			SpawnVolumeActors.AddUnique(Test);
		}
	}

	HandleNewState(EBatteryPlayState::EPlaying);

	for (FConstControllerIterator It = World->GetControllerIterator(); It; It++)
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(*It))
		{
			if (ANetworkCppDemoCharacter* BatteryCharacter = Cast<ANetworkCppDemoCharacter>(PlayerController->GetPawn()))
			{
				MyGameState->PowerToWin = BatteryCharacter->GetInitialPower() * PowerToWinMultiplier;
				break;
			}
		}
	}
}

float ANetworkCppDemoGameMode::GetDecayRate()
{
	return DecayRate;
}

float ANetworkCppDemoGameMode::GetPowerToWinMultiplier()
{
	return PowerToWinMultiplier;
}

void ANetworkCppDemoGameMode::DrainPowerOverTime()
{
	UWorld* World = GetWorld();
	check(World);
	ANetworkCppDemoGameState* MyGameState = Cast<ANetworkCppDemoGameState>(GameState);
	check(MyGameState);

	for (FConstControllerIterator It = World->GetControllerIterator(); It; It++)
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(*It))
		{
			if (ANetworkCppDemoCharacter* BatteryCharacter = Cast<ANetworkCppDemoCharacter>(PlayerController->GetPawn()))
			{
				if (BatteryCharacter->GetCurrentPower() > MyGameState->PowerToWin)
				{
					MyGameState->WinningPlayerName = BatteryCharacter->GetName();
					HandleNewState(EBatteryPlayState::EWon);
				}
				else if (BatteryCharacter->GetCurrentPower() > 0)
				{
					float Delta = -PowerDrainDelay*DecayRate*(BatteryCharacter->GetInitialPower());
					BatteryCharacter->UpdatePower(Delta);
				}
				else
				{
					// Poor player died.
					BatteryCharacter->OnPlayerDeath();
					DeadPlayerCount += 1;

					if (DeadPlayerCount >= GetNumPlayers())
					{
						HandleNewState(EBatteryPlayState::EGameOver);
					}
				}
			}
		}
	}
}

void ANetworkCppDemoGameMode::HandleNewState(EBatteryPlayState NewState)
{
	UWorld* World = GetWorld();
	check(World);
	ANetworkCppDemoGameState* MyGameState = Cast<ANetworkCppDemoGameState>(GameState);
	check(MyGameState);

	if (NewState != MyGameState->GetCurrentState()) {
		MyGameState->SetCurrentState(NewState);
		switch (NewState) {
		case EBatteryPlayState::EPlaying:
		{
			for (ASpawnVolume* SpawnVolume : SpawnVolumeActors) {
				SpawnVolume->SetSpawningActive(true);
			}
			GetWorldTimerManager().SetTimer(PowerDrainTimer, this, &ANetworkCppDemoGameMode::DrainPowerOverTime, PowerDrainDelay, true);
			break;
		}
		case EBatteryPlayState::EGameOver:
		{
			for (ASpawnVolume* SpawnVolume : SpawnVolumeActors) {
				SpawnVolume->SetSpawningActive(false);
			}
			GetWorldTimerManager().ClearTimer(PowerDrainTimer);
			break;
		}
		case EBatteryPlayState::EWon:
		{
			for (ASpawnVolume* SpawnVolume : SpawnVolumeActors) {
				SpawnVolume->SetSpawningActive(false);
			}
			GetWorldTimerManager().ClearTimer(PowerDrainTimer);
			break;
		}
		case EBatteryPlayState::EUnknown:
			break;
		default:
			break;
		}
	}
}
