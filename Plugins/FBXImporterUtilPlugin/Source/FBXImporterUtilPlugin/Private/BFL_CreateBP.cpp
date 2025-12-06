// Fill out your copyright notice in the Description page of Project Settings.


#include "BFL_CreateBP.h"

#if WITH_EDITOR

#include "Kismet2/KismetEditorUtilities.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Modules/ModuleManager.h"
#include "UObject/Package.h"
#include "Misc/PackageName.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "FBXParserLibrary.h"
#include "Editor.h"

#endif // WITH_EDITOR



void UBFL_CreateBP::GeneratePlanesBlueprint(const FString& FBXPath, const FString& AssetPath, UStaticMesh* PlaneMesh)
{
    TArray<FParsedMeshData> meshArray = UFBXParserLibrary::ParseFBXFile(FBXPath);

    for (int32 MeshIndex = 0; MeshIndex < meshArray.Num(); MeshIndex++) {
        const FParsedMeshData& MeshData = meshArray[MeshIndex]; // 取当前网格数据（const&避免拷贝）
        UBlueprint* NewBP = UBFL_CreateBP::CreatePlanesBlueprint(
            AssetPath + TEXT("_") + MeshData.MeshName, MeshData.CenterPoints, PlaneMesh
        );

        //if (MeshData.MeshName == "RW15_EdgeLights") {
        //    UE_LOG(LogTemp, Log, TEXT("===== 找到跑道 15 边灯 ====="));

        //    UBlueprint* NewBP = UBFL_CreateBP::CreatePlanesBlueprint(
        //        AssetPath + TEXT("_") + MeshData.MeshName, MeshData.CenterPoints, PlaneMesh
        //    );
        //    break;
        //}

        //UBlueprint* NewBP = UBFL_CreateBP::CreatePlanesBlueprint(
        //    AssetPath + TEXT("_") + FString::FromInt(MeshIndex), MeshData.CenterPoints, PlaneMesh
        //);
    }
}

UBlueprint* UBFL_CreateBP::CreatePlanesBlueprint(const FString& AssetPath, const TArray<FVector>& CenterPoints, UStaticMesh* PlaneMesh)
{
#if WITH_EDITOR

    if (!PlaneMesh) {
        UE_LOG(LogTemp, Error, TEXT("CreatePlanesBlueprintAsset: PlaneMesh is null"));
        return nullptr;
    }

    if (AssetPath.IsEmpty()) {
        UE_LOG(LogTemp, Error, TEXT("CreatePlanesBlueprintAsset: AssetPath is empty"));
        return nullptr;
    }

    FString PackageName = AssetPath;
    if (!FPackageName::IsValidLongPackageName(PackageName)) {
        UE_LOG(LogTemp, Error, TEXT("Invalid package name: %s"), *PackageName);
        return nullptr;
    }

    const FString AssetName = FPackageName::GetLongPackageAssetName(PackageName);
    if (AssetName.IsEmpty()) {
        UE_LOG(LogTemp, Error, TEXT("Failed to get asset name from %s"), *PackageName);
        return nullptr;
    }

    // 1. 如果有旧资产，先卸载/销毁，避免部分加载
    UObject* ExistingObj = StaticFindObject(UObject::StaticClass(), /*Outer=*/nullptr, *PackageName);
    if (ExistingObj) {
        UPackage* ExistingPackage = ExistingObj->GetOutermost();
        if (ExistingPackage && ExistingPackage->IsFullyLoaded() == false) {
            // 强制完全加载一次，避免 partial
            ExistingPackage->FullyLoad();
        }

        // 这里简单粗暴：删除旧资产（你也可以选择在原资产上修改 SCS）
        ExistingObj->ClearFlags(RF_Public | RF_Standalone);
        ExistingObj->MarkAsGarbage();
    }

    // 2. 创建新的 Package
    UPackage* Package = CreatePackage(*PackageName);
    if (!Package) {
        UE_LOG(LogTemp, Error, TEXT("Failed to create package %s"), *PackageName);
        return nullptr;
    }
    Package->FullyLoad(); // 确保是 fully loaded

    // 3. 创建 Actor 蓝图
    UBlueprint* NewBP = FKismetEditorUtilities::CreateBlueprint(
        AActor::StaticClass(),
        Package,
        *AssetName,
        EBlueprintType::BPTYPE_Normal,
        UBlueprint::StaticClass(),
        UBlueprintGeneratedClass::StaticClass(),
        FName("MyPlaneBlueprintCreation")
    );

    if (!NewBP || !NewBP->GeneratedClass) {
        UE_LOG(LogTemp, Error, TEXT("CreateBlueprint failed for %s"), *AssetName);
        return nullptr;
    }

    // 4. 用 SCS 创建组件模板（让蓝图有正常组件，可以拖进关卡）
    USimpleConstructionScript* SCS = NewBP->SimpleConstructionScript;
    if (!SCS) {
        UE_LOG(LogTemp, Error, TEXT("Blueprint has no SimpleConstructionScript: %s"), *AssetName);
        return nullptr;
    }

    // 4.1 Root 节点
    USCS_Node* RootNode = nullptr;
    const TArray<USCS_Node*>& RootNodes = SCS->GetRootNodes();
    if (RootNodes.Num() == 0) {
        RootNode = SCS->CreateNode(USceneComponent::StaticClass(), FName(TEXT("DefaultSceneRoot")));
        SCS->AddNode(RootNode);

        if (USceneComponent* RootTemplate = Cast<USceneComponent>(RootNode->ComponentTemplate)) {
            RootTemplate->SetMobility(EComponentMobility::Static);
        }
    }
    else {
        RootNode = RootNodes[0];
    }

    // 4.2 清理旧的子节点（如果你打算复用同名 BP 的话）
    {
        TArray<USCS_Node*> Children = RootNode->ChildNodes;
        for (USCS_Node* Child : Children) {
            SCS->RemoveNodeAndPromoteChildren(Child);
        }
    }

    // 4.3 为每个 CenterPoint 创建一个 StaticMeshComponent 节点
    for (int32 i = 0; i < CenterPoints.Num(); ++i) {
        const FVector& Loc = CenterPoints[i];
        const FString CompName = FString::Printf(TEXT("Plane_%d"), i);

        USCS_Node* PlaneNode = SCS->CreateNode(UStaticMeshComponent::StaticClass(), *CompName);
        RootNode->AddChildNode(PlaneNode);

        if (UStaticMeshComponent* PlaneTemplate = Cast<UStaticMeshComponent>(PlaneNode->ComponentTemplate)) {
            PlaneTemplate->SetStaticMesh(PlaneMesh);
            PlaneTemplate->SetMobility(EComponentMobility::Static);
            PlaneTemplate->SetRelativeLocation(Loc);
        }
    }

    // 5. 编译蓝图，确保 Class/CDO 完整
    FKismetEditorUtilities::CompileBlueprint(NewBP);

    // 6. 标记 & 通知 AssetRegistry
    NewBP->MarkPackageDirty();
    Package->MarkPackageDirty();

    FAssetRegistryModule& AssetRegistryModule =
        FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
    AssetRegistry.AssetCreated(NewBP);

    // 可选：触发编辑器刷新
    NewBP->PostEditChange();
    NewBP->MarkPackageDirty();

    UE_LOG(LogTemp, Log, TEXT("CreatePlanesBlueprintAsset: Created %s with %d planes."),
        *NewBP->GetPathName(), CenterPoints.Num());

    return NewBP;

#else
    UE_LOG(LogTemp, Warning, TEXT("CreatePlanesBlueprintAsset can only be used in Editor."));
    return nullptr;
#endif
}
