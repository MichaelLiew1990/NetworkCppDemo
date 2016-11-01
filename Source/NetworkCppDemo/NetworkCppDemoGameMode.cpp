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
}
