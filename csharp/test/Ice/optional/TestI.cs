//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice.optional
{
    public class Initial : Test.IInitial
    {
        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();

        public AnyClass pingPong(AnyClass obj, Current current) => obj;

        public void opOptionalException(int? a, string? b, Test.OneOptional? o, Current current) =>
            throw new Test.OptionalException(false, a, b, o);

        public void opDerivedException(int? a, string? b, Test.OneOptional? o, Current current) =>
            throw new Test.DerivedException(false, a, b, o, b, o);

        public void opRequiredException(int? a,
                                        string? b,
                                        Test.OneOptional? o,
                                        Current current)
        {
            Test.RequiredException e = new Test.RequiredException();
            e.a = a;
            e.b = b;
            e.o = o;
            if (b != null)
            {
                e.ss = b;
            }

            if (o != null)
            {
                e.o2 = o;
            }
            throw e;
        }

        public (byte?, byte?) opByte(byte? p1, Current current) => (p1, p1);

        public (bool?, bool?) opBool(bool? p1, Current current) => (p1, p1);

        public (short?, short?) opShort(short? p1, Current current) => (p1, p1);

        public (int?, int?) opInt(int? p1, Current current) => (p1, p1);

        public (long?, long?) opLong(long? p1, Current current) => (p1, p1);

        public (float?, float?) opFloat(float? p1, Current current) => (p1, p1);

        public (double?, double?) opDouble(double? p1, Current current) => (p1, p1);

        public (string?, string?) opString(string? p1, Current current) => (p1, p1);

        public (Test.MyEnum?, Test.MyEnum?) opMyEnum(Test.MyEnum? p1, Current current) => (p1, p1);

        public (Test.SmallStruct?, Test.SmallStruct?) opSmallStruct(Test.SmallStruct? p1, Current current) => (p1, p1);

        public (Test.FixedStruct?, Test.FixedStruct?) opFixedStruct(Test.FixedStruct? p1, Current current) => (p1, p1);

        public (Test.VarStruct?, Test.VarStruct?) opVarStruct(Test.VarStruct? p1, Current current) => (p1, p1);

        public (Test.OneOptional?, Test.OneOptional?) opOneOptional(Test.OneOptional? p1, Current current) => (p1, p1);

        public (IObjectPrx?, IObjectPrx?) opOneOptionalProxy(IObjectPrx? p1, Current current) => (p1, p1);

        public (byte[]?, byte[]?) opByteSeq(byte[]? p1, Current current) => (p1, p1);

        public (bool[]?, bool[]?) opBoolSeq(bool[]? p1, Current current) => (p1, p1);

        public (short[]?, short[]?) opShortSeq(short[]? p1, Current current) => (p1, p1);

        public (int[]?, int[]?) opIntSeq(int[]? p1, Current current) => (p1, p1);

        public (long[]?, long[]?) opLongSeq(long[]? p1, Current current) => (p1, p1);

        public (float[]?, float[]?) opFloatSeq(float[]? p1, Current current) => (p1, p1);

        public (double[]?, double[]?) opDoubleSeq(double[]? p1, Current current) => (p1, p1);

        public (string[]?, string[]?) opStringSeq(string[]? p1, Current current) => (p1, p1);

        public (Test.SmallStruct[]?, Test.SmallStruct[]?) opSmallStructSeq(Test.SmallStruct[]? p1, Current current) =>
            (p1, p1);

        public (List<Test.SmallStruct>?, List<Test.SmallStruct>?)
        opSmallStructList(List<Test.SmallStruct>? p1, Current current) => (p1, p1);

        public (Test.FixedStruct[]?, Test.FixedStruct[]?)
        opFixedStructSeq(Test.FixedStruct[]? p1, Current current) => (p1, p1);

        public (LinkedList<Test.FixedStruct>?, LinkedList<Test.FixedStruct>?)
        opFixedStructList(LinkedList<Test.FixedStruct>? p1, Current current) => (p1, p1);

        public (Test.VarStruct[]?, Test.VarStruct[]?) opVarStructSeq(Test.VarStruct[]? p1, Current current) => (p1, p1);

        public (Test.SerializableClass?, Test.SerializableClass?)
        opSerializable(Test.SerializableClass? p1, Current current) => (p1, p1);

        public (Dictionary<int, int>?, Dictionary<int, int>?)
        opIntIntDict(Dictionary<int, int>? p1, Current current) => (p1, p1);

        public (Dictionary<string, int>?, Dictionary<string, int>?)
        opStringIntDict(Dictionary<string, int>? p1, Current current) => (p1, p1);

        public (Dictionary<int, Test.OneOptional>?, Dictionary<int, Test.OneOptional>?)
        opIntOneOptionalDict(Dictionary<int, Test.OneOptional>? p1, Current current) => (p1, p1);

        public void opClassAndUnknownOptional(Test.A p, Current current)
        {
        }

        public void sendOptionalClass(bool req, Test.OneOptional? o, Current current)
        {
        }

        public Test.OneOptional? returnOptionalClass(bool req, Current current) => new Test.OneOptional(53);

        public Test.G opG(Test.G g, Current current) => g;

        public void opVoid(Current current)
        {
        }

        public Test.IInitial.OpMStruct1MarshaledReturnValue
        opMStruct1(Current current) => new Test.IInitial.OpMStruct1MarshaledReturnValue(new Test.SmallStruct(), current);

        public Test.IInitial.OpMStruct2MarshaledReturnValue
        opMStruct2(Test.SmallStruct? p1, Current current) =>
            new Test.IInitial.OpMStruct2MarshaledReturnValue(p1, p1, current);

        public Test.IInitial.OpMSeq1MarshaledReturnValue
        opMSeq1(Current current) => new Test.IInitial.OpMSeq1MarshaledReturnValue(new string[0], current);

        public Test.IInitial.OpMSeq2MarshaledReturnValue
        opMSeq2(string[]? p1, Current current) => new Test.IInitial.OpMSeq2MarshaledReturnValue(p1, p1, current);

        public Test.IInitial.OpMDict1MarshaledReturnValue
        opMDict1(Current current) =>
            new Test.IInitial.OpMDict1MarshaledReturnValue(new Dictionary<string, int>(), current);

        public Test.IInitial.OpMDict2MarshaledReturnValue
        opMDict2(Dictionary<string, int>? p1, Current current) =>
            new Test.IInitial.OpMDict2MarshaledReturnValue(p1, p1, current);

        public Test.IInitial.OpMG1MarshaledReturnValue
        opMG1(Current current) => new Test.IInitial.OpMG1MarshaledReturnValue(new Test.G(), current);

        public Test.IInitial.OpMG2MarshaledReturnValue
        opMG2(Test.G? p1, Current current) => new Test.IInitial.OpMG2MarshaledReturnValue(p1, p1, current);

        public bool supportsRequiredParams(Current current) => false;

        public bool supportsJavaSerializable(Current current) => false;

        public bool supportsCsharpSerializable(Current current) => true;

        public bool supportsCppStringView(Current current) => false;

        public bool supportsNullOptional(Current current) => true;
    }
}
