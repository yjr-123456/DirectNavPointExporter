#include "DirectNavPointSampler.h"

#include "DirectNavPointExporterRuntime.h"
#include "Detour/DetourStatus.h"
#include "NavMesh/RecastNavMesh.h"
#include "NavigationData.h"
#include "NavigationSystem.h"

FDirectNavPointSampler::FDirectNavPointSampler()
	: NavMesh(nullptr)
	, NavQuery(nullptr)
	, QueryFilter(nullptr)
	, CachedBounds(ForceInit)
	, bGroundRegionComputed(false)
{
}

FDirectNavPointSampler::~FDirectNavPointSampler()
{
	if (NavQuery)
	{
		dtFreeNavMeshQuery(NavQuery);
		NavQuery = nullptr;
	}

	if (QueryFilter)
	{
		delete QueryFilter;
		QueryFilter = nullptr;
	}

	NavMesh = nullptr;
}

bool FDirectNavPointSampler::Initialize(UWorld* InWorld)
{
	if (!InWorld)
	{
		UE_LOG(LogDirectNavPointExporter, Error, TEXT("Initialize failed: invalid world"));
		return false;
	}

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(InWorld);
	if (!NavSys)
	{
		UE_LOG(LogDirectNavPointExporter, Error, TEXT("Initialize failed: no navigation system"));
		return false;
	}

	ANavigationData* NavData = NavSys->GetDefaultNavDataInstance(FNavigationSystem::ECreateIfEmpty::DontCreate);
	ARecastNavMesh* RecastNavMesh = Cast<ARecastNavMesh>(NavData);
	if (!RecastNavMesh)
	{
		UE_LOG(LogDirectNavPointExporter, Error, TEXT("Initialize failed: no RecastNavMesh"));
		return false;
	}

	NavMesh = RecastNavMesh->GetRecastMesh();
	if (!NavMesh)
	{
		UE_LOG(LogDirectNavPointExporter, Error, TEXT("Initialize failed: RecastNavMesh has no dtNavMesh"));
		return false;
	}

	NavQuery = dtAllocNavMeshQuery();
	if (!NavQuery)
	{
		UE_LOG(LogDirectNavPointExporter, Error, TEXT("Initialize failed: could not allocate NavMeshQuery"));
		return false;
	}

#ifdef USE_DETOUR_BUILT_INTO_UE4
	dtQuerySpecialLinkFilter LinkFilter;
	const dtStatus Status = NavQuery->init(const_cast<dtNavMesh*>(NavMesh), 0, &LinkFilter);
#else
	const dtStatus Status = NavQuery->init(const_cast<dtNavMesh*>(NavMesh), 0);
#endif
	if (dtStatusFailed(Status))
	{
		UE_LOG(LogDirectNavPointExporter, Error, TEXT("Initialize failed: could not initialize NavMeshQuery"));
		return false;
	}

	QueryFilter = new dtQueryFilter();
	if (!QueryFilter)
	{
		UE_LOG(LogDirectNavPointExporter, Error, TEXT("Initialize failed: could not allocate QueryFilter"));
		return false;
	}

	CalculateBounds();
	FindLargestConnectedRegion();

	return true;
}

FBox FDirectNavPointSampler::GetNavMeshBounds() const
{
	return CachedBounds;
}

bool FDirectNavPointSampler::IsValidPoint(const FVector& Point, const FVector& Extent) const
{
	if (!IsValid())
	{
		return false;
	}

	dtReal RecastPoint[3];
	dtReal RecastExtent[3];
	UnrealToRecast(Point, RecastPoint);

	const FVector AbsExtent = Extent.GetAbs();
	RecastExtent[0] = AbsExtent.X;
	RecastExtent[1] = AbsExtent.Z;
	RecastExtent[2] = AbsExtent.Y;

	dtPolyRef PolyRef = 0;
	dtReal NearestPoint[3] = { 0.0f, 0.0f, 0.0f };
	const dtStatus Status = NavQuery->findNearestPoly(RecastPoint, RecastExtent, QueryFilter, &PolyRef, NearestPoint);

	return dtStatusSucceed(Status) && PolyRef != 0;
}

bool FDirectNavPointSampler::SampleGridPoints(const FDirectNavSamplerConfig& Config, FDirectNavSamplingResult& OutResult)
{
	if (!CachedBounds.IsValid)
	{
		UE_LOG(LogDirectNavPointExporter, Error, TEXT("SampleGridPoints failed: invalid NavMesh bounds"));
		return false;
	}

	return SampleGridPointsInRegion(CachedBounds, Config, OutResult);
}

