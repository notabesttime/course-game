// Fill out your copyright notice in the Description page of Project Settings.

#include "HealingOrb.h"
#include "PlayerCharacter.h"
#include "CombatAttributeUtils.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogHealingOrbPool, Log, All);

namespace
{
	TMap<UClass*, TArray<TWeakObjectPtr<AHealingOrb>>> GHealingOrbPool;
}

void AHealingOrb::GetPoolStats(int32& OutTotal, int32& OutActive, int32& OutInactive)
{
	OutTotal = 0;
	OutActive = 0;
	OutInactive = 0;

	for (TPair<UClass*, TArray<TWeakObjectPtr<AHealingOrb>>>& Pair : GHealingOrbPool)
	{
		TArray<TWeakObjectPtr<AHealingOrb>>& Pool = Pair.Value;
		for (int32 Index = Pool.Num() - 1; Index >= 0; --Index)
		{
			AHealingOrb* Orb = Pool[Index].Get();
			if (!IsValid(Orb))
			{
				Pool.RemoveAtSwap(Index);
				continue;
			}

			++OutTotal;
			if (Orb->bIsPooledActive)
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

void AHealingOrb::LogPoolStats()
{
	int32 Total = 0;
	int32 Active = 0;
	int32 Inactive = 0;
	GetPoolStats(Total, Active, Inactive);

	UE_LOG(LogHealingOrbPool, Log, TEXT("HealingOrbPool: total=%d active=%d inactive=%d"), Total, Active, Inactive);
}

AHealingOrb* AHealingOrb::SpawnOrReuse(UWorld* World, TSubclassOf<AHealingOrb> OrbClass,
	const FVector& Location, const FRotator& Rotation, AActor* Owner)
{
	if (!World || !*OrbClass)
	{
		return nullptr;
	}

	UClass* ClassKey = OrbClass.Get();
	TArray<TWeakObjectPtr<AHealingOrb>>& Pool = GHealingOrbPool.FindOrAdd(ClassKey);

	for (const TWeakObjectPtr<AHealingOrb>& Entry : Pool)
	{
		AHealingOrb* Orb = Entry.Get();
		if (IsValid(Orb) && !Orb->bIsPooledActive)
		{
			Orb->ActivateFromPool(Location, Rotation, Owner);
			return Orb;
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Owner;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHealingOrb* NewOrb = World->SpawnActor<AHealingOrb>(OrbClass, Location, Rotation, SpawnParams);
	if (!NewOrb)
	{
		return nullptr;
	}

	Pool.Add(NewOrb);
	NewOrb->ActivateFromPool(Location, Rotation, Owner);
	return NewOrb;
}

AHealingOrb::AHealingOrb()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetSphereRadius(40.f);
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RootComponent = CollisionSphere;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AHealingOrb::ActivateFromPool(const FVector& Location, const FRotator& Rotation, AActor* InOwner)
{
	bIsPooledActive = true;
	SetOwner(InOwner);
	SetActorLocationAndRotation(Location, Rotation);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	if (!CachedPlayer || !IsValid(CachedPlayer))
	{
		CachedPlayer = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	}

	GetWorldTimerManager().ClearTimer(AutoReturnTimer);
	GetWorldTimerManager().SetTimer(AutoReturnTimer, this, &AHealingOrb::ReturnToPool, MaxActiveLifetime, false);
}

void AHealingOrb::ReturnToPool()
{
	bIsPooledActive = false;
	SetActorTickEnabled(false);
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetWorldTimerManager().ClearTimer(AutoReturnTimer);
}

void AHealingOrb::BeginPlay()
{
	Super::BeginPlay();

	CachedPlayer = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

void AHealingOrb::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsPooledActive)
	{
		return;
	}

	if (!CachedPlayer || !IsValid(CachedPlayer))
	{
		return;
	}

	// Stay still for AttackPauseTime after the player's last attack
	float TimeSinceAttack = GetWorld()->GetTimeSeconds() - CachedPlayer->GetLastAttackTime();
	if (TimeSinceAttack < AttackPauseTime)
	{
		return;
	}

	FVector ToPlayer = CachedPlayer->GetActorLocation() - GetActorLocation();
	float DistSq = ToPlayer.SizeSquared();

	if (DistSq <= PickupRadius * PickupRadius)
	{
		CombatAttributeUtils::ApplyHealthDelta(CachedPlayer, HealAmount);
		CombatAttributeUtils::ApplyManaDelta(CachedPlayer, 10.f);
		ReturnToPool();
		return;
	}

	// One sqrt for movement direction (unavoidable), reusing DistSq already computed
	AddActorWorldOffset(ToPlayer * (MoveSpeed * DeltaTime / FMath::Sqrt(DistSq)));
}
