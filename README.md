# TResult - Rust-Style Error Handling for Unreal Engine

A type-safe error handling library for Unreal Engine C++ inspired by Rust's `Result<T, E>` type. This implementation provides a robust alternative to exceptions, making error paths explicit and easier to reason about.  

## Features

- **Type-safe error propagation** - Compile-time guarantees that errors are handled 
- **Rust-inspired API** - Familiar methods like `Unwrap()`, `Map()`, `AndThen()`, and `OrElse()` 
- **Zero runtime exceptions** - All error handling is explicit and predictable 
- **Functional composition** - Chain operations with monadic transformations 
- **Unreal Engine integration** - Built on `TOptional` and UE conventions 
- **Move semantics support** - Efficient value handling with C++11/14 features 
- **Comprehensive test coverage** - Extensive unit tests demonstrating all functionality 

## Tech Stack

- **Language**: C++11/14 
- **Engine**: Unreal Engine 4/5 
- **Dependencies**: CoreMinimal, Templates/UnrealTemplate, Misc/Optional 
- **Testing**: Unreal Engine Automation Testing Framework 

## Installation

### Prerequisites

- Unreal Engine 4.27+ or Unreal Engine 5.x 
- C++11 or later compiler 
- Visual Studio 2019/2022 (Windows) or Xcode (macOS) 

### Setup Steps

1. Copy `Result.h` to your project's source directory (e.g., `YourProject/Source/YourProject/ResultType/`) 

2. Include the header in your C++ files:

```cpp
#include "ResultType/Result.h"
```

3. Rebuild your project to compile the new header 

## Usage

### Basic Construction

Create successful or error results using tag-based constructors : 

```cpp
// Success case
TResult<int32, FString> OkResult(ResultHelpers::Ok, 42);

// Error case
TResult<int32, FString> ErrResult(ResultHelpers::Err, TEXT("Error message"));
```

### Factory Functions

Use convenience factory functions for cleaner syntax : 

```cpp
auto FailableOperation = [](bool bShouldFail) -> TResult<int32, FString>
{
    if (bShouldFail)
    {
        return Error(FString("Operation failed"));
    }
    return Ok(5);
};

auto Result = FailableOperation(false);
if (Result.IsOk())
{
    int32 Value = Result.Unwrap(); // Value = 5
}
```

### Query Methods

Check result state with safe query methods : 

```cpp
TResult<int32, FString> Result(ResultHelpers::Ok, 10);

if (Result.IsOk())
{
    UE_LOG(LogTemp, Log, TEXT("Success!"));
}

// Conditional queries with predicates
bool IsLargeValue = Result.IsOkAnd([](int32 Val) { return Val > 5; });
bool HasSpecificError = ErrorResult.IsErrAnd([](const FString& Err) { 
    return Err.Contains(TEXT("Fatal")); 
});
```

### Unwrapping Values

Extract values from results with various unwrap methods : 

```cpp
TResult<int32, FString> Result(ResultHelpers::Ok, 42);

// Direct unwrap (crashes if Err)
int32 Value = Result.Unwrap();

// Unwrap with custom message
int32 Value2 = Result.Expect(TEXT("Expected a valid value"));

// Unwrap with default fallback
int32 Value3 = Result.UnwrapOr(999);

// Unwrap with computed fallback
int32 Value4 = Result.UnwrapOrElse([](const FString& Err) { 
    return Err.Len(); 
});
```

### Transformations

Transform results using functional composition patterns : 

```cpp
TResult<int32, FString> Original(ResultHelpers::Ok, 5);

// Map transforms the Ok value
auto Doubled = Original.Map([](int32 Val) { return Val * 2; });
// Doubled contains Ok(10)

// MapErr transforms the Err value
auto MappedError = ErrorResult.MapErr([](const FString& Err) { 
    return Err + TEXT(" - Enhanced"); 
});

// AndThen chains operations that return Results
auto Chained = Original.AndThen([](int32 Val) {
    return TResult<int32, FString>(ResultHelpers::Ok, Val * 2);
});

// OrElse provides fallback for errors
auto Recovered = ErrorResult.OrElse([](const FString& Err) {
    return TResult<int32, FString>(ResultHelpers::Ok, 42);
});
```

