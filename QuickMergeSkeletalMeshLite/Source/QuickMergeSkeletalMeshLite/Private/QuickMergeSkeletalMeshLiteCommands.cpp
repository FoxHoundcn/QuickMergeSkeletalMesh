// Copyright Epic Games, Inc. All Rights Reserved.

#include "QuickMergeSkeletalMeshLiteCommands.h"

#define LOCTEXT_NAMESPACE "FQuickMergeSkeletalMeshLiteModule"

void FQuickMergeSkeletalMeshLiteCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "QuickMergeSkeletalMeshLite", "Execute QuickMergeSkeletalMeshLite action", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
