// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoadSurface.h"
#include "JunctionSurface.generated.h"

class UProceduralMeshComponent;

UENUM(BlueprintType)
enum class ELaneTurningOptioms : uint8
{
	ALL = 0 UMETA(DisplayName = "Any"),
	LEFT = 1  UMETA(DisplayName = "Left Only"),
	LEFTFORWARD = 2     UMETA(DisplayName = "Left and Forward"),
	FORWARD = 3 UMETA(DisplayName = "Forward Only"),
	FORWARDRIGHT = 4 UMETA(DisplayName = "Forward and Right"),
	RIGHT = 5 UMETA(DisplayName = "Right Only"),
};

UENUM(BlueprintType)
enum class ELaneDrivingType : uint8
{
	NONE = 0 UMETA(DisplayName = "None"),
	DRIVING = 1  UMETA(DisplayName = "Driving"),
	SHOULDER = 2     UMETA(DisplayName = "Shoulder"),
	BICYCLE = 3 UMETA(DisplayName = "Bicycle"),
};



//Input Road Lane Data -- should match our data from ARoadSurface 
USTRUCT()
struct FJunctionLaneData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float LaneWidth = 350.0f;
	//UPROPERTY(EditAnywhere)
	//TArray<float> LaneWidthAlongU = { 1.0f,1.0f };
	UPROPERTY(EditAnywhere)
	float LaneFormingStrength = 0.0f;
	UPROPERTY(EditAnywhere)
	float Start = 0.0f;
	UPROPERTY(EditAnywhere)
	float End = 1.0f;
	UPROPERTY(EditAnywhere)
	int LaneType;
	UPROPERTY(EditAnywhere)
	int LaneWidthResoution;
	UPROPERTY(EditAnywhere)
	UMaterialInterface* Material;
	UPROPERTY(EditAnywhere)
	FVector2D UVTiling = FVector2D(1.0f, 1.0f);
	UPROPERTY(EditAnywhere)
	FVector2D UVOffset = FVector2D(0.0f, 0.0f);
	UPROPERTY(EditAnywhere)
	TArray<FLaneMarking> LaneMarkings;
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ELaneTurningOptioms> TurningRule = ELaneTurningOptioms::ALL;
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ELaneDrivingType> RoadType = ELaneDrivingType::NONE;

	FJunctionLaneData()
	{

	}
};


//Use this for marking where a turning lane starts and ends
USTRUCT()
struct FJunctionTurningLanePoint
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	FVector Location;
	UPROPERTY(VisibleAnywhere)
	int JunctionID;
	UPROPERTY(VisibleAnywhere)
	int LaneID;
	UPROPERTY(VisibleAnywhere)
	int LaneDirection; //0 = Forward 1 = Backwards (This is so we can automatically mark the driving direction)
	UPROPERTY(VisibleAnywhere)
	TEnumAsByte<ELaneTurningOptioms> TurningRule = ELaneTurningOptioms::ALL;
	UPROPERTY(VisibleAnywhere)
	TEnumAsByte<ELaneDrivingType> RoadType = ELaneDrivingType::NONE;
	UPROPERTY(VisibleAnywhere)
	FVector ForwardVector;

	FJunctionTurningLanePoint()
	{

	}
};

USTRUCT()
struct FJunctionTurningLane
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	int TurningLaneID;
	UPROPERTY(VisibleAnywhere)
	TArray<FVector> TurningLanePoints;

	FJunctionTurningLane()
	{

	}
};



USTRUCT()
struct FTurningLaneConnections
{
	GENERATED_BODY()

	UPROPERTY()
	FJunctionTurningLanePoint StartPoint;
	UPROPERTY()	
	TArray<FJunctionTurningLanePoint> EndPoints;

	FTurningLaneConnections()
	{

	}

};



USTRUCT()
struct FBezierCornerPoints
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	int CornerID;
	UPROPERTY(VisibleAnywhere)
	TArray<FVector> Position;
	UPROPERTY(VisibleAnywhere)
	TArray<FVector> Normal;
	UPROPERTY(VisibleAnywhere)
	int StartJunctionID;
	UPROPERTY(VisibleAnywhere)
	int EndJunctionID;

	FBezierCornerPoints()
	{
	}

};

USTRUCT() //DEP
struct FJunctionRoadProperties
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int RoadID;
	UPROPERTY(EditAnywhere)
	float LaneCount;
	UPROPERTY(EditAnywhere)
	float RoadWidth;
	UPROPERTY(EditAnywhere)
	float LeftAngle;
	UPROPERTY(EditAnywhere)
	float RightAngle;

	FJunctionRoadProperties()
	{

	}
};



