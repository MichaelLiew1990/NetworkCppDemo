// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameMode.h"
#include "NetworkCppDemoGameMode.generated.h"

UCLASS(minimalapi)
class ANetworkCppDemoGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ANetworkCppDemoGameMode();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category = "Power")
	float GetDecayRate();

	UFUNCTION(BlueprintPure, Category = "Power")
	float GetPowerToWinMultiplier();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Power")
	float PowerDrainDelay;
	FTimerHandle PowerDrainTimer;

	//The rate at which characters lose power (% of InitialPower per second)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Power", meta = (BlueprintProtected = "true"))
	float DecayRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Power", meta = (BlueprintProtected = "true"))
	float PowerToWinMultiplier;

	int32 DeadPlayerCount;

private:
	void DrainPowerOverTime();
};



