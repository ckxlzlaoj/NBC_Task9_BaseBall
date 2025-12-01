// Fill out your copyright notice in the Description page of Project Settings.


#include "CXGameStateBase.h"

#include "Kismet/GameplayStatics.h"
#include "CXPlayerController.h"


void ACXGameStateBase::MulticastRPCBroadcastLoginMessage_Implementation(const FString& InNameString)
{
    FString NotificationString = InNameString + TEXT(" has joined the game.");

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ACXPlayerController* CXPC = Cast<ACXPlayerController>(It->Get());
        if (IsValid(CXPC))
        {
            CXPC->PrintChatMessageString(NotificationString);
        }
    }
}