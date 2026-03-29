// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameTimerWidget.generated.h"

UCLASS()
class GAME_COURSE_API UGameTimerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Called by SpawnerManager on each new wave
	void StartFlash(int32 WaveNumber);

	void StopTimer() { bStopped = true; }
	float GetElapsedTime() const { return ElapsedTime; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// Add a TextBlock named "TimerText" in the Blueprint subclass
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TimerText;

	// Total duration of the timer pulse animation
	UPROPERTY(EditDefaultsOnly, Category = "Timer|Flash")
	float FlashDuration = 2.f;

	// How many times the timer pulses during the animation
	UPROPERTY(EditDefaultsOnly, Category = "Timer|Flash")
	float PulseCount = 3.f;

	// Peak scale during each pulse (1.0 = normal size)
	UPROPERTY(EditDefaultsOnly, Category = "Timer|Flash")
	float MaxPulseScale = 1.6f;

private:
	float ElapsedTime = 0.f;
	float LastDisplayedTime = -1.f;
	float LastDisplayedShieldTime = -1.f;
	bool bFlashing = false;
	bool bStopped = false;
	float FlashTimeRemaining = 0.f;

	UPROPERTY()
	class APlayerCharacter* CachedPlayer = nullptr;

	UPROPERTY()
	class UWaveAnnouncementWidget* ActiveAnnouncement = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* ShieldLabel = nullptr;
};
