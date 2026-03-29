// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MinionMageProjectile.generated.h"

UCLASS(Blueprintable)
class GAME_COURSE_API AMinionMageProjectile : public AActor
{
	GENERATED_BODY()

public:
	AMinionMageProjectile();
	static AMinionMageProjectile* SpawnOrReuse(UWorld* World, TSubclassOf<AMinionMageProjectile> ProjectileClass,
		const FVector& Location, const FRotator& Rotation, AActor* Owner = nullptr, APawn* InstigatorPawn = nullptr);
	static void GetPoolStats(int32& OutTotal, int32& OutActive, int32& OutInactive);
	static void LogPoolStats();

	void ActivateFromPool(const FVector& Location, const FRotator& Rotation, AActor* InOwner, APawn* InstigatorPawn);
	void ReturnToPool();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* SphereComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UProjectileMovementComponent* ProjectileMovement;

	// Damage dealt to the player on hit
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float DamageAmount = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float PooledLifetime = 5.f;

	// Called on impact — implement in Blueprint for VFX/sound
	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile")
	void OnProjectileImpact(FVector ImpactLocation);

private:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	bool bHasImpacted = false;
	bool bIsPooledActive = true;
	FTimerHandle ReturnTimerHandle;
};
