// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "QuickMergeSkeletalMeshLiteStyle.h"

class FQuickMergeSkeletalMeshLiteCommands : public TCommands<FQuickMergeSkeletalMeshLiteCommands>
{
public:

	FQuickMergeSkeletalMeshLiteCommands()
		: TCommands<FQuickMergeSkeletalMeshLiteCommands>(TEXT("QuickMergeSkeletalMeshLite"), 
		NSLOCTEXT("Contexts", "QuickMergeSkeletalMeshLite", "QuickMergeSkeletalMeshLite Plugin"), NAME_None, FQuickMergeSkeletalMeshLiteStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
