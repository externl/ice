// Copyright (c) ZeroC, Inc.

["java:identifier:test.Slice.escape.Clash"]
module Clash
{
    interface Intf
    {
        void context();
        void current();
        void response();
        void typeId();
        void del();
        void cookie();
        void sync();
        void inS();
        void istr();
        void obj();

        void op(string context, string current, string response, string ex, string sent, string cookie,
                string sync, string result, string istr, string ostr, optional(1) string proxy, optional(2) int obj);
        void opOut(out string context, out string current, out string response, out string ex,
                   out string sent, out string cookie, out string sync, out string result, out string istr,
                   out string ostr, out optional(1) string proxy, out optional(2) int obj);
    }

    class Cls
    {
        Intf* s;
        string context;
        int current;
        short response;
        int typeId;
        short del;
        optional(1) short cookie;
        string ex;
        int result;
        string istr;
        string ostr;
        string inS;
        string in;
        string proxy;
        int obj;
        int getCookie;
    }

    struct St
    {
        string v;
        short istr;
        int ostr;
        int rhs;
        string hashCode;
    }

    exception Ex
    {
        short istr;
        int ostr;
        string cause;
    }
}
