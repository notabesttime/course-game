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
#include "HealthComponent.h"
#include "ManaComponent.h"
#include "BaseAttributeSet.h"
#include "Kismet/GameplayStatics.h"

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

	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &APlayerCharacter::OnHealthChanged);
	}
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateHoveredEnemy();

	// Mana regeneration: 20 per second
	if (AbilitySystemComponent && AttributeSet)
	{
		const float CurrentMana = AttributeSet->GetMana();
		const float MaxMana     = AttributeSet->GetMaxMana();
		if (CurrentMana < MaxMana)
		{
			AttributeSet->SetMana(FMath::Min(CurrentMana + 20.f * DeltaTime, MaxMana));
		}
	}
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
		LastAttackTime = GetWorld()->GetTimeSeconds();
		if (MeleeAttackSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, MeleeAttackSound, GetActorLocation());
		}
	}
}

void APlayerCharacter::ActivateRanged()
{
	if (!AbilitySystemComponent) return;

	// Check and consume 10 mana
	if (AttributeSet)
	{
		if (AttributeSet->GetMana() < 10.f) return;
		AttributeSet->SetMana(AttributeSet->GetMana() - 10.f);
	}

	AbilitySystemComponent->TryActivateAbility(RangedAbilityHandle);
	LastAttackTime = GetWorld()->GetTimeSeconds();
	if (RangedAttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, RangedAttackSound, GetActorLocation());
	}
}

void APlayerCharacter::OnHealthChanged(float NewValue, float OldValue, float MaxValue)
{
	if (NewValue <= 0.0f && DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}
}
