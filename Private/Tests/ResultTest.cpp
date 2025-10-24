#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "ResultType/Result.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTResultConstructorTest, "ResultErrorHandling.TResult.Constructor", 
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTResultConstructorTest::RunTest(const FString& Parameters)
{
    // Test Ok constructor with value
    TResult<int32, FString> OkResult(ResultHelpers::Ok, 42);
    TestTrue("Ok result should be Ok", OkResult.IsOk());
    TestFalse("Ok result should not be Err", OkResult.IsErr());
    TestEqual("Ok value should match", OkResult.Unwrap(), 42);

    // Test Err constructor with value
    TResult<int32, FString> ErrResult(ResultHelpers::Err, TEXT("Error message"));
    TestFalse("Err result should not be Ok", ErrResult.IsOk());
    TestTrue("Err result should be Err", ErrResult.IsErr());
    TestEqual("Err value should match", ErrResult.UnwrapErr(), TEXT("Error message"));

    // Test move constructors
    TResult<int32, FString> MovedOk(ResultHelpers::Ok, 100);
    TResult<int32, FString> OkCopy(MoveTemp(MovedOk));
    TestTrue("Moved Ok result should be Ok", OkCopy.IsOk());
    TestEqual("Moved Ok value should match", OkCopy.Unwrap(), 100);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTResultCopyMoveTest, "ResultErrorHandling.TResult.CopyMove", 
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTResultCopyMoveTest::RunTest(const FString& Parameters)
{
    // Test copy constructor
    TResult<int32, FString> Original(ResultHelpers::Ok, 123);
    TResult<int32, FString> Copied(Original);
    TestTrue("Copied result should be Ok", Copied.IsOk());
    TestEqual("Copied value should match original", Copied.Unwrap(), 123);

    // Test copy assignment
    TResult<int32, FString> Assigned(ResultHelpers::Err, TEXT("Initial"));
    Assigned = Original;
    TestTrue("Assigned result should be Ok", Assigned.IsOk());
    TestEqual("Assigned value should match original", Assigned.Unwrap(), 123);

    // Test move assignment
    TResult<int32, FString> Source(ResultHelpers::Ok, 456);
    TResult<int32, FString> Target(ResultHelpers::Err, TEXT("Target"));
    Target = MoveTemp(Source);
    TestTrue("Move assigned result should be Ok", Target.IsOk());
    TestEqual("Move assigned value should match", Target.Unwrap(), 456);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTResultQueryTest, "ResultErrorHandling.TResult.Query", 
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTResultQueryTest::RunTest(const FString& Parameters)
{
    TResult<int32, FString> OkResult(ResultHelpers::Ok, 10);
    TResult<int32, FString> ErrResult(ResultHelpers::Err, TEXT("Error"));

    // Test basic queries
    TestTrue("Ok result IsOk should return true", OkResult.IsOk());
    TestFalse("Ok result IsErr should return false", OkResult.IsErr());
    TestFalse("Err result IsOk should return false", ErrResult.IsOk());
    TestTrue("Err result IsErr should return true", ErrResult.IsErr());

    // Test IsOkAnd
    TestTrue("IsOkAnd with true predicate", OkResult.IsOkAnd([](int32 Val) { return Val > 5; }));
    TestFalse("IsOkAnd with false predicate", OkResult.IsOkAnd([](int32 Val) { return Val > 15; }));
    TestFalse("IsOkAnd on Err result", ErrResult.IsOkAnd([](int32 Val) { return true; }));

    // Test IsErrAnd
    TestTrue("IsErrAnd with true predicate", ErrResult.IsErrAnd([](const FString& Err) { return Err.Contains(TEXT("Error")); }));
    TestFalse("IsErrAnd with false predicate", ErrResult.IsErrAnd([](const FString& Err) { return Err.Contains(TEXT("Success")); }));
    TestFalse("IsErrAnd on Ok result", OkResult.IsErrAnd([](const FString& Err) { return true; }));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTResultUnwrapTest, "ResultErrorHandling.TResult.Unwrap", 
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTResultUnwrapTest::RunTest(const FString& Parameters)
{
    TResult<int32, FString> OkResult(ResultHelpers::Ok, 42);
    TResult<int32, FString> ErrResult(ResultHelpers::Err, TEXT("Test Error"));

    // Test Unwrap on Ok result
    TestEqual("Unwrap should return Ok value", OkResult.Unwrap(), 42);

    // Test Expect on Ok result
    TestEqual("Expect should return Ok value", OkResult.Expect(TEXT("Should not fail")), 42);

    // Test UnwrapOr
    TestEqual("UnwrapOr on Ok should return Ok value", OkResult.UnwrapOr(999), 42);
    TestEqual("UnwrapOr on Err should return default", ErrResult.UnwrapOr(999), 999);

    // Test UnwrapOrElse
    auto DefaultFunc = [](const FString& Error) { return Error.Len(); };
    TestEqual("UnwrapOrElse on Ok should return Ok value", OkResult.UnwrapOrElse(DefaultFunc), 42);
    TestEqual("UnwrapOrElse on Err should call function", ErrResult.UnwrapOrElse(DefaultFunc), 10); // "Test Error" length

    // Test UnwrapErr on Err result
    TestEqual("UnwrapErr should return Err value", ErrResult.UnwrapErr(), TEXT("Test Error"));

    // Test ExpectErr on Err result
    TestEqual("ExpectErr should return Err value", ErrResult.ExpectErr(TEXT("Should not fail")), TEXT("Test Error"));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTResultMapTest, "ResultErrorHandling.TResult.Map", 
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTResultMapTest::RunTest(const FString& Parameters)
{
    TResult<int32, FString> OkResult(ResultHelpers::Ok, 5);
    TResult<int32, FString> ErrResult(ResultHelpers::Err, TEXT("Error"));

    // Test Map on Ok result
    auto MappedOk = OkResult.Map([](int32 Val) { return Val * 2; });
    TestTrue("Mapped Ok result should be Ok", MappedOk.IsOk());
    TestEqual("Mapped value should be transformed", MappedOk.Unwrap(), 10);

    // Test Map on Err result
    auto MappedErr = ErrResult.Map([](int32 Val) { return Val * 2; });
    TestTrue("Mapped Err result should remain Err", MappedErr.IsErr());
    TestEqual("Error should be preserved", MappedErr.UnwrapErr(), TEXT("Error"));

    // Test MapErr on Ok result
    auto MapErrOk = OkResult.MapErr([](const FString& Err) { return Err + TEXT(" mapped"); });
    TestTrue("MapErr on Ok should remain Ok", MapErrOk.IsOk());
    TestEqual("Ok value should be preserved", MapErrOk.Unwrap(), 5);

    // Test MapErr on Err result
    auto MapErrErr = ErrResult.MapErr([](const FString& Err) { return Err + TEXT(" mapped"); });
    TestTrue("MapErr on Err should remain Err", MapErrErr.IsErr());
    TestEqual("Error should be transformed", MapErrErr.UnwrapErr(), TEXT("Error mapped"));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTResultAndThenOrElseTest, "ResultErrorHandling.TResult.AndThenOrElse", 
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTResultAndThenOrElseTest::RunTest(const FString& Parameters)
{
    TResult<int32, FString> OkResult(ResultHelpers::Ok, 5);
    TResult<int32, FString> ErrResult(ResultHelpers::Err, TEXT("Error"));

    // Test AndThen on Ok result returning Ok
    auto AndThenOkOk = OkResult.AndThen([](int32 Val) {
        return TResult<int32, FString>(ResultHelpers::Ok, Val * 2);
    });
    TestTrue("AndThen Ok->Ok should be Ok", AndThenOkOk.IsOk());
    TestEqual("AndThen Ok->Ok value should be transformed", AndThenOkOk.Unwrap(), 10);

    // Test AndThen on Ok result returning Err
    auto AndThenOkErr = OkResult.AndThen([](int32 Val) {
        return TResult<int32, FString>(ResultHelpers::Err, TEXT("Function error"));
    });
    TestTrue("AndThen Ok->Err should be Err", AndThenOkErr.IsErr());
    TestEqual("AndThen Ok->Err error should match", AndThenOkErr.UnwrapErr(), TEXT("Function error"));

    // Test AndThen on Err result
    auto AndThenErr = ErrResult.AndThen([](int32 Val) {
        return TResult<int32, FString>(ResultHelpers::Ok, Val * 2);
    });
    TestTrue("AndThen on Err should remain Err", AndThenErr.IsErr());
    TestEqual("AndThen on Err should preserve error", AndThenErr.UnwrapErr(), TEXT("Error"));

    // Test OrElse on Err result returning Ok
    auto OrElseErrOk = ErrResult.OrElse([](const FString& Err) {
        return TResult<int32, FString>(ResultHelpers::Ok, 42);
    });
    TestTrue("OrElse Err->Ok should be Ok", OrElseErrOk.IsOk());
    TestEqual("OrElse Err->Ok value should match", OrElseErrOk.Unwrap(), 42);

    // Test OrElse on Ok result
    auto OrElseOk = OkResult.OrElse([](const FString& Err) {
        return TResult<int32, FString>(ResultHelpers::Err, TEXT("New error"));
    });
    TestTrue("OrElse on Ok should remain Ok", OrElseOk.IsOk());
    TestEqual("OrElse on Ok should preserve value", OrElseOk.Unwrap(), 5);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTResultConvertToOptionalTest, "ResultErrorHandling.TResult.ConvertToOptional", 
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTResultConvertToOptionalTest::RunTest(const FString& Parameters)
{
    TResult<int32, FString> OkResult(ResultHelpers::Ok, 42);
    TResult<int32, FString> ErrResult(ResultHelpers::Err, TEXT("Error"));

    // Test Ok() conversion
    auto OkOptional = OkResult.Ok();
    TestTrue("Ok result converted to Optional should have value", OkOptional.IsSet());
    TestEqual("Ok Optional value should match", OkOptional.GetValue(), 42);

    auto ErrToOkOptional = ErrResult.Ok();
    TestFalse("Err result converted to Ok Optional should be unset", ErrToOkOptional.IsSet());

    // Test Err() conversion
    auto ErrOptional = ErrResult.Err();
    TestTrue("Err result converted to Optional should have value", ErrOptional.IsSet());
    TestEqual("Err Optional value should match", ErrOptional.GetValue(), TEXT("Error"));

    auto OkToErrOptional = OkResult.Err();
    TestFalse("Ok result converted to Err Optional should be unset", OkToErrOptional.IsSet());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTResultBooleanOperatorsTest, "ResultErrorHandling.TResult.BooleanOperators", 
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTResultBooleanOperatorsTest::RunTest(const FString& Parameters)
{
    TResult<int32, FString> Ok1(ResultHelpers::Ok, 1);
    TResult<int32, FString> Ok2(ResultHelpers::Ok, 2);
    TResult<int32, FString> Err1(ResultHelpers::Err, TEXT("Error1"));
    TResult<int32, FString> Err2(ResultHelpers::Err, TEXT("Error2"));

    // Test And operator
    auto OkAndOk = Ok1.And(Ok2);
    TestTrue("Ok And Ok should be Ok", OkAndOk.IsOk());
    TestEqual("Ok And Ok should return second value", OkAndOk.Unwrap(), 2);

    auto OkAndErr = Ok1.And(Err1);
    TestTrue("Ok And Err should be Err", OkAndErr.IsErr());
    TestEqual("Ok And Err should return error", OkAndErr.UnwrapErr(), TEXT("Error1"));

    auto ErrAndOk = Err1.And(Ok1);
    TestTrue("Err And Ok should be Err", ErrAndOk.IsErr());
    TestEqual("Err And Ok should return first error", ErrAndOk.UnwrapErr(), TEXT("Error1"));

    // Test Or operator
    auto OkOrOk = Ok1.Or(Ok2);
    TestTrue("Ok Or Ok should be Ok", OkOrOk.IsOk());
    TestEqual("Ok Or Ok should return first value", OkOrOk.Unwrap(), 1);

    auto OkOrErr = Ok1.Or(Err1);
    TestTrue("Ok Or Err should be Ok", OkOrErr.IsOk());
    TestEqual("Ok Or Err should return Ok value", OkOrErr.Unwrap(), 1);

    auto ErrOrOk = Err1.Or(Ok1);
    TestTrue("Err Or Ok should be Ok", ErrOrOk.IsOk());
    TestEqual("Err Or Ok should return Ok value", ErrOrOk.Unwrap(), 1);

    auto ErrOrErr = Err1.Or(Err2);
    TestTrue("Err Or Err should be Err", ErrOrErr.IsErr());
    TestEqual("Err Or Err should return second error", ErrOrErr.UnwrapErr(), TEXT("Error2"));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTResultInspectionTest, "ResultErrorHandling.TResult.Inspection", 
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTResultInspectionTest::RunTest(const FString& Parameters)
{
    TResult<int32, FString> OkResult(ResultHelpers::Ok, 42);
    TResult<int32, FString> ErrResult(ResultHelpers::Err, TEXT("Error"));

    int32 InspectedValue = 0;
    FString InspectedError;

    // Test Inspect on Ok result
    auto InspectedOk = OkResult.Inspect([&](int32 Val) { InspectedValue = Val; });
    TestEqual("Inspected value should be captured", InspectedValue, 42);
    TestTrue("Inspect should return the same result", InspectedOk.IsOk());
    TestEqual("Inspect should preserve value", InspectedOk.Unwrap(), 42);

    // Test Inspect on Err result (should not call function)
    InspectedValue = 0;
    auto InspectedErr = ErrResult.Inspect([&](int32 Val) { InspectedValue = Val; });
    TestEqual("Inspect on Err should not call function", InspectedValue, 0);
    TestTrue("Inspect on Err should remain Err", InspectedErr.IsErr());

    // Test InspectErr on Err result
    auto InspectedErrErr = ErrResult.InspectErr([&](const FString& Err) { InspectedError = Err; });
    TestEqual("InspectErr should capture error", InspectedError, TEXT("Error"));
    TestTrue("InspectErr should return the same result", InspectedErrErr.IsErr());
    TestEqual("InspectErr should preserve error", InspectedErrErr.UnwrapErr(), TEXT("Error"));

    // Test InspectErr on Ok result (should not call function)
    InspectedError.Empty();
    auto InspectedOkErr = OkResult.InspectErr([&](const FString& Err) { InspectedError = Err; });
    TestTrue("InspectErr error should remain empty", InspectedError.IsEmpty());
    TestTrue("InspectErr on Ok should remain Ok", InspectedOkErr.IsOk());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTResultComparisonTest, "ResultErrorHandling.TResult.Comparison", 
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTResultComparisonTest::RunTest(const FString& Parameters)
{
    TResult<int32, FString> Ok1(ResultHelpers::Ok, 42);
    TResult<int32, FString> Ok2(ResultHelpers::Ok, 42);
    TResult<int32, FString> Ok3(ResultHelpers::Ok, 24);
    TResult<int32, FString> Err1(ResultHelpers::Err, TEXT("Error"));
    TResult<int32, FString> Err2(ResultHelpers::Err, TEXT("Error"));
    TResult<int32, FString> Err3(ResultHelpers::Err, TEXT("Different"));

    // Test equality
    TestTrue("Equal Ok results should be equal", Ok1 == Ok2);
    TestFalse("Different Ok results should not be equal", Ok1 == Ok3);
    TestTrue("Equal Err results should be equal", Err1 == Err2);
    TestFalse("Different Err results should not be equal", Err1 == Err3);
    TestFalse("Ok and Err results should not be equal", Ok1 == Err1);

    // Test inequality
    TestFalse("Equal Ok results should not be unequal", Ok1 != Ok2);
    TestTrue("Different Ok results should be unequal", Ok1 != Ok3);
    TestTrue("Ok and Err results should be unequal", Ok1 != Err1);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTResultHelperFunctionsTest, "ResultErrorHandling.TResult.HelperFunctions", 
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTResultHelperFunctionsTest::RunTest(const FString& Parameters)
{
    // Test MakeOk helper
    auto OkFactory = MakeOk(42);
    auto OkResult = OkFactory(FString{});
    TestTrue("MakeOk should create Ok result", OkResult.IsOk());
    TestEqual("MakeOk value should match", OkResult.Unwrap(), 42);

    // Test MakeErr helper
    auto ErrFactory = MakeErr(FString(TEXT("Test Error")));
    auto ErrResult = ErrFactory(int32{});
    TestTrue("MakeErr should create Err result", ErrResult.IsErr());
    TestEqual("MakeErr error should match", ErrResult.UnwrapErr(), TEXT("Test Error"));

    return true;
}
