// Fill out your copyright notice in the Description page of Project Settings.

#include "GlassSphereHealthWidget.h"
#include "HealthComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

// ---------------------------------------------------------------------------
// Drawing helpers (file-local)
// ---------------------------------------------------------------------------
namespace
{
	// Build a filled circle as a triangle fan.
	static void AddFilledCircle(TArray<FSlateVertex>& Verts, TArray<SlateIndex>& Indices,
		const FSlateRenderTransform& RT, FVector2f Center, float R, FColor Col, int32 Segs = 48)
	{
		const SlateIndex Base = (SlateIndex)Verts.Num();
		Verts.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(RT, Center,
			FVector2f(0.5f, 0.5f), FVector2f(0.5f, 0.5f), Col));

		for (int32 i = 0; i <= Segs; i++)
		{
			const float A = (float)i / Segs * 2.f * PI;
			const FVector2f P = Center + FVector2f(FMath::Cos(A), FMath::Sin(A)) * R;
			Verts.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(RT, P,
				FVector2f(0.5f + FMath::Cos(A) * 0.5f, 0.5f + FMath::Sin(A) * 0.5f),
				FVector2f(0.5f, 0.5f), Col));
			if (i > 0)
			{
				Indices.Add(Base);
				Indices.Add(Base + i);
				Indices.Add(Base + i + 1);
			}
		}
	}

	// Build the liquid segment: a circular segment below the fill level,
	// with a sinusoidal wave on the surface and a tilt based on lateral motion.
	static void AddLiquidSegment(TArray<FSlateVertex>& Verts, TArray<SlateIndex>& Indices,
		const FSlateRenderTransform& RT, FVector2f Center, float R,
		float Percent, float Tilt, float Amp, float Phase, FColor Col,
		int32 WaveSegs = 48, int32 ArcSegs = 36)
	{
		if (Percent <= 0.f) return;
		if (Percent >= 1.f) { AddFilledCircle(Verts, Indices, RT, Center, R, Col); return; }

		// Base fill top Y: at Percent=0 → bottom (Center.Y+R), at Percent=1 → top (Center.Y-R)
		const float BaseY  = Center.Y + R * (1.f - 2.f * Percent);
		const float DY     = BaseY - Center.Y;
		const float XHalf  = FMath::Sqrt(FMath::Max(0.f, R * R - DY * DY));

		// Angular positions of the two left/right intercepts with the circle
		const float A_right = FMath::Atan2(DY,  XHalf);
		const float A_left  = FMath::Atan2(DY, -XHalf);

		// Sweep forward (increasing angle) from right intercept through bottom to left intercept
		float ArcEnd = A_left;
		if (ArcEnd <= A_right) ArcEnd += 2.f * PI;

		// --- Build the polygon ---
		TArray<FVector2f> Poly;
		Poly.Reserve(WaveSegs + ArcSegs + 2);

		// 1. Wavy top surface from left edge to right edge (x increasing)
		const float LeftX  = Center.X - XHalf;
		const float RightX = Center.X + XHalf;

		for (int32 i = 0; i <= WaveSegs; i++)
		{
			const float T  = (float)i / WaveSegs;
			const float X  = LeftX + T * (RightX - LeftX);
			const float NX = (X - Center.X) / R;           // normalized −1…1
			const float Slope = Tilt * NX;                  // lateral tilt
			const float Wave  = Amp * FMath::Sin(Phase + NX * 2.f * PI);
			float Y = BaseY + Slope + Wave;

			// Clamp inside the circle
			const float XOff = X - Center.X;
			const float YSpan = FMath::Sqrt(FMath::Max(0.f, R * R - XOff * XOff));
			Y = FMath::Clamp(Y, Center.Y - YSpan, Center.Y + YSpan);

			Poly.Add(FVector2f(X, Y));
		}

		// 2. Circle arc from right intercept through bottom back to left intercept
		for (int32 i = 1; i <= ArcSegs; i++)
		{
			const float A = FMath::Lerp(A_right, ArcEnd, (float)i / ArcSegs);
			Poly.Add(Center + FVector2f(FMath::Cos(A), FMath::Sin(A)) * R);
		}

		// Fan-triangulate from centroid (valid as long as Amp stays small)
		FVector2f Centroid(0.f, 0.f);
		for (const FVector2f& P : Poly) Centroid += P;
		Centroid /= (float)Poly.Num();

		const SlateIndex Base = (SlateIndex)Verts.Num();
		Verts.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(RT, Centroid,
			FVector2f(0.5f, 0.5f), FVector2f(0.5f, 0.5f), Col));

		for (const FVector2f& P : Poly)
		{
			Verts.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(RT, P,
				FVector2f(0.5f, 0.5f), FVector2f(0.5f, 0.5f), Col));
		}

		const int32 N = Poly.Num();
		for (int32 i = 0; i < N; i++)
		{
			Indices.Add(Base);
			Indices.Add(Base + 1 + i);
			Indices.Add(Base + 1 + (i + 1) % N);
		}
	}

	// Glass highlight: a bright wedge arc in the upper-left + a small specular dot.
	static void AddGlassHighlight(TArray<FSlateVertex>& Verts, TArray<SlateIndex>& Indices,
		const FSlateRenderTransform& RT, FVector2f Center, float R)
	{
		// ---- Bright arc band (upper-left quadrant) ----
		const int32 Segs   = 20;
		const float A0     = PI;           // left
		const float A1     = PI * 1.45f;   // upper-left
		const float InnerR = R * 0.52f;

		const SlateIndex ArcBase = (SlateIndex)Verts.Num();
		for (int32 i = 0; i <= Segs; i++)
		{
			const float A    = FMath::Lerp(A0, A1, (float)i / Segs);
			const float Fade = FMath::Sin((float)i / Segs * PI); // fade in/out along arc
			const FVector2f OuterP = Center + FVector2f(FMath::Cos(A), FMath::Sin(A)) * R * 0.92f;
			const FVector2f InnerP = Center + FVector2f(FMath::Cos(A), FMath::Sin(A)) * InnerR;

			Verts.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(RT, OuterP,
				FVector2f(0.5f, 0.5f), FVector2f(0.5f, 0.5f), FColor(255, 255, 255, (uint8)(Fade * 55))));
			Verts.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(RT, InnerP,
				FVector2f(0.5f, 0.5f), FVector2f(0.5f, 0.5f), FColor(255, 255, 255, (uint8)(Fade * 110))));

			if (i > 0)
			{
				const SlateIndex B = ArcBase + (SlateIndex)((i - 1) * 2);
				Indices.Add(B);     Indices.Add(B + 1); Indices.Add(B + 2);
				Indices.Add(B + 1); Indices.Add(B + 3); Indices.Add(B + 2);
			}
		}

		// ---- Small specular dot (radial gradient: opaque center, transparent edge) ----
		const FVector2f SpecCenter = Center + FVector2f(-R * 0.28f, -R * 0.32f);
		const float     SpecR      = R * 0.14f;
		const int32     SpecSegs   = 12;
		const SlateIndex SpecBase  = (SlateIndex)Verts.Num();

		Verts.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(RT, SpecCenter,
			FVector2f(0.5f, 0.5f), FVector2f(0.5f, 0.5f), FColor(255, 255, 255, 200)));

		for (int32 i = 0; i <= SpecSegs; i++)
		{
			const float A = (float)i / SpecSegs * 2.f * PI;
			const FVector2f P = SpecCenter + FVector2f(FMath::Cos(A), FMath::Sin(A)) * SpecR;
			Verts.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(RT, P,
				FVector2f(0.5f, 0.5f), FVector2f(0.5f, 0.5f), FColor(255, 255, 255, 0)));

			if (i > 0)
			{
				Indices.Add(SpecBase);
				Indices.Add(SpecBase + i);
				Indices.Add(SpecBase + i + 1);
			}
		}
	}

	// Thin ring outline.
	static void AddRingOutline(TArray<FSlateVertex>& Verts, TArray<SlateIndex>& Indices,
		const FSlateRenderTransform& RT, FVector2f Center, float R,
		FColor OuterCol, FColor InnerCol, int32 Segs = 48)
	{
		const float     InnerR = R - 2.5f;
		const SlateIndex Base  = (SlateIndex)Verts.Num();

		for (int32 i = 0; i <= Segs; i++)
		{
			const float A = (float)i / Segs * 2.f * PI;
			const FVector2f OuterP = Center + FVector2f(FMath::Cos(A), FMath::Sin(A)) * R;
			const FVector2f InnerP = Center + FVector2f(FMath::Cos(A), FMath::Sin(A)) * InnerR;

			Verts.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(RT, OuterP,
				FVector2f(0.5f, 0.5f), FVector2f(0.5f, 0.5f), OuterCol));
			Verts.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(RT, InnerP,
				FVector2f(0.5f, 0.5f), FVector2f(0.5f, 0.5f), InnerCol));

			if (i > 0)
			{
				const SlateIndex B = Base + (SlateIndex)((i - 1) * 2);
				Indices.Add(B);     Indices.Add(B + 1); Indices.Add(B + 2);
				Indices.Add(B + 1); Indices.Add(B + 3); Indices.Add(B + 2);
			}
		}
	}
} // anonymous namespace

