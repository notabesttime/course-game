// Fill out your copyright notice in the Description page of Project Settings.

#include "MinionMageProjectile.h"
#include "PlayerCharacter.h"
#include "CombatAttributeUtils.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogMageProjectilePool, Log, All);

namespace
{
	TMap<UClass*, TArray<TWeakObjectPtr<AMinionMageProjectile>>> GMageProjectilePool;
}

void AMinionMageProjectile::GetPoolStats(int32& OutTotal, int32& OutActive, int32& OutInactive)
{
	OutTotal = 0;
	OutActive = 0;
	OutInactive = 0;

	for (TPair<UClass*, TArray<TWeakObjectPtr<AMinionMageProjectile>>>& Pair : GMageProjectilePool)
	{
		TArray<TWeakObjectPtr<AMinionMageProjectile>>& Pool = Pair.Value;
		for (int32 Index = Pool.Num() - 1; Index >= 0; --Index)
		{
			AMinionMageProjectile* Projectile = Pool[Index].Get();
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

void AMinionMageProjectile::LogPoolStats()
{
	int32 Total = 0;
	int32 Active = 0;
	int32 Inactive = 0;
	GetPoolStats(Total, Active, Inactive);

	UE_LOG(LogMageProjectilePool, Log, TEXT("MageProjectilePool: total=%d active=%d inactive=%d"), Total, Active, Inactive);
}

AMinionMageProjectile* AMinionMageProjectile::SpawnOrReuse(UWorld* World, TSubclassOf<AMinionMageProjectile> ProjectileClass,
	const FVector& Location, const FRotator& Rotation, AActor* Owner, APawn* InstigatorPawn)
{
	if (!World || !*ProjectileClass)
	{
		return nullptr;
	}

	UClass* ClassKey = ProjectileClass.Get();
	TArray<TWeakObjectPtr<AMinionMageProjectile>>& Pool = GMageProjectilePool.FindOrAdd(ClassKey);

	for (const TWeakObjectPtr<AMinionMageProjectile>& Entry : Pool)
	{
		AMinionMageProjectile* Projectile = Entry.Get();
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
	AMinionMageProjectile* NewProjectile = World->SpawnActor<AMinionMageProjectile>(ProjectileClass, Location, Rotation, SpawnParams);
	if (!NewProjectile)
	{
		return nullptr;
	}

	Pool.Add(NewProjectile);
	NewProjectile->ActivateFromPool(Location, Rotation, Owner, InstigatorPawn);
	return NewProjectile;
}

AMinionMageProjectile::AMinionMageProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetSphereRadius(20.f);
	SphereComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RootComponent = SphereComponent;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 1200.f;
	ProjectileMovement->MaxSpeed = 1200.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bRotationFollowsVelocity = true;

	InitialLifeSpan = 0.f;
}

void AMinionMageProjectile::BeginPlay()
{
	Super::BeginPlay();
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AMinionMageProjectile::OnOverlapBegin);
}

void AMinionMageProjectile::ActivateFromPool(const FVector& Location, const FRotator& Rotation, AActor* InOwner, APawn* InstigatorPawn)
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
	GetWorldTimerManager().SetTimer(ReturnTimerHandle, this, &AMinionMageProjectile::ReturnToPool, PooledLifetime, false);
}

void AMinionMageProjectile::ReturnToPool()
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

void AMinionMageProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bIsPooledActive || bHasImpacted || !OtherActor || OtherActor == GetOwner())
	{
		return;
	}

	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	bHasImpacted = true;

	CombatAttributeUtils::ApplyHealthDelta(Player, -DamageAmount);

	OnProjectileImpact(GetActorLocation());
	ReturnToPool();
}
