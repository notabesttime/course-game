// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "HealthBarWidget.generated.h"

UCLASS()
class GAME_COURSE_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHealthComponent(class UHealthComponent* InHealthComponent);

protected:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;

	UFUNCTION()
	void UpdateHealthBar(float NewValue, float OldValue, float MaxValue);

private:
	UPROPERTY()
	class UHealthComponent* HealthComponent;
};
