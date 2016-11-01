// Fill out your copyright notice in the Description page of Project Settings.

#include "NetworkCppDemo.h"
#include "BatteryPickup.h"




ABatteryPickup::ABatteryPickup()
{
	bReplicateMovement = true;

	GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
	GetStaticMeshComponent()->SetSimulatePhysics(true);
}

void ABatteryPickup::WasCollected_Implementation()
{
	Super::WasCollected_Implementation();

	Destroy();
}
