// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HealingOrb.generated.h"

UCLASS()
class GAME_COURSE_API AHealingOrb : public AActor
{
	GENERATED_BODY()

public:
	AHealingOrb();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UStaticMeshComponent* Mesh;

	// Health restored to the player on pickup
	UPROPERTY(EditDefaultsOnly, Category = "Orb")
	float HealAmount = 25.f;

	// Units per second the orb flies toward the player
	// Enemies move at 300, player runs at 600 — 450 sits between them
	UPROPERTY(EditDefaultsOnly, Category = "Orb")
	float MoveSpeed = 450.f;

	// Distance at which the orb is collected
	UPROPERTY(EditDefaultsOnly, Category = "Orb")
	float PickupRadius = 80.f;

	// Seconds after an attack during which the orb stays still
	UPROPERTY(EditDefaultsOnly, Category = "Orb")
	float AttackPauseTime = 0.2f;

private:
	UPROPERTY()
	class APlayerCharacter* CachedPlayer;
};
