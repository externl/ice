// Copyright (c) ZeroC, Inc.

#pragma once

module Test
{
    struct S
    {
        string str;
    }

    class Base
    {
        S theS;
        string str;
    }

    class B;
    class C;

    class A
    {
        B theB;
        C theC;

        bool preMarshalInvoked;
        bool postUnmarshalInvoked;
    }

    class B extends A
    {
        A theA;
    }

    class C
    {
        B theB;

        bool preMarshalInvoked;
        bool postUnmarshalInvoked;
    }

    class D
    {
        A theA;
        B theB;
        C theC;

        bool preMarshalInvoked;
        bool postUnmarshalInvoked;
    }

    // Exercise empty class with non-empty base
    class G extends Base
    {
    }

    interface I
    {
    }

    interface J extends I
    {
    }

    sequence<Base> BaseSeq;

    class CompactExt;

    class Compact(1)
    {
    }

    const int CompactExtId = 789;

    class CompactExt(CompactExtId) extends Compact
    {
    }

    class A1
    {
        string name;
    }

    class B1
    {
        A1 a1;
        A1 a2;
    }

    class D1 extends B1
    {
        A1 a3;
        A1 a4;
    }

    exception EBase
    {
        A1 a1;
        A1 a2;
    }

    exception EDerived extends EBase
    {
        A1 a3;
        A1 a4;
    }

    class Recursive
    {
        Recursive v;
    }

    class K
    {
        Value value;
    }

    class L
    {
        string data;
    }

    sequence<Value> ValueSeq;
    dictionary<string, Value> ValueMap;

    // Forward declarations
    class F1;
    interface F2;

    class F3
    {
        F1 f1;
        F2* f2;
    }

    interface Initial
    {
        void shutdown();
        B getB1();
        B getB2();
        C getC();
        D getD();

        void setRecursive(Recursive p);

        void setCycle(Recursive r);
        bool acceptsClassCycles();

        ["marshaled-result"] B getMB();
        ["amd"] ["marshaled-result"] B getAMDMB();

        void getAll(out B b1, out B b2, out C theC, out D theD);

        K getK();

        Value opValue(Value v1, out Value v2);
        ValueSeq opValueSeq(ValueSeq v1, out ValueSeq v2);
        ValueMap opValueMap(ValueMap v1, out ValueMap v2);

        D1 getD1(D1 d1);
        void throwEDerived() throws EDerived;

        void setG(G theG);

        BaseSeq opBaseSeq(BaseSeq inSeq, out BaseSeq outSeq);

        Compact getCompact();

        F1 opF1(F1 f11, out F1 f12);
        F2* opF2(F2* f21, out F2* f22);
        F3 opF3(F3 f31, out F3 f32);
        bool hasF3();
    }

    ["php:identifier:_Empty"]
    class Empty
    {
    }

    class AlsoEmpty
    {
    }

    interface UnexpectedObjectExceptionTest
    {
        Empty op();
    }

    //
    // Remaining definitions are here to ensure that the generated code compiles.
    //

    class COneMember
    {
        Empty e;
    }

    class CTwoMembers
    {
        Empty e1;
        Empty e2;
    }

    exception EOneMember
    {
        Empty e;
    }

    exception ETwoMembers
    {
        Empty e1;
        Empty e2;
    }

    struct SOneMember
    {
        Empty e;
    }

    struct STwoMembers
    {
        Empty e1;
        Empty e2;
    }

    dictionary<int, COneMember> DOneMember;
    dictionary<int, CTwoMembers> DTwoMembers;
}
