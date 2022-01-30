// Fill out your copyright notice in the Description page of Project Settings.


#include "MasterShifter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/BoxComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AMasterShifter::AMasterShifter()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collider"));
	CollisionComp->BodyInstance.SetCollisionProfileName("BlockAllDynamic");
	//CollisionComp->OnComponentHit.AddDynamic(this, &AMasterShifter::OnCompHit);
	CollisionComp->SetBoxExtent(FVector(60.0f, 60.0f, 60.0f));
	RootComponent = CollisionComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	MeshComp->SetupAttachment(CollisionComp);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));

	PostProcessComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("Post Process Comp"));
	PostProcessComp->SetupAttachment(CollisionComp);
	PostProcessComp->bUnbound = true;

	BoxTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxTrigger"));
	BoxTrigger->SetupAttachment(RootComponent);
	BoxTrigger->BodyInstance.SetCollisionProfileName("OverlapOnlyPawn");
	BoxTrigger->OnComponentBeginOverlap.AddDynamic(this, &AMasterShifter::OnComponentBeginOverlap);
	BoxTrigger->OnComponentEndOverlap.AddDynamic(this, &AMasterShifter::OnComponentEndOverlap);

	DynamicMat = nullptr;
	DynamicPPMat = nullptr;
}

void AMasterShifter::ShiftCoreMaterial(const FLinearColor& Black, const FLinearColor& White)
{
	// Get Material for Orb in Mesh
	if (!DynamicMat)
	{
		// Make new Dynamic Material instance for Core material
		if (UMaterialInterface* Material = MeshComp->GetMaterial(1))
		{
			DynamicMat = UMaterialInstanceDynamic::Create(Material, this);

			MeshComp->SetMaterial(1, DynamicMat);
		}
	}

	FLinearColor coreColor;
	DynamicMat->GetVectorParameterValue(TEXT("EmissiveColor"), coreColor);

	// if face is white
	if (coreColor.R > 0.5f)
	{
		// Change face to Black
		DynamicMat->SetVectorParameterValue(TEXT("EmissiveColor"), Black);

		PositiveState = true;
	}
	else
	{
		// Change face to White
		DynamicMat->SetVectorParameterValue(TEXT("EmissiveColor"), White);

		PositiveState = false;
	}
}

void AMasterShifter::ShiftPostProcessMaterial(const FLinearColor& Black, const FLinearColor& White)
{
	// Get post process material
	if (!DynamicPPMat)
	{
		UMaterialInterface* Material = Cast<UMaterialInterface>(PostProcessComp->Settings.WeightedBlendables.Array[0].Object);
		// Make new Dynamic Material instance for Post Process material
		if (Material)
			DynamicPPMat = UMaterialInstanceDynamic::Create(Material, this);

		FWeightedBlendable newWeightedBlendable;
		newWeightedBlendable.Object = DynamicPPMat;
		newWeightedBlendable.Weight = 1;

		PostProcessComp->Settings.WeightedBlendables.Array.Add(newWeightedBlendable);
	}

	FLinearColor faceColor;
	DynamicPPMat->GetVectorParameterValue(TEXT("Face Color"), faceColor);
	FLinearColor lineColor;
	DynamicPPMat->GetVectorParameterValue(TEXT("Line Color"), lineColor);

	// if face is white and line is black
	if (faceColor.R > 0.5f && lineColor.R < 0.5f)
	{
		// Change face to Black
		DynamicPPMat->SetVectorParameterValue(TEXT("Face Color"), Black);
		// Change Line to White
		DynamicPPMat->SetVectorParameterValue(TEXT("Line Color"), White);
	}
	else
	{
		// Change face to White
		DynamicPPMat->SetVectorParameterValue(TEXT("Face Color"), White);
		// Change Line to Black
		DynamicPPMat->SetVectorParameterValue(TEXT("Line Color"), Black);
	}
}

void AMasterShifter::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		PostProcessComp->Priority = 1.0f;
	}
}

void AMasterShifter::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		PostProcessComp->Priority = 0.0f;
	}
}

void AMasterShifter::Shift()
{
	const FLinearColor White(0.875f, 0.875f, 0.875f, 1.0f);
	const FLinearColor Black(0.04f, 0.04f, 0.04f, 1.0f);

	ShiftPostProcessMaterial(Black, White);

	ShiftCoreMaterial(Black, White);

	TArray<AActor*> Shiftables;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShiftable::StaticClass(), Shiftables);
	for (AActor* actor : Shiftables)
	{
		AShiftable* shiftable = Cast<AShiftable>(actor);
		shiftable->WorldStateChange(PositiveState);
	}
}

void AMasterShifter::BeginPlay()
{
	Super::BeginPlay();

	if (BoxTrigger)
	{
		BoxTrigger->OnComponentBeginOverlap.AddDynamic(this, &AMasterShifter::OnComponentBeginOverlap);
		BoxTrigger->OnComponentEndOverlap.AddDynamic(this, &AMasterShifter::OnComponentEndOverlap);
	}

	if (!PositiveState) Shift();
}

void AMasterShifter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//AddActorLocalRotation(FRotator(45.0f, 90.0f, 90.0f).Quaternion() * DeltaTime);
}
