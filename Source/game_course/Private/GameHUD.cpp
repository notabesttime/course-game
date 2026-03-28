// Fill out your copyright notice in the Description page of Project Settings.


#include "GameHUD.h"
#include "HealthBarWidget.h"
#include "BaseCharacter.h"
#include "HealthComponent.h"
#include "Blueprint/UserWidget.h"

void AGameHUD::BeginPlay()
{
	Super::BeginPlay();

	if (HealthBarClass)
	{
		HealthBarWidget = CreateWidget<UHealthBarWidget>(GetWorld(), HealthBarClass);
		if (HealthBarWidget)
		{
			HealthBarWidget->AddToViewport();
		}
	}

	// Defer one tick so the player character's BeginPlay (and BindToASC) completes first
	GetWorldTimerManager().SetTimerForNextTick(this, &AGameHUD::BindPlayerHealth);
}

void AGameHUD::BindPlayerHealth()
{
	if (!HealthBarWidget)
	{
		return;
	}

	APawn* PlayerPawn = GetOwningPawn();
	if (PlayerPawn)
	{
		if (UHealthComponent* HealthComp = PlayerPawn->FindComponentByClass<UHealthComponent>())
		{
			HealthBarWidget->SetHealthComponent(HealthComp);
		}
	}
}
