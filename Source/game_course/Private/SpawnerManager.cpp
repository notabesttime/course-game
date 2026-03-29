// Fill out your copyright notice in the Description page of Project Settings.

#include "SpawnerManager.h"
#include "EnemySpawner.h"
#include "GameHUD.h"
#include "GameTimerWidget.h"
#include "NavigationSystem.h"
#include "GameFramework/PlayerController.h"

ASpawnerManager::ASpawnerManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASpawnerManager::BeginPlay()
{
	Super::BeginPlay();

	if (SpawnerClass)
	{
		GetWorldTimerManager().SetTimer(
			SpawnTimerHandle,
			this,
			&ASpawnerManager::PlaceWave,
			SpawnInterval,
			true
		);
	}
}

void ASpawnerManager::PlaceWave()
{
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys || !SpawnerClass)
	{
		return;
	}

	TArray<FVector> PlacedLocations;

	for (int32 i = 0; i < NextWaveCount; i++)
	{
		FNavLocation RandomLocation;
		bool bFound = false;

		// Retry up to 10 times to find a spot far enough from already-placed spawners
		for (int32 Attempt = 0; Attempt < 10; Attempt++)
		{
			if (!NavSys->GetRandomReachablePointInRadius(GetActorLocation(), SearchRadius, RandomLocation))
			{
				break;
			}

			bool bTooClose = false;
			for (const FVector& Existing : PlacedLocations)
			{
				if (FVector::Dist2D(RandomLocation.Location, Existing) < MinSpawnerDistance)
				{
					bTooClose = true;
					break;
				}
			}

			if (!bTooClose)
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			// Fallback: use whatever the last random point was (ignoring distance constraint)
			NavSys->GetRandomReachablePointInRadius(GetActorLocation(), SearchRadius, RandomLocation);
			bFound = true;
		}

		if (bFound)
		{
			PlacedLocations.Add(RandomLocation.Location);
			// Queue for drip-spawning instead of spawning all at once
			PendingSpawnLocations.Add(RandomLocation.Location + FVector(0.f, 0.f, 88.f));
		}
	}

	NextWaveCount = FMath::Min(NextWaveCount * 2, 16);
	WaveNumber++;

	// Drip-spawn one spawner every 0.05s to avoid a single-frame hitch
	if (PendingSpawnLocations.Num() > 0)
	{
		GetWorldTimerManager().SetTimer(
			DripTimerHandle, this, &ASpawnerManager::SpawnNextPending, 0.05f, true, 0.0f);
	}

	// After wave 6, increase enemy limits by 1 each wave, capped at 16
	if (WaveNumber > 6)
	{
		SpawnLimitBonus = FMath::Min(SpawnLimitBonus + 1, 16);
	}

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AGameHUD* HUD = Cast<AGameHUD>(PC->GetHUD()))
		{
			if (UGameTimerWidget* TimerWidget = HUD->GetTimerWidget())
			{
				TimerWidget->StartFlash(WaveNumber);
			}
		}
	}
}

void ASpawnerManager::SpawnNextPending()
{
	if (PendingSpawnLocations.Num() == 0)
	{
		GetWorldTimerManager().ClearTimer(DripTimerHandle);
		return;
	}

	FVector SpawnLocation = PendingSpawnLocations[0];
	PendingSpawnLocations.RemoveAt(0);

	AEnemySpawner* Spawner = GetWorld()->SpawnActor<AEnemySpawner>(SpawnerClass, SpawnLocation, FRotator::ZeroRotator);
	if (Spawner && SpawnLimitBonus > 0)
	{
		Spawner->MaxWarriors += SpawnLimitBonus;
		Spawner->MaxMages    += SpawnLimitBonus;
	}

	if (PendingSpawnLocations.Num() == 0)
	{
		GetWorldTimerManager().ClearTimer(DripTimerHandle);
	}
}
