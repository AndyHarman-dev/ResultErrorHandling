// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#pragma once

#include "CoreMinimal.h"
#include "Templates/UnrealTemplate.h"
#include "Misc/Optional.h"

// Forward declarations
template<typename T, typename E>
class TResult;

namespace ResultHelpers
{
    struct OkTag {};
    struct ErrTag {};
    
    constexpr OkTag Ok{};
    constexpr ErrTag Err{};
}

template<typename TValueType>
class TSimpleResult
{
public:
    TSimpleResult(const ResultHelpers::OkTag&, const TValueType& Value) : Value(Value), bIsOk(true) {}
    TSimpleResult(const ResultHelpers::OkTag&, TValueType&& Value) : Value(MoveTemp(Value)), bIsOk(true) {}
    TSimpleResult(const ResultHelpers::ErrTag&) : bIsOk(false){}
    TSimpleResult(const TSimpleResult& Other)
    {
        Value = Other.Value;
        bIsOk = Other.bIsOk;
    }
    TSimpleResult(TSimpleResult&& Other)
    {
        Value = Other.Value;
        bIsOk = Other.bIsOk;
        Other.bIsOk = false;
        Other.Value.Reset();
    }
    
    TSimpleResult& operator=(const TSimpleResult& Other)
    {
        if (this != &Other)
        {
            this->~TResult();
            new(this) TResult(Other);
        }
        return *this;
    }
    
    TSimpleResult& operator=(TSimpleResult&& Other)
    {
        if (this != &Other)
        {
            this->~TResult();
            new(this) TResult(MoveTemp(Other));
        }
        return *this;
    }

    bool IsOk() const { return bIsOk; }
    bool IsErr() const { return !bIsOk; }
    template<typename Predicate>
    bool IsOkAnd(Predicate&& Pred) const
    {
        return bIsOk && Pred(Value.GetValue());
    }
    
    const TValueType& Expect(const TCHAR* Message) const
    {
        if (!IsOk())
        {
            UE_LOG(LogTemp, Fatal, TEXT("Result::Expect failed: %s"), Message);
        }
        return Value.GetValue();
    }

    const TValueType& Unwrap() const
    {
        if (!bIsOk)
        {
            UE_LOG(LogTemp, Fatal, TEXT("Called Unwrap on an Err Result"));
        }
        return Value.GetValue();
    }
    TValueType UnwrapOr(const TValueType& DefaultValue) const
    {
        return Value.Get(DefaultValue);
    }
protected:
    TOptional<TValueType> Value;
    bool bIsOk;
};

/**
 * A C++ implementation similar to Rust's Result<T, E> for Unreal Engine
 * Represents either a successful value (Ok) or an error (Err)
 */
template<typename TValueType, typename TErrorType>
class TResult : public TSimpleResult<TValueType>
{
    using Super = TSimpleResult<TValueType>;
public:
    using ValueType = TValueType;
    using ErrorType = TErrorType;

    using Super::IsOk;
    using Super::Value;

    TResult(const ResultHelpers::OkTag&, const TValueType& Value) : Super(ResultHelpers::Ok, Value) {}
    TResult(const ResultHelpers::OkTag&, TValueType&& Value) : Super(ResultHelpers::Ok, MoveTemp(Value)) {}
    TResult(const ResultHelpers::ErrTag&, const TErrorType& Error) : Super(ResultHelpers::Err), Error(Error) {}
    TResult(const ResultHelpers::ErrTag&, TErrorType&& Error) : Super(ResultHelpers::Err), Error(MoveTemp(Error)) {}
    TResult(const TResult& Other) : Super(Other)
    {
        if (Other.IsErr())
        {
            Error = Other.Error;
        }
    }
    TResult(TResult&& Other) : Super(Other)
    {
       if (Other.IsErr())
       {
           Error = MoveTemp(Other.Error);
           Other.Error.Reset();
       }
    }

    // Assignment operators
    TResult& operator=(const TResult& Other)
    {
        if (this != &Other)
        {
            this->~TResult();
            new(this) TResult(Other);
        }
        return *this;
    }
    TResult& operator=(TResult&& Other) noexcept
    {
        if (this != &Other)
        {
            this->~TResult();
            new(this) TResult(MoveTemp(Other));
        }
        return *this;
    }