USTRUCT()
struct FJunctionCenterPoints
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;
	UPROPERTY()
	FVector ForwardVector;
	UPROPERTY()
	FVector RightVector;
	UPROPERTY()
	float AngleFromCenter;
	UPROPERTY()
	int JunctionPointID; 
	UPROPERTY()
	int PositionIndex; //-1= Furthest left //0= Centers //1 Furthers Right //2 Total Center

	FJunctionCenterPoints()
	{
		Location = FVector(0, 0, 0);
		ForwardVector = FVector(1, 0, 0);
		RightVector = FVector(0, 1, 0);
		AngleFromCenter = 0.0f;
		JunctionPointID = 0;
		PositionIndex = 0;
	}
};




USTRUCT()
struct FLaneCenterLinePoints
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;
	UPROPERTY()
	FVector ForwardVector;
	UPROPERTY()
	FVector RightVector;

	FLaneCenterLinePoints()
	{
		Location = FVector(0, 0, 0);
		ForwardVector = FVector(1, 0, 0);
		RightVector = FVector(0, 1, 0);
	}

};




//These are Points that come from the Road Manager
//TO DO: Find a way to make our own custom data type we can share between all classes
USTRUCT()
struct FPointsFromRoadManager
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FVector WorldSpacePosition;
	UPROPERTY(EditAnywhere)
	FVector WorldSpaceNormal;
	UPROPERTY(EditAnywhere)
	FVector WorldSpaceRightVector;
	UPROPERTY(EditAnywhere)
	ARoadSurface* ConnectedRoad;

	FPointsFromRoadManager()
	{

	}
};


USTRUCT()
struct FJunctionOrder
{
	GENERATED_BODY()

	UPROPERTY()
	int OriginalJunctionID;
	UPROPERTY()
	float AngleToCenter;

	FJunctionOrder()
	{
	}


};


USTRUCT()
struct FIntersectPoints
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	FVector Location;
	UPROPERTY(VisibleAnywhere)
	int IntersectedPointID;
	UPROPERTY(VisibleAnywhere)
	int JunctionID;
	UPROPERTY(VisibleAnywhere)
	int PointType;
	UPROPERTY(VisibleAnywhere)
	float IntersectDistance;
	UPROPERTY(VisibleAnywhere)
	FVector ForwardVector;

	FIntersectPoints()
	{
	}

};


USTRUCT()
struct FJunctionCapCornerPoints
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	TArray<FVector> Location;
	UPROPERTY(VisibleAnywhere)
	TArray<int> PointID;
	UPROPERTY(VisibleAnywhere)
	TArray<int> JunctionIDs; //Original Junction IDs

	FJunctionCapCornerPoints()
	{

	}
};


//Struct to pass data from Lane Road Ends to the Junction Center Geo
USTRUCT()
struct FCapPoints
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	FVector Location;
	UPROPERTY(VisibleAnywhere)
	int JunctionID;
	UPROPERTY(VisibleAnywhere)
	int PointType; //-1 Left Most Point 0 = lane point 1 = Right Point 2 = Center Point
	UPROPERTY(VisibleAnywhere)
	FVector ForwardVector;
	UPROPERTY(VisibleAnywhere)
	FVector RightVector;
	UPROPERTY(VisibleAnywhere)
	float AngleFromCenter;
	UPROPERTY(VisibleAnywhere)
	int PointID;
	UPROPERTY(VisibleAnywhere)
	float OffsetDistance;
	UPROPERTY(VisibleAnywhere)
	float UValue;
	UPROPERTY()
	float Offset = 0.0;
	UPROPERTY()
	int LaneDirection = 0; //Write data into here for the direction of the lane - since we use this for turning lane points 
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ELaneTurningOptioms> TurningRule = ELaneTurningOptioms::ALL;
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ELaneDrivingType> RoadType = ELaneDrivingType::NONE;

	FCapPoints()
	{

	}
};

//This is a struct to hold an array of points. This struct will exist in another array, which is indexed by junction
//This is so we can easily sort through points via junction ID without needing to do lots of sorting/conditionals
USTRUCT()
struct FJunctionIDPoints
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	TArray<FCapPoints> Points;

	FJunctionIDPoints()
	{

	}

};


