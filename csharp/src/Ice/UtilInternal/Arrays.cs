// Copyright (c) ZeroC, Inc.

using System.Collections;

namespace Ice.UtilInternal;

public sealed class Arrays
{
    public static bool Equals(object[] arr1, object[] arr2)
    {
        if (object.ReferenceEquals(arr1, arr2))
        {
            return true;
        }

        if ((arr1 == null && arr2 != null) || (arr1 != null && arr2 == null))
        {
            return false;
        }

        if (arr1.Length == arr2.Length)
        {
            for (int i = 0; i < arr1.Length; i++)
            {
                if (arr1[i] == null)
                {
                    if (arr2[i] != null)
                    {
                        return false;
                    }
                }
                else if (!arr1[i].Equals(arr2[i]))
                {
                    return false;
                }
            }

            return true;
        }

        return false;
    }

    public static bool Equals(Array arr1, Array arr2)
    {
        if (object.ReferenceEquals(arr1, arr2))
        {
            return true;
        }

        if ((arr1 == null && arr2 != null) || (arr1 != null && arr2 == null))
        {
            return false;
        }

        if (arr1.Length == arr2.Length)
        {
            IEnumerator e1 = arr1.GetEnumerator();
            IEnumerator e2 = arr2.GetEnumerator();
            while (e1.MoveNext())
            {
                e2.MoveNext();
                if (e1.Current == null)
                {
                    if (e2.Current != null)
                    {
                        return false;
                    }
                }
                else if (!e1.Current.Equals(e2.Current))
                {
                    return false;
                }
            }

            return true;
        }

        return false;
    }
}
