// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "NetworkCppDemo.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "NetworkCppDemoCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Pickup.h"
#include "BatteryPickup.h"

//////////////////////////////////////////////////////////////////////////
// ANetworkCppDemoCharacter

ANetworkCppDemoCharacter::ANetworkCppDemoCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	CollectionSphereRadius = 150.f;

	// Create a follow CollectionSphere
	CollectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollectionSphere"));
	CollectionSphere->SetupAttachment(RootComponent);
	CollectionSphere->SetSphereRadius(CollectionSphereRadius);

	InitialPower = 2000.f;
	CurrentPower = InitialPower;

	BaseSpeed = 10.f;
	SpeedFactor = 0.75f;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void ANetworkCppDemoCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetworkCppDemoCharacter, CollectionSphereRadius);
	DOREPLIFETIME(ANetworkCppDemoCharacter, InitialPower);
	DOREPLIFETIME(ANetworkCppDemoCharacter, CurrentPower);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ANetworkCppDemoCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ANetworkCppDemoCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ANetworkCppDemoCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ANetworkCppDemoCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ANetworkCppDemoCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ANetworkCppDemoCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ANetworkCppDemoCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ANetworkCppDemoCharacter::OnResetVR);

	//handle collecting pickups
	PlayerInputComponent->BindAction("CollectPickups", IE_Pressed, this, &ANetworkCppDemoCharacter::CollectPickups);
}

void ANetworkCppDemoCharacter::CollectPickups()
{
	ServerCollectPickups();
}

bool ANetworkCppDemoCharacter::ServerCollectPickups_Validate()
{
	return true;
}

void ANetworkCppDemoCharacter::OnPlayerDeath_Implementation()
{
	//disconnect controller form pawn
	DetachFromControllerPendingDestroy();
	if (GetMesh()) {
		GetMesh()->SetCollisionProfileName(FName("Ragdoll"));
	}
	SetActorEnableCollision(true);

	//Ragdoll (init physics)
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->WakeAllRigidBodies();
	GetMesh()->bBlendPhysics = true;

	//disable movement
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->SetComponentTickEnabled(false);

	//disable collisions on the capsule
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void ANetworkCppDemoCharacter::OnRep_CurrentPower()
{
	PowerChangeEffect();
}

void ANetworkCppDemoCharacter::ServerCollectPickups_Implementation()
{
	float TotalPower = 0.0f;

	if (Role == ROLE_Authority)
	{
		TArray<AActor*> CollectedActors;
		CollectionSphere->GetOverlappingActors(CollectedActors);
		for (int i = 0; i < CollectedActors.Num(); i++)
		{
			APickup* TestPickup = Cast<APickup>(CollectedActors[i]);
			if (TestPickup != NULL && !TestPickup->IsPendingKill() && TestPickup->IsActive())
			{
				if (ABatteryPickup* const TestBattery = Cast<ABatteryPickup>(TestPickup))
				{
					TotalPower += TestBattery->GetPower();
				}

				TestPickup->PickedUpBy(this);
				TestPickup->SetActive(false);
			}
		}
	}

	// Change the character's power based on what we picked up
	if (!FMath::IsNearlyZero(TotalPower)) {
		UpdatePower(TotalPower);
	}
}

float ANetworkCppDemoCharacter::GetInitialPower()
{
	return InitialPower;
}

float ANetworkCppDemoCharacter::GetCurrentPower()
{
	return CurrentPower;
}

void ANetworkCppDemoCharacter::UpdatePower(float Delta)
{
	if (Role == ROLE_Authority)
	{
		CurrentPower += Delta;
		GetCharacterMovement()->MaxWalkSpeed = BaseSpeed + SpeedFactor*CurrentPower;

		//Fake the rep notify (litten server doesn't get the RepNotify automatically)
		OnRep_CurrentPower();
	}
}

void ANetworkCppDemoCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ANetworkCppDemoCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	// jump, but only on the first touch
	if (FingerIndex == ETouchIndex::Touch1)
	{
		Jump();
	}
}

void ANetworkCppDemoCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	if (FingerIndex == ETouchIndex::Touch1)
	{
		StopJumping();
	}
}

void ANetworkCppDemoCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ANetworkCppDemoCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ANetworkCppDemoCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ANetworkCppDemoCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
