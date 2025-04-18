// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Text;

namespace Ice.UtilInternal;

public sealed class StringUtil
{
    //
    // Return the index of the first character in str to
    // appear in match, starting from 0. Returns -1 if none is
    // found.
    //
    public static int findFirstOf(string str, string match) => findFirstOf(str, match, 0);

    //
    // Return the index of the first character in str to
    // appear in match, starting from start. Returns -1 if none is
    // found.
    //
    public static int findFirstOf(string str, string match, int start)
    {
        int len = str.Length;
        for (int i = start; i < len; i++)
        {
            char ch = str[i];
            if (match.Contains(ch, StringComparison.Ordinal))
            {
                return i;
            }
        }

        return -1;
    }

    //
    // Return the index of the first character in str which does
    // not appear in match, starting from 0. Returns -1 if none is
    // found.
    //
    public static int findFirstNotOf(string str, string match) => findFirstNotOf(str, match, 0);

    //
    // Return the index of the first character in str which does
    // not appear in match, starting from start. Returns -1 if none is
    // found.
    //
    public static int findFirstNotOf(string str, string match, int start)
    {
        int len = str.Length;
        for (int i = start; i < len; i++)
        {
            char ch = str[i];
            if (!match.Contains(ch, StringComparison.Ordinal))
            {
                return i;
            }
        }

        return -1;
    }

    private static void
    encodeChar(char c, StringBuilder sb, string special, Ice.ToStringMode toStringMode)
    {
        switch (c)
        {
            case '\\':
            {
                sb.Append("\\\\");
                break;
            }
            case '\'':
            {
                sb.Append("\\'");
                break;
            }
            case '"':
            {
                sb.Append("\\\"");
                break;
            }
            case '\a':
            {
                if (toStringMode == Ice.ToStringMode.Compat)
                {
                    // Octal escape for compatibility with 3.6 and earlier
                    sb.Append("\\007");
                }
                else
                {
                    sb.Append("\\a");
                }
                break;
            }
            case '\b':
            {
                sb.Append("\\b");
                break;
            }
            case '\f':
            {
                sb.Append("\\f");
                break;
            }
            case '\n':
            {
                sb.Append("\\n");
                break;
            }
            case '\r':
            {
                sb.Append("\\r");
                break;
            }
            case '\t':
            {
                sb.Append("\\t");
                break;
            }
            case '\v':
            {
                if (toStringMode == Ice.ToStringMode.Compat)
                {
                    // Octal escape for compatibility with 3.6 and earlier
                    sb.Append("\\013");
                }
                else
                {
                    sb.Append("\\v");
                }
                break;
            }
            default:
            {
                if (special != null && special.Contains(c, StringComparison.Ordinal))
                {
                    sb.Append('\\');
                    sb.Append(c);
                }
                else
                {
                    int i = (int)c;
                    if (i < 32 || i > 126)
                    {
                        if (toStringMode == Ice.ToStringMode.Compat)
                        {
                            //
                            // When ToStringMode=Compat, c is a UTF-8 byte
                            //
                            Debug.Assert(i < 256);

                            sb.Append('\\');
                            string octal = System.Convert.ToString(i, 8);
                            //
                            // Add leading zeroes so that we avoid problems during
                            // decoding. For example, consider the encoded string
                            // \0013 (i.e., a character with value 1 followed by
                            // the character '3'). If the leading zeroes were omitted,
                            // the result would be incorrectly interpreted by the
                            // decoder as a single character with value 11.
                            //
                            for (int j = octal.Length; j < 3; j++)
                            {
                                sb.Append('0');
                            }
                            sb.Append(octal);
                        }
                        else if (i < 32 || i == 127 || toStringMode == Ice.ToStringMode.ASCII)
                        {
                            // append \\unnnn
                            sb.Append("\\u");
                            string hex = System.Convert.ToString(i, 16);
                            for (int j = hex.Length; j < 4; j++)
                            {
                                sb.Append('0');
                            }
                            sb.Append(hex);
                        }
                        else
                        {
                            // keep as is
                            sb.Append(c);
                        }
                    }
                    else
                    {
                        // printable ASCII character
                        sb.Append(c);
                    }
                }
                break;
            }
        }
    }

