// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MainCharacter.generated.h"

UCLASS()
class IZOU_API AMainCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMainCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	// -------------------------          Movements function          -------------------------

	void MoveForward(float Value);
	void MoveRight(float Value);

	/*Called via input to turn/lookup at a given rate
	//@param Rate is a normalized rate, i.e 1.0 mean 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);
	void LookUpRate(float Rate);
	// ----------------------------------------------------------------------------------------

	/*Rotate controller based on mouse X or Y movement
	@param Value : the input value from mouse movement
	*/
	void Turn(float Value);
	void LookUp(float Value);

	//Called when the fireButton is Pressed
	void FireWeapon();

	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);

	//Set bIsAiming to true or false when button presed/released
	void AimingButtonPressed();
	void AimingButtonReleased();

	//Interpolate to smoothly zoom 
	void ZoomInterpolation(float DeltaTime);

	//Set the correct base turn/look up rates if aiming or not
	void SetLookRates();


	void CalculateCrosshairSpread(float DeltaTime);
	void CalculateCrosshairInAir(float DeltaTime);
	void CalculateCrosshairAimFactor(float DeltaTime);
	void CalculateCrosshairShootingFactor(float DeltaTime);

	void StartCrosshairBulletFire();

	UFUNCTION() //Has to be a UFUNCTION as it is going to be a callback function for a TimerHandle
	void FinishCrosshairBulletFire();


	void FireButtonPressed();
	void FireButtonReleased();
	void StartFireTimer();

	UFUNCTION()	//UFUNCTION because it's going to be a callback for our timer	
	void AutoFireReset();

	//Line trace for items under the crosshair
	bool TraceUnderCrosshair(FHitResult& OutHitResult, FVector& OutHitLocation);

	//Trace for Items if overlapped count is positive
	void TraceForItems();


	void SpawnDefaultWeapon();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


private:

	//Create a Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

	//Camera boom position the camera behind the camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	//Base turn rate in degrees per/sec. Other scaling may affect final turn rate
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	//Turn rate while  not aiming
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float HipTurnRate;

	//Look up rate when not aiming
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float HipLookUpRate;

	//Turn rate while aiming
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float AimingTurnRate;

	//Look up rate when aiming
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float AimingLookUpRate;

	//------------------------------ Varaible for the Mouse input -----------------------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		float MouseHipTurnRate;

	//Look up rate when not aiming
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		float MouseHipLookUpRate;

	//Turn rate while aiming
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		float MouseAimingTurnRate;

	//Look up rate when aiming
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		float MouseAimingLookUpRate;

	//------------------------------------------------------------------------------------------------------

	//Randomized gunshot cue
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound;

	//Flash spawned at barrel socket
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;

	//Montage for firing the weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* HipFireMontage;

	//Particles spawned upon bullet impact
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;
	
	//Smoke trail for bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;

	//True when aiming
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bIsAiming;

	//Default camera field of view value
	float CameraDefaultFieldOfView;

	//Field of view value when zoomed in
	float CameraZoomedFieldOFView;

	//Current field of view this frame
	float CameraCurrentFOV;

	//Interp speed for zooming when aiming
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;

	//Determines the spread of the crosshair
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshair", meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier;

	//Velocity component for crosshair spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshair", meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;

	//In Air component for crosshair spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshair", meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor;
	
	//Aim component for crosshair spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshair", meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor;

	//Shooting component for crosshair spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshair", meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor;

	float ShootTimeDuration;
	bool bIsFiringBullet;
	FTimerHandle CrosshairShootTimer;

	//Is Left mouse button or right console trigger pressed
	bool bIsFireButtonPressed;

	//True when we can fire, false when waiting for the timer
	bool bShouldFire;

	//Rate opf automatic rate fire
	float AutomaticFireRate;

	//Sets a timer between gunshots
	FTimerHandle AutoFireTimer;

	//true if we should trace every frame for items
	bool bShouldTraceForItems;

	//Number of overlapped AItems
	int8 OverlappedItemCount;

	//The Aitem we hit last frame
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items", meta = (AllowPrivateAccess = "true"))
	class AItem* TraceHitItemLastFrame;

	//Currently equipped weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class AWeapon* EquippedWeapon;

	//Set this in BP for the defauts weapon class
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeaponClass;

public:

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }
	
	FORCEINLINE bool GetAiming() const { return bIsAiming; }

	UFUNCTION(BlueprintCallable)	//UFUNCTION cannot be inline
	float GetCrosshairSpreadMultiplier() const;

	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }

	//Add/Substract to/from OverlappedItemCount and update bShouldTraceForItems
	void IncrementOverlappedItemCount(int8 Amount);
};
