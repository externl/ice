//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
//
// Ice version 3.7.3
//
// <auto-generated>
//
// Generated from file `Version.ice'
//
// Warning: do not edit this file.
//
// </auto-generated>
//

using _System = global::System;

#pragma warning disable 1591

namespace Ice
{
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
    [global::System.Serializable]
    public partial struct ProtocolVersion
    {
        #region Slice data members

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.3")]
        public byte major;

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.3")]
        public byte minor;

        #endregion

        partial void ice_initialize();

        #region Constructor

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.3")]
        public ProtocolVersion(byte major, byte minor)
        {
            this.major = major;
            this.minor = minor;
            ice_initialize();
        }

        #endregion

        #region Object members

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.3")]
        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::ProtocolVersion");
            global::IceInternal.HashUtil.hashAdd(ref h_, major);
            global::IceInternal.HashUtil.hashAdd(ref h_, minor);
            return h_;
        }

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.3")]
        public override bool Equals(object other)
        {
            if(!(other is ProtocolVersion))
            {
                return false;
            }
            ProtocolVersion o = (ProtocolVersion)other;
            if(!this.major.Equals(o.major))
            {
                return false;
            }
            if(!this.minor.Equals(o.minor))
            {
                return false;
            }
            return true;
        }

        #endregion

        #region Comparison members

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.3")]
        public static bool operator==(ProtocolVersion lhs, ProtocolVersion rhs)
        {
            return Equals(lhs, rhs);
        }

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.3")]
        public static bool operator!=(ProtocolVersion lhs, ProtocolVersion rhs)
        {
            return !Equals(lhs, rhs);
        }

        #endregion

        #region Marshaling support

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.3")]
        public void ice_writeMembers(OutputStream ostr)
        {
            ostr.WriteByte(this.major);
            ostr.WriteByte(this.minor);
        }

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.3")]
        public void ice_readMembers(InputStream istr)
        {
            this.major = istr.ReadByte();
            this.minor = istr.ReadByte();
        }

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.3")]
        public static void ice_write(OutputStream ostr, ProtocolVersion v)
        {
            v.ice_writeMembers(ostr);
        }

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.3")]
        public static ProtocolVersion ice_read(InputStream istr)
        {
            var v = new ProtocolVersion();
            v.ice_readMembers(istr);
            return v;
        }

        #endregion
    }

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
    [global::System.Serializable]
    public partial struct EncodingVersion
    {
        #region Slice data members

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.3")]
        public byte major;

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.3")]
        public byte minor;

        #endregion

        partial void ice_initialize();

        #region Constructor

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.3")]
        public EncodingVersion(byte major, byte minor)
        {
            this.major = major;
            this.minor = minor;
            ice_initialize();
        }

        #endregion

        #region Object members

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.3")]
        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::EncodingVersion");
            global::IceInternal.HashUtil.hashAdd(ref h_, major);
            global::IceInternal.HashUtil.hashAdd(ref h_, minor);
            return h_;
        }

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.3")]
        public override bool Equals(object other)
        {
            if(!(other is EncodingVersion))
            {
                return false;
            }
            EncodingVersion o = (EncodingVersion)other;
            if(!this.major.Equals(o.major))
            {
                return false;
            }
            if(!this.minor.Equals(o.minor))
            {
                return false;
            }
            return true;
        }

        #endregion

        #region Comparison members

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.3")]
        public static bool operator==(EncodingVersion lhs, EncodingVersion rhs)
        {
            return Equals(lhs, rhs);
        }

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.3")]
        public static bool operator!=(EncodingVersion lhs, EncodingVersion rhs)
        {
            return !Equals(lhs, rhs);
        }

        #endregion

        #region Marshaling support

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.3")]
        public void ice_writeMembers(OutputStream ostr)
        {
            ostr.WriteByte(this.major);
            ostr.WriteByte(this.minor);
        }

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.3")]
        public void ice_readMembers(InputStream istr)
        {
            this.major = istr.ReadByte();
            this.minor = istr.ReadByte();
        }

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.3")]
        public static void ice_write(OutputStream ostr, EncodingVersion v)
        {
            v.ice_writeMembers(ostr);
        }

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.3")]
        public static EncodingVersion ice_read(InputStream istr)
        {
            var v = new EncodingVersion();
            v.ice_readMembers(istr);
            return v;
        }

        #endregion
    }
}
