// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "HighScoreSaveGame.generated.h"

UCLASS()
class GAME_COURSE_API UHighScoreSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// Top 5 survival times in seconds, sorted descending
	UPROPERTY()
	TArray<float> TopScores;
};
