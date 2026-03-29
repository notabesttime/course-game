// Fill out your copyright notice in the Description page of Project Settings.

#include "ManaComponent.h"
#include "AbilitySystemComponent.h"
#include "BaseAttributeSet.h"

UManaComponent::UManaComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UManaComponent::BindToASC(UAbilitySystemComponent* InASC)
{
	ASC = InASC;
	if (!ASC) return;

	AttributeSet = ASC->GetSet<UBaseAttributeSet>();
	if (AttributeSet)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetManaAttribute()).AddUObject(this, &UManaComponent::HandleManaChange);
	}
}

float UManaComponent::GetMana() const
{
	return AttributeSet ? AttributeSet->GetMana() : 0.f;
}

float UManaComponent::GetMaxMana() const
{
	return AttributeSet ? AttributeSet->GetMaxMana() : 0.f;
}

void UManaComponent::HandleManaChange(const FOnAttributeChangeData& Data)
{
	OnManaChanged.Broadcast(Data.NewValue, Data.OldValue, GetMaxMana());
}