bool FDirectNavPointSampler::SampleGridPointsInRegion(const FBox& Region, const FDirectNavSamplerConfig& Config, FDirectNavSamplingResult& OutResult)
{
	OutResult.Reset();

	if (!IsValid())
	{
		UE_LOG(LogDirectNavPointExporter, Error, TEXT("SampleGridPointsInRegion failed: sampler not initialized"));
		return false;
	}

	if (!Region.IsValid)
	{
		UE_LOG(LogDirectNavPointExporter, Error, TEXT("SampleGridPointsInRegion failed: invalid region"));
		return false;
	}

	const float GridSpacing = FMath::Max(Config.GridSpacing, 1.0f);
	const FVector RegionMin = Region.Min;
	const FVector RegionMax = Region.Max;

	const int32 GridSizeX = FMath::CeilToInt((RegionMax.X - RegionMin.X) / GridSpacing) + 1;
	const int32 GridSizeY = FMath::CeilToInt((RegionMax.Y - RegionMin.Y) / GridSpacing) + 1;

	const float ExtentXY = FMath::Max(Config.ValidationExtent.X, GridSpacing * 0.6f);
	const float ExtentZ = FMath::Abs(CachedBounds.Max.Z - CachedBounds.Min.Z) + 100.0f;
	const float QueryZ = (RegionMin.Z + RegionMax.Z) * 0.5f;

	dtReal RecastExtent[3];
	RecastExtent[0] = ExtentXY;
	RecastExtent[1] = ExtentZ;
	RecastExtent[2] = ExtentXY;

	int32 ValidCount = 0;
	int32 TotalCount = 0;
	int32 ConnectivityFilteredCount = 0;

	for (int32 GridY = 0; GridY < GridSizeY; ++GridY)
	{
		for (int32 GridX = 0; GridX < GridSizeX; ++GridX)
		{
			const FVector UnrealPoint(
				RegionMin.X + GridX * GridSpacing,
				RegionMin.Y + GridY * GridSpacing,
				QueryZ
			);

			dtReal RecastPoint[3];
			UnrealToRecast(UnrealPoint, RecastPoint);

			dtPolyRef PolyRef = 0;
			dtReal NearestPoint[3] = { 0.0f, 0.0f, 0.0f };

			const dtStatus Status = NavQuery->findNearestPoly(RecastPoint, RecastExtent, QueryFilter, &PolyRef, NearestPoint);
			++TotalCount;

			if (!dtStatusSucceed(Status) || PolyRef == 0)
			{
				continue;
			}

			if (!IsInLargestRegion(PolyRef))
			{
				++ConnectivityFilteredCount;
				continue;
			}

			const FVector ResultPoint = RecastToUnreal(NearestPoint);
			const FVector Delta = (ResultPoint - UnrealPoint).GetAbs();

			if (Delta.X <= ExtentXY && Delta.Y <= ExtentXY && Delta.Z <= ExtentZ)
			{
				OutResult.SamplePoints.Add(FDirectNavSamplePoint(ResultPoint, true));
				++ValidCount;
			}
		}
	}

	OutResult.TotalPoints = TotalCount;
	OutResult.ValidPoints = ValidCount;
	OutResult.NavMeshBounds = CachedBounds;
	OutResult.TotalArea = ValidCount * GridSpacing * GridSpacing;

	UE_LOG(
		LogDirectNavPointExporter,
		Log,
		TEXT("Sampling complete. Total: %d, Valid: %d, Filtered by connectivity: %d"),
		TotalCount,
		ValidCount,
		ConnectivityFilteredCount
	);

	return true;
}

FVector FDirectNavPointSampler::RecastToUnreal(const dtReal RecastPoint[3]) const
{
	return FVector(static_cast<float>(-RecastPoint[0]), static_cast<float>(-RecastPoint[2]), static_cast<float>(RecastPoint[1]));
}

void FDirectNavPointSampler::UnrealToRecast(const FVector& UnrealPoint, dtReal RecastPoint[3]) const
{
	RecastPoint[0] = -UnrealPoint.X;
	RecastPoint[1] = UnrealPoint.Z;
	RecastPoint[2] = -UnrealPoint.Y;
}

void FDirectNavPointSampler::CalculateBounds()
{
	CachedBounds = FBox(ForceInit);

	if (!NavMesh)
	{
		return;
	}

	const int32 MaxTiles = NavMesh->getMaxTiles();
	for (int32 TileIndex = 0; TileIndex < MaxTiles; ++TileIndex)
	{
		const dtMeshTile* Tile = NavMesh->getTile(TileIndex);
		if (!Tile || !Tile->header || !Tile->dataSize)
		{
			continue;
		}

		const FVector UnrealMin = RecastToUnreal(Tile->header->bmin);
		const FVector UnrealMax = RecastToUnreal(Tile->header->bmax);
		CachedBounds += UnrealMin;
		CachedBounds += UnrealMax;
	}
}

