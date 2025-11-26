// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyTestLog.generated.h"

UCLASS()
class UNREALCPPSTUDY_API AMyTestLog : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyTestLog();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	float MemberFloat = 10.0f;

	UPROPERTY(VisibleAnywhere)
	int MemberInt = 10;
};
