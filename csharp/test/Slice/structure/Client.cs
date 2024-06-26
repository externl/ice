// Copyright (c) ZeroC, Inc.

using Test;

public class Client : TestHelper
{
    private static void allTests(Ice.Communicator communicator)
    {
        Console.Out.Write("testing equals() for Slice structures... ");
        Console.Out.Flush();

        //
        // Define some default values.
        //
        C def_cls = new C(5);
        S1 def_s = new S1("name");
        string[] def_ss = new string[] { "one", "two", "three" };
        var def_il = new List<int> { 1, 2, 3 };
        Dictionary<string, string> def_sd = new Dictionary<string, string>();
        def_sd.Add("abc", "def");
        Ice.ObjectPrx def_prx = communicator.stringToProxy("test");
        S2 def_s2 = new S2(true, (byte)98, (short)99, 100, 101, (float)1.0, 2.0, "string", def_ss, def_il, def_sd,
                           def_s, def_cls, def_prx);

        //
        // Change one primitive member at a time.
        //
        {
            S2 v;

            v = def_s2.Clone();
            test(v.Equals(def_s2));

            v = def_s2.Clone();
            v.bo = false;
            test(!v.Equals(def_s2));

            v = def_s2.Clone();
            v.by--;
            test(!v.Equals(def_s2));

            v = def_s2.Clone();
            v.sh--;
            test(!v.Equals(def_s2));

            v = def_s2.Clone();
            v.i--;
            test(!v.Equals(def_s2));

            v = def_s2.Clone();
            v.l--;
            test(!v.Equals(def_s2));

            v = def_s2.Clone();
            v.f--;
            test(!v.Equals(def_s2));

            v = def_s2.Clone();
            v.d--;
            test(!v.Equals(def_s2));

            v = def_s2.Clone();
            v.str = "";
            test(!v.Equals(def_s2));
        }

        //
        // String member
        //
        {
            S2 v1, v2;

            v1 = def_s2.Clone();
            v1.str = (string)def_s2.str.Clone();
            test(v1.Equals(def_s2));

            v1 = def_s2.Clone();
            v2 = def_s2.Clone();
            v1.str = null;
            test(!v1.Equals(v2));

            v1 = def_s2.Clone();
            v2 = def_s2.Clone();
            v2.str = null;
            test(!v1.Equals(v2));

            v1 = def_s2.Clone();
            v2 = def_s2.Clone();
            v1.str = null;
            v2.str = null;
            test(v1.Equals(v2));
        }

        //
        // Sequence member
        //
        {
            S2 v1, v2;

            v1 = def_s2.Clone();
            v1.ss = (string[])def_s2.ss.Clone();
            test(v1.Equals(def_s2));

            v1 = def_s2.Clone();
            v2 = def_s2.Clone();
            v1.ss = null;
            test(!v1.Equals(v2));

            v1 = def_s2.Clone();
            v2 = def_s2.Clone();
            v2.ss = null;
            test(!v1.Equals(v2));
        }

        //
        // Custom sequence member
        //
        {
            S2 v1, v2;

            v1 = def_s2.Clone();
            v1.il = new List<int>(def_s2.il);
            test(v1.Equals(def_s2));

            v1 = def_s2.Clone();
            v1.il = new List<int> { 0, 0, 0 };
            test(!v1.Equals(def_s2));

            v1 = def_s2.Clone();
            v2 = def_s2.Clone();
            v1.il = null;
            test(!v1.Equals(v2));

            v1 = def_s2.Clone();
            v2 = def_s2.Clone();
            v2.il = null;
            test(!v1.Equals(v2));
        }

        //
        // Dictionary member
        //
        {
            S2 v1, v2;

            v1 = def_s2.Clone();
            v1.sd = new Dictionary<string, string>(def_s2.sd);
            test(v1.Equals(def_s2));

            v1 = def_s2.Clone();
            v1.sd = new Dictionary<string, string>();
            test(!v1.Equals(def_s2));

            v1 = def_s2.Clone();
            v2 = def_s2.Clone();
            v1.sd = null;
            test(!v1.Equals(v2));

            v1 = def_s2.Clone();
            v2 = def_s2.Clone();
            v2.sd = null;
            test(!v1.Equals(v2));
        }

        //
        // Struct member
        //
        {
            S2 v1, v2;

            v1 = def_s2.Clone();
            v1.s = def_s2.s.Clone();
            test(v1.Equals(def_s2));

            v1 = def_s2.Clone();
            v1.s = new S1("name");
            test(v1.Equals(def_s2));

            v1 = def_s2.Clone();
            v1.s = new S1("noname");
            test(!v1.Equals(def_s2));

            v1 = def_s2.Clone();
            v2 = def_s2.Clone();
            v1.s = null;
            test(!v1.Equals(v2));

            v1 = def_s2.Clone();
            v2 = def_s2.Clone();
            v2.s = null;
            test(!v1.Equals(v2));
        }

        //
        // Class member
        //
        {
            S2 v1, v2;

            v1 = def_s2.Clone();
            v1.cls = (C)def_s2.cls.Clone();
            test(!v1.Equals(def_s2));

            v1 = def_s2.Clone();
            v2 = def_s2.Clone();
            v1.cls = null;
            test(!v1.Equals(v2));

            v1 = def_s2.Clone();
            v2 = def_s2.Clone();
            v2.cls = null;
            test(!v1.Equals(v2));
        }

        //
        // Proxy member
        //
        {
            S2 v1, v2;

            v1 = def_s2.Clone();
            v1.prx = communicator.stringToProxy("test");
            test(v1.Equals(def_s2));

            v1 = def_s2.Clone();
            v1.prx = communicator.stringToProxy("test2");
            test(!v1.Equals(def_s2));

            v1 = def_s2.Clone();
            v2 = def_s2.Clone();
            v1.prx = null;
            test(!v1.Equals(v2));

            v1 = def_s2.Clone();
            v2 = def_s2.Clone();
            v2.prx = null;
            test(!v1.Equals(v2));
        }

        Console.Out.WriteLine("ok");
    }

    public override void run(string[] args)
    {
        using (var communicator = initialize(ref args))
        {
            allTests(communicator);
        }
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
