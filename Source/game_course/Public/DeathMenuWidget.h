// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DeathMenuWidget.generated.h"

UCLASS()
class GAME_COURSE_API UDeathMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Show(float SurvivalTime, const TArray<float>& TopScores);

protected:
	virtual void NativeOnInitialized() override;

private:
	UFUNCTION()
	void OnRestartClicked();

	UFUNCTION()
	void OnExitClicked();

	UFUNCTION()
	void OnClearScoresClicked();

	class UVerticalBox* RootBox = nullptr;
	class UTextBlock* ScoreLabel = nullptr;
	class UTextBlock* Top5Label = nullptr;
	class UTextBlock* HighScoreList = nullptr;
	TArray<float> CachedTopScores;
};
