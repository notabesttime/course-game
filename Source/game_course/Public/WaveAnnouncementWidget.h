// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WaveAnnouncementWidget.generated.h"

UCLASS()
class GAME_COURSE_API UWaveAnnouncementWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Show(int32 WaveNumber);

	// Called every frame by GameTimerWidget
	void TickAnimation(float DeltaTime);

	bool IsAnimating() const { return bAnimating; }

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(EditDefaultsOnly, Category = "Wave")
	float AnimDuration = 2.f;

	UPROPERTY(EditDefaultsOnly, Category = "Wave")
	float StartScale = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Wave")
	float EndScale = 4.f;

	UPROPERTY(EditDefaultsOnly, Category = "Wave")
	int32 FontSize = 96;

private:
	class UTextBlock* WaveLabel = nullptr;
	float TimeRemaining = 0.f;
	bool bAnimating = false;
};
