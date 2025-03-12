#pragma once

#include "type_wrapper.h"

using namespace Types;

/* IDL code

class TestType
{
    float TestFloat = 123.456;
    int32 TestInt = 123;
    bool TestBool = false;
    string TestTring = "Default_Value";
};

*/
namespace AssetTypes
{
    class TestType : public TypeWraper
    {
    public:
        DEFINE_TYPE(TestType);

        static void Register(TypeDatabase& typeDB)
        {
            auto* type = typeDB.CreateType(TypeName);
            type->AddPrimitiveField<float>("TestFloat", 123.456f);
            type->AddPrimitiveField<int32_t>("TestInt", 123);
            type->AddPrimitiveField<bool>("TestBool", false);
            type->AddPrimitiveField<std::string>("TestString", "Default_Value");
        }

        const float& GetTestFloat() const { return ValuePtr->GetFieldPrimitiveValue<float>(0); }
        void SetTestFloat(const float& value) { ValuePtr->SetFieldPrimitiveValue<float>(0, value); }
        void ResetTestFloat() { ValuePtr->ResetFieldToDefault(0); }

        const int32_t& GetTestInt() const { return ValuePtr->GetFieldPrimitiveValue<int32_t>(1); }
        void SetTestInt(const int32_t& value) { ValuePtr->SetFieldPrimitiveValue<int32_t>(1, value); }
        void ResetTestInt() { Value->ResetFieldToDefault(1); }

        const bool& GetTestBool() const { return ValuePtr->GetFieldPrimitiveValue<bool>(2); }
        void SetTestBool(const bool& value) { ValuePtr->SetFieldPrimitiveValue<bool>(2, value); }
        void ResetTestBool() { ValuePtr->ResetFieldToDefault(2); }

        const std::string& GetTestString() const { return ValuePtr->GetFieldPrimitiveValue<std::string>(3); }
        void SetTestString(const std::string& value) { ValuePtr->SetFieldPrimitiveValue<std::string>(3, value); }
        void ResetTestString() { ValuePtr->ResetFieldToDefault(3); }
    };

/* IDL code

class TestTypeWithEmbed
{
    TestType TestField;
    double TestDouble = 123.456;
};

*/
    class TestTypeWithEmbed : public TypeWraper
    {
    public:
        DEFINE_TYPE(TestTypeWithEmbed);

        static void Register(TypeDatabase& typeDB)
        {
            auto* type = typeDB.CreateType(TypeName);
            type->AddTypeField("TestField", TestType::TypeName);
            type->AddPrimitiveField<double>("TestDouble", 123.456);
        }
        
        TestType GetTestField() const { return TestType(ValuePtr->GetTypeFieldValue(0)); }

        const double& GetTestDouble() const { return ValuePtr->GetFieldPrimitiveValue<double>(1); }
        void SetTestDouble(const double& value) { ValuePtr->SetFieldPrimitiveValue<double>(1, value); }
        void ResetTestDouble() { ValuePtr->ResetFieldToDefault(1); }
    };

/* IDL code

class TestDerivedType : TestTypeWithEmbed
{
    string DerivedString = "Default1";
};

*/
    class TestDerivedType : public TestTypeWithEmbed
    {
    public:
        DEFINE_DERIVED_TYPE(TestDerivedType, TestTypeWithEmbed)

        static void Register(TypeDatabase& typeDB)
        {
            auto* type = typeDB.CreateType(TypeName, TestTypeWithEmbed::TypeName);
            type->AddPrimitiveField<std::string>("DerivedString", "Default1");
        }

        TestType GetTestField() const { return TestType(ValuePtr->GetTypeFieldValue(0)); }

        const std::string& GetDerivedString() const { return ValuePtr->GetFieldPrimitiveValue<std::string>(2); }
        void SetDerivedString(const std::string& value) { ValuePtr->SetFieldPrimitiveValue<std::string>(2, value); }
        void ResetDerivedString() { ValuePtr->ResetFieldToDefault(2); }
    };

/* IDL code

class TestContainerType
{
    bool TestBool = false;
    int32 TestIntList[];
    string TestStringList[];
};

*/
    class TestContainerType : public TypeWraper
    {
    public:
        DEFINE_TYPE(TestContainerType);

        static void Register(TypeDatabase& typeDB)
        {
            auto* type = typeDB.CreateType(TypeName);

            type->AddPrimitiveField<bool>("TestBool", false);
           
            type->AddPrimitiveListField("TestIntList", Types::PrimitiveType::Int32);
            type->AddPrimitiveListField("TestStringList", Types::PrimitiveType::String);
        }

