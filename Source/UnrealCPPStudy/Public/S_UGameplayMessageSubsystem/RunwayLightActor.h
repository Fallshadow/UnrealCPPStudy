// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RunwayLightActor.generated.h"

UCLASS()
class UNREALCPPSTUDY_API ARunwayLightActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARunwayLightActor();

	UFUNCTION()
	void SetIntensity(int32 value);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	int CurIntensity = 1;
	float TimeAcc = 0.0f;
	float TimeTick = 2.0f;
};