    template<typename Predicate>
    bool IsErrAnd(Predicate&& Pred) const
    {
        return !IsOk() && Pred(Error.GetValue());
    }

    const TErrorType& UnwrapErr() const
    {
        if (IsOk())
        {
            UE_LOG(LogTemp, Fatal, TEXT("Called UnwrapErr on an Ok Result"));
        }
        return Error.GetValue();
    }
    template<typename TFunctor>
    TValueType UnwrapOrElse(TFunctor&& Func) const
    {
        return IsOk() ? Value.GetValue() : Func(Error.GetValue());
    }
    
    const TErrorType& ExpectErr(const TCHAR* Message) const
    {
        if (IsOk())
        {
            UE_LOG(LogTemp, Fatal, TEXT("Result::ExpectErr failed: %s"), Message);
        }
        return Error.GetValue();
    }

    template<typename TFunctor>
    TResult<TInvokeResult_T<TFunctor, TValueType>, TErrorType> Map(TFunctor&& Func) const
    {
        if (IsOk())
        {
            return TResult<TInvokeResult_T<TFunctor, TValueType>, TErrorType>(ResultHelpers::Ok, Func(Value.GetValue()));
        }
        else
        {
            return TResult<TInvokeResult_T<TFunctor, TValueType>, TErrorType>(ResultHelpers::Err, Error.GetValue());
        }
    }

    template<typename TFunctor>
    TResult<TValueType, TInvokeResult_T<TFunctor, TErrorType>> MapErr(TFunctor&& Func) const
    {
        if (IsOk())
        {
            return TResult<TValueType, TInvokeResult_T<TFunctor, TErrorType>>(ResultHelpers::Ok, Value.GetValue());
        }
        else
        {
            return TResult<TValueType, TInvokeResult_T<TFunctor, TErrorType>>(ResultHelpers::Err, Func(Error.GetValue()));
        }
    }

    template<typename TFunctor>
    TResult<typename TInvokeResult_T<TFunctor, TValueType>::ValueType, TErrorType> AndThen(TFunctor&& Func) const
    {
        if (IsOk())
        {
            return Func(Value.GetValue());
        }
        else
        {
            return TResult<typename TInvokeResult_T<TFunctor, TValueType>::ValueType, TErrorType>(ResultHelpers::Err, Error.GetValue());
        }
    }

    template<typename TFunctor>
    TResult<TValueType, typename TInvokeResult_T<TFunctor, TErrorType>::ErrorType> OrElse(TFunctor&& Func) const
    {
        if (IsOk())
        {
            return TResult<TValueType, typename TInvokeResult_T<TFunctor, TErrorType>::ErrorType>(ResultHelpers::Ok, Value.GetValue());
        }
        else
        {
            return Func(Error.GetValue());
        }
    }

    template<typename TOtherValue>
    TResult<TOtherValue, TErrorType> And(const TResult<TOtherValue, TErrorType>& Other) const
    {
        return IsOk() ? Other : TResult<TOtherValue, TErrorType>(ResultHelpers::Err, Error.GetValue());
    }
    template<typename NewE>
    TResult<TValueType, NewE> Or(const TResult<TValueType, NewE>& Other) const
    {
        return IsOk() ? TResult<TValueType, NewE>(ResultHelpers::Ok, Value.GetValue()) : Other;
    }

    template<typename TFunctor>
    const TResult& Inspect(TFunctor&& Func) const
    {
        if (IsOk())
        {
            Func(Value.GetValue());
        }
        return *this;
    }
    template<typename TFunctor>
    const TResult& InspectErr(TFunctor&& Func) const
    {
        if (!IsOk())
        {
            Func(Error.GetValue());
        }
        return *this;
    }

    bool operator==(const TResult& Other) const
    {
        if (IsOk() != Other.IsOk()) return false;
        return IsOk() ? (Value == Other.Value) : (Error == Other.Error);
    }
    bool operator!=(const TResult& Other) const
    {
        return !(*this == Other);
    }

private:
    TOptional<TErrorType> Error;
};

