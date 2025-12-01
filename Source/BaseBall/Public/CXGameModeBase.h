// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CXGameModeBase.generated.h"


class ACXPlayerController;
/**
 * 
 */
UCLASS()
class BASEBALL_API ACXGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	void PrintChatMessageString(ACXPlayerController* InChattingPlayerController, const FString& InChatMessageString);

	virtual void OnPostLogin(AController* NewPlayer) override;

	FString GenerateSecretNumber();

	bool IsGuessNumberString(const FString& InNumberString);

	FString JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString);

	void IncreaseGuessCount(ACXPlayerController* InChattingPlayerController);

	void ResetGame();

	void JudgeGame(ACXPlayerController* InChattingPlayerController, int InStrikeCount);

	UFUNCTION()
	void OnMainTimerElapsed();

	UFUNCTION()
	void AdvanceTurn();
	FTimerHandle ResetTimerHandle;
	FTimerHandle MainTimerHandle;
	int32 CurrentGuessPlayerIndex = 0;
protected:
	FString SecretNumberString;

	TArray<TObjectPtr<ACXPlayerController>> AllPlayerControllers;
};
