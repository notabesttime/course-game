// Fill out your copyright notice in the Description page of Project Settings.

#include "CourseGameMode.h"
#include "SpawnerManager.h"
#include "GameHUD.h"
#include "GameTimerWidget.h"
#include "DeathMenuWidget.h"
#include "HighScoreSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void ACourseGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (SpawnerManagerClass)
	{
		SpawnerManager = GetWorld()->SpawnActor<ASpawnerManager>(
			SpawnerManagerClass, FVector::ZeroVector, FRotator::ZeroRotator);
	}

	if (LevelMusic.Num() > 0)
	{
		int32 Index = FMath::RandRange(0, LevelMusic.Num() - 1);
		if (LevelMusic[Index])
		{
			UGameplayStatics::PlaySound2D(GetWorld(), LevelMusic[Index]);
		}
	}
}

void ACourseGameMode::OnPlayerDied()
{
	if (bGameOver) return;
	bGameOver = true;

	// Stop spawner manager
	if (SpawnerManager)
	{
		SpawnerManager->GetWorldTimerManager().ClearAllTimersForObject(SpawnerManager);
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	// Stop timer and get survival time
	float SurvivalTime = 0.f;
	if (AGameHUD* HUD = Cast<AGameHUD>(PC->GetHUD()))
	{
		if (UGameTimerWidget* TimerWidget = HUD->GetTimerWidget())
		{
			TimerWidget->StopTimer();
			SurvivalTime = TimerWidget->GetElapsedTime();
		}
	}

	TArray<float> TopScores = AddAndSaveScore(SurvivalTime);

	// Show death menu
	UDeathMenuWidget* DeathMenu = CreateWidget<UDeathMenuWidget>(PC, UDeathMenuWidget::StaticClass());
	if (DeathMenu)
	{
		DeathMenu->Show(SurvivalTime, TopScores);
	}
}

void ACourseGameMode::SaveScoreOnly()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	float SurvivalTime = 0.f;
	if (AGameHUD* HUD = Cast<AGameHUD>(PC->GetHUD()))
	{
		if (UGameTimerWidget* TimerWidget = HUD->GetTimerWidget())
		{
			SurvivalTime = TimerWidget->GetElapsedTime();
		}
	}

	AddAndSaveScore(SurvivalTime);
}

TArray<float> ACourseGameMode::AddAndSaveScore(float SurvivalTime)
{
	UHighScoreSaveGame* Save = Cast<UHighScoreSaveGame>(
		UGameplayStatics::LoadGameFromSlot(TEXT("HighScores"), 0));
	if (!Save)
	{
		Save = Cast<UHighScoreSaveGame>(
			UGameplayStatics::CreateSaveGameObject(UHighScoreSaveGame::StaticClass()));
	}

	Save->TopScores.Add(SurvivalTime);
	Save->TopScores.Sort([](const float A, const float B) { return A > B; });
	if (Save->TopScores.Num() > 5)
	{
		Save->TopScores.SetNum(5);
	}
	UGameplayStatics::SaveGameToSlot(Save, TEXT("HighScores"), 0);
	return Save->TopScores;
}
