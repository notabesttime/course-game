// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthBarWidget.h"
#include "Components/ProgressBar.h"
#include "HealthComponent.h"

void UHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (HealthComponent)
	{
		UpdateHealthBar(HealthComponent->GetHealth(), 0.0f, HealthComponent->GetMaxHealth());
	}
}

void UHealthBarWidget::SetHealthComponent(UHealthComponent* InHealthComponent)
{
	HealthComponent = InHealthComponent;
	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &UHealthBarWidget::UpdateHealthBar);
		
		// Initial update
		UpdateHealthBar(HealthComponent->GetHealth(), 0.0f, HealthComponent->GetMaxHealth());
	}
}

void UHealthBarWidget::UpdateHealthBar(float NewValue, float OldValue, float MaxValue)
{
	if (HealthBar && MaxValue > 0.0f)
	{
		HealthBar->SetPercent(NewValue / MaxValue);
	}

	if (HealthText)
	{
		int32 Percent = MaxValue > 0.0f ? FMath::RoundToInt((NewValue / MaxValue) * 100.0f) : 0;
		FText HealthString = FText::Format(NSLOCTEXT("UI", "HealthPercentFormat", "{0}%"), FText::AsNumber(Percent));
		HealthText->SetText(HealthString);
	}
}