    //
    // Add escape sequences (such as "\n", or "\007") to the input string
    //
    public static string escapeString(string s, string special, Ice.ToStringMode toStringMode)
    {
        if (special != null)
        {
            for (int i = 0; i < special.Length; ++i)
            {
                if (special[i] < 32 || special[i] > 126)
                {
                    throw new ArgumentException("special characters must be in ASCII range 32-126", nameof(special));
                }
            }
        }

        if (toStringMode == Ice.ToStringMode.Compat)
        {
            // Encode UTF-8 bytes

            var utf8 = new UTF8Encoding();
            byte[] bytes = utf8.GetBytes(s);

            var result = new StringBuilder(bytes.Length);
            for (int i = 0; i < bytes.Length; i++)
            {
                encodeChar((char)bytes[i], result, special, toStringMode);
            }

            return result.ToString();
        }
        else
        {
            var result = new StringBuilder(s.Length);

            for (int i = 0; i < s.Length; i++)
            {
                char c = s[i];
                if (toStringMode == Ice.ToStringMode.Unicode || !char.IsSurrogate(c))
                {
                    encodeChar(c, result, special, toStringMode);
                }
                else
                {
                    Debug.Assert(toStringMode == Ice.ToStringMode.ASCII && char.IsSurrogate(c));
                    if (i + 1 == s.Length)
                    {
                        throw new System.ArgumentException("High surrogate without low surrogate");
                    }
                    else
                    {
                        i++;
                        int codePoint = char.ConvertToUtf32(c, s[i]);
                        // append \Unnnnnnnn
                        result.Append("\\U");
                        string hex = System.Convert.ToString(codePoint, 16);
                        for (int j = hex.Length; j < 8; j++)
                        {
                            result.Append('0');
                        }
                        result.Append(hex);
                    }
                }
            }

            return result.ToString();
        }
    }

    private static char
    checkChar(string s, int pos)
    {
        char c = s[pos];
        if (c < 32 || c == 127)
        {
            string msg;
            if (pos > 0)
            {
                msg = $"character after `{s[..pos]}'";
            }
            else
            {
                msg = "first character";
            }
            msg += " is not a printable ASCII character (ordinal " + (int)c + ")";
            throw new System.ArgumentException(msg);
        }
        return c;
    }

