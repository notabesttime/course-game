// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseCompanion.h"
#include "BaseCompanionAIController.h"
#include "BaseEnemy.h"
#include "PlayerCharacter.h"
#include "BaseAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "Engine/OverlapResult.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HealthComponent.h"

ABaseCompanion::ABaseCompanion()
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = ABaseCompanionAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ABaseCompanion::BeginPlay()
{
	Super::BeginPlay();

	PlayerOwner = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &ABaseCompanion::OnHealthChanged);
	}

	GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);

	SetAnimState(ECompanionAnimState::Idle);
}

void ABaseCompanion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateBehavior();
	UpdateAnimations();
}

void ABaseCompanion::UpdateBehavior()
{
	if (!PlayerOwner)
	{
		return;
	}

	AAIController* AIC = Cast<AAIController>(GetController());
	if (!AIC)
	{
		return;
	}

	float DistToPlayer = FVector::Dist(GetActorLocation(), PlayerOwner->GetActorLocation());

	// Too far from player — follow them, ignore enemies
	if (DistToPlayer > FollowDistance)
	{
		CurrentTarget = nullptr;
		AIC->MoveToActor(PlayerOwner, 100.f);
		return;
	}

	// Validate current target every tick
	if (CurrentTarget && !IsValid(CurrentTarget))
	{
		CurrentTarget = nullptr;
	}

	// Find new target if we don't have one
	if (!CurrentTarget)
	{
		CurrentTarget = FindNearestEnemy();
	}

	if (CurrentTarget)
	{
		float DistToEnemy = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

		if (DistToEnemy <= AttackRange)
		{
			AIC->StopMovement();
			if (!bAttackOnCooldown)
			{
				PerformAttack(CurrentTarget);
			}
		}
		else
		{
			AIC->MoveToActor(CurrentTarget, AttackRange * 0.5f);
		}
	}
}

ABaseEnemy* ABaseCompanion::FindNearestEnemy() const
{
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		GetActorLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(DetectionRadius),
		Params
	);

	ABaseEnemy* Nearest = nullptr;
	float NearestDist = FLT_MAX;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		ABaseEnemy* Enemy = Cast<ABaseEnemy>(Overlap.GetActor());
		if (!Enemy || !IsValid(Enemy))
		{
			continue;
		}

		float Dist = FVector::Dist(GetActorLocation(), Enemy->GetActorLocation());
		if (Dist < NearestDist)
		{
			NearestDist = Dist;
			Nearest = Enemy;
		}
	}

	return Nearest;
}

void ABaseCompanion::UpdateAnimations()
{
	if (CurrentAnimState == ECompanionAnimState::Attacking)
	{
		return;
	}

	FVector Velocity = GetCharacterMovement()->Velocity;
	float Speed = Velocity.Size();

	if (Speed > 50.f)
	{
		// Face movement direction
		FVector MoveDir = Velocity.GetSafeNormal();
		MoveDir.Z = 0.f;
		if (!MoveDir.IsNearlyZero())
		{
			SetActorRotation(MoveDir.Rotation());
		}

		if (Speed < WalkSpeedThreshold && WalkAnimation)
		{
			SetAnimState(ECompanionAnimState::Walking);
		}
		else
		{
			SetAnimState(ECompanionAnimState::Running);
		}
	}
	else
	{
		SetAnimState(ECompanionAnimState::Idle);
	}
}

void ABaseCompanion::SetAnimState(ECompanionAnimState NewState)
{
	if (CurrentAnimState == NewState)
	{
		return;
	}

	CurrentAnimState = NewState;

	switch (NewState)
	{
	case ECompanionAnimState::Idle:
		if (IdleAnimation)
		{
			GetMesh()->PlayAnimation(IdleAnimation, true);
		}
		break;
	case ECompanionAnimState::Walking:
		if (WalkAnimation)
		{
			GetMesh()->PlayAnimation(WalkAnimation, true);
		}
		break;
	case ECompanionAnimState::Running:
		if (RunAnimation)
		{
			GetMesh()->PlayAnimation(RunAnimation, true);
		}
		break;
	case ECompanionAnimState::Attacking:
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

void ABaseCompanion::PerformAttack(ABaseEnemy* Target)
{
	if (!Target || !IsValid(Target))
	{
		return;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		GetActorLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(AttackRadius),
		Params
	);

	TSet<AActor*> DamagedActors;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		ABaseEnemy* HitEnemy = Cast<ABaseEnemy>(Overlap.GetActor());
		if (!HitEnemy || !IsValid(HitEnemy) || DamagedActors.Contains(HitEnemy))
		{
			continue;
		}

		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(HitEnemy))
		{
			UAbilitySystemComponent* TargetASC = ASI->GetAbilitySystemComponent();
			if (!TargetASC) continue;

			const UBaseAttributeSet* AttrSet = TargetASC->GetSet<UBaseAttributeSet>();
			if (!AttrSet) continue;

			float NewHealth = FMath::Max(0.f, AttrSet->GetHealth() - AttackDamage);
			TargetASC->SetNumericAttributeBase(UBaseAttributeSet::GetHealthAttribute(), NewHealth);
			DamagedActors.Add(HitEnemy);
		}
	}

	FVector ToEnemy = Target->GetActorLocation() - GetActorLocation();
	ToEnemy.Z = 0.f;
	if (!ToEnemy.IsNearlyZero())
	{
		SetActorRotation(ToEnemy.Rotation());
	}

	if (AttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation(), 0.5f);
	}
	OnCompanionAttack(Target);

	SetAnimState(ECompanionAnimState::Attacking);

	bAttackOnCooldown = true;
	GetWorldTimerManager().SetTimer(
		AttackCooldownTimer,
		[this]()
		{
			bAttackOnCooldown = false;
			CurrentAnimState = ECompanionAnimState::Idle;
		},
		AttackCooldown,
		false
	);
}

void ABaseCompanion::OnHealthChanged(float NewValue, float OldValue, float MaxValue)
{
	if (NewValue <= 0.0f)
	{
		if (DeathSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation(), 0.5f);
		}
		Destroy();
	}
}
