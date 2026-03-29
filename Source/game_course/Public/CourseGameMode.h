// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CourseGameMode.generated.h"

UCLASS()
class GAME_COURSE_API ACourseGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	void OnPlayerDied();
	void SaveScoreOnly();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	TSubclassOf<class ASpawnerManager> SpawnerManagerClass;

	// Add sound cues here — one will be picked at random on level start
	UPROPERTY(EditDefaultsOnly, Category = "Music")
	TArray<USoundBase*> LevelMusic;

private:
	UPROPERTY()
	class ASpawnerManager* SpawnerManager = nullptr;

	bool bGameOver = false;

	TArray<float> AddAndSaveScore(float SurvivalTime);
};
