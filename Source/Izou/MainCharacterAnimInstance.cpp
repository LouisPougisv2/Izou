// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacterAnimInstance.h"
#include "MainCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UMainCharacterAnimInstance::NativeInitializeAnimation()
{
	MainCharacter = Cast<AMainCharacter>(TryGetPawnOwner());
}

void UMainCharacterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (!MainCharacter)
	{
		MainCharacter = Cast<AMainCharacter>(TryGetPawnOwner());
	}
	if (MainCharacter)
	{
		//Get character's lateral speed from velocity
		//Speed will be the magnitude of this velocity
		FVector Velocity{ MainCharacter->GetVelocity() };
		Velocity.Z = 0.0f;
		Speed = Velocity.Size();

		//Is the Character in the air?
		bIsInAir = MainCharacter->GetCharacterMovement()->IsFalling();
		
		//Is the character moving?
		if (MainCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0)
		{
			bIsAccelerating = true;
		}
		else { bIsAccelerating = false; }

		//Movement Offset Yaw calculation for future animation blending
		FRotator AimRotation = MainCharacter->GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(MainCharacter->GetVelocity());
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

		if (MainCharacter->GetVelocity().Size() > 0)
		{
			LastMovementOffsetYaw = MovementOffsetYaw;
		}
		
		bIsAiming = MainCharacter->GetAiming();
	}

}
