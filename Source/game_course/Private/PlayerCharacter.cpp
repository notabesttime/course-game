// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
#include "PlayerMeleeAbility.h"
#include "PlayerRangedAbility.h"
#include "BaseEnemy.h"
#include "EnemySpawner.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/EngineTypes.h"
#include "HealthComponent.h"
#include "ManaComponent.h"
#include "CameraOcclusionComponent.h"
#include "PlayerShieldComponent.h"
#include "BaseAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "CourseGameMode.h"
#include "GameFramework/WorldSettings.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputCoreTypes.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	CameraOcclusionComponent = CreateDefaultSubobject<UCameraOcclusionComponent>(TEXT("CameraOcclusionComponent"));
	ShieldComponent = CreateDefaultSubobject<UPlayerShieldComponent>(TEXT("ShieldComponent"));

	HoverObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
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

	// Mana drain (time slow) or regen — single pass
	if (AbilitySystemComponent && AttributeSet)
	{
		const float CurrentMana = AttributeSet->GetMana();
		float NewMana = CurrentMana;

		if (bTimeSlowActive)
		{
			NewMana -= 30.f * DeltaTime;
			if (NewMana <= 0.f)
			{
				AttributeSet->SetMana(0.f);
				DeactivateTimeSlow();
			}
			else
			{
				AttributeSet->SetMana(NewMana);
			}
		}
		else
		{
			const float MaxMana = AttributeSet->GetMaxMana();
			if (CurrentMana < MaxMana)
			{
				AttributeSet->SetMana(FMath::Min(CurrentMana + 20.f * DeltaTime, MaxMana));
			}
		}
	}
}

void APlayerCharacter::UpdateHoveredEnemy()
{
	// Throttle to 20Hz — hover detection doesn't need sub-frame precision
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastHoverUpdateTime < 0.05f) return;
	LastHoverUpdateTime = Now;

	AActor* NewHovered = nullptr;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		FHitResult HitResult;
		if (PC->GetHitResultUnderCursorForObjects(HoverObjectTypes, false, HitResult))
		{
			AActor* Hit = HitResult.GetActor();
			if (Hit && (Hit->IsA<ABaseEnemy>() || Hit->IsA<AEnemySpawner>()))
			{
				NewHovered = Hit;
			}
		}
	}

	if (NewHovered != HoveredEnemy)
	{
		// Highlight only works on ABaseEnemy (has mesh); spawners don't highlight
		if (ABaseEnemy* OldEnemy = Cast<ABaseEnemy>(HoveredEnemy))
		{
			OldEnemy->StopHighlight();
		}
		HoveredEnemy = NewHovered;
		if (ABaseEnemy* NewEnemy = Cast<ABaseEnemy>(HoveredEnemy))
		{
			NewEnemy->StartHighlight();
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
		if (MoveInputAction)
		{
			EIC->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		}
		if (RestartInputAction)
		{
			EIC->BindAction(RestartInputAction, ETriggerEvent::Started, this, &APlayerCharacter::QuickRestart);
		}
		if (DebugShieldInputAction)
		{
			EIC->BindAction(DebugShieldInputAction, ETriggerEvent::Started, this, &APlayerCharacter::DebugActivateShield);
		}
		if (TimeSlowInputAction)
		{
			EIC->BindAction(TimeSlowInputAction, ETriggerEvent::Started,   this, &APlayerCharacter::ActivateTimeSlow);
			EIC->BindAction(TimeSlowInputAction, ETriggerEvent::Completed, this, &APlayerCharacter::DeactivateTimeSlow);
		}
	}

	PlayerInputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &APlayerCharacter::DebugDie);
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (Axis.IsNearlyZero()) return;

	// Use camera yaw so WASD aligns with what's on screen regardless of camera angle
	float CameraYaw = GetControlRotation().Yaw;
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		FVector CamLoc;
		FRotator CamRot;
		PC->GetPlayerViewPoint(CamLoc, CamRot);
		CameraYaw = CamRot.Yaw;
	}

	const FRotator YawRotation(0.f, CameraYaw, 0.f);
	const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDir   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDir, Axis.Y);
	AddMovementInput(RightDir,   Axis.X);
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

void APlayerCharacter::DebugDie()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !PC->IsInputKeyDown(EKeys::Tab))
	{
		return;
	}

	if (AttributeSet)
	{
		// Force health to 0, which triggers OnHealthChanged → full death flow
		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->SetNumericAttributeBase(
				UBaseAttributeSet::GetHealthAttribute(), 0.f);
		}
	}
}

void APlayerCharacter::QuickRestart()
{
	if (ACourseGameMode* GM = Cast<ACourseGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GM->SaveScoreOnly();
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetShowMouseCursor(true);
		FInputModeGameAndUI Mode;
		Mode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(Mode);
	}

	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	if (!CurrentLevelName.IsEmpty())
	{
		UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
	}
}

void APlayerCharacter::DebugActivateShield()
{
	if (ShieldComponent)
	{
		ShieldComponent->ActivateShield(60.f);
	}
}

void APlayerCharacter::ActivateTimeSlow()
{
	if (bTimeSlowActive) return;
	if (!AttributeSet || AttributeSet->GetMana() <= 0.f) return;

	bTimeSlowActive = true;

	// Slow the whole world to 20%, counter-dilate the player to stay at 1x real speed
	GetWorld()->GetWorldSettings()->SetTimeDilation(0.2f);
	CustomTimeDilation = 5.0f;
}

void APlayerCharacter::DeactivateTimeSlow()
{
	if (!bTimeSlowActive) return;
	bTimeSlowActive = false;

	GetWorld()->GetWorldSettings()->SetTimeDilation(1.0f);
	CustomTimeDilation = 1.0f;
}

void APlayerCharacter::OnHealthChanged(float NewValue, float OldValue, float MaxValue)
{
	// Shield blocks all damage — restore health back to pre-hit value
	if (ShieldComponent && ShieldComponent->IsShieldActive() && NewValue < OldValue)
	{
		if (AbilitySystemComponent && AttributeSet)
		{
			AbilitySystemComponent->SetNumericAttributeBase(
				UBaseAttributeSet::GetHealthAttribute(), OldValue);
		}
		return;
	}

	if (NewValue <= 0.0f)
	{
		DeactivateTimeSlow();

		if (DeathSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
		}

		if (ACourseGameMode* GM = Cast<ACourseGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
		{
			GM->OnPlayerDied();
		}

		// Disable input and hide player
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			DisableInput(PC);
		}

		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->StopMovementImmediately();
			MoveComp->DisableMovement();
		}

		SetActorHiddenInGame(true);
		// Keep collision enabled so the camera target doesn't fall through landscape on mid-air death.
		SetActorEnableCollision(true);
	}
}