### Boolean Operators

Combine results using logical operators : 

```cpp
TResult<int32, FString> Ok1(ResultHelpers::Ok, 1);
TResult<int32, FString> Ok2(ResultHelpers::Ok, 2);
TResult<int32, FString> Err1(ResultHelpers::Err, TEXT("Error"));

// And - returns second result if first is Ok
auto Result1 = Ok1.And(Ok2); // Contains Ok(2)
auto Result2 = Ok1.And(Err1); // Contains Err("Error")

// Or - returns first Ok result, or second if first is Err
auto Result3 = Ok1.Or(Ok2); // Contains Ok(1)
auto Result4 = Err1.Or(Ok1); // Contains Ok(1)
```

### Inspection

Perform side effects without consuming the result : 

```cpp
int32 InspectedValue = 0;

auto Result = TResult<int32, FString>(ResultHelpers::Ok, 42)
    .Inspect([&](int32 Val) { 
        InspectedValue = Val; 
        UE_LOG(LogTemp, Log, TEXT("Got value: %d"), Val);
    })
    .InspectErr([](const FString& Err) {
        UE_LOG(LogTemp, Error, TEXT("Error: %s"), *Err);
    });
```

### Comparison

Compare results for equality : 

```cpp
TResult<int32, FString> A(ResultHelpers::Ok, 42);
TResult<int32, FString> B(ResultHelpers::Ok, 42);
TResult<int32, FString> C(ResultHelpers::Ok, 24);

bool AreEqual = (A == B); // true
bool AreDifferent = (A != C); // true
```

## API Documentation

### Core Types

- **`TResult<TValueType, TErrorType>`** - Primary result type containing either a value or error 
- **`TSimpleResult<TValueType>`** - Base class for results with value-only operations 
- **`ResultHelpers::Ok`** - Tag type for successful construction 
- **`ResultHelpers::Err`** - Tag type for error construction 

### Query Methods

| Method | Description | Example |
|--------|-------------|---------|
| `IsOk()` | Returns true if contains success value | `result.IsOk()`   |
| `IsErr()` | Returns true if contains error | `result.IsErr()`   |
| `IsOkAnd(Pred)` | Checks if Ok and predicate passes | `result.IsOkAnd([](int v) { return v > 0; })`   |
| `IsErrAnd(Pred)` | Checks if Err and predicate passes | `result.IsErrAnd([](auto e) { return e.Len() > 0; })`   |

### Extraction Methods

| Method | Description | Panics On |
|--------|-------------|-----------|
| `Unwrap()` | Extracts Ok value | Err   |
| `UnwrapErr()` | Extracts Err value | Ok   |
| `Expect(Msg)` | Extracts Ok with message | Err   |
| `ExpectErr(Msg)` | Extracts Err with message | Ok   |
| `UnwrapOr(Default)` | Extracts Ok or returns default | Never   |
| `UnwrapOrElse(Fn)` | Extracts Ok or computes fallback | Never   |

### Transformation Methods

| Method | Signature | Description |
|--------|-----------|-------------|
| `Map(Fn)` | `TResult<U, E>` | Transform Ok value   |
| `MapErr(Fn)` | `TResult<T, F>` | Transform Err value   |
| `AndThen(Fn)` | `TResult<U, E>` | Chain fallible operations   |
| `OrElse(Fn)` | `TResult<T, F>` | Provide error recovery   |
| `And(Other)` | `TResult<U, E>` | Logical AND combination   |
| `Or(Other)` | `TResult<T, F>` | Logical OR combination   |

## Contributing

Contributions are welcome! When contributing to this project : 

1. Write comprehensive unit tests for new features following the existing test structure 
2. Ensure all tests pass using Unreal Engine's automation testing framework 
3. Follow Unreal Engine coding standards and naming conventions 
4. Document all public APIs with clear comments 
5. Use the BDD (Given-When-Then) style for test organization 
