//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
//
// Ice version 3.7.10
//
// <auto-generated>
//
// Generated from file `Properties.ice'
//
// Warning: do not edit this file.
//
// </auto-generated>
//


using _System = global::System;

#pragma warning disable 1591

namespace Ice
{
    [global::System.Runtime.InteropServices.ComVisible(false)]
    [global::System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704")]
    [global::System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1707")]
    [global::System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1709")]
    [global::System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1710")]
    [global::System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1711")]
    [global::System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1715")]
    [global::System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1716")]
    [global::System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1720")]
    [global::System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1722")]
    [global::System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1724")]
    public partial interface Properties
    {
        #region Slice operations


        /// <summary>
        /// Get a property by key.
        /// If the property is not set, an empty string is returned.
        /// </summary>
        ///  <param name="key">The property key.
        ///  </param>
        /// <returns>The property value.
        ///  </returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        string getProperty(string key);


        /// <summary>
        /// Get a property by key.
        /// If the property is not set, the given default value is returned.
        /// </summary>
        ///  <param name="key">The property key.
        ///  </param>
        /// <param name="value">The default value to use if the property does not exist.
        ///  </param>
        /// <returns>The property value or the default value.
        ///  </returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        string getPropertyWithDefault(string key, string value);


        /// <summary>
        /// Get a property as an integer.
        /// If the property is not set, 0 is returned.
        /// </summary>
        ///  <param name="key">The property key.
        ///  </param>
        /// <returns>The property value interpreted as an integer.
        ///  </returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        int getPropertyAsInt(string key);


        /// <summary>
        /// Get a property as an integer.
        /// If the property is not set, the given default value is returned.
        /// </summary>
        ///  <param name="key">The property key.
        ///  </param>
        /// <param name="value">The default value to use if the property does not exist.
        ///  </param>
        /// <returns>The property value interpreted as an integer, or the default value.
        ///  </returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        int getPropertyAsIntWithDefault(string key, int value);


        /// <summary>
        /// Get a property as a list of strings.
        /// The strings must be separated by whitespace or comma. If the property is
        ///  not set, an empty list is returned. The strings in the list can contain whitespace and commas if they are
        ///  enclosed in single or double quotes. If quotes are mismatched, an empty list is returned. Within single quotes
        ///  or double quotes, you can escape the quote in question with a backslash, e.g. O'Reilly can be written as
        ///  O'Reilly, "O'Reilly" or 'O\'Reilly'.
        /// </summary>
        ///  <param name="key">The property key.
        ///  </param>
        /// <returns>The property value interpreted as a list of strings.
        ///  </returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        string[] getPropertyAsList(string key);


        /// <summary>
        /// Get a property as a list of strings.
        /// The strings must be separated by whitespace or comma. If the property is
        ///  not set, the default list is returned. The strings in the list can contain whitespace and commas if they are
        ///  enclosed in single or double quotes. If quotes are mismatched, the default list is returned. Within single
        ///  quotes or double quotes, you can escape the quote in question with a backslash, e.g. O'Reilly can be written as
        ///  O'Reilly, "O'Reilly" or 'O\'Reilly'.
        /// </summary>
        ///  <param name="key">The property key.
        ///  </param>
        /// <param name="value">The default value to use if the property is not set.
        ///  </param>
        /// <returns>The property value interpreted as list of strings, or the default value.
        ///  </returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        string[] getPropertyAsListWithDefault(string key, string[] value);


        /// <summary>
        /// Get all properties whose keys begins with prefix.
        /// If prefix is an empty string, then all
        ///  properties are returned.
        /// </summary>
        ///  <param name="prefix">The prefix to search for (empty string if none).
        ///  </param>
        /// <returns>The matching property set.</returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        global::System.Collections.Generic.Dictionary<string, string> getPropertiesForPrefix(string prefix);


        /// <summary>
        /// Set a property.
        /// To unset a property, set it to the empty string.
        /// </summary>
        ///  <param name="key">The property key.
        ///  </param>
        /// <param name="value">The property value.
        ///  </param>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        void setProperty(string key, string value);


        /// <summary>
        /// Get a sequence of command-line options that is equivalent to this property set.
        /// Each element of the returned
        ///  sequence is a command-line option of the form --key=value.
        /// </summary>
        ///  <returns>The command line options for this property set.</returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        string[] getCommandLineOptions();


        /// <summary>
        /// Convert a sequence of command-line options into properties.
        /// All options that begin with
        ///  --prefix. are converted into properties. If the prefix is empty, all options that begin with
        ///  -- are converted to properties.
        /// </summary>
        ///  <param name="prefix">The property prefix, or an empty string to convert all options starting with --.
        ///  </param>
        /// <param name="options">The command-line options.
        ///  </param>
        /// <returns>The command-line options that do not start with the specified prefix, in their original order.</returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        string[] parseCommandLineOptions(string prefix, string[] options);


        /// <summary>
        /// Convert a sequence of command-line options into properties.
        /// All options that begin with one of the following
        ///  prefixes are converted into properties: --Ice, --IceBox, --IceGrid,
        ///  --IcePatch2, --IceSSL, --IceStorm, --Freeze, and --Glacier2.
        /// </summary>
        ///  <param name="options">The command-line options.
        ///  </param>
        /// <returns>The command-line options that do not start with one of the listed prefixes, in their original order.</returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        string[] parseIceCommandLineOptions(string[] options);


        /// <summary>
        /// Load properties from a file.
        /// </summary>
        /// <param name="file">The property file.</param>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        void load(string file);


        /// <summary>
        /// Create a copy of this property set.
        /// </summary>
        /// <returns>A copy of this property set.</returns>

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
        Properties ice_clone_();

        #endregion
    }
}

namespace Ice
{
}