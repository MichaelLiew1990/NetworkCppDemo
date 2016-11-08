// Fill out your copyright notice in the Description page of Project Settings.

#include "NetworkCppDemo.h"
#include "NetworkCppDemoGameState.h"
#include "Net/UnrealNetwork.h"




ANetworkCppDemoGameState::ANetworkCppDemoGameState()
{
	CurrentState = EBatteryPlayState::EUnknown;
}

void ANetworkCppDemoGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetworkCppDemoGameState, PowerToWin);
	DOREPLIFETIME(ANetworkCppDemoGameState, CurrentState);
	DOREPLIFETIME(ANetworkCppDemoGameState, WinningPlayerName);
}

EBatteryPlayState ANetworkCppDemoGameState::GetCurrentState() const
{
	return CurrentState;
}

void ANetworkCppDemoGameState::SetCurrentState(EBatteryPlayState State)
{
	if (Role == ROLE_Authority)
	{
		CurrentState = State;
	}
}

void ANetworkCppDemoGameState::OnRep_CurrentState()
{

}