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

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* SphereComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UProjectileMovementComponent* ProjectileMovement;

	// Damage dealt to the player on hit
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float DamageAmount = 10.f;

	// Called on impact — implement in Blueprint for VFX/sound
	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile")
	void OnProjectileImpact(FVector ImpactLocation);

private:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	bool bHasImpacted = false;
};
