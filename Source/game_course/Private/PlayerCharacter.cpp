// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
#include "PlayerMeleeAbility.h"
#include "PlayerRangedAbility.h"
#include "BaseEnemy.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "Engine/EngineTypes.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		if (MeleeAbilityClass)
		{
			MeleeAbilityHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(MeleeAbilityClass, 1));
		}
		if (RangedAbilityClass)
		{
			RangedAbilityHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(RangedAbilityClass, 1));
		}
	}
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateHoveredEnemy();
}

void APlayerCharacter::UpdateHoveredEnemy()
{
	ABaseEnemy* NewHovered = nullptr;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		FHitResult HitResult;
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
		if (PC->GetHitResultUnderCursorForObjects(ObjectTypes, false, HitResult))
		{
			NewHovered = Cast<ABaseEnemy>(HitResult.GetActor());
		}
	}

	if (NewHovered != HoveredEnemy)
	{
		if (HoveredEnemy)
		{
			HoveredEnemy->StopHighlight();
		}
		HoveredEnemy = NewHovered;
		if (HoveredEnemy)
		{
			HoveredEnemy->StartHighlight();
		}
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MeleeInputAction)
		{
			EIC->BindAction(MeleeInputAction, ETriggerEvent::Started, this, &APlayerCharacter::ActivateMelee);
		}
		if (RangedInputAction)
		{
			EIC->BindAction(RangedInputAction, ETriggerEvent::Started, this, &APlayerCharacter::ActivateRanged);
		}
	}
}

void APlayerCharacter::ActivateMelee()
{
	if (HoveredEnemy && AbilitySystemComponent)
	{
		AbilitySystemComponent->TryActivateAbility(MeleeAbilityHandle);
	}
}

void APlayerCharacter::ActivateRanged()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->TryActivateAbility(RangedAbilityHandle);
	}
}
