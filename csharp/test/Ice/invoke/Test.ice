// Copyright (c) ZeroC, Inc.

#pragma once

["cs:identifier:Ice.invoke.Test"]
module Test
{
    exception MyException
    {
    }

    interface MyClass
    {
        void opOneway();

        string opString(string s1, out string s2);

        void opException() throws MyException;

        void shutdown();
    }
}
