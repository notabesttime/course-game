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
	static AHealingOrb* SpawnOrReuse(UWorld* World, TSubclassOf<AHealingOrb> OrbClass,
		const FVector& Location, const FRotator& Rotation, AActor* Owner = nullptr);
	static void GetPoolStats(int32& OutTotal, int32& OutActive, int32& OutInactive);
	static void LogPoolStats();

	void ActivateFromPool(const FVector& Location, const FRotator& Rotation, AActor* InOwner);
	void ReturnToPool();

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

	// Safety timeout so pooled orbs don't remain active forever
	UPROPERTY(EditDefaultsOnly, Category = "Orb")
	float MaxActiveLifetime = 20.f;

private:
	UPROPERTY()
	class APlayerCharacter* CachedPlayer;

	bool bIsPooledActive = true;
	FTimerHandle AutoReturnTimer;
};
