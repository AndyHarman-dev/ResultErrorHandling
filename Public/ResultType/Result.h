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

    template<typename T, typename E>
    struct FOkOrErrValue
    {
        FOkOrErrValue() = default;
        
        FOkOrErrValue(OkTag, const T& Value)
        {
            SetOkValue(Value);
        }
        
        FOkOrErrValue(OkTag, T&& Value)
        {
            SetOkValue(MoveTemp(Value));
        }

        FOkOrErrValue(ErrTag, const E& Error)
        {
            SetErrValue(Error);
        }

        FOkOrErrValue(ErrTag, E&& Error)
        {
            SetErrValue(MoveTemp(Error));
        }
        
        T& GetOkValue()
        {
            return OKValue;
        }

        E& GetErrValue()
        {
            return ERRValue;
        }

        const T& GetOkValue() const
        {
            return OKValue;
        }

        const E& GetErrValue() const
        {
            return ERRValue;
        }

        void SetOkValue(const T& Value)
        {
            OKValue = Value;
        }

        void SetOkValue(T&& Value)
        {
            OKValue = MoveTemp(Value);
        }

        void SetErrValue(const E& Err)
        {
            ERRValue = Err;
        }

        void SetErrValue(E&& Err)
        {
            ERRValue = MoveTemp(Err);
        }

        void ResetOk()
        {
            OKValue = T();
        }

        void ResetErr()
        {
            ERRValue = E();
        }

    private:

        T OKValue;
        E ERRValue;
    };
}

/**
 * A C++ implementation similar to Rust's Result<T, E> for Unreal Engine
 * Represents either a successful value (Ok) or an error (Err)
 */
template<typename T, typename E>
class RESULTERRORHANDLINGTYPE_API TResult
{
private:
    bool bIsOk;

    ResultHelpers::FOkOrErrValue<T, E> OkOrErrValue;

#define OK_VALUE OkOrErrValue.GetOkValue()
#define ERR_VALUE OkOrErrValue.GetErrValue()

public:

    using OkValueType = T;
    using ErrValueType = E;
    
    // Constructors
    TResult(const ResultHelpers::OkTag& InTag, const T& Value) : bIsOk(true), OkOrErrValue(InTag, Value) {}
    TResult(const ResultHelpers::OkTag& InTag, T&& Value) : bIsOk(true), OkOrErrValue(InTag, MoveTemp(Value)) {}
    
    TResult(const ResultHelpers::ErrTag& InTag, const E& Error) : bIsOk(false), OkOrErrValue(InTag, Error) {}
    TResult(const ResultHelpers::ErrTag& InTag, E&& Error) : bIsOk(false), OkOrErrValue(InTag, MoveTemp(Error)) {}

    // Copy constructor
    TResult(const TResult& Other) : bIsOk(Other.bIsOk)
    {
        if (bIsOk)
        {
            OkOrErrValue.SetOkValue(Other.OkOrErrValue.GetOkValue());
        }
        else
        {
            OkOrErrValue.SetErrValue(Other.OkOrErrValue.GetErrValue());
        }
    }

