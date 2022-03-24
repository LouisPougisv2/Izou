// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Animation/AnimMontage.h"
#include "DrawDebugHelpers.h"
#include "Item.h"
#include "Components/WidgetComponent.h"
#include "Weapon.h"




// Sets default values
AMainCharacter::AMainCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create a Camera boom(pull in toward character if there's a collision
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 180.0f;
	CameraBoom->bUsePawnControlRotation = true; //Whenever our controller moves, the Camera will use this rotation
	CameraBoom->SocketOffset = FVector(0.0f, 50.0f, 70.0f);

	//Create a Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; // We don;t want the camera to rotate relative to the arm
	
	// Don't rotate when the controller rotates. Let the controller only affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // ... at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	//Base rates for turning/looking up
	BaseTurnRate = 45.0f;
	BaseLookUpRate = 45.0f;
	bIsAiming = false;
	//Turn rates for aiming/not aiming (Controller gameplay inputs)
	HipTurnRate = 80.0f;
	HipLookUpRate= 80.0f;
	AimingTurnRate = 35.0f;
	AimingLookUpRate = 35.0f;
	////Turn rates for aiming/not aiming (Mouse gameplay inputs)
	MouseHipTurnRate = 1.0f;
	MouseHipLookUpRate = -1.0f;
	MouseAimingTurnRate = 0.8f;
	MouseAimingLookUpRate = -0.9f;

	//Camera field of view values
	CameraDefaultFieldOfView = 0.0f; //Set in BeginPlay
	CameraZoomedFieldOFView = 35.0f;
	CameraCurrentFOV = 0.0f;
	ZoomInterpSpeed = 20.0f;

	//Crosshair spread factors
	CrosshairSpreadMultiplier = 0.0f;
	CrosshairAimFactor = 0.0f;
	CrosshairInAirFactor = 0.0f;
	CrosshairVelocityFactor = 0.0f;
	CrosshairShootingFactor = 0.0f;

	//Bullet Fire Timer Variables
	ShootTimeDuration = 0.05f;
	bIsFiringBullet = false;

	//Automatic Fire Variables
	AutomaticFireRate = 0.15f;
	bShouldFire = true;
	bIsFireButtonPressed = false;

	//Item trace variables
	bShouldTraceForItems = false;
}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (Camera)
	{
		CameraDefaultFieldOfView = GetCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFieldOfView;
	}
	//Spawn the default Weapon and attach it to the mesh
	SpawnDefaultWeapon();
}

void AMainCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f) )
	{
		const FRotator Rotation = GetActorRotation();
		const FRotator YawRotation{ 0.0f, Rotation.Yaw, 0.0f };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };
		AddMovementInput(Direction, Value);

	}
}

void AMainCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		const FRotator Rotation = GetActorRotation();
		const FRotator YawRotation{ 0.0f, Rotation.Yaw, 0.0f };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };
		AddMovementInput(Direction, Value);

	}
}

void AMainCharacter::TurnAtRate(float Rate)
{
	float DeltaTime = GetWorld()->DeltaTimeSeconds;
	AddControllerYawInput(Rate * BaseTurnRate * DeltaTime);
}

void AMainCharacter::LookUpRate(float Rate)
{
	float DeltaTime = GetWorld()->DeltaTimeSeconds;
	AddControllerPitchInput(Rate * BaseLookUpRate * DeltaTime);

}

void AMainCharacter::FireWeapon()
{
	//Playing sound effect
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}

	//Playing Firing particles
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector BeamEndPoint;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEndPoint);
		if (bBeamEnd)
		{
			//Spawn impact particle after updtaing BeamEndPoint
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEndPoint);
			}

			if (BeamParticles) //Bullet trace particle
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
				if (Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEndPoint);
				}
			}
		}
	}

	//Playing Firing animation
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}

	//Start bullet fire timer for crosshair
	StartCrosshairBulletFire();
}

bool AMainCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	//Check for crosshair hit
	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshair(CrosshairHitResult, OutBeamLocation);
	
	if (bCrosshairHit)
	{
		//Tentative beam location (still need to trace from the barrel)
		OutBeamLocation = CrosshairHitResult.Location;
	}
	else //no crosshair trace hit
	{
		//OutBeamLocation is the end location for the line trace
	}

	//Perform a second trace, this time from the gun barrel
	FHitResult WeaponTraceHit;
	FVector WeaponTraceStart{ MuzzleSocketLocation };
	FVector StartToEnd{ OutBeamLocation - MuzzleSocketLocation };
	FVector WeaponTraceEnd{ MuzzleSocketLocation + StartToEnd * 1.25f }; //makes the line trace 25% longer instead of stopping at the previous hitlocation
	GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

	if (WeaponTraceHit.bBlockingHit) //object between barrel and BeamEndPoint?
	{
		OutBeamLocation = WeaponTraceHit.Location;
		return true;
	}
	return false;
}

void AMainCharacter::AimingButtonPressed()
{
	bIsAiming = true;
}

void AMainCharacter::AimingButtonReleased()
{
	bIsAiming = false;
}

void AMainCharacter::ZoomInterpolation(float DeltaTime)
{
	//Set current camera field of view
	if (bIsAiming)
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFieldOFView, DeltaTime, ZoomInterpSpeed);
	}
	else
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFieldOfView, DeltaTime, ZoomInterpSpeed);
	}
	Camera->SetFieldOfView(CameraCurrentFOV);
}

