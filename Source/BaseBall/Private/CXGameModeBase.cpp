// Fill out your copyright notice in the Description page of Project Settings.


#include "CXGameModeBase.h"
#include "CXGameStateBase.h"
#include "CXPlayerController.h"
#include "EngineUtils.h"
#include "CXPlayerState.h"

void ACXGameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	/*ACXGameStateBase* CXGameStateBase = GetGameState<ACXGameStateBase>();
	if (IsValid(CXGameStateBase) == true)
	{
		CXGameStateBase->MulticastRPCBroadcastLoginMessage(TEXT("XXXXXXX"));
	}

	ACXPlayerController* CXPlayerController = Cast<ACXPlayerController>(NewPlayer);
	if (IsValid(CXPlayerController) == true)
	{
		AllPlayerControllers.Add(CXPlayerController);
	}*/
	ACXPlayerController* CXPlayerController = Cast<ACXPlayerController>(NewPlayer);
	if (IsValid(CXPlayerController) == true)
	{
		CXPlayerController->NotificationText = FText::FromString(TEXT("Connected to the game server."));
		AllPlayerControllers.Add(CXPlayerController);

		ACXPlayerState* CXPS = CXPlayerController->GetPlayerState<ACXPlayerState>();
		if (IsValid(CXPS) == true)
		{
			CXPS->PlayerNameString = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());
		}

		ACXGameStateBase* CXGameStateBase = GetGameState<ACXGameStateBase>();
		if (IsValid(CXGameStateBase) == true)
		{
			CXGameStateBase->MulticastRPCBroadcastLoginMessage(CXPS->PlayerNameString);
		}
	}
}

FString ACXGameModeBase::GenerateSecretNumber()
{
	TArray<int32> Numbers;
	for (int32 i = 1; i <= 9; ++i)
	{
		Numbers.Add(i);
	}

	FMath::RandInit(FDateTime::Now().GetTicks());
	Numbers = Numbers.FilterByPredicate([](int32 Num) { return Num > 0; });

	FString Result;
	for (int32 i = 0; i < 3; ++i)
	{
		int32 Index = FMath::RandRange(0, Numbers.Num() - 1);
		Result.Append(FString::FromInt(Numbers[Index]));
		Numbers.RemoveAt(Index);
	}

	return Result;
}

bool ACXGameModeBase::IsGuessNumberString(const FString& InNumberString)
{
	bool bCanPlay = false;
	ACXPlayerState* CurrentGuessPlayerState = AllPlayerControllers[CurrentGuessPlayerIndex]->GetPlayerState<ACXPlayerState>();

	do {

		if (InNumberString.Len() != 3)
		{
			break;
		}

		bool bIsUnique = true;
		TSet<TCHAR> UniqueDigits;
		for (TCHAR C : InNumberString)
		{
			if (FChar::IsDigit(C) == false || C == '0')
			{
				bIsUnique = false;
				break;
			}
			if (UniqueDigits.Contains(C))
			{
				bIsUnique = false;
				break;
			}

			UniqueDigits.Add(C);
		}

		if (bIsUnique == false)
		{
			break;
		}

		// 시간이 다 되었으면 플레이 불가
		if (CurrentGuessPlayerState->CurrentTime <= KINDA_SMALL_NUMBER)
		{
			break; // false 반환
		}
		bCanPlay = true;

	} while (false);


	


	return bCanPlay;
}

FString ACXGameModeBase::JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString)
{
	int32 StrikeCount = 0, BallCount = 0;

	for (int32 i = 0; i < 3; ++i)
	{
		if (InSecretNumberString[i] == InGuessNumberString[i])
		{
			StrikeCount++;
		}
		else
		{
			FString PlayerGuessChar = FString::Printf(TEXT("%c"), InGuessNumberString[i]);
			if (InSecretNumberString.Contains(PlayerGuessChar))
			{
				BallCount++;
			}
		}
	}

	if (StrikeCount == 0 && BallCount == 0)
	{
		return TEXT("OUT");
	}

	return FString::Printf(TEXT("%dS%dB"), StrikeCount, BallCount);
}

void ACXGameModeBase::IncreaseGuessCount(ACXPlayerController* InChattingPlayerController)
{
	ACXPlayerState* CXPS = InChattingPlayerController->GetPlayerState<ACXPlayerState>();
	if (IsValid(CXPS) == true)
	{
		CXPS->CurrentGuessCount++;
	}
}

void ACXGameModeBase::ResetGame()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan, TEXT("[ResetGame] GAME RESET IN 10SEC"));
	SecretNumberString = GenerateSecretNumber();

	for (const auto& CXPlayerController : AllPlayerControllers)
	{
		ACXPlayerState* CXPS = CXPlayerController->GetPlayerState<ACXPlayerState>();
		if (IsValid(CXPS) == true)
		{
			CXPS->CurrentGuessCount = 0;
			CXPS->CurrentTime = CXPS->MaxTime;
		}
	}

	GetWorldTimerManager().SetTimer(
		MainTimerHandle,
		this,
		&ACXGameModeBase::OnMainTimerElapsed,
		1.f,
		true
	);

	
	AdvanceTurn();
}

