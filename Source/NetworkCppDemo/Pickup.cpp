// Fill out your copyright notice in the Description page of Project Settings.

#include "NetworkCppDemo.h"
#include "Net/UnrealNetwork.h"
#include "Pickup.h"


APickup::APickup()
{
	//Tell UE4 to replicate this actor
	bReplicates = true;

	//Pickups do not need to tick every frame
	PrimaryActorTick.bCanEverTick = false;

	if (Role == ROLE_Authority)
	{
		bIsActive = true;
	}
}

void APickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APickup, bIsActive);
}

bool APickup::IsActive()
{
	return bIsActive;
}

void APickup::SetActiveState(bool b)
{
	if (Role == ROLE_Authority)
	{
		bIsActive = b;
	}
}

void APickup::OnRep_IsActive()
{

}
