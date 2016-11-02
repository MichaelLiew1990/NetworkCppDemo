// Fill out your copyright notice in the Description page of Project Settings.

#include "NetworkCppDemo.h"
#include "BatteryPickup.h"
#include "Net/UnrealNetwork.h"



ABatteryPickup::ABatteryPickup()
{
	bReplicateMovement = true;

	GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
	GetStaticMeshComponent()->SetSimulatePhysics(true);

	BatteryPower = 200.f;
}

void ABatteryPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABatteryPickup, BatteryPower);
}

void ABatteryPickup::PickedUpBy(APawn * Pawn)
{
	Super::PickedUpBy(Pawn);

	if (Role == ROLE_Authority)
	{
		//Get clients time to play vfx, etc... before destroying the battery
		SetLifeSpan(2.f);//destroy self by time
	}
}

float ABatteryPickup::GetPower()
{
	return BatteryPower;
}
