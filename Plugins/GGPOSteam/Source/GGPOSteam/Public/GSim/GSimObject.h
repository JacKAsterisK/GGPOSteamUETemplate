// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GSimObject.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UGSimObject : public UInterface
{
    GENERATED_BODY()
};

class GGPOSTEAM_API IGSimObject
{
    GENERATED_BODY()

public:
	virtual void Init() = 0;
	virtual void Tick(float DeltaTime) = 0;
	virtual void Destroy() = 0;
};