USTRUCT()
struct FJunctionPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int RoadID;
	UPROPERTY(EditAnywhere, Meta = (MakeEditWidget = true))
	FVector Location;
	UPROPERTY(EditAnywhere)
	FVector EndLocation;
	UPROPERTY(EditAnywhere)
	int JunctionType; //0 is passive 1 = dominant
	UPROPERTY(EditAnywhere)
	TArray<FJunctionLaneData> LeftLanes;
	UPROPERTY(EditAnywhere)
	TArray<FJunctionLaneData> RightLanes;
	UPROPERTY(EditAnywhere)
	float AngleFromCenter;
	UPROPERTY(EditAnywhere)
	int Index;
	UPROPERTY(EditAnywhere)
	float ULength = 0.5f;
	UPROPERTY(EditAnywhere)
	int UResolution = 50;


	UPROPERTY(EditAnywhere)
	FVector ForwardVector;
	UPROPERTY(EditAnywhere)
	FVector RightVector;
	TArray<FLaneCenterLinePoints> CenterLinePoints;




	FJunctionPoint()
	{
		RoadID = int(0);
		Location = FVector(0, 0, 0);
		JunctionType = int(0);
		AngleFromCenter = 0.0f;
		Index = int(0);
	}

	FORCEINLINE bool operator()(const FJunctionPoint& A, const FJunctionPoint& B) const
	{
		return A.AngleFromCenter > B.AngleFromCenter;
	}

};

//Store the data about each of our junction parts so we can use this data to generate the mesh later
//and easily expose how we want to control the visual look of the junction
USTRUCT()
struct FJunctionData
{
	GENERATED_BODY()

	UPROPERTY()
	int SectionCount;
	int SectionID;
	TArray<FVector> PointPosition;
	TArray<int> PointID;
	int JunctionType; //0 - Stop Section 1 - Pass Through
	FVector SectionCenterPosition;


	FJunctionData()
	{
		SectionCount = int(0);
		SectionID = int(0);
		JunctionType = int(0);
		SectionCenterPosition = FVector(0, 0, 0);
	}

};



UCLASS()
class ROADTOOLS_API AJunctionSurface : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AJunctionSurface();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//User Input Data
	UPROPERTY(EditAnywhere, Meta = (MakeEditWidget = true))
	TArray<FVector> JunctionEndPoints;
	UPROPERTY(EditAnywhere, Meta = (MakeEditWidget = true))
	FVector JunctionCenter;

	//Preset Vars
	TArray<FColor> JunctionColorCodes = { FColor::Red, FColor::Green, FColor::Blue, FColor::Yellow, FColor::Turquoise, FColor::Orange, FColor::Magenta, FColor::Emerald, FColor::White, FColor::Black, FColor::Purple, FColor::Silver, FColor::Yellow };

	//Functions
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION()
	void GenerateJunctionPoints();
	UFUNCTION()
	void DrawRoadPoints();
	void DrawJunctionShape();
	void DrawJunctionVolume();

	//ManualEditMode
	void ManualEditDrawJunctionShape();
	UFUNCTION()
	void ManualEditInitialiseJunction();
	UFUNCTION()
	void ManualEditCreateLaneBoundaries(TArray<FJunctionLaneData> Lanes, TArray<FLaneCenterLinePoints> CenterLinePoints);
	UFUNCTION()
	void ManualEditCreateLaneVertices(int index, int PointCount, int LaneCount, float LaneLength, FVector2D UVTiling, FVector2D UVOffset);
	UFUNCTION()
	void ManualEditCreateLaneMarkingVertices(int index, int PointCount, int LaneCount, float LaneLength, FVector2D UVTiling, FVector2D UVOffset, float MarkingWidth, float MarkingOffset);
	UFUNCTION()
	void ManualEditCreateLaneTriangles(int PointCount, bool ReverseWinding);
	UFUNCTION()
	TArray<int>ReturnTriangleIndicesGrid(TArray<FVector> Vertices, int ColumnCount, bool ReverseWindingOrder);
	UFUNCTION()
	TArray<int>ReturnTriangleIndicesFan(TArray<FVector> Vertices, int RootPointIndex, bool ReverseWindingOrder);
	UFUNCTION()
	TArray<FLaneCenterLinePoints> ManualEditCreateCenterLine(FJunctionPoint JunctionPoint);
	UFUNCTION()
	void ManualEditBuildCenterGeo();
	UFUNCTION()
	void ManualEditPointsAngleFromCenter();
	UFUNCTION()
	void ManualEditFindIntersectPoints();
	UFUNCTION()
	FVector LineIntersection(FVector PointAStart, FVector PointAEnd, FVector PointBStart, FVector PointBEnd);
	//UFUNCTION()
	//void CreateLaneMarkings();
	UFUNCTION()
	FVector GenerateJunctionBounds();
	UFUNCTION()
	void ManualEditBuildGenterMarkings();
	UFUNCTION()
	void CreateTurningLanePoints();

	UFUNCTION()
	void DrawVertices(TArray<FVector>Vertices);


	//Generated Var
	//TArray<FJunctionPoint> JunctionPoints;

	//Points to Desribe where on the RoadCurve does the junction Start
	UPROPERTY(EditAnywhere, Meta = (MakeEditWidget = true))
	TArray<FVector> JunctionCenterPoints;
	//Points to describe where in worldspace does the Volume of the junction exist
	UPROPERTY(EditAnywhere)
	TArray<FVector> JunctionVolumePoints;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere)
	UProceduralMeshComponent* JunctionSurface;
	UPROPERTY(EditAnywhere)
	UProceduralMeshComponent* JunctionCenterSurface;
	UPROPERTY(EditAnywhere)
	UProceduralMeshComponent* LaneMarkingsSurface;

