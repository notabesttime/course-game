// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerAbilityProjectile.h"
#include "BaseEnemy.h"
#include "BaseAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/OverlapResult.h"

APlayerAbilityProjectile::APlayerAbilityProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetSphereRadius(20.0f);
	SphereComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RootComponent = SphereComponent;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 1500.0f;
	ProjectileMovement->MaxSpeed = 1500.0f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
}

void APlayerAbilityProjectile::BeginPlay()
{
	Super::BeginPlay();
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &APlayerAbilityProjectile::OnOverlapBegin);
}

void APlayerAbilityProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Ignore owner (the player) and prevent double-triggering
	if (!OtherActor || OtherActor == GetOwner() || bHasImpacted)
	{
		return;
	}

	// Only detonate on enemy contact
	if (!OtherActor->IsA<ABaseEnemy>())
	{
		return;
	}

	bHasImpacted = true;
	FVector ImpactLocation = GetActorLocation();

	ApplyAoEDamage(ImpactLocation);
	OnProjectileImpact(ImpactLocation);

	Destroy();
}

void APlayerAbilityProjectile::ApplyAoEDamage(FVector Origin)
{
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());

	GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		Origin,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(ExplosionRadius),
		Params
	);

	TSet<AActor*> DamagedActors;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor || !HitActor->IsA<ABaseEnemy>() || DamagedActors.Contains(HitActor))
		{
			continue;
		}

		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(HitActor))
		{
			UAbilitySystemComponent* TargetASC = ASI->GetAbilitySystemComponent();
			if (!TargetASC) continue;

			const UBaseAttributeSet* AttrSet = TargetASC->GetSet<UBaseAttributeSet>();
			if (!AttrSet) continue;

			float NewHealth = FMath::Max(0.f, AttrSet->GetHealth() - DamageAmount);
			TargetASC->SetNumericAttributeBase(UBaseAttributeSet::GetHealthAttribute(), NewHealth);
			DamagedActors.Add(HitActor);
		}
	}
}
