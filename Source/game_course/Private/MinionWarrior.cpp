// Fill out your copyright notice in the Description page of Project Settings.

#include "MinionWarrior.h"
#include "MinionWarriorAIController.h"
#include "PlayerCharacter.h"
#include "BaseAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Animation/AnimSequence.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AMinionWarrior::AMinionWarrior()
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = AMinionWarriorAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AMinionWarrior::BeginPlay()
{
	Super::BeginPlay();

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

	if (GetMesh()->GetSkeletalMeshAsset())
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		SetAnimState(EWarriorAnimState::Idle);
	}
}

void AMinionWarrior::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateAnimations();
	TryAttackPlayer();
}

void AMinionWarrior::UpdateAnimations()
{
	if (CurrentAnimState == EWarriorAnimState::Attacking)
	{
		return;
	}

	FVector Velocity = GetCharacterMovement()->Velocity;
	float Speed = Velocity.Size();

	if (Speed > 50.f)
	{
		SetAnimState(EWarriorAnimState::Running);
	}
	else
	{
		SetAnimState(EWarriorAnimState::Idle);
	}
}

void AMinionWarrior::SetAnimState(EWarriorAnimState NewState)
{
	if (CurrentAnimState == NewState)
	{
		return;
	}

	CurrentAnimState = NewState;

	switch (NewState)
	{
	case EWarriorAnimState::Idle:
		if (IdleAnimation)
		{
			GetMesh()->PlayAnimation(IdleAnimation, true);
		}
		break;
	case EWarriorAnimState::Running:
		if (RunAnimation)
		{
			GetMesh()->PlayAnimation(RunAnimation, true);
		}
		break;
	case EWarriorAnimState::Attacking:
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

void AMinionWarrior::TryAttackPlayer()
{
	if (bAttackOnCooldown)
	{
		return;
	}

	APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!Player || !IsValid(Player))
	{
		return;
	}

	float DistToPlayer = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
	if (DistToPlayer > AttackRange)
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

	// Apply damage
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Player))
	{
		UAbilitySystemComponent* TargetASC = ASI->GetAbilitySystemComponent();
		if (TargetASC)
		{
			const UBaseAttributeSet* AttrSet = TargetASC->GetSet<UBaseAttributeSet>();
			if (AttrSet)
			{
				float NewHealth = FMath::Max(0.f, AttrSet->GetHealth() - AttackDamage);
				TargetASC->SetNumericAttributeBase(UBaseAttributeSet::GetHealthAttribute(), NewHealth);
			}
		}
	}

	SetAnimState(EWarriorAnimState::Attacking);

	bAttackOnCooldown = true;
	GetWorldTimerManager().SetTimer(
		AttackCooldownTimer,
		[this]()
		{
			bAttackOnCooldown = false;
			CurrentAnimState = EWarriorAnimState::Idle;
		},
		AttackCooldown,
		false
	);
}