void FDirectNavPointSampler::FindLargestConnectedRegion()
{
	if (!NavMesh || bGroundRegionComputed)
	{
		return;
	}

	GroundRegionPolys.Empty();

	TSet<dtPolyRef> AllPolys;
	TMap<dtPolyRef, TSet<dtPolyRef>> AdjacencyMap;

	const int32 MaxTiles = NavMesh->getMaxTiles();
	for (int32 TileIndex = 0; TileIndex < MaxTiles; ++TileIndex)
	{
		const dtMeshTile* Tile = NavMesh->getTile(TileIndex);
		if (!Tile || !Tile->header || !Tile->dataSize)
		{
			continue;
		}

		const dtPolyRef PolyRefBase = NavMesh->getPolyRefBase(Tile);
		for (int32 PolyIndex = 0; PolyIndex < Tile->header->polyCount; ++PolyIndex)
		{
			const dtPoly* Poly = &Tile->polys[PolyIndex];
			if (Poly->getType() != DT_POLYTYPE_GROUND)
			{
				continue;
			}

			const dtPolyRef PolyRef = PolyRefBase | static_cast<dtPolyRef>(PolyIndex);
			AllPolys.Add(PolyRef);
			AdjacencyMap.Add(PolyRef, TSet<dtPolyRef>());
		}
	}

	for (int32 TileIndex = 0; TileIndex < MaxTiles; ++TileIndex)
	{
		const dtMeshTile* Tile = NavMesh->getTile(TileIndex);
		if (!Tile || !Tile->header || !Tile->dataSize)
		{
			continue;
		}

		const dtPolyRef PolyRefBase = NavMesh->getPolyRefBase(Tile);
		for (int32 PolyIndex = 0; PolyIndex < Tile->header->polyCount; ++PolyIndex)
		{
			const dtPoly* Poly = &Tile->polys[PolyIndex];
			if (Poly->getType() != DT_POLYTYPE_GROUND)
			{
				continue;
			}

			const dtPolyRef PolyRef = PolyRefBase | static_cast<dtPolyRef>(PolyIndex);
			for (unsigned int LinkIndex = Poly->firstLink; LinkIndex != DT_NULL_LINK; LinkIndex = Tile->links[LinkIndex].next)
			{
				const dtPolyRef NeighborRef = Tile->links[LinkIndex].ref;
				if (AllPolys.Contains(NeighborRef))
				{
					AdjacencyMap[PolyRef].Add(NeighborRef);
				}
			}
		}
	}

	TSet<dtPolyRef> Visited;
	TArray<TArray<dtPolyRef>> Regions;

	for (dtPolyRef StartPoly : AllPolys)
	{
		if (Visited.Contains(StartPoly))
		{
			continue;
		}

		TArray<dtPolyRef> Region;
		TArray<dtPolyRef> Queue;
		Queue.Add(StartPoly);
		Visited.Add(StartPoly);

		while (Queue.Num() > 0)
		{
			const dtPolyRef CurrentPoly = Queue.Pop(EAllowShrinking::No);
			Region.Add(CurrentPoly);

			if (const TSet<dtPolyRef>* Neighbors = AdjacencyMap.Find(CurrentPoly))
			{
				for (dtPolyRef Neighbor : *Neighbors)
				{
					if (!Visited.Contains(Neighbor))
					{
						Visited.Add(Neighbor);
						Queue.Add(Neighbor);
					}
				}
			}
		}

		Regions.Add(Region);
	}

	int32 LargestRegionIndex = INDEX_NONE;
	int32 LargestRegionSize = 0;
	for (int32 RegionIndex = 0; RegionIndex < Regions.Num(); ++RegionIndex)
	{
		const int32 RegionSize = Regions[RegionIndex].Num();
		if (RegionSize > LargestRegionSize)
		{
			LargestRegionSize = RegionSize;
			LargestRegionIndex = RegionIndex;
		}
	}

	if (LargestRegionIndex != INDEX_NONE)
	{
		for (dtPolyRef PolyRef : Regions[LargestRegionIndex])
		{
			GroundRegionPolys.Add(PolyRef);
		}
	}

	bGroundRegionComputed = true;
}

bool FDirectNavPointSampler::IsInLargestRegion(dtPolyRef PolyRef) const
{
	return GroundRegionPolys.Contains(PolyRef);
}
