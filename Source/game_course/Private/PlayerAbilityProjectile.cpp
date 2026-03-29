// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerAbilityProjectile.h"
#include "BaseEnemy.h"
#include "EnemySpawner.h"
#include "CombatAttributeUtils.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/OverlapResult.h"

DEFINE_LOG_CATEGORY_STATIC(LogPlayerProjectilePool, Log, All);

namespace
{
	TMap<UClass*, TArray<TWeakObjectPtr<APlayerAbilityProjectile>>> GPlayerProjectilePool;
}

void APlayerAbilityProjectile::GetPoolStats(int32& OutTotal, int32& OutActive, int32& OutInactive)
{
	OutTotal = 0;
	OutActive = 0;
	OutInactive = 0;

	for (TPair<UClass*, TArray<TWeakObjectPtr<APlayerAbilityProjectile>>>& Pair : GPlayerProjectilePool)
	{
		TArray<TWeakObjectPtr<APlayerAbilityProjectile>>& Pool = Pair.Value;
		for (int32 Index = Pool.Num() - 1; Index >= 0; --Index)
		{
			APlayerAbilityProjectile* Projectile = Pool[Index].Get();
			if (!IsValid(Projectile))
			{
				Pool.RemoveAtSwap(Index);
				continue;
			}

			++OutTotal;
			if (Projectile->bIsPooledActive)
			{
				++OutActive;
			}
			else
			{
				++OutInactive;
			}
		}
	}
}

void APlayerAbilityProjectile::LogPoolStats()
{
	int32 Total = 0;
	int32 Active = 0;
	int32 Inactive = 0;
	GetPoolStats(Total, Active, Inactive);

	UE_LOG(LogPlayerProjectilePool, Log, TEXT("PlayerProjectilePool: total=%d active=%d inactive=%d"), Total, Active, Inactive);
}

APlayerAbilityProjectile* APlayerAbilityProjectile::SpawnOrReuse(UWorld* World, TSubclassOf<APlayerAbilityProjectile> ProjectileClass,
	const FVector& Location, const FRotator& Rotation, AActor* Owner, APawn* InstigatorPawn)
{
	if (!World || !*ProjectileClass)
	{
		return nullptr;
	}

	UClass* ClassKey = ProjectileClass.Get();
	TArray<TWeakObjectPtr<APlayerAbilityProjectile>>& Pool = GPlayerProjectilePool.FindOrAdd(ClassKey);

	for (const TWeakObjectPtr<APlayerAbilityProjectile>& Entry : Pool)
	{
		APlayerAbilityProjectile* Projectile = Entry.Get();
		if (IsValid(Projectile) && !Projectile->bIsPooledActive)
		{
			Projectile->ActivateFromPool(Location, Rotation, Owner, InstigatorPawn);
			return Projectile;
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Owner;
	SpawnParams.Instigator = InstigatorPawn;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	APlayerAbilityProjectile* NewProjectile = World->SpawnActor<APlayerAbilityProjectile>(ProjectileClass, Location, Rotation, SpawnParams);
	if (!NewProjectile)
	{
		return nullptr;
	}

	Pool.Add(NewProjectile);
	NewProjectile->ActivateFromPool(Location, Rotation, Owner, InstigatorPawn);
	return NewProjectile;
}

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

	InitialLifeSpan = 0.0f;
}

void APlayerAbilityProjectile::BeginPlay()
{
	Super::BeginPlay();
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &APlayerAbilityProjectile::OnOverlapBegin);
}

void APlayerAbilityProjectile::ActivateFromPool(const FVector& Location, const FRotator& Rotation, AActor* InOwner, APawn* InstigatorPawn)
{
	bIsPooledActive = true;
	bHasImpacted = false;

	SetOwner(InOwner);
	SetInstigator(InstigatorPawn);
	SetActorLocationAndRotation(Location, Rotation);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	ProjectileMovement->Velocity = GetActorForwardVector() * ProjectileMovement->InitialSpeed;
	ProjectileMovement->Activate(true);

	GetWorldTimerManager().ClearTimer(ReturnTimerHandle);
	GetWorldTimerManager().SetTimer(ReturnTimerHandle, this, &APlayerAbilityProjectile::ReturnToPool, PooledLifetime, false);
}

void APlayerAbilityProjectile::ReturnToPool()
{
	bIsPooledActive = false;
	bHasImpacted = false;
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->Deactivate();
	GetWorldTimerManager().ClearTimer(ReturnTimerHandle);
}

void APlayerAbilityProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Ignore owner (the player) and prevent double-triggering
	if (!bIsPooledActive || !OtherActor || OtherActor == GetOwner() || bHasImpacted)
	{
		return;
	}

	// Only detonate on enemy or spawner contact
	if (!OtherActor->IsA<ABaseEnemy>() && !OtherActor->IsA<AEnemySpawner>())
	{
		return;
	}

	bHasImpacted = true;
	FVector ImpactLocation = GetActorLocation();

	ApplyAoEDamage(ImpactLocation);
	OnProjectileImpact(ImpactLocation);

	ReturnToPool();
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
		if (!HitActor || (!HitActor->IsA<ABaseEnemy>() && !HitActor->IsA<AEnemySpawner>()) || DamagedActors.Contains(HitActor))
		{
			continue;
		}

		if (CombatAttributeUtils::ApplyHealthDelta(HitActor, -DamageAmount))
		{
			DamagedActors.Add(HitActor);
		}
	}
}
