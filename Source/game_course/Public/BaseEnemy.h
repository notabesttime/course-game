// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "BaseEnemy.generated.h"

UCLASS(Abstract)
class GAME_COURSE_API ABaseEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	ABaseEnemy();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UHealthBarWidget> HealthBarWidgetClass;

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Highlight")
	void StartHighlight();
	virtual void StartHighlight_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Highlight")
	void StopHighlight();
	virtual void StopHighlight_Implementation();

protected:
	// Auto-created at startup. Assign a material here in Class Defaults to override
	// (required for packaged builds since shader compilation is editor-only)
	UPROPERTY(EditDefaultsOnly, Category = "Highlight")
	UMaterialInterface* HighlightMaterial = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	class UWidgetComponent* HealthBarWidgetComponent;

private:
	void CreateHighlightMaterial();

	UFUNCTION()
	void OnHealthChanged(float NewValue, float OldValue, float MaxValue);
};
