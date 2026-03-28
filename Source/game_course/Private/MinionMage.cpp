// Fill out your copyright notice in the Description page of Project Settings.

#include "MinionMage.h"
#include "MinionMageAIController.h"
#include "MinionMageProjectile.h"
#include "PlayerCharacter.h"
#include "Animation/AnimSequence.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AMinionMage::AMinionMage()
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = AMinionMageAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AMinionMage::BeginPlay()
{
	Super::BeginPlay();

	if (SkinVariants.Num() > 0)
	{
		USkeletalMesh* ChosenSkin = SkinVariants[FMath::RandRange(0, SkinVariants.Num() - 1)];
		if (ChosenSkin)
		{
			GetMesh()->SetSkeletalMesh(ChosenSkin);
		}
	}

	if (GetMesh()->GetSkeletalMeshAsset())
	{
		GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		SetAnimState(EMageAnimState::Idle);
	}
}

void AMinionMage::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateAnimations();
	TryAttackPlayer();
}

void AMinionMage::UpdateAnimations()
{
	if (CurrentAnimState == EMageAnimState::Casting)
	{
		return;
	}

	float Speed = GetCharacterMovement()->Velocity.Size();
	SetAnimState(Speed > 50.f ? EMageAnimState::Running : EMageAnimState::Idle);
}

void AMinionMage::SetAnimState(EMageAnimState NewState)
{
	if (CurrentAnimState == NewState)
	{
		return;
	}

	CurrentAnimState = NewState;

	switch (NewState)
	{
	case EMageAnimState::Idle:
		if (IdleAnimation)
		{
			GetMesh()->PlayAnimation(IdleAnimation, true);
		}
		break;
	case EMageAnimState::Running:
		if (RunAnimation)
		{
			GetMesh()->PlayAnimation(RunAnimation, true);
		}
		break;
	case EMageAnimState::Casting:
		if (CastAnimations.Num() > 0)
		{
			UAnimSequence* Anim = CastAnimations[FMath::RandRange(0, CastAnimations.Num() - 1)];
			if (Anim)
			{
				GetMesh()->PlayAnimation(Anim, false);
			}
		}
		break;
	}
}

void AMinionMage::TryAttackPlayer()
{
	if (bAttackOnCooldown || !ProjectileClass)
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

	// Face player before firing
	FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.f;
	if (!ToPlayer.IsNearlyZero())
	{
		SetActorRotation(ToPlayer.Rotation());
	}

	// Spawn projectile slightly in front of and above the mage
	FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 60.f + FVector(0.f, 0.f, 60.f);
	FVector Direction = (Player->GetActorLocation() + FVector(0.f, 0.f, 50.f) - SpawnLocation).GetSafeNormal();
	FRotator SpawnRotation = Direction.Rotation();

	FActorSpawnParameters Params;
	Params.Instigator = this;
	Params.Owner = this;
	GetWorld()->SpawnActor<AMinionMageProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, Params);

	SetAnimState(EMageAnimState::Casting);

	bAttackOnCooldown = true;
	GetWorldTimerManager().SetTimer(
		AttackCooldownTimer,
		[this]()
		{
			bAttackOnCooldown = false;
			CurrentAnimState = EMageAnimState::Idle;
		},
		AttackCooldown,
		false
	);
}