        const bool& GetTestBool() const { return ValuePtr->GetFieldPrimitiveValue<bool>(0); }
        void SetTestBool(const bool& value) { ValuePtr->SetFieldPrimitiveValue<bool>(0, value); }
        void ResetTestBool() { ValuePtr->ResetFieldToDefault(0); }

        PrimitiveListFieldValue<int32_t>& GetTestIntList() const { return ValuePtr->GetPrimitiveListFieldValue<int32_t>(1); }
        PrimitiveListFieldValue<std::string>& GetTestStringList() const { return ValuePtr->GetPrimitiveListFieldValue<std::string>(2); }
    };


    /* IDL Code
    class TestTypeWithPointer
    {
        TestTypeWithEmbed *TestPointer = TestDerivedType();

        TestTypeWithEmbed *TestNullablePointer = null;
    };
    */

    class TestTypeWithPointer : public TypeWraper
    {
    public:
        DEFINE_TYPE(TestTypeWithPointer);

        static void Register(TypeDatabase& typeDB)
        {
            auto* type = typeDB.CreateType(TypeName);

            type->AddTypeField("TestPointer", TestTypeWithEmbed::TypeName, true, TestDerivedType::TypeName);
            type->AddTypeField("TestNullablePointer", TestTypeWithEmbed::TypeName, true);
        }

        TestTypeWithEmbed GetTestPointer() const 
        {
            auto* value = ValuePtr->GetTypeFieldValue(0);
            return TestTypeWithEmbed(value);
        }

        template<class T>
        T GetTestPointerAs() const
        {
            auto* value = ValuePtr->GetTypeFieldValue(0);
            return T(value);
        }

        const TypeInfo* GetTestPointerType() const
        {
            const auto* value = ValuePtr->GetTypeFieldValue(0);
            if (value == nullptr)
                return nullptr;

            return value->GetType();
        }

        void ClearTestPointer()
        {
            ValuePtr->SetTypeFieldPointer(nullptr, 0);
        }

        template<class T>
        T SetTestPointer()
        {
            return T(ValuePtr->SetTypeFieldPointer(T::TypeName, 0));
        }

        TestTypeWithEmbed GetTestTestNullablePointer() const
        {
            auto* value = ValuePtr->GetTypeFieldValue(1);
            return TestTypeWithEmbed(value);
        }

        template<class T>
        T GetTestNullablePointerAs() const
        {
            auto* value = ValuePtr->GetTypeFieldValue(1);
            return T(value);
        }

        const TypeInfo* GetTestNullablePointerType() const
        {
            const auto* value = ValuePtr->GetTypeFieldValue(1);
            if (value == nullptr)
                return nullptr;

            return value->GetType();
        }

        void ClearTestNullablePointer()
        {
            ValuePtr->SetTypeFieldPointer(nullptr, 1);
        }

        template<class T>
        T SetTestNullablePointer()
        {
            return T(ValuePtr->SetTypeFieldPointer(T::TypeName, 1));
        }
    };

/* IDL Code
   class TestTypeWithTypeList
   {
       TestType TestTypeList[];
   };
*/
    class TestTypeWithTypeList : public TypeWraper
    {
    public:
        DEFINE_TYPE(TestTypeWithTypeList);

        static void Register(TypeDatabase& typeDB)
        {
            auto* type = typeDB.CreateType(TypeName);
            type->AddTypeListField("TestTypeList", TestType::TypeName);
        }

        TypeListWrapper<TestType> GetTestTypeList() const { return TypeListWrapper<TestType>(ValuePtr->GetTypeListFieldValue(0)); }
    };

/* IDL Code
    class TestTypeWithPtrTypeList
    {
        TestTypeWithEmbed *TestPtrTypeList[];
    };
*/

    class TestTypeWithPtrTypeList : public TypeWraper
    {
    public:
        DEFINE_TYPE(TestTypeWithPtrTypeList);

        static void Register(TypeDatabase& typeDB)
        {
            auto* type = typeDB.CreateType(TypeName);
            type->AddTypeListField("TestPtrTypeList", TestTypeWithEmbed::TypeName, true);
        }

        TypePtrListWrapper GetTestPtrTypeList() const { return TypePtrListWrapper(ValuePtr->GetTypeListFieldValue(0)); }
    };
}