// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SpawnerIndicatorWidget.generated.h"

UCLASS()
class GAME_COURSE_API USpawnerIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// Add an Image named "Icon" in the Blueprint subclass and assign your spawner icon texture
	UPROPERTY(meta = (BindWidget))
	class UImage* Icon;
};