// ---------------------------------------------------------------------------
// UGlassSphereHealthWidget
// ---------------------------------------------------------------------------

void UGlassSphereHealthWidget::SetHealthComponent(UHealthComponent* InHealthComponent)
{
	HealthComponent = InHealthComponent;
	if (HealthComponent)
	{
		const float MaxHP = HealthComponent->GetMaxHealth();
		HealthPercent = MaxHP > 0.f ? HealthComponent->GetHealth() / MaxHP : 1.f;
		HealthComponent->OnHealthChanged.AddDynamic(this, &UGlassSphereHealthWidget::OnHealthChanged);
	}
}

void UGlassSphereHealthWidget::OnHealthChanged(float NewValue, float OldValue, float MaxValue)
{
	HealthPercent = MaxValue > 0.f ? NewValue / MaxValue : 0.f;
	HealthPercent = FMath::Clamp(HealthPercent, 0.f, 1.f);
}

FVector2D UGlassSphereHealthWidget::NativeGetDesiredSize() const
{
	const float D = (SphereRadius + 6.f) * 2.f;
	return FVector2D(D, D);
}

void UGlassSphereHealthWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Sample player lateral acceleration to drive the wobble
	float LateralAccel = 0.f;
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ACharacter* Char = Cast<ACharacter>(PC->GetPawn()))
		{
			const FVector Vel      = Char->GetCharacterMovement()->Velocity;
			const FVector DeltaVel = Vel - LastVelocity;
			LastVelocity = Vel;

			// Use the player's right-axis acceleration for tilt (X in world space is a rough proxy)
			LateralAccel = DeltaVel.X * WobbleVelocityScale;
		}
	}

	// Spring-damper: tilt (surface slope driven by lateral acceleration)
	WobbleTiltVel += (-WobbleStiffness * WobbleTilt - WobbleDamping * WobbleTiltVel + LateralAccel * SphereRadius) * InDeltaTime;
	WobbleTilt    += WobbleTiltVel * InDeltaTime;
	WobbleTilt     = FMath::Clamp(WobbleTilt, -MaxWobble, MaxWobble);

	// Spring-damper: wave amplitude driven by overall speed
	const float Speed     = LastVelocity.Size2D();
	const float TargetAmp = FMath::Min(Speed * WobbleVelocityScale, MaxWobble);
	WobbleAmpVel += (-WobbleStiffness * (WobbleAmp - TargetAmp) - WobbleDamping * WobbleAmpVel) * InDeltaTime;
	WobbleAmp    += WobbleAmpVel * InDeltaTime;
	WobbleAmp     = FMath::Max(0.f, WobbleAmp);

	// Advance wave phase
	WobblePhase += InDeltaTime * 4.f;
}

