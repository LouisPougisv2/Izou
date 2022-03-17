// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UCLASS()
class IZOU_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item properties", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ItemMesh;

	//Line trace collides with box to show HUD widget
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item properties", meta= (AllowPrivateAccess = "true"))
	class UBoxComponent* CollisionBox;

	//Pop up Widget for when the player looks at the item
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item properties", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* PickupWidget; 


public:

};
