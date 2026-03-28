// Fill out your copyright notice in the Description page of Project Settings.


#include "GameHUD.h"
#include "HealthBarWidget.h"
#include "GameTimerWidget.h"
#include "BaseCharacter.h"
#include "HealthComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanelSlot.h"

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

	if (TimerWidgetClass)
	{
		TimerWidget = CreateWidget<UGameTimerWidget>(GetWorld(), TimerWidgetClass);
		if (TimerWidget)
		{
			TimerWidget->AddToViewport(1);

			// Anchor top-center
			if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(TimerWidget->Slot))
			{
				Slot->SetAnchors(FAnchors(0.5f, 0.f, 0.5f, 0.f));
				Slot->SetAlignment(FVector2D(0.5f, 0.f));
				Slot->SetPosition(FVector2D(0.f, 20.f));
				Slot->SetAutoSize(true);
			}
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
