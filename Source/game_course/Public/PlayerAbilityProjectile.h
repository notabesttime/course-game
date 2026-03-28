// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerAbilityProjectile.generated.h"

UCLASS(Blueprintable)
class GAME_COURSE_API APlayerAbilityProjectile : public AActor
{
	GENERATED_BODY()

public:
	APlayerAbilityProjectile();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* SphereComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UProjectileMovementComponent* ProjectileMovement;

	// Radius around impact point to damage enemies
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float ExplosionRadius = 200.0f;

	// Damage applied to each enemy in the explosion radius
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float DamageAmount = 50.0f;

	// Called on impact — implement in Blueprint for visuals (explosion VFX, sound)
	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile")
	void OnProjectileImpact(FVector ImpactLocation);

private:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void ApplyAoEDamage(FVector Origin);

	bool bHasImpacted = false;
};
