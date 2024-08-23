// Copyright Epic Games, Inc. All Rights Reserved.

#include "QuickMergeSkeletalMeshLite.h"
#include "QuickMergeSkeletalMeshLiteStyle.h"
#include "QuickMergeSkeletalMeshLiteCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "Misc/FileHelper.h"
#include "ContentBrowserModule.h"
#include "SkeletalMeshMerge.h"
#include "AssetRegistry/AssetRegistryModule.h"
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 1
#include "Engine/SkinnedAssetCommon.h"
#endif

static const FName QuickMergeSkeletalMeshLiteTabName("QuickMergeSkeletalMeshLite");

#define LOCTEXT_NAMESPACE "FQuickMergeSkeletalMeshLiteModule"

void FQuickMergeSkeletalMeshLiteModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FQuickMergeSkeletalMeshLiteStyle::Initialize();
	FQuickMergeSkeletalMeshLiteStyle::ReloadTextures();

	if (!IsRunningCommandlet())
	{
		// 获取内容浏览器模块中的扩展委托列表
		FContentBrowserModule& ContentBrowserModule = FModuleManager::GetModuleChecked<FContentBrowserModule>("ContentBrowser");
		TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders(); //
		// 添加委托
		FContentBrowserMenuExtender_SelectedAssets& AddedDelegate = CBMenuExtenderDelegates.Add_GetRef(
			FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FQuickMergeSkeletalMeshLiteModule::OnExtendContentBrowserAssetSelectionMenu));
	}
}

USkeletalMesh* FQuickMergeSkeletalMeshLiteModule::MergeSkeletalMesh(TArray<USkeletalMesh*> MeshList)
{
	if (MeshList.Num() <= 1) return nullptr;

	TArray<USkeletalMesh*>& SourceMeshList = MeshList;
	FString fileName = FPaths::GetBaseFilename(MeshList[0]->GetName()) + TEXT("_Merge");
	FString pathName = FPaths::GetPath(MeshList[0]->GetPathName()) + TEXT("/") + fileName;
	UPackage* NewMeshPack = CreatePackage(*pathName);
	USkeletalMesh* CompositeMesh = NewObject<USkeletalMesh>(NewMeshPack, *fileName, RF_Public | RF_Standalone);
	TArray<FSkelMeshMergeSectionMapping> InForceSectionMapping;
	FSkeletalMeshMerge MeshMergeUtil(CompositeMesh, SourceMeshList, InForceSectionMapping, 0);
	if (!MeshMergeUtil.DoMerge())
	{
		return nullptr;
	}
	CompositeMesh->SetSkeleton(MeshList[0]->GetSkeleton()); //设置新的 *SkeletalMesh* 的骨架
	FAssetRegistryModule::AssetCreated(CompositeMesh);
	CompositeMesh->MarkPackageDirty();
	return CompositeMesh;
}

TSharedRef<FExtender> FQuickMergeSkeletalMeshLiteModule::OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& Assets)
{
	TSharedRef<FExtender> Extender(new FExtender());

	Extender->AddMenuExtension("GetAssetActions", EExtensionHook::After, nullptr, FMenuExtensionDelegate::CreateLambda([Assets](FMenuBuilder& MenuBuilder)
		                           {
			                           // 创建新的 Section
			                           MenuBuilder.BeginSection("CustomMenu", FText::FromString(TEXT("FX Actions")));

			                           MenuBuilder.AddMenuEntry(
				                           FText::FromString(TEXT("Merge Skeletal Mesh Test")),
				                           FText::FromString(TEXT("Merge Skeletal Mesh Test")),
				                           FSlateIcon(FQuickMergeSkeletalMeshLiteStyle::GetStyleSetName(), "MERAGEASSET.ICON"),
				                           FUIAction(FExecuteAction::CreateLambda([Assets]()
					                           {
						                           TArray<USkeletalMesh*> mergeList;
						                           USkeleton* skeleton = nullptr;
						                           FString meshName = TEXT("");
						                           for (const FAssetData& AssetData : Assets)
						                           {
							                           FSoftObjectPath refPath = AssetData.GetAsset()->GetFName().ToString() + TEXT("'") + AssetData.ToSoftObjectPath().ToString() + TEXT("'");
							                           UObject* refObj = refPath.TryLoad();
							                           USkeletalMesh* skeletalMesh = Cast<USkeletalMesh>(refObj);
							                           if (skeletalMesh != nullptr)
							                           {
								                           if (skeleton != nullptr)
								                           {
									                           USkeleton* skeletonNew = skeletalMesh->GetSkeleton();
									                           if (skeleton->GetPathName() == skeletonNew->GetPathName())
									                           {
										                           mergeList.Add(skeletalMesh);
										                           meshName += TEXT("Merge : ") + skeletalMesh->GetName() + TEXT("\n");
									                           }
									                           else
									                           {
										                           meshName += TEXT("Different Skeleton : ") + skeletalMesh->GetName() + TEXT("\n");
										                           FString error = TEXT("Merge Skeletal Mesh Error: Different Skeleton : ") + skeleton->GetPathName() + TEXT(" -> ") + skeletonNew->GetPathName();
										                           UE_LOG(LogTemp, Log, TEXT("%s"), *error);
									                           }
								                           }
								                           else
								                           {
									                           skeleton = skeletalMesh->GetSkeleton();
									                           mergeList.Add(skeletalMesh);
									                           meshName += TEXT("Base : ") + skeletalMesh->GetName() + TEXT("\n");
								                           }
							                           }
						                           }
						                           if (mergeList.Num() > 1)
						                           {
							                           if (MergeSkeletalMesh(mergeList) != nullptr)
							                           {
								                           FText const Title = FText::FromString(TEXT("Merge Complete"));
								                           FText const errorText = FText::Format(LOCTEXT("Output", "Merge Complete ！\n{0}"), FText::FromString(meshName));
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 2
								                           FMessageDialog::Open(EAppMsgType::Ok, errorText, Title);
#else
								                           FMessageDialog::Open(EAppMsgType::Ok, errorText, &Title);
#endif
							                           }
							                           else
							                           {
								                           FText const Title = FText::FromString(TEXT("Merge Failure"));
								                           FText const errorText = FText::Format(LOCTEXT("Output", "Merge Failure ！\n{0}"), FText::FromString(meshName));
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 2
								                           FMessageDialog::Open(EAppMsgType::Ok, errorText, Title);
#else
								                           FMessageDialog::Open(EAppMsgType::Ok, errorText, &Title);
#endif
							                           }
						                           }
						                           else
						                           {
							                           FText const Title = FText::FromString(TEXT("Merge Failure"));
							                           FText const errorText = FText::Format(LOCTEXT("Output", "Skeletal Mesh Count < 2 ！\n{0}"), FText::FromString(meshName));
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 2
							                           FMessageDialog::Open(EAppMsgType::Ok, errorText, Title);
#else
							                           FMessageDialog::Open(EAppMsgType::Ok, errorText, &Title);
#endif
							                           return;
						                           }
					                           }
				                           ))
			                           );
			                           MenuBuilder.EndSection();
		                           }
	                           ));

	return Extender;
}

void FQuickMergeSkeletalMeshLiteModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FQuickMergeSkeletalMeshLiteStyle::Shutdown();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FQuickMergeSkeletalMeshLiteModule, QuickMergeSkeletalMeshLite)
