// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CXPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BASEBALL_API ACXPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	ACXPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	FString PlayerNameString;

	UPROPERTY(Replicated)
	int32 CurrentGuessCount;

	UPROPERTY(Replicated)
	int32 MaxGuessCount;

	UPROPERTY(Replicated,BlueprintReadOnly)
	float MaxTime;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentTime, BlueprintReadOnly)
	float CurrentTime;
	UFUNCTION()
	void OnRep_CurrentTime();

	FString GetPlayerInfoString();
};

