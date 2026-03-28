// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GameHUD.generated.h"

UCLASS()
class GAME_COURSE_API AGameHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UHealthBarWidget> HealthBarClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UGameTimerWidget> TimerWidgetClass;

public:
	UGameTimerWidget* GetTimerWidget() const { return TimerWidget; }

private:
	void BindPlayerHealth();

	UPROPERTY()
	class UHealthBarWidget* HealthBarWidget;

	UPROPERTY()
	class UGameTimerWidget* TimerWidget;
};
