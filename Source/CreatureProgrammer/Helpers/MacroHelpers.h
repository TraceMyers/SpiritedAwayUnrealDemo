#pragma once

#include "CoreMinimal.h"
#include <type_traits>
#include "../Generated/Generated.h"
#include "CreatureProgrammer/Multithreading/ThreadingTypes.h"

#define TURN_ON_PROFILERS UE_BUILD_DEBUG | UE_BUILD_DEVELOPMENT
#define PROFILE_FUNCTION() TRACE_CPUPROFILER_EVENT_SCOPE_STR_CONDITIONAL(__FUNCTION__, TURN_ON_PROFILERS)

template<typename T>
struct FScopedPointer
{
	FScopedPointer(T** Field, T* Value)
	{
		check(Value != nullptr);
		*Field = Value;
		FieldPtr = Field;
	}
	
	~FScopedPointer()
	{
		*FieldPtr = nullptr;
	}
	
	T** FieldPtr = nullptr;
};

#define SET_SCOPED_OBJECT_POINTER(Field, Value) \
	check(IsValid(Value)) \
	FScopedPointer __ScopedPointer_ ## Field (&Field, Value);

template<typename T>
struct FScopedObjectPointer : FScopedPointer<T>
{
	FScopedObjectPointer(T** Field, T* Value) : FScopedPointer<T>(Field, Value)
	{
		check(IsValid(Cast<UObject>(Value)));
	}
};

struct FCreateComponentParams
{
	const wchar_t* CollisionProfile = L"";
	bool bCastShadow      = false;
	bool bVisible         = false;
	bool bReceivesDecals  = false;
	bool bSimulatePhysics = false;
	ECollisionEnabled::Type CollisionEnabled = ECollisionEnabled::NoCollision;
	EComponentMobility::Type ComponentMobility = EComponentMobility::Movable;
};

#define CREATE_ROOT_COMPONENT()												\
	if (!GetRootComponent())												\
	{																		\
		SetRootComponent(CreateDefaultSubobject<USceneComponent>(L"Root")); \
	}

#define CREATE_PRIMITIVE_COMPONENT_AS_ROOT(PropName, ...) \
	do {                                                                                                    \
		PropName = CreateDefaultSubobject<std::remove_reference_t<decltype(*PropName)>>(TEXT(#PropName));   \
		SetRootComponent(PropName); \
		const FCreateComponentParams Params = {__VA_ARGS__};														\
		if (*Params.CollisionProfile != L'\0') {															\
			PropName->SetCollisionProfileName(Params.CollisionProfile);										\
		}																									\
		PropName->CastShadow = Params.bCastShadow;															\
		PropName->SetVisibility(Params.bVisible);															\
		PropName->bReceivesDecals = Params.bReceivesDecals;													\
		PropName->SetSimulatePhysics(Params.bSimulatePhysics);												\
		PropName->SetCollisionEnabled(Params.CollisionEnabled);												\
		PropName->SetMobility(Params.ComponentMobility);													\
	} while (0);

// convenience helper for creating components in actor constructors.
// e.g. CREATE_PRIMITIVE_COMPONENT(MyComponentVar, .bVisible=true);
#define CREATE_PRIMITIVE_COMPONENT(PropName, ...) \
	do {                                                                                                    \
		PropName = CreateDefaultSubobject<std::remove_reference_t<decltype(*PropName)>>(TEXT(#PropName));   \
		PropName->SetupAttachment(GetRootComponent());													    \
		const FCreateComponentParams Params = {__VA_ARGS__};														\
		if (*Params.CollisionProfile != L'\0') {															\
			PropName->SetCollisionProfileName(Params.CollisionProfile);										\
		}																									\
		PropName->CastShadow = Params.bCastShadow;															\
		PropName->SetVisibility(Params.bVisible);															\
		PropName->bReceivesDecals = Params.bReceivesDecals;													\
		PropName->SetSimulatePhysics(Params.bSimulatePhysics);												\
		PropName->SetCollisionEnabled(Params.CollisionEnabled);												\
		PropName->SetMobility(Params.ComponentMobility);													\
} while (0);

#define CREATE_SCENE_COMPONENT(PropName, ...) \
	do {                                                                                                    \
		PropName = CreateDefaultSubobject<std::remove_reference_t<decltype(*PropName)>>(TEXT(#PropName));   \
		PropName->SetupAttachment(GetRootComponent());													    \
		const FCreateComponentParams Params = {__VA_ARGS__};														\
		PropName->SetVisibility(Params.bVisible);															\
	} while (0);

#define CREATE_ACTOR_COMPONENT(PropName) \
	do {                                                                                                    \
		PropName = CreateDefaultSubobject<std::remove_reference_t<decltype(*PropName)>>(TEXT(#PropName));   \
	} while (0);

// if VarToNull is valid, do the call. else, nullify the var.
// needing to use this a lot is probably not great, but it's always necessary to some extent.
#define CALL_OR_NULLIFY(Call, VarToNull)	\
	if (IsValid(VarToNull)) {				\
		Call;								\
	}										\
	else									\
	{										\
		VarToNull = nullptr;				\
	}

// I prefer this over storing material pointers. same (small) amount of work, less redundant data.
#define DEFINE_DYNAMIC_MATERIAL_INSTANCE_GETTER(FuncName, MeshVarName, Slot) \
FORCEINLINE UMaterialInstanceDynamic* FuncName() const \
{ \
	return CastChecked<UMaterialInstanceDynamic>(MeshVarName->GetMaterial(Slot)); \
}

#define DEFINE_EXCITEMENT_SCALED_GETTER(MinValue, MaxMultiplier) \
FORCEINLINE double Get ## MinValue() const \
{ \
	return FMath::Lerp((Settings.MinValue), (Settings.MinValue) * (Settings.MaxMultiplier), Excitement); \
}

#define ADD_READ_ONLY_FIELD_PART_AFTER_ACCESS_SPECIFIER(Type, Name) \
	Type Name; \
public:\
	const Type& Get ## Name() { return Name; }

#define ADD_READ_ONLY_FIELD(Type, Name) \
private:\
ADD_READ_ONLY_FIELD_PART_AFTER_ACCESS_SPECIFIER(Type, Name)

#define ADD_READ_ONLY_UPROPERTY(Type, Name, Uproperties) \
private:\
	UPROPERTY(Uproperties) \
ADD_READ_ONLY_FIELD_PART_AFTER_ACCESS_SPECIFIER(Type, Name)

// simple game thread access protector. can optionally allow reading the array num from outside the game thread.
#define DECLARE_GAME_THREAD_ONLY_ARRAY(ElementType, ArrayName, bCanReadNumOutsideGameThread) \
public:	\
	TArray<ElementType>& Get ## ArrayName() \
	{ \
		check(IsInGameThread()) \
		return ArrayName; \
	} \
	const TArray<ElementType>& Get ## ArrayName() const \
	{ \
		check(IsInGameThread()) \
		return ArrayName; \
	} \
	int32 ArrayName ## Num() const {  \
		check(bCanRead ## ArrayName ## NumOutsideGameThread); \
		return ArrayName.Num();  \
	} \
protected: \
	static constexpr bool bCanRead ## ArrayName ## NumOutsideGameThread = bCanReadNumOutsideGameThread; \
	UPROPERTY() \
	TArray<ElementType> ArrayName;