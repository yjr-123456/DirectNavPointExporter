#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "DirectNavPointExporterTypes.generated.h"

USTRUCT(BlueprintType)
struct DIRECTNAVPOINTEXPORTERRUNTIME_API FDirectNavSamplePoint
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Direct Nav Point Exporter")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Direct Nav Point Exporter")
	bool bIsValid = false;

	FDirectNavSamplePoint() = default;

	FDirectNavSamplePoint(const FVector& InLocation, bool bInIsValid)
		: Location(InLocation)
		, bIsValid(bInIsValid)
	{
	}
};

USTRUCT(BlueprintType)
struct DIRECTNAVPOINTEXPORTERRUNTIME_API FDirectNavSamplerConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	float GridSpacing = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	FVector ValidationExtent = FVector(10.0f, 10.0f, 10.0f);
};

USTRUCT(BlueprintType)
struct DIRECTNAVPOINTEXPORTERRUNTIME_API FDirectNavWorldFilterConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	bool bFilterUnderOverhangs = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	float MinClearHeightAbove = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	float OverhangTraceDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	TEnumAsByte<ECollisionChannel> OverhangTraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	bool bFilterOnObstacles = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	TEnumAsByte<ECollisionChannel> ObstacleTraceChannel = ECC_Visibility;
};

USTRUCT(BlueprintType)
struct DIRECTNAVPOINTEXPORTERRUNTIME_API FDirectNavSamplingResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Direct Nav Point Exporter")
	TArray<FDirectNavSamplePoint> SamplePoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Direct Nav Point Exporter")
	int32 TotalPoints = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Direct Nav Point Exporter")
	int32 ValidPoints = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Direct Nav Point Exporter")
	FBox NavMeshBounds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Direct Nav Point Exporter")
	float TotalArea = 0.0f;

	void Reset()
	{
		SamplePoints.Empty();
		TotalPoints = 0;
		ValidPoints = 0;
		NavMeshBounds = FBox(ForceInit);
		TotalArea = 0.0f;
	}
};

USTRUCT(BlueprintType)
struct DIRECTNAVPOINTEXPORTERRUNTIME_API FDirectNavReachablePointQueryConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	float GridSpacing = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	FVector ValidationExtent = FVector(10.0f, 10.0f, 10.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	bool bApplyWorldFilter = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	bool bFilterUnderOverhangs = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	float MinClearHeightAbove = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	float OverhangTraceDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	TEnumAsByte<ECollisionChannel> OverhangTraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	bool bFilterOnObstacles = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	TEnumAsByte<ECollisionChannel> ObstacleTraceChannel = ECC_Visibility;
};

USTRUCT(BlueprintType)
struct DIRECTNAVPOINTEXPORTERRUNTIME_API FDirectNavCacheStatus
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Direct Nav Point Exporter")
	bool bContextReady = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Direct Nav Point Exporter")
	bool bHasDefaultResult = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Direct Nav Point Exporter")
	FString MapName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Direct Nav Point Exporter")
	int32 CachedQueryCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Direct Nav Point Exporter")
	int32 CachedPointCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Direct Nav Point Exporter")
	int32 ContextVersion = 0;
};

USTRUCT(BlueprintType)
struct DIRECTNAVPOINTEXPORTERRUNTIME_API FDirectNavReachableAreaData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Direct Nav Point Exporter")
	TArray<FVector> CellCenters;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Direct Nav Point Exporter")
	float GridSpacing = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Direct Nav Point Exporter")
	FBox CoveredBounds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Direct Nav Point Exporter")
	int32 CellCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Direct Nav Point Exporter")
	FString MapName;

	void Reset()
	{
		CellCenters.Empty();
		GridSpacing = 100.0f;
		CoveredBounds = FBox(ForceInit);
		CellCount = 0;
		MapName.Reset();
	}
};

USTRUCT(BlueprintType)
struct DIRECTNAVPOINTEXPORTERRUNTIME_API FDirectNavReachableAreaDisplayConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	FLinearColor Color = FLinearColor(0.1f, 0.9f, 0.2f, 0.35f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	float Duration = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	bool bPersistent = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	float HeightOffset = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	float LineThickness = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	int32 MaxCellsToDraw = 5000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	bool bDrawFilledCells = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	bool bDrawCellOutline = false;
};

USTRUCT(BlueprintType)
struct DIRECTNAVPOINTEXPORTERRUNTIME_API FDirectNavRadiusPointQueryConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	FVector Center = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	float Radius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direct Nav Point Exporter")
	bool bUse2DDistance = true;
};
