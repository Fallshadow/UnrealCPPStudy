// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FBXParserLibrary.h"

#include "FBXParseActor.generated.h"

UCLASS()
class FBXIMPORTERUTILPLUGIN_API AFBXParseActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFBXParseActor();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UPROPERTY(EditAnywhere, Category = "FBX Parser")
	FString FBXPath;	
	
	UPROPERTY(EditAnywhere, Category = "FBX Parser")
	FString ExportFBXJsonName;
};