public:


	//User Editor Properties (Manual Creation Mode)
	UPROPERTY(EditAnywhere)
	bool ManualEditMode = false;
	UPROPERTY(EditAnywhere)
	bool LaneMarkings = false;
	UPROPERTY(EditAnywhere)
	bool DrawLaneConnections = false;
	UPROPERTY(EditAnywhere)
	int DebugLaneConnectionID = 0;

	UPROPERTY(EditAnywhere)
	TArray<FJunctionPoint> JunctionInputRoads;
	UPROPERTY()
	FVector JunctionWorldLocation = FVector(0,0,0);
	UPROPERTY(EditAnywhere)
	TArray<FVector> DebugPointPositions;
	TArray<FVector> LaneBoundaryPositions;
	TArray<FVector> VertexPositions;
	TArray<int> TriangleIndices;
	TArray<FVector2D> UVs;
	TArray<FVector> LaneMarkingVertices;
	TArray<FJunctionCenterPoints> JunctionCenterBoundaryPoints; //Dep
	TArray<FCapPoints> JunctionCapPoints;
	UPROPERTY(VisibleAnywhere)
	TArray<FIntersectPoints> FoundIntersectPoints;
	UPROPERTY(VisibleAnywhere)
	TArray<FJunctionIDPoints> JunctionIDSortedPoints;
	UPROPERTY(VisibleAnywhere)
	TArray<FJunctionCapCornerPoints> IntersectCornerPoints; //the three points which make up the 'triangle' for the junction corner piece
	UPROPERTY(EditAnywhere)
	TArray<FBezierCornerPoints> BezierEdgePoints;
	FVector JunctionBoundingBox;
	UPROPERTY(VisibleAnywhere)
	TArray<FJunctionTurningLanePoint> TurningLanePoints;
	UPROPERTY(VisibleAnywhere)
	TArray<FTurningLaneConnections> TurningLaneConnections;
	UPROPERTY(VisibleAnywhere)
	TArray<FCapPoints> JunctionCenterLineEndPoint;
	UPROPERTY(VisibleAnywhere)
	TArray<FJunctionTurningLane> TurningLanes; //Should we keep a reference to our ALaneSpline here?


	UPROPERTY(EditAnywhere)
	UMaterialInterface* JunctionSurfaceMaterial;

	//Properties That are Set by RoadManager
	UPROPERTY(EditAnywhere)
	int RoadCount;
	UPROPERTY(EditAnywhere)
	int JunctionID;
	UPROPERTY(EditAnywhere)
	FVector Location;
	UPROPERTY(EditAnywhere)
	TArray<ARoadSurface*> ConnectedRoads;
	UPROPERTY(EditAnywhere)
	TArray<int> ConnectedRoadPoints;
	UPROPERTY(EditAnywhere)
	TArray<FJunctionPoint> JunctionPoints;
	UPROPERTY(EditAnywhere)
	TArray<FPointsFromRoadManager> RoadManagerPoints;

	UPROPERTY(EditAnywhere)
	int VertexIndexTest = 0;


	UFUNCTION()
	void BuildJunction();

	UPROPERTY(EditAnywhere)
	TArray<FJunctionRoadProperties> JunctionRoads;

};