void AMainCharacter::SetLookRates()
{
	APlayerController* PlayerController = CastChecked<APlayerController>(Controller);
	if (bIsAiming)
	{
		PlayerController->InputYawScale = MouseAimingTurnRate;
		PlayerController->InputPitchScale = MouseAimingLookUpRate;
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		PlayerController->InputYawScale = MouseHipTurnRate;
		PlayerController->InputPitchScale = MouseHipLookUpRate;
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

void AMainCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	//Vector representing the range of the speed
	FVector2D WalkSpeedRange{ 0.0f, 600.0f };
	FVector2D VelocityMultiplierRange{ 0.0f, 1.0f };

	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0.0f; //We only want the lateral component

	//Calculation of the crosshair based on the velocity
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	//Calculation of the Crosshair if the player is jumping
	CalculateCrosshairInAir(DeltaTime);

	//Calculation of the crosshair when the player is aiming
	CalculateCrosshairAimFactor(DeltaTime);

	//Calulation of the crosshair shen the player is shooting
	CalculateCrosshairShootingFactor(DeltaTime);


	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
}

void AMainCharacter::CalculateCrosshairInAir(float DeltaTime)
{
	if (GetCharacterMovement()->IsFalling()) //is in air
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);	//spread the crosshair slowly when falling
	}
	else //if on the ground
	{
		//Shrink the crosshair rapidly while back on the ground
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.0f, DeltaTime, 20.0f);
	}
}

void AMainCharacter::CalculateCrosshairAimFactor(float DeltaTime)
{
	if (bIsAiming)
	{
		//Shrink the crosshair very quickly to a small amount
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.5f, DeltaTime, 20.0f);
	}
	else
	{
		//Spread crosshair back to normal quickly
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.0f, DeltaTime, 20.0f);
	}
}

void AMainCharacter::CalculateCrosshairShootingFactor(float DeltaTime)
{
	//0.05 seconds after firing if true
	if (bIsFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 45.0f);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.0f, DeltaTime, 20.0f);
	}
}

void AMainCharacter::StartCrosshairBulletFire()
{
	bIsFiringBullet = true;
	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &AMainCharacter::FinishCrosshairBulletFire, ShootTimeDuration);
}

void AMainCharacter::FinishCrosshairBulletFire()
{
	bIsFiringBullet = false;
}

void AMainCharacter::FireButtonPressed()
{
	bIsFireButtonPressed = true;
	StartFireTimer(); //Start the fire after actually Firing the Weapon
}

void AMainCharacter::FireButtonReleased()
{
	bIsFireButtonPressed = false;
}

void AMainCharacter::StartFireTimer()
{
	if (bShouldFire)
	{
		FireWeapon();
		bShouldFire = false;
		GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AMainCharacter::AutoFireReset, AutomaticFireRate);
	}
}

void AMainCharacter::AutoFireReset()
{
	bShouldFire = true;
	if (bIsFireButtonPressed)
	{
		StartFireTimer();
	}
}

bool AMainCharacter::TraceUnderCrosshair(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	//Get current size of the viewport
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport) // GEngine holds the viewport
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	//Get Screen space location of crosshair
	FVector2D CrosshairLocation{ ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f };

	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	//Get position and direction of crosshair
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		//Trace from crosshair location outward
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ Start + CrosshairWorldDirection * 50'000.0 };
		OutHitLocation = End;

		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if (OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}

	return false;
}

void AMainCharacter::TraceForItems()
{
	if (bShouldTraceForItems)
	{
		FHitResult ItemTraceResult;
		FVector HitLocation;
		TraceUnderCrosshair(ItemTraceResult, HitLocation);
		if (ItemTraceResult.bBlockingHit)
		{
			AItem* HitItem = Cast<AItem>(ItemTraceResult.Actor);
			if (HitItem && HitItem->GetPickupWidget())
			{
				//Show Item's Pickup Widget
				HitItem->GetPickupWidget()->SetVisibility(true);
			}

			//We hit an AItem last frame
			if (TraceHitItemLastFrame)
			{
				if (HitItem != TraceHitItemLastFrame)
				{
					//We are hitting a different AItem this frame from the last one
					//Or AItem is Null
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
				}
			}
			//Store a reference to hit item for next frame
			TraceHitItemLastFrame = HitItem;
		}
	}
	else if (TraceHitItemLastFrame)
	{
		//No longer overlapping any items
		//Item last frame shouldn't show widget
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
	}
}

void AMainCharacter::SpawnDefaultWeapon()
{
	//check the TSubclassOf variable
	if (DefaultWeaponClass)
	{
		AWeapon* DefaultWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass); // Spawn a weapon
		
		//get the hand socket
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(DefaultWeapon, GetMesh());	// attach the weapon to the hand socket
		}
		//Set Equipped Weapon to newly spawned one
		EquippedWeapon = DefaultWeapon;
	}
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ZoomInterpolation(DeltaTime);

	//Change look sensitivity based on aiming
	SetLookRates();

	//Calculate Crosshair Spread Multiplier
	CalculateCrosshairSpread(DeltaTime);

	//Check Overlapped items, then trace for items
	TraceForItems();

}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//ActionMapping
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("FireButton", EInputEvent::IE_Pressed, this, &AMainCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", EInputEvent::IE_Released, this, &AMainCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("AimingButton", EInputEvent::IE_Pressed, this, &AMainCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", EInputEvent::IE_Released, this, &AMainCharacter::AimingButtonReleased);

	//Axis Mapping
	PlayerInputComponent->BindAxis("MoveForward", this, &AMainCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMainCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMainCharacter::LookUpRate);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	

}

float AMainCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier; 
}

void AMainCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}