    //
    // Decode the character or escape sequence starting at start and appends it to result;
    // returns the index of the first character following the decoded character
    // or escape sequence.
    //
    private static int
    decodeChar(string s, int start, int end, string special, StringBuilder result, UTF8Encoding utf8Encoding)
    {
        Debug.Assert(start >= 0);
        Debug.Assert(start < end);
        Debug.Assert(end <= s.Length);

        if (s[start] != '\\')
        {
            result.Append(checkChar(s, start++));
        }
        else if (start + 1 == end)
        {
            ++start;
            result.Append('\\'); // trailing backslash
        }
        else
        {
            char c = s[++start];

            switch (c)
            {
                case '\\':
                case '\'':
                case '"':
                case '?':
                {
                    ++start;
                    result.Append(c);
                    break;
                }
                case 'a':
                {
                    ++start;
                    result.Append('\a');
                    break;
                }
                case 'b':
                {
                    ++start;
                    result.Append('\b');
                    break;
                }
                case 'f':
                {
                    ++start;
                    result.Append('\f');
                    break;
                }
                case 'n':
                {
                    ++start;
                    result.Append('\n');
                    break;
                }
                case 'r':
                {
                    ++start;
                    result.Append('\r');
                    break;
                }
                case 't':
                {
                    ++start;
                    result.Append('\t');
                    break;
                }
                case 'v':
                {
                    ++start;
                    result.Append('\v');
                    break;
                }
                case 'u':
                case 'U':
                {
                    int codePoint = 0;
                    bool inBMP = c == 'u';
                    int size = inBMP ? 4 : 8;
                    ++start;
                    while (size > 0 && start < end)
                    {
                        c = s[start++];
                        int charVal;
                        if (c >= '0' && c <= '9')
                        {
                            charVal = c - '0';
                        }
                        else if (c >= 'a' && c <= 'f')
                        {
                            charVal = 10 + (c - 'a');
                        }
                        else if (c >= 'A' && c <= 'F')
                        {
                            charVal = 10 + (c - 'A');
                        }
                        else
                        {
                            break; // while
                        }
                        codePoint = (codePoint * 16) + charVal;
                        --size;
                    }
                    if (size > 0)
                    {
                        throw new System.ArgumentException("Invalid universal character name: too few hex digits");
                    }
                    if (codePoint >= 0xD800 && codePoint <= 0xDFFF)
                    {
                        throw new System.ArgumentException("A universal character name cannot designate a surrogate");
                    }
                    if (inBMP || codePoint <= 0xFFFF)
                    {
                        result.Append((char)codePoint);
                    }
                    else
                    {
                        result.Append(char.ConvertFromUtf32(codePoint));
                    }
                    break;
                }

                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case 'x':
                {
                    // UTF-8 byte sequence encoded with octal escapes

                    byte[] arr = new byte[end - start];
                    int i = 0;
                    bool more = true;
                    while (more)
                    {
                        int val = 0;
                        if (c == 'x')
                        {
                            int size = 2;
                            ++start;
                            while (size > 0 && start < end)
                            {
                                c = s[start++];
                                int charVal;
                                if (c >= '0' && c <= '9')
                                {
                                    charVal = c - '0';
                                }
                                else if (c >= 'a' && c <= 'f')
                                {
                                    charVal = 10 + (c - 'a');
                                }
                                else if (c >= 'A' && c <= 'F')
                                {
                                    charVal = 10 + (c - 'A');
                                }
                                else
                                {
                                    --start; // move back
                                    break; // while
                                }
                                val = (val * 16) + charVal;
                                --size;
                            }
                            if (size == 2)
                            {
                                throw new System.ArgumentException("Invalid \\x escape sequence: no hex digit");
                            }
                        }
                        else
                        {
                            for (int j = 0; j < 3 && start < end; ++j)
                            {
                                int charVal = s[start++] - '0';
                                if (charVal < 0 || charVal > 7)
                                {
                                    --start; // move back
                                    Debug.Assert(j != 0); // must be at least one digit
                                    break; // for
                                }
                                val = (val * 8) + charVal;
                            }
                            if (val > 255)
                            {
                                string msg =
                                    "octal value \\" +
                                    System.Convert.ToString(val, 8) +
                                    " (" + val + ") is out of range";
                                throw new System.ArgumentException(msg);
                            }
                        }

                        arr[i++] = (byte)val;

                        more = false;

                        if ((start + 1 < end) && s[start] == '\\')
                        {
                            c = s[start + 1];
                            if (c == 'x' || (c >= '0' && c <= '9'))
                            {
                                start++;
                                more = true;
                            }
                        }
                    }

                    result.Append(utf8Encoding.GetString(arr, 0, i)); // May raise ArgumentException.
                    break;
                }
                default:
                {
                    if (string.IsNullOrEmpty(special) || !special.Contains(c, StringComparison.Ordinal))
                    {
                        result.Append('\\'); // not in special, so we keep the backslash
                    }
                    result.Append(checkChar(s, start++));
                    break;
                }
            }
        }
        return start;
    }

    //
    // Remove escape sequences added by escapeString. Throws System.ArgumentException
    // for an invalid input string.
    //
    public static string unescapeString(string s, int start, int end, string special)
    {
        Debug.Assert(start >= 0 && start <= end && end <= s.Length);

        if (special != null)
        {
            for (int i = 0; i < special.Length; ++i)
            {
                if (special[i] < 32 || special[i] > 126)
                {
                    throw new ArgumentException("special characters must be in ASCII range 32-126", nameof(special));
                }
            }
        }

        // Optimization for strings without escapes
        if (start == end || s.IndexOf('\\', start, end - start) == -1)
        {
            int p = start;
            while (p < end)
            {
                checkChar(s, p++);
            }
            return s[start..end];
        }
        else
        {
            var sb = new StringBuilder(end - start);
            var utf8Encoding = new UTF8Encoding(false, true);
            while (start < end)
            {
                start = decodeChar(s, start, end, special, sb, utf8Encoding);
            }
            return sb.ToString();
        }
    }