void ACXGameModeBase::JudgeGame(ACXPlayerController* InChattingPlayerController, int InStrikeCount)
{
	bool bIsGameEnded = false;
	if (3 == InStrikeCount)
	{
		bIsGameEnded = true;
		GetWorldTimerManager().ClearTimer(MainTimerHandle);

		ACXPlayerState* CXPS = InChattingPlayerController->GetPlayerState<ACXPlayerState>();
		for (const auto& CXPlayerController : AllPlayerControllers)
		{
			if (IsValid(CXPS) == true)
			{
				FString CombinedMessageString = CXPS->PlayerNameString + TEXT(" has won the game.");
				CXPlayerController->NotificationText = FText::FromString(CombinedMessageString);

				
				GetWorldTimerManager().SetTimer(ResetTimerHandle, [this]()
					{
						ResetGame();
					}, 10.f, false);
			}
		}
	}
	else
	{

		bool bIsDraw = true;
		for (const auto& CXPlayerController : AllPlayerControllers)
		{
			ACXPlayerState* CXPS = CXPlayerController->GetPlayerState<ACXPlayerState>();
			if (IsValid(CXPS) == true)
			{
				if (CXPS->CurrentGuessCount < CXPS->MaxGuessCount)
				{
					bIsDraw = false;
					break;
				}
			}
		}

		if (true == bIsDraw)
		{
			bIsGameEnded = true;
			GetWorldTimerManager().ClearTimer(MainTimerHandle);

			for (const auto& CXPlayerController : AllPlayerControllers)
			{
				CXPlayerController->NotificationText = FText::FromString(TEXT("Draw..."));

				
				GetWorldTimerManager().SetTimer(ResetTimerHandle, [this]()
					{
						ResetGame();
					}, 10.f, false);
			}
		}
	}
	if (!bIsGameEnded)
	{
		AdvanceTurn(); //// 추가된 부분 ////
	}
}

void ACXGameModeBase::OnMainTimerElapsed()
{

	if (AllPlayerControllers.Num() == 0) return;

	ACXPlayerState* CurrentPlayerState = AllPlayerControllers[CurrentGuessPlayerIndex]->GetPlayerState<ACXPlayerState>();
	if (!CurrentPlayerState) return;



	if (CurrentPlayerState->CurrentTime > 0.f)
	{
		CurrentPlayerState->CurrentTime -= 1.f; // 서버에서 감소

		
	}
	

	if (CurrentPlayerState->CurrentTime <= 0.f)
	{
		IncreaseGuessCount(AllPlayerControllers[CurrentGuessPlayerIndex]);

		AdvanceTurn(); // 턴 전환
	}
}

void ACXGameModeBase::AdvanceTurn()
{
	if (AllPlayerControllers.Num() == 0) return;

	ACXPlayerController* PrevController = AllPlayerControllers[CurrentGuessPlayerIndex];
	if (IsValid(PrevController))
	{
		PrevController->NotificationText = FText::FromString(TEXT("Wait for your turn..."));
	}

	CurrentGuessPlayerIndex = (CurrentGuessPlayerIndex + 1) % AllPlayerControllers.Num();

	ACXPlayerController* NextController = AllPlayerControllers[CurrentGuessPlayerIndex];
	ACXPlayerState* NextPlayerState = AllPlayerControllers[CurrentGuessPlayerIndex]->GetPlayerState<ACXPlayerState>();

	if (NextPlayerState)
	{
		NextPlayerState->CurrentTime = NextPlayerState->MaxTime;
		NextController->NotificationText = FText::FromString(TEXT("It's your turn!"));
	}

}

void ACXGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	SecretNumberString = GenerateSecretNumber();

	UE_LOG(LogTemp, Error, TEXT("SecretNumberString: %s"), *SecretNumberString);

	GetWorldTimerManager().SetTimerForNextTick([this]()
		{
			GetWorldTimerManager().SetTimer(
				MainTimerHandle,
				this,
				&ACXGameModeBase::OnMainTimerElapsed,
				1.f,
				true
			);
		});
}

void ACXGameModeBase::PrintChatMessageString(ACXPlayerController* InChattingPlayerController, const FString& InChatMessageString)
{
	if (AllPlayerControllers[CurrentGuessPlayerIndex] != InChattingPlayerController)
	{
		InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("It's not your turn! Wait for your turn."));
		return;
	}
	ACXPlayerState* CXPS = InChattingPlayerController->GetPlayerState<ACXPlayerState>();
	if (IsValid(CXPS) && CXPS->CurrentGuessCount >= CXPS->MaxGuessCount)
	{
		InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("No more attempts available."));
		return;  // ? 게임 로직 수행 금지
	}
	

	/*int Index = InChatMessageString.Len() - 3;
	FString GuessNumberString = InChatMessageString.RightChop(Index);*/

	FString GuessNumberString = InChatMessageString;

	FString Dummy;
	if (GuessNumberString.Split(TEXT(": "), &Dummy, &GuessNumberString))
	{
		GuessNumberString = GuessNumberString.TrimStartAndEnd();
	}
	if (IsGuessNumberString(GuessNumberString) == true)
	{
		FString JudgeResultString = JudgeResult(SecretNumberString, GuessNumberString);

		IncreaseGuessCount(InChattingPlayerController);

		for (TActorIterator<ACXPlayerController> It(GetWorld()); It; ++It)
		{
			ACXPlayerController* CXPlayerController = *It;
			if (IsValid(CXPlayerController) == true)
			{
				FString CombinedMessageString = InChatMessageString + TEXT(" -> ") + JudgeResultString;
				CXPlayerController->ClientRPCPrintChatMessageString(CombinedMessageString);

				int32 StrikeCount = FCString::Atoi(*JudgeResultString.Left(1));
				JudgeGame(InChattingPlayerController, StrikeCount);
				
			}
		}
		
	}
	else
	{
		for (TActorIterator<ACXPlayerController> It(GetWorld()); It; ++It)
		{
			ACXPlayerController* CXPlayerController = *It;
			if (IsValid(CXPlayerController))
			{
				FString ErrorMessage = FString::Printf(
					TEXT("Invalid input: '%s' (Enter exactly 3 unique digits from 1~9)"),
					*InChatMessageString
				);

				CXPlayerController->ClientRPCPrintChatMessageString(ErrorMessage);
			}
		}
	}
}