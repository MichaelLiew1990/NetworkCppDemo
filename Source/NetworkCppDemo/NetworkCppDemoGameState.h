// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameState.h"
#include "NetworkCppDemoGameState.generated.h"

UENUM(BlueprintType)
enum EBatteryPlayState
{
	EPlaying,
	EGameOver,
	EWon,
	EUnknown
};

/**
 * 
 */
UCLASS()
class NETWORKCPPDEMO_API ANetworkCppDemoGameState : public AGameState
{
	GENERATED_BODY()
	
	
public:
	ANetworkCppDemoGameState();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Power")
	float PowerToWin;

	UFUNCTION(BlueprintPure, Category = "Power")
	EBatteryPlayState GetCurrentState() const;

	void SetCurrentState(EBatteryPlayState State);

	UFUNCTION()
	void OnRep_CurrentState();

private:
	UPROPERTY(ReplicatedUsing= OnRep_CurrentState)
	TEnumAsByte<enum EBatteryPlayState> CurrentState;
};
