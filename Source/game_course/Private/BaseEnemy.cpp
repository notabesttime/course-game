// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseEnemy.h"
#include "HealthBarWidget.h"
#include "HealthComponent.h"
#include "BaseAttributeSet.h"
#include "Components/WidgetComponent.h"
#include "Components/MeshComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant4Vector.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HealingOrb.h"

ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCharacterMovement()->MaxWalkSpeed = 300.0f;

	HealthBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidgetComponent"));
	HealthBarWidgetComponent->SetupAttachment(GetRootComponent());
	HealthBarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarWidgetComponent->SetDrawSize(FVector2D(200.0f, 50.0f));
	HealthBarWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
}

void ABaseEnemy::CreateHighlightMaterial()
{
#if WITH_EDITOR
	UMaterial* Mat = NewObject<UMaterial>(this, NAME_None, RF_Transient);

	UMaterialExpressionConstant4Vector* ColorExpr = NewObject<UMaterialExpressionConstant4Vector>(Mat);
	ColorExpr->Constant = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // red

	Mat->GetExpressionCollection().AddExpression(ColorExpr);
	Mat->GetEditorOnlyData()->EmissiveColor.Expression = ColorExpr;
	Mat->BlendMode = BLEND_Additive;
	Mat->bUsedWithSkeletalMesh = true;
	Mat->PostEditChange(); // triggers shader compilation

	HighlightMaterial = Mat;
#endif
}

void ABaseEnemy::StartHighlight_Implementation()
{
	TArray<UMeshComponent*> Meshes;
	GetComponents<UMeshComponent>(Meshes);
	for (UMeshComponent* MeshComp : Meshes)
	{
		MeshComp->SetOverlayMaterial(HighlightMaterial);
	}
}

void ABaseEnemy::StopHighlight_Implementation()
{
	TArray<UMeshComponent*> Meshes;
	GetComponents<UMeshComponent>(Meshes);
	for (UMeshComponent* MeshComp : Meshes)
	{
		MeshComp->SetOverlayMaterial(nullptr);
	}
}

void ABaseEnemy::OnHealthChanged(float NewValue, float OldValue, float MaxValue)
{
	if (NewValue <= 0.0f)
	{
		if (HealthBarWidgetComponent)
		{
			HealthBarWidgetComponent->SetVisibility(false);
		}
		if (DeathSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation(), 0.25f);
		}
		if (HealingOrbClass)
		{
			AHealingOrb::SpawnOrReuse(GetWorld(), HealingOrbClass, GetActorLocation(), FRotator::ZeroRotator, this);
		}
		OnDied();
		Destroy();
	}
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (!HighlightMaterial)
	{
		CreateHighlightMaterial();
	}

	// Override to full health — base character defaults to 75
	if (AttributeSet)
	{
		AttributeSet->InitMaxHealth(100.0f);
		AttributeSet->InitHealth(100.0f);
	}

	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &ABaseEnemy::OnHealthChanged);
	}

	if (HealthBarWidgetComponent)
	{
		// If a class is explicitly set on this property, override the component's class
		if (HealthBarWidgetClass)
		{
			HealthBarWidgetComponent->SetWidgetClass(HealthBarWidgetClass);
			HealthBarWidgetComponent->InitWidget();
		}

		// Always bind health component to whatever widget is on the component
		if (UHealthBarWidget* HealthWidget = Cast<UHealthBarWidget>(HealthBarWidgetComponent->GetWidget()))
		{
			HealthWidget->SetHealthComponent(HealthComponent);
		}
	}
}
