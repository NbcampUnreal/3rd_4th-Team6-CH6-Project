// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Enemy/NotifyState/SFAnimNotifyState_AttackWindow.h"

bool USFAnimNotifyState_AttackWindow::Received_NotifyBegin(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) const
{
	return true;
}

bool USFAnimNotifyState_AttackWindow::Received_NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference) const
{
	return true;
}
