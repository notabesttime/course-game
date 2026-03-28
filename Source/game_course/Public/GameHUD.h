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
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UHealthBarWidget> HealthBarClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UGameTimerWidget> TimerWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class USpawnerIndicatorWidget> SpawnerIndicatorClass;

	// How far above the spawner the icon floats when on screen
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	float IndicatorHeightOffset = 150.f;

	// Horizontal screen-space offset applied to the icon position
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	float IndicatorHorizontalOffset = 0.f;

	// Padding from screen edges when clamped
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	float IndicatorEdgeMargin = 10.f;


public:
	UGameTimerWidget* GetTimerWidget() const { return TimerWidget; }

private:
	void BindPlayerHealth();
	void UpdateSpawnerIndicators();
	FVector2D ClampToScreenEdge(FVector2D Dir, FVector2D Center, float EffectiveMargin) const;

	UFUNCTION()
	void OnSpawnerDestroyed(AActor* DestroyedActor);

	UPROPERTY()
	class UHealthBarWidget* HealthBarWidget;

	UPROPERTY()
	class UGameTimerWidget* TimerWidget;

	UPROPERTY()
	TMap<class AEnemySpawner*, class USpawnerIndicatorWidget*> SpawnerIndicators;
};
