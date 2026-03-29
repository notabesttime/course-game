// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemySpawner.h"
#include "MinionWarrior.h"
#include "MinionMage.h"
#include "MinionBrute.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

int32 AEnemySpawner::TotalSpawnCount  = 0;
int32 AEnemySpawner::LiveWarriorCount = 0;
int32 AEnemySpawner::LiveMageCount    = 0;
int32 AEnemySpawner::LiveBruteCount   = 0;

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::Disabled;
}

void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AddActorLocalRotation(FRotator(0.f, SpinSpeed * DeltaTime, 0.f));
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	AddActorLocalRotation(FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f));

	if (AppearEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(AppearEffect, GetRootComponent());
	}

	if (AppearSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AppearSound, GetActorLocation(), 0.25f);
	}

	GetWorldTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&AEnemySpawner::TrySpawn,
		SpawnInterval,
		true
	);
}

void AEnemySpawner::TrySpawn()
{
	const int32 Warriors = LiveWarriorCount;
	const int32 Mages    = LiveMageCount;
	const int32 Brutes   = LiveBruteCount;

	// Spawn transform: offset slightly so multiple enemies don't stack on the exact same point
	FVector SpawnOffset(FMath::FRandRange(-80.f, 80.f), FMath::FRandRange(-80.f, 80.f), 0.f);
	FTransform SpawnTransform(FRotator(0.f, GetActorRotation().Yaw, 0.f), GetActorLocation() + SpawnOffset);

	bool bSpawned = false;

	// Every 10th spawn is a brute instead of a warrior
	const bool bSpawnBrute = (TotalSpawnCount % 10 == 9) && BruteClass && Brutes < MaxBrutes;

	if (bSpawnBrute)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		GetWorld()->SpawnActor<AMinionBrute>(BruteClass, SpawnTransform, Params);
		bSpawned = true;
		TotalSpawnCount++;
		LiveBruteCount++;
	}
	else if (WarriorClass && Warriors < MaxWarriors)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		GetWorld()->SpawnActor<AMinionWarrior>(WarriorClass, SpawnTransform, Params);
		bSpawned = true;
		TotalSpawnCount++;
		LiveWarriorCount++;
	}

	// Try to spawn a mage if under the limit and ratio allows (1 mage per 2 warriors)
	if (MageClass && Mages < MaxMages && Mages * 2 < Warriors)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		GetWorld()->SpawnActor<AMinionMage>(MageClass, SpawnTransform, Params);
		bSpawned = true;
		LiveMageCount++;
	}

	if (bSpawned && SpawnSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SpawnSound, GetActorLocation(), 0.25f);
	}
}