    // Move constructor
    TResult(TResult&& Other) noexcept : bIsOk(Other.bIsOk)
    {
        if (bIsOk)
        {
            OkOrErrValue.SetOkValue(Other.OkOrErrValue.GetOkValue());
            Other.OkOrErrValue.ResetOk();
        }
        else
        {
            OkOrErrValue.SetErrValue(Other.OkOrErrValue.GetErrValue());
            Other.OkOrErrValue.ResetErr();
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

    // Querying the variant
    bool IsOk() const { return bIsOk; }
    bool IsErr() const { return !bIsOk; }

    template<typename Predicate>
    bool IsOkAnd(Predicate&& Pred) const
    {
        return bIsOk && Pred(OK_VALUE);
    }

    template<typename Predicate>
    bool IsErrAnd(Predicate&& Pred) const
    {
        return !bIsOk && Pred(ERR_VALUE);
    }

    // Extracting contained values
    const T& Expect(const TCHAR* Message) const
    {
        if (!bIsOk)
        {
            UE_LOG(LogTemp, Fatal, TEXT("Result::Expect failed: %s"), Message);
        }
        return OK_VALUE;
    }

    const T& Unwrap() const
    {
        if (!bIsOk)
        {
            UE_LOG(LogTemp, Fatal, TEXT("Called Unwrap on an Err Result"));
        }
        return OK_VALUE;
    }

    T UnwrapOr(const T& DefaultValue) const
    {
        return bIsOk ? OK_VALUE : DefaultValue;
    }

    template<typename F>
    T UnwrapOrElse(F&& Func) const
    {
        return bIsOk ? OK_VALUE : Func(ERR_VALUE);
    }

    const E& ExpectErr(const TCHAR* Message) const
    {
        if (bIsOk)
        {
            UE_LOG(LogTemp, Fatal, TEXT("Result::ExpectErr failed: %s"), Message);
        }
        return ERR_VALUE;
    }

    const E& UnwrapErr() const
    {
        if (bIsOk)
        {
            UE_LOG(LogTemp, Fatal, TEXT("Called UnwrapErr on an Ok Result"));
        }
        return ERR_VALUE;
    }

    // Transforming contained values
    template<typename F>
    TResult<TInvokeResult_T<F, T>, E> Map(F&& Func) const
    {
        if (bIsOk)
        {
            return TResult<TInvokeResult_T<F, T>, E>(ResultHelpers::Ok, Func(OK_VALUE));
        }
        else
        {
            return TResult<TInvokeResult_T<F, T>, E>(ResultHelpers::Err, ERR_VALUE);
        }
    }

    template<typename F>
    TResult<T, TInvokeResult_T<F, E>> MapErr(F&& Func) const
    {
        if (bIsOk)
        {
            return TResult<T, TInvokeResult_T<F, E>>(ResultHelpers::Ok, OK_VALUE);
        }
        else
        {
            return TResult<T, TInvokeResult_T<F, E>>(ResultHelpers::Err, Func(ERR_VALUE));
        }
    }

    template<typename F>
    TResult<typename TInvokeResult_T<F, T>::OkValueType, E> AndThen(F&& Func) const
    {
        if (bIsOk)
        {
            return Func(OK_VALUE);
        }
        else
        {
            return TResult<typename TInvokeResult_T<F, T>::OkValueType, E>(ResultHelpers::Err, ERR_VALUE);
        }
    }

    template<typename F>
    TResult<T, typename TInvokeResult_T<F, E>::ErrValueType> OrElse(F&& Func) const
    {
        if (bIsOk)
        {
            return TResult<T, typename TInvokeResult_T<F, E>::ErrValueType>(ResultHelpers::Ok, OK_VALUE);
        }
        else
        {
            return Func(ERR_VALUE);
        }
    }

    // Convert to Optional
    TOptional<T> Ok() const
    {
        return bIsOk ? TOptional<T>(OK_VALUE) : TOptional<T>();
    }

    TOptional<E> Err() const
    {
        return !bIsOk ? TOptional<E>(ERR_VALUE) : TOptional<E>();
    }

    // Boolean operators
    template<typename U>
    TResult<U, E> And(const TResult<U, E>& Other) const
    {
        return bIsOk ? Other : TResult<U, E>(ResultHelpers::Err, ERR_VALUE);
    }

    template<typename NewE>
    TResult<T, NewE> Or(const TResult<T, NewE>& Other) const
    {
        return bIsOk ? TResult<T, NewE>(ResultHelpers::Ok, OK_VALUE) : Other;
    }

    // Inspection (for debugging/logging)
    template<typename F>
    const TResult& Inspect(F&& Func) const
    {
        if (bIsOk)
        {
            Func(OK_VALUE);
        }
        return *this;
    }

    template<typename F>
    const TResult& InspectErr(F&& Func) const
    {
        if (!bIsOk)
        {
            Func(ERR_VALUE);
        }
        return *this;
    }

    // Comparison operators
    bool operator==(const TResult& Other) const
    {
        if (bIsOk != Other.bIsOk) return false;
        return bIsOk ? (OK_VALUE == Other.OK_VALUE) : (ERR_VALUE == Other.ERR_VALUE);
    }

    bool operator!=(const TResult& Other) const
    {
        return !(*this == Other);
    }
};

// Helper functions for creating Results
template<typename T>
auto MakeOk(T&& Value)
{
    
    return [v = Forward<T>(Value)](auto ErrorType) mutable
    {
        using E = decltype(ErrorType);
        return TResult<T, E>(ResultHelpers::Ok, MoveTemp(v));
    };
}

template<typename E>
auto MakeErr(E&& Error)
{
    return [e = Forward<E>(Error)](auto OkType) mutable
    {
        using T = decltype(OkType);
        return TResult<T, E>(ResultHelpers::Err, MoveTemp(e));
    };
}

#undef OK_VALUE
#undef ERR_VALUE

