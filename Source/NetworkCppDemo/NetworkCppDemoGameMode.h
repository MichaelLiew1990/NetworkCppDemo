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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Power")
	float PowerDrainDelay;
	FTimerHandle PowerDrainTimer;

protected:
	//The rate at which characters lose power (% of InitialPower per second)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Power", meta = (BlueprintProtected = "true"))
	float DecayRate;

private:
	void DrainPowerOverTime();
};



