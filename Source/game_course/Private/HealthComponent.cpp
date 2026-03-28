// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "AbilitySystemComponent.h"
#include "BaseAttributeSet.h"
#include "AbilitySystemInterface.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UHealthComponent::BindToASC(UAbilitySystemComponent* InASC)
{
	ASC = InASC;
	if (!ASC)
	{
		return;
	}

	AttributeSet = ASC->GetSet<UBaseAttributeSet>();
	if (AttributeSet)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddUObject(this, &UHealthComponent::HandleHealthChange);
	}
}

float UHealthComponent::GetHealth() const
{
	return AttributeSet ? AttributeSet->GetHealth() : 0.0f;
}

float UHealthComponent::GetMaxHealth() const
{
	return AttributeSet ? AttributeSet->GetMaxHealth() : 0.0f;
}

void UHealthComponent::HandleHealthChange(const FOnAttributeChangeData& Data)
{
	OnHealthChanged.Broadcast(Data.NewValue, Data.OldValue, GetMaxHealth());
}
