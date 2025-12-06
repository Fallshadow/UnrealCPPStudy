// Fill out your copyright notice in the Description page of Project Settings.


#include "EmissivePlanesComponent.h"

// Sets default values for this component's properties
UEmissivePlanesComponent::UEmissivePlanesComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UEmissivePlanesComponent::BeginPlay()
{
	Super::BeginPlay();

	CollectPlaneMeshes();
	CreateSharedMID();
}


// Called every frame
void UEmissivePlanesComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UEmissivePlanesComponent::CollectPlaneMeshes()
{
    PlaneComponents.Empty();

    if (AActor* Owner = GetOwner()) {
        TArray<UStaticMeshComponent*> AllSMCs;
        Owner->GetComponents<UStaticMeshComponent>(AllSMCs);

        for (UStaticMeshComponent* Comp : AllSMCs) {
            if (!Comp) continue;

            if (ComponentNameContains.IsEmpty()) {
                PlaneComponents.Add(Comp);
            }
            else {
                const FString Name = Comp->GetName();
                if (Name.Contains(ComponentNameContains)) {
                    PlaneComponents.Add(Comp);
                }
            }
        }
    }
}

void UEmissivePlanesComponent::CreateSharedMID()
{
    SharedMID = nullptr;

    if (PlaneComponents.Num() == 0) {
        UE_LOG(LogTemp, Warning, TEXT("EmissivePlanesComponent: No plane components found on %s"),
            *GetOwner()->GetName());
        return;
    }

    UStaticMeshComponent* FirstPlane = PlaneComponents[0];
    if (!FirstPlane) {
        return;
    }

    UMaterialInterface* BaseMat = FirstPlane->GetMaterial(0);
    if (!BaseMat) {
        UE_LOG(LogTemp, Warning, TEXT("EmissivePlanesComponent: FirstPlane has no material."));
        return;
    }

    SharedMID = UMaterialInstanceDynamic::Create(BaseMat, this);
    if (!SharedMID) {
        return;
    }

    // 把这个 MID 应用到所有 Plane 的 slot 0
    for (UStaticMeshComponent* Comp : PlaneComponents) {
        if (Comp) {
            Comp->SetMaterial(0, SharedMID);
        }
    }
}

void UEmissivePlanesComponent::SetEmissiveIntensity(float NewIntensity)
{
    if (SharedMID) {
        SharedMID->SetScalarParameterValue(EmissiveParamName, NewIntensity);
    }
}