int32 UGlassSphereHealthWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FVector2D   Size   = AllottedGeometry.GetLocalSize();
	const FVector2f   Center(Size.X * 0.5f, Size.Y * 0.5f);
	const float       R      = FMath::Min(Size.X, Size.Y) * 0.5f - 4.f;
	const FSlateRenderTransform RT = AllottedGeometry.GetAccumulatedRenderTransform();

	ISlateRenderer* Renderer = FSlateApplication::Get().GetRenderer();
	if (!Renderer || R <= 0.f) return LayerId;

	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("GenericWhiteBox");
	const FSlateResourceHandle Handle = Renderer->GetResourceHandle(*WhiteBrush);

	TArray<FSlateVertex> Verts;
	TArray<SlateIndex>   Indices;

	auto Flush = [&](int32 Layer)
	{
		if (Verts.Num() > 0)
		{
			FSlateDrawElement::MakeCustomVerts(OutDrawElements, Layer, Handle, Verts, Indices, nullptr, 0, 0);
			Verts.Reset();
			Indices.Reset();
		}
	};

	// 1. Sphere body (very dark maroon — the glass itself)
	AddFilledCircle(Verts, Indices, RT, Center, R, FColor(8, 2, 2, 250));
	Flush(LayerId);

	// 2. Liquid fill (red)
	AddLiquidSegment(Verts, Indices, RT, Center, R,
		HealthPercent, WobbleTilt, WobbleAmp, WobblePhase, FColor(195, 12, 12, 235));
	Flush(LayerId + 1);

	// 3. Soft inner shadow to give depth to the glass
	AddFilledCircle(Verts, Indices, RT, Center, R * 0.88f, FColor(0, 0, 0, 22));
	Flush(LayerId + 2);

	// 4. Glass highlight and specular
	AddGlassHighlight(Verts, Indices, RT, Center, R);
	Flush(LayerId + 3);

	// 5. Rim (bright outer edge fading inward — gives the glass edge feel)
	AddRingOutline(Verts, Indices, RT, Center, R,
		FColor(210, 190, 190, 160), FColor(100, 70, 70, 0));
	Flush(LayerId + 4);

	return LayerId + 5;
}