    //
    // Split string helper; returns null for unmatched quotes
    //
    public static string[] splitString(string str, string delim)
    {
        var l = new List<string>();
        char[] arr = new char[str.Length];
        int pos = 0;

        int n = 0;
        char quoteChar = '\0';
        while (pos < str.Length)
        {
            if (quoteChar == '\0' && (str[pos] == '"' || str[pos] == '\''))
            {
                quoteChar = str[pos++];
                continue; // Skip the quote.
            }
            else if (quoteChar == '\0' && str[pos] == '\\' && pos + 1 < str.Length &&
                    (str[pos + 1] == '\'' || str[pos + 1] == '"'))
            {
                ++pos; // Skip the backslash
            }
            else if (quoteChar != '\0' && str[pos] == '\\' && pos + 1 < str.Length && str[pos + 1] == quoteChar)
            {
                ++pos; // Skip the backslash
            }
            else if (quoteChar != '\0' && str[pos] == quoteChar)
            {
                ++pos;
                quoteChar = '\0';
                continue; // Skip the quote.
            }
            else if (delim.Contains(str[pos], StringComparison.Ordinal))
            {
                if (quoteChar == '\0')
                {
                    ++pos;
                    if (n > 0)
                    {
                        l.Add(new string(arr, 0, n));
                        n = 0;
                    }
                    continue;
                }
            }

            if (pos < str.Length)
            {
                arr[n++] = str[pos++];
            }
        }

        if (n > 0)
        {
            l.Add(new string(arr, 0, n));
        }
        if (quoteChar != '\0')
        {
            return null; // Unmatched quote.
        }
        return l.ToArray();
    }

    public static int checkQuote(string s) => checkQuote(s, 0);

    //
    // If a single or double quotation mark is found at the start position,
    // then the position of the matching closing quote is returned. If no
    // quotation mark is found at the start position, then 0 is returned.
    // If no matching closing quote is found, then -1 is returned.
    //
    public static int checkQuote(string s, int start)
    {
        char quoteChar = s[start];
        if (quoteChar == '"' || quoteChar == '\'')
        {
            start++;
            int len = s.Length;
            int pos;
            while (start < len && (pos = s.IndexOf(quoteChar, start)) != -1)
            {
                if (s[pos - 1] != '\\')
                {
                    return pos;
                }
                start = pos + 1;
            }
            return -1; // Unmatched quote
        }
        return 0; // Not quoted
    }

    public static bool match(string s, string pat, bool emptyMatch)
    {
        Debug.Assert(s.Length > 0);
        Debug.Assert(pat.Length > 0);

        //
        // If pattern does not contain a wildcard just compare strings.
        //
        int beginIndex = pat.IndexOf('*', StringComparison.Ordinal);
        if (beginIndex < 0)
        {
            return s.Equals(pat, StringComparison.Ordinal);
        }

        //
        // Make sure start of the strings match
        //
        if (beginIndex > s.Length ||
            !s[..beginIndex].Equals(pat[..beginIndex], StringComparison.Ordinal))
        {
            return false;
        }

        //
        // Make sure there is something present in the middle to match the
        // wildcard. If emptyMatch is true, allow a match of "".
        //
        int endLength = pat.Length - beginIndex - 1;
        if (endLength > s.Length)
        {
            return false;
        }
        int endIndex = s.Length - endLength;
        if (endIndex < beginIndex || (!emptyMatch && endIndex == beginIndex))
        {
            return false;
        }

        //
        // Make sure end of the strings match
        //
        if (!s[endIndex..].Equals(
               pat.Substring(beginIndex + 1, pat.Length - beginIndex - 1),
               StringComparison.Ordinal))
        {
            return false;
        }

        return true;
    }

    private class OrdinalStringComparerImpl : IComparer<string>
    {
        public int Compare(string l, string r) => string.CompareOrdinal(l, r);
    }

    public static IComparer<string> OrdinalStringComparer = new OrdinalStringComparerImpl();
}
