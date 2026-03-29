// Fill out your copyright notice in the Description page of Project Settings.

#include "MinionBrute.h"
#include "MinionBruteAIController.h"
#include "EnemySpawner.h"
#include "PlayerCharacter.h"
#include "PlayerShieldComponent.h"
#include "BaseAttributeSet.h"
#include "CombatAttributeUtils.h"
#include "Animation/AnimSequence.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AMinionBrute::AMinionBrute()
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = AMinionBruteAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AMinionBrute::BeginPlay()
{
	Super::BeginPlay();

	// 400 health
	if (AbilitySystemComponent && AttributeSet)
	{
		AttributeSet->InitMaxHealth(400.f);
		AttributeSet->InitHealth(400.f);
	}

	GetCharacterMovement()->MaxWalkSpeed = 200.f;

	// Pick a random skin
	if (SkinVariants.Num() > 0)
	{
		USkeletalMesh* ChosenSkin = SkinVariants[FMath::RandRange(0, SkinVariants.Num() - 1)];
		if (ChosenSkin)
		{
			GetMesh()->SetSkeletalMesh(ChosenSkin);
		}
	}

	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

	CachedPlayer = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (GetMesh()->GetSkeletalMeshAsset())
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		SetAnimState(EBruteAnimState::Idle);
	}
}

void AMinionBrute::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateAnimations();
	TryAttackPlayer();
}

void AMinionBrute::UpdateAnimations()
{
	if (CurrentAnimState == EBruteAnimState::Attacking)
	{
		return;
	}

	float Speed = GetCharacterMovement()->Velocity.Size();

	if (Speed > 50.f)
	{
		SetAnimState(EBruteAnimState::Running);
	}
	else
	{
		SetAnimState(EBruteAnimState::Idle);
	}
}

void AMinionBrute::SetAnimState(EBruteAnimState NewState)
{
	if (CurrentAnimState == NewState)
	{
		return;
	}

	CurrentAnimState = NewState;

	switch (NewState)
	{
	case EBruteAnimState::Idle:
		if (IdleAnimation)
		{
			GetMesh()->PlayAnimation(IdleAnimation, true);
		}
		break;
	case EBruteAnimState::Running:
		if (RunAnimation)
		{
			GetMesh()->PlayAnimation(RunAnimation, true);
		}
		break;
	case EBruteAnimState::Attacking:
		if (AttackAnimations.Num() > 0)
		{
			UAnimSequence* Anim = AttackAnimations[FMath::RandRange(0, AttackAnimations.Num() - 1)];
			if (Anim)
			{
				GetMesh()->PlayAnimation(Anim, false);
			}
		}
		break;
	}
}

void AMinionBrute::OnDied()
{
	AEnemySpawner::LiveBruteCount = FMath::Max(0, AEnemySpawner::LiveBruteCount - 1);

	if (CachedPlayer)
	{
		if (UPlayerShieldComponent* Shield = CachedPlayer->GetShieldComponent())
		{
			Shield->ActivateShield(3.f);
		}
	}
}

void AMinionBrute::TryAttackPlayer()
{
	if (bAttackOnCooldown || !CachedPlayer)
	{
		return;
	}

	APlayerCharacter* Player = CachedPlayer;

	float DistSq = FVector::DistSquared(GetActorLocation(), Player->GetActorLocation());
	if (DistSq > AttackRange * AttackRange)
	{
		return;
	}

	// Face player
	FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.f;
	if (!ToPlayer.IsNearlyZero())
	{
		SetActorRotation(ToPlayer.Rotation());
	}

	CombatAttributeUtils::ApplyHealthDelta(Player, -AttackDamage);

	SetAnimState(EBruteAnimState::Attacking);

	if (AttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation(), 0.25f);
	}

	bAttackOnCooldown = true;
	GetWorldTimerManager().SetTimer(
		AttackCooldownTimer,
		[this]()
		{
			bAttackOnCooldown = false;
			CurrentAnimState = EBruteAnimState::Idle;
		},
		AttackCooldown,
		false
	);
}
