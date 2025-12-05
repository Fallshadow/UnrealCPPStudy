// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMessage.generated.h"


USTRUCT(BlueprintType)
struct FRunWayLightIntensityChangedMessage {
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString RunWay = "0";

    UPROPERTY(BlueprintReadWrite)
    int32 Intensity = 0;
};