// Copyright (c) ZeroC, Inc.

#nullable enable

using System.Reflection;

namespace Ice.Internal;

public sealed class Patcher
{
    public static System.Action<T?> arrayReadValue<T>(T?[] arr, int index) where T : Ice.Value
    {
        return (T? v) => { arr[index] = v; };
    }

    public static System.Action<T?> listReadValue<T>(List<T?> seq, int index) where T : Ice.Value
    {
        return (T? v) =>
        {
            int count = seq.Count;
            if (index >= count) // Need to grow the sequence.
            {
                for (int i = count; i < index; i++)
                {
                    seq.Add(null);
                }
                seq.Add(v);
            }
            else
            {
                seq[index] = v;
            }
        };
    }

    public static System.Action<T?> customSeqReadValue<T>(IEnumerable<T?> seq, int index) where T : Ice.Value
    {
        return (T? v) =>
        {
            var info = getInvokeInfo<T>(seq.GetType());
            int count = info.getCount(seq);
            if (index >= count) // Need to grow the sequence.
            {
                for (int i = count; i < index; i++)
                {
                    info.invokeAdd(seq, null);
                }
                info.invokeAdd(seq, v);
            }
            else
            {
                info.invokeSet(seq, index, v);
            }
        };
    }

    private static InvokeInfo getInvokeInfo<T>(Type t)
    {
        lock (_methodTable)
        {
            if (_methodTable.TryGetValue(t, out InvokeInfo? i))
            {
                return i;
            }

            MethodInfo? am = t.GetMethod("Add", [typeof(T)]);
            if (am == null)
            {
                throw new Ice.MarshalException("Cannot patch a collection without an Add() method");
            }

            PropertyInfo? pi = t.GetProperty("Item");
            if (pi == null)
            {
                throw new Ice.MarshalException("Cannot patch a collection without an indexer");
            }
            MethodInfo? sm = pi.GetSetMethod();
            if (sm == null)
            {
                throw new Ice.MarshalException("Cannot patch a collection without an indexer to set a value");
            }

            pi = t.GetProperty("Count");
            if (pi == null)
            {
                throw new Ice.MarshalException("Cannot patch a collection without a Count property");
            }
            MethodInfo? cm = pi.GetGetMethod();
            if (cm == null)
            {
                throw new Ice.MarshalException("Cannot patch a collection without a readable Count property");
            }

            i = new InvokeInfo(am, sm, cm);
            _methodTable.Add(t, i);
            return i;
        }
    }

    private class InvokeInfo
    {
        public InvokeInfo(MethodInfo am, MethodInfo sm, MethodInfo cm)
        {
            _addMethod = am;
            _setMethod = sm;
            _countMethod = cm;
        }

        internal int getCount(System.Collections.IEnumerable seq)
        {
            try
            {
                return (int)_countMethod.Invoke(seq, null)!;
            }
            catch (System.Exception ex)
            {
                throw new Ice.MarshalException("Could not read Count property during patching", ex);
            }
        }

        internal void invokeAdd(System.Collections.IEnumerable seq, object? v)
        {
            try
            {
                var arg = new object?[] { v };
                _addMethod.Invoke(seq, arg);
            }
            catch (System.Exception ex)
            {
                throw new Ice.MarshalException("Could not invoke Add method during patching", ex);
            }
        }

        internal void invokeSet(System.Collections.IEnumerable seq, int index, object? v)
        {
            try
            {
                var args = new object?[] { index, v };
                _setMethod.Invoke(seq, args);
            }
            catch (System.Exception ex)
            {
                throw new Ice.MarshalException("Could not call indexer during patching", ex);
            }
        }

        private MethodInfo _addMethod;
        private MethodInfo _setMethod;
        private MethodInfo _countMethod;
    }

    private static Dictionary<Type, InvokeInfo> _methodTable = new Dictionary<Type, InvokeInfo>();
}
