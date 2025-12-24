// Fill out your copyright notice in the Description page of Project Settings.


#include "S004_DT/S_DTReader.h"

// Sets default values
AS_DTReader::AS_DTReader()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AS_DTReader::BeginPlay()
{
	Super::BeginPlay();

	
	// 1: 直接

	//FName _rowname = FName(TEXT("A"));
	//FString ContextString = TEXT("TEXT");
	//if (MyFruitTable) {
	//	FFruit* _data = MyFruitTable->FindRow<FFruit>(_rowname, ContextString, false);
	//	if (_data) {
	//		UE_LOG(LogTemp, Warning, TEXT("Name:%s, Price:%f, Type:%d, Weight:%f, IsFresh:%d"), *_data->Name, _data->Price, _data->Type, _data->Weight, _data->IsFresh);
	//	}
	//	else {
	//		UE_LOG(LogTemp, Warning, TEXT("Not Found DT Row"));
	//	}
	//}
	//else {
	//	UE_LOG(LogTemp, Warning, TEXT("Not Found DT"));
	//}

	// 2: 资源路径
	// 直接对资源右键复制引用就可以获取 TEXT 中的字符串

	//MyFruitTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Script/Engine.DataTable'/Game/Levels/004DataTable/004DT_Fruit.004DT_Fruit'")));
	//FName _rownameB = FName(TEXT("B"));
	//FString ContextString = TEXT("TEXT");
	//if (MyFruitTable) {
	//	FFruit* _data = MyFruitTable->FindRow<FFruit>(_rownameB, ContextString, false);
	//	if (_data) {
	//		UE_LOG(LogTemp, Warning, TEXT("Name:%s, Price:%f, Type:%d, Weight:%f, IsFresh:%d"), *_data->Name, _data->Price, _data->Type, _data->Weight, _data->IsFresh);
	//	}
	//	else {
	//		UE_LOG(LogTemp, Warning, TEXT("Not Found DT Row"));
	//	}
	//}
	//else {
	//	UE_LOG(LogTemp, Warning, TEXT("Not Found DT"));
	//}

	// 3: GetRowNames 获取所有

	//if (MyFruitTable) {
	//	FString ContextString = TEXT("TEXT");
	//	TArray<FName> _rowNames;
	//	_rowNames = MyFruitTable->GetRowNames();
	//	for (auto _rowName : _rowNames) {
	//		FFruit* _data = MyFruitTable->FindRow<FFruit>(_rowName, ContextString, false);
	//		UE_LOG(LogTemp, Warning, TEXT("Name:%s, Price:%f, Type:%d, Weight:%f, IsFresh:%d"), *_data->Name, _data->Price, _data->Type, _data->Weight, _data->IsFresh);
	//	}
	//}
	//else {
	//	UE_LOG(LogTemp, Warning, TEXT("Not Found DT"));
	//}

	// 4: GetAllRows 获取所有

	//if (MyFruitTable) {
	//	FString ContextString = TEXT("TEXT");
	//	TArray<FFruit*> _rows;
	//	MyFruitTable->GetAllRows(ContextString, _rows);
	//	for (auto _row : _rows) {
	//		UE_LOG(LogTemp, Warning, TEXT("Name:%s, Price:%f, Type:%d, Weight:%f, IsFresh:%d"), *_row->Name, _row->Price, _row->Type, _row->Weight, _row->IsFresh);
	//	}
	//}
	//else {
	//	UE_LOG(LogTemp, Warning, TEXT("Not Found DT"));
	//}

	// 5: GetAllRows 获取所有

	//if (MyFruitTable) {

	//	auto _rowmap = MyFruitTable->GetRowMap();
	//	for (auto _row : _rowmap) {
	//		FFruit* _data = (FFruit*)_row.Value;
	//		UE_LOG(LogTemp, Warning, TEXT("Name:%s, Price:%f, Type:%d, Weight:%f, IsFresh:%d"), *_data->Name, _data->Price, _data->Type, _data->Weight, _data->IsFresh);
	//	}
	//}
	//else {
	//	UE_LOG(LogTemp, Warning, TEXT("Not Found DT"));
	//}

	// 6: AddRow 写入

	if (MyFruitTable) {
		FName rowName = FName(TEXT("SSR"));

		FFruit _addData;
		_addData.Name = FString(TEXT("Gold Peach"));
		_addData.Price = 99.9f;
		_addData.Weight = 9.99f;
		_addData.IsFresh = true;
		_addData.Type = EFruitType::Peach;

		MyFruitTable->AddRow(rowName, _addData);

		FString ContextString = TEXT("TEXT");
		TArray<FFruit*> _rows;
		MyFruitTable->GetAllRows(ContextString, _rows);
		for (auto _row : _rows) {
			UE_LOG(LogTemp, Warning, TEXT("Name:%s, Price:%f, Type:%d, Weight:%f, IsFresh:%d"), *_row->Name, _row->Price, _row->Type, _row->Weight, _row->IsFresh);
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Not Found DT"));
	}
}

// Called every frame
void AS_DTReader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

