// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "ManaComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnManaChanged, float, NewValue, float, OldValue, float, MaxValue);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAME_COURSE_API UManaComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UManaComponent();

	void BindToASC(UAbilitySystemComponent* InASC);

	UPROPERTY(BlueprintAssignable, Category = "Mana")
	FOnManaChanged OnManaChanged;

	UFUNCTION(BlueprintCallable, Category = "Mana")
	float GetMana() const;

	UFUNCTION(BlueprintCallable, Category = "Mana")
	float GetMaxMana() const;

private:
	void HandleManaChange(const FOnAttributeChangeData& Data);

	UPROPERTY()
	class UAbilitySystemComponent* ASC;

	UPROPERTY()
	const class UBaseAttributeSet* AttributeSet;
};
