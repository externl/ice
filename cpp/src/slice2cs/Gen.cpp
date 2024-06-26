//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Gen.h"
#include "IceUtil/FileUtil.h"
#include "IceUtil/StringUtil.h"

#include <limits>
#ifndef _WIN32
#    include <unistd.h>
#else
#    include <direct.h>
#endif

#include "../Slice/FileTracker.h"
#include "../Slice/Util.h"
#include "DotNetNames.h"
#include "IceUtil/UUID.h"
#include <algorithm>
#include <cassert>
#include <iterator>
#include <string.h>

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

namespace
{
    string sliceModeToIceMode(Operation::Mode opMode, string ns)
    {
        string mode;
        switch (opMode)
        {
            case Operation::Normal:
            {
                mode = CsGenerator::getUnqualified("Ice.OperationMode.Normal", ns);
                break;
            }
            case Operation::Idempotent:
            {
                mode = CsGenerator::getUnqualified("Ice.OperationMode.Idempotent", ns);
                break;
            }
            default:
            {
                assert(false);
                break;
            }
        }
        return mode;
    }

    string opFormatTypeToString(const OperationPtr& op, string ns)
    {
        switch (op->format())
        {
            case DefaultFormat:
            {
                return CsGenerator::getUnqualified("Ice.FormatType.DefaultFormat", ns);
            }
            case CompactFormat:
            {
                return CsGenerator::getUnqualified("Ice.FormatType.CompactFormat", ns);
            }
            case SlicedFormat:
            {
                return CsGenerator::getUnqualified("Ice.FormatType.SlicedFormat", ns);
            }
            default:
            {
                assert(false);
            }
        }

        return "???";
    }

    string getDeprecationMessageForComment(const ContainedPtr& p1, const string& type)
    {
        string deprecateReason;
        if (p1->isDeprecated(true))
        {
            if (auto reason = p1->getDeprecationReason(true))
            {
                deprecateReason = *reason;
            }
            else
            {
                deprecateReason = "This " + type + " has been deprecated.";
            }
        }
        return deprecateReason;
    }

    void emitObsoleteAttribute(const ContainedPtr& p1, Output& out)
    {
        if (p1->isDeprecated(true))
        {
            if (auto reason = p1->getDeprecationReason(true))
            {
                out << nl << "[global::System.Obsolete(\"" << *reason << "\")]";
            }
            else
            {
                out << nl << "[global::System.Obsolete]";
            }
        }
    }

    string getEscapedParamName(const OperationPtr& p, const string& name)
    {
        ParamDeclList params = p->parameters();

        for (ParamDeclList::const_iterator i = params.begin(); i != params.end(); ++i)
        {
            if ((*i)->name() == name)
            {
                return name + "_";
            }
        }
        return name;
    }

    string getEscapedParamName(const DataMemberList& params, const string& name)
    {
        for (DataMemberList::const_iterator i = params.begin(); i != params.end(); ++i)
        {
            if ((*i)->name() == name)
            {
                return name + "_";
            }
        }
        return name;
    }

    string resultStructReturnValueName(const ParamDeclList& outParams)
    {
        for (ParamDeclList::const_iterator i = outParams.begin(); i != outParams.end(); ++i)
        {
            if ((*i)->name() == "returnValue")
            {
                return "returnValue_";
            }
        }
        return "returnValue";
    }
}

Slice::CsVisitor::CsVisitor(Output& out) : _out(out) {}

Slice::CsVisitor::~CsVisitor() {}

void
Slice::CsVisitor::writeMarshalUnmarshalParams(
    const ParamDeclList& params,
    const OperationPtr& op,
    bool marshal,
    const string& ns,
    bool resultStruct,
    bool publicNames,
    const string& customStream)
{
    ParamDeclList optionals;

    string paramPrefix = "";
    string returnValueS = "ret";

    if (op && resultStruct)
    {
        if ((op->returnType() && !params.empty()) || params.size() > 1)
        {
            paramPrefix = "ret.";
            returnValueS = resultStructReturnValueName(params);
        }
    }

    for (ParamDeclList::const_iterator pli = params.begin(); pli != params.end(); ++pli)
    {
        string param = paramPrefix.empty() && !publicNames ? "iceP_" + (*pli)->name() : fixId((*pli)->name());
        TypePtr type = (*pli)->type();
        if (!marshal && isClassType(type))
        {
            ostringstream os;
            os << '(' << typeToString(type, ns) << " v) => { " << paramPrefix << param << " = v; }";
            param = os.str();
        }
        else
        {
            param = paramPrefix + param;
        }

        if ((*pli)->optional())
        {
            optionals.push_back(*pli);
        }
        else
        {
            writeMarshalUnmarshalCode(_out, type, ns, param, marshal, customStream);
        }
    }

    TypePtr ret;

    if (op && op->returnType())
    {
        ret = op->returnType();
        string param;
        if (!marshal && isClassType(ret))
        {
            ostringstream os;
            os << '(' << typeToString(ret, ns) << " v) => {" << paramPrefix << returnValueS << " = v; }";
            param = os.str();
        }
        else
        {
            param = paramPrefix + returnValueS;
        }

        if (!op->returnIsOptional())
        {
            writeMarshalUnmarshalCode(_out, ret, ns, param, marshal, customStream);
        }
    }

    //
    // Sort optional parameters by tag.
    //
    class SortFn
    {
    public:
        static bool compare(const ParamDeclPtr& lhs, const ParamDeclPtr& rhs) { return lhs->tag() < rhs->tag(); }
    };
    optionals.sort(SortFn::compare);

    //
    // Handle optional parameters.
    //
    bool checkReturnType = op && op->returnIsOptional();
    for (ParamDeclList::const_iterator pli = optionals.begin(); pli != optionals.end(); ++pli)
    {
        if (checkReturnType && op->returnTag() < (*pli)->tag())
        {
            string param;
            if (!marshal && isClassType(ret))
            {
                ostringstream os;
                os << '(' << typeToString(ret, ns) << " v) => {" << paramPrefix << returnValueS << " = v; }";
                param = os.str();
            }
            else
            {
                param = paramPrefix + returnValueS;
            }
            writeOptionalMarshalUnmarshalCode(_out, ret, ns, param, op->returnTag(), marshal, customStream);
            checkReturnType = false;
        }

        string param = paramPrefix.empty() && !publicNames ? "iceP_" + (*pli)->name() : fixId((*pli)->name());
        TypePtr type = (*pli)->type();
        if (!marshal && isClassType(type))
        {
            ostringstream os;
            os << '(' << typeToString(type, ns) << " v) => {" << paramPrefix << param << " = v; }";
            param = os.str();
        }
        else
        {
            param = paramPrefix + param;
        }

        writeOptionalMarshalUnmarshalCode(_out, type, ns, param, (*pli)->tag(), marshal, customStream);
    }

    if (checkReturnType)
    {
        string param;
        if (!marshal && isClassType(ret))
        {
            ostringstream os;
            os << '(' << typeToString(ret, ns) << " v) => {" << paramPrefix << returnValueS << " = v; }";
            param = os.str();
        }
        else
        {
            param = paramPrefix + returnValueS;
        }
        writeOptionalMarshalUnmarshalCode(_out, ret, ns, param, op->returnTag(), marshal, customStream);
    }
}

void
Slice::CsVisitor::writeMarshalDataMember(
    const DataMemberPtr& member,
    const string& name,
    const string& ns,
    bool forStruct)
{
    if (member->optional())
    {
        assert(!forStruct);
        writeOptionalMarshalUnmarshalCode(_out, member->type(), ns, name, member->tag(), true, "ostr_");
    }
    else
    {
        string stream = forStruct ? "" : "ostr_";
        string memberName = name;
        if (forStruct)
        {
            memberName = "this." + memberName;
        }

        writeMarshalUnmarshalCode(_out, member->type(), ns, memberName, true, stream);
    }
}

void
Slice::CsVisitor::writeUnmarshalDataMember(
    const DataMemberPtr& member,
    const string& name,
    const string& ns,
    bool forStruct)
{
    string param = name;
    if (isClassType(member->type()))
    {
        ostringstream os;
        os << '(' << typeToString(member->type(), ns) << " v) => { this." << name << " = v; }";
        param = os.str();
    }
    else if (forStruct)
    {
        param = "this." + name;
    }

    if (member->optional())
    {
        assert(!forStruct);
        writeOptionalMarshalUnmarshalCode(_out, member->type(), ns, param, member->tag(), false, "istr_");
    }
    else
    {
        writeMarshalUnmarshalCode(_out, member->type(), ns, param, false, forStruct ? "" : "istr_");
    }
}

void
Slice::CsVisitor::writeInheritedOperations(const InterfaceDefPtr& p)
{
    InterfaceList bases = p->bases();
    if (!bases.empty())
    {
        _out << sp << nl << "#region Inherited Slice operations";

        OperationList allOps;
        for (InterfaceList::const_iterator q = bases.begin(); q != bases.end(); ++q)
        {
            OperationList tmp = (*q)->allOperations();
            allOps.splice(allOps.end(), tmp);
        }
        allOps.sort();
        allOps.unique();
        for (OperationList::const_iterator i = allOps.begin(); i != allOps.end(); ++i)
        {
            string retS;
            vector<string> params, args;
            string ns = getNamespace(p);
            string name = getDispatchParams(*i, retS, params, args, ns);
            _out << sp << nl << "public abstract " << retS << " " << name << spar << params << epar << ';';
        }

        _out << sp << nl << "#endregion"; // Inherited Slice operations
    }
}

void
Slice::CsVisitor::writeDispatch(const InterfaceDefPtr& p)
{
    string name = fixId(p->name());
    string scoped = p->scoped();
    string ns = getNamespace(p);

    _out << sp << nl << "#region Slice type-related members";
    _out << sp;
    _out << nl << "public override string ice_id(" << getUnqualified("Ice.Current", ns)
         << " current) => ice_staticId();";

    _out << sp;
    _out << nl << "public static new string ice_staticId() => \"" << scoped << "\";";

    _out << sp << nl << "#endregion"; // Slice type-related members

    OperationList ops = p->operations();
    if (ops.size() != 0)
    {
        _out << sp << nl << "#region Operation dispatch";
    }

    for (OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        InterfaceDefPtr interface = op->interface();
        assert(interface);

        string opName = op->name();
        _out << sp;
        _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Design\", \"CA1011\")]";
        _out << nl << "public static global::System.Threading.Tasks.Task<" << getUnqualified("Ice.OutputStream", ns)
             << ">";
        _out << nl << "iceD_" << opName << "(" << name << " obj, "
             << "global::Ice.Internal.Incoming inS, " << getUnqualified("Ice.Current", ns) << " current)";
        _out << sb;

        TypePtr ret = op->returnType();
        ParamDeclList inParams = op->inParameters();
        ParamDeclList outParams = op->outParameters();

        _out << nl << getUnqualified("Ice.ObjectImpl", ns) << ".iceCheckMode(" << sliceModeToIceMode(op->mode(), ns)
             << ", current.mode);";
        if (!inParams.empty())
        {
            //
            // Unmarshal 'in' parameters.
            //
            _out << nl << "var istr = inS.startReadParams();";
            for (ParamDeclList::const_iterator pli = inParams.begin(); pli != inParams.end(); ++pli)
            {
                string param = "iceP_" + (*pli)->name();
                string typeS = typeToString((*pli)->type(), ns, (*pli)->optional());

                _out << nl << typeS << ' ' << param << (isClassType((*pli)->type()) ? " = null;" : ";");
            }
            writeMarshalUnmarshalParams(inParams, 0, false, ns);
            if (op->sendsClasses(false))
            {
                _out << nl << "istr.readPendingValues();";
            }
            _out << nl << "inS.endReadParams();";
        }
        else
        {
            _out << nl << "inS.readEmptyParams();";
        }

        if (op->format() != DefaultFormat)
        {
            _out << nl << "inS.setFormat(" << opFormatTypeToString(op, ns) << ");";
        }

        vector<string> inArgs;
        for (ParamDeclList::const_iterator pli = inParams.begin(); pli != inParams.end(); ++pli)
        {
            inArgs.push_back("iceP_" + (*pli)->name());
        }

        const bool amd = p->hasMetaData("amd") || op->hasMetaData("amd");
        if (op->hasMarshaledResult())
        {
            _out << nl << "return inS." << (amd ? "setMarshaledResultTask" : "setMarshaledResult");
            _out << "(obj." << opName << (amd ? "Async" : "") << spar << inArgs << "current" << epar << ");";
            _out << eb;
        }
        else if (amd)
        {
            string retS = resultType(op, ns);
            _out << nl << "return inS.setResultTask" << (retS.empty() ? "" : ('<' + retS + '>'));
            _out << "(obj." << opName << "Async" << spar << inArgs << "current" << epar;
            if (!retS.empty())
            {
                _out << ",";
                _out.inc();
                if (!ret && outParams.size() == 1)
                {
                    _out << nl << "(ostr, "
                         << "iceP_" << outParams.front()->name() << ") =>";
                }
                else
                {
                    _out << nl << "(ostr, ret) =>";
                }
                _out << sb;
                writeMarshalUnmarshalParams(outParams, op, true, ns, true);
                if (op->returnsClasses(false))
                {
                    _out << nl << "ostr.writePendingValues();";
                }
                _out << eb;
                _out.dec();
            }
            _out << ");";
            _out << eb;
        }
        else
        {
            for (ParamDeclList::const_iterator pli = outParams.begin(); pli != outParams.end(); ++pli)
            {
                string typeS = typeToString((*pli)->type(), ns, (*pli)->optional());
                _out << nl << typeS << ' ' << "iceP_" + (*pli)->name() << ";";
            }

            //
            // Call on the servant.
            //
            _out << nl;
            if (ret)
            {
                _out << "var ret = ";
            }
            _out << "obj." << fixId(opName, DotNet::ICloneable, true) << spar << inArgs;
            for (ParamDeclList::const_iterator pli = outParams.begin(); pli != outParams.end(); ++pli)
            {
                _out << "out iceP_" + (*pli)->name();
            }
            _out << "current" << epar << ';';

            //
            // Marshal 'out' parameters and return value.
            //
            if (!outParams.empty() || ret)
            {
                _out << nl << "var ostr = inS.startWriteParams();";
                writeMarshalUnmarshalParams(outParams, op, true, ns);
                if (op->returnsClasses(false))
                {
                    _out << nl << "ostr.writePendingValues();";
                }
                _out << nl << "inS.endWriteParams(ostr);";
                _out << nl << "return inS.setResult(ostr);";
            }
            else
            {
                _out << nl << "return inS.setResult(inS.writeEmptyParams());";
            }
            _out << eb;
        }
    }

    OperationList allOps = p->allOperations();
    if (!allOps.empty())
    {
        StringList allOpNames;
        transform(
            allOps.begin(),
            allOps.end(),
            back_inserter(allOpNames),
            [](const ContainedPtr& it) { return it->name(); });
        allOpNames.push_back("ice_id");
        allOpNames.push_back("ice_ids");
        allOpNames.push_back("ice_isA");
        allOpNames.push_back("ice_ping");
        allOpNames.sort();
        allOpNames.unique();

        _out << sp << nl << "private static readonly string[] _all =";
        _out << sb;
        for (StringList::const_iterator q = allOpNames.begin(); q != allOpNames.end();)
        {
            _out << nl << '"' << *q << '"';
            if (++q != allOpNames.end())
            {
                _out << ',';
            }
        }
        _out << eb << ';';

        _out << sp;
        _out << nl << "public override global::System.Threading.Tasks.Task<" << getUnqualified("Ice.OutputStream", ns)
             << ">?";
        _out << nl << "iceDispatch(global::Ice.Internal.Incoming inS, " << getUnqualified("Ice.Current", ns)
             << " current)";
        _out << sb;
        _out << nl << "int pos = global::System.Array.BinarySearch(_all, current.operation, "
             << "global::Ice.UtilInternal.StringUtil.OrdinalStringComparer);";
        _out << nl << "if(pos < 0)";
        _out << sb;
        _out << nl << "throw new " << getUnqualified("Ice.OperationNotExistException", ns)
             << "(current.id, current.facet, current.operation);";
        _out << eb;
        _out << sp << nl << "switch(pos)";
        _out << sb;
        int i = 0;
        for (StringList::const_iterator q = allOpNames.begin(); q != allOpNames.end(); ++q)
        {
            string opName = *q;

            _out << nl << "case " << i++ << ':';
            _out << sb;
            if (opName == "ice_id")
            {
                _out << nl << "return " << getUnqualified("Ice.ObjectImpl", ns) << ".iceD_ice_id(this, inS, current);";
            }
            else if (opName == "ice_ids")
            {
                _out << nl << "return " << getUnqualified("Ice.ObjectImpl", ns) << ".iceD_ice_ids(this, inS, current);";
            }
            else if (opName == "ice_isA")
            {
                _out << nl << "return " << getUnqualified("Ice.ObjectImpl", ns) << ".iceD_ice_isA(this, inS, current);";
            }
            else if (opName == "ice_ping")
            {
                _out << nl << "return " << getUnqualified("Ice.ObjectImpl", ns)
                     << ".iceD_ice_ping(this, inS, current);";
            }
            else
            {
                //
                // There's probably a better way to do this
                //
                for (OperationList::const_iterator t = allOps.begin(); t != allOps.end(); ++t)
                {
                    if ((*t)->name() == (*q))
                    {
                        InterfaceDefPtr interface = (*t)->interface();
                        assert(interface);
                        if (interface->scoped() == p->scoped())
                        {
                            _out << nl << "return iceD_" << opName << "(this, inS, current);";
                        }
                        else
                        {
                            _out << nl << "return " << getUnqualified(interface, ns, "", "Disp_") << ".iceD_" << opName
                                 << "(this, inS, current);";
                        }
                        break;
                    }
                }
            }
            _out << eb;
        }
        _out << eb;
        _out << sp << nl << "global::System.Diagnostics.Debug.Assert(false);";
        _out << nl << "throw new " << getUnqualified("Ice.OperationNotExistException", ns)
             << "(current.id, current.facet, current.operation);";
        _out << eb;
    }

    if (ops.size() != 0)
    {
        _out << sp << nl << "#endregion"; // Operation dispatch
    }
}

void
Slice::CsVisitor::writeMarshaling(const ClassDefPtr& p)
{
    string name = fixId(p->name());
    string scoped = p->scoped();
    string ns = getNamespace(p);
    ClassDefPtr base = p->base();

    //
    // Marshaling support
    //
    DataMemberList allClassMembers = p->allClassDataMembers();
    DataMemberList members = p->dataMembers();
    DataMemberList optionalMembers = p->orderedOptionalDataMembers();
    DataMemberList classMembers = p->classDataMembers();

    _out << sp << nl << "#region Marshaling support";

    _out << sp;
    emitGeneratedCodeAttribute();

    _out << nl << "protected override void iceWriteImpl(" << getUnqualified("Ice.OutputStream", ns) << " ostr_)";
    _out << sb;
    _out << nl << "ostr_.startSlice(ice_staticId(), " << p->compactId() << (!base ? ", true" : ", false") << ");";
    for (DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
    {
        if (!(*d)->optional())
        {
            writeMarshalDataMember(*d, fixId((*d)->name(), DotNet::ICloneable, true), ns);
        }
    }
    for (DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
    {
        writeMarshalDataMember(*d, fixId((*d)->name(), DotNet::ICloneable, true), ns);
    }
    _out << nl << "ostr_.endSlice();";
    if (base)
    {
        _out << nl << "base.iceWriteImpl(ostr_);";
    }
    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();

    _out << nl << "protected override void iceReadImpl(" << getUnqualified("Ice.InputStream", ns) << " istr_)";
    _out << sb;
    _out << nl << "istr_.startSlice();";
    for (DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
    {
        if (!(*d)->optional())
        {
            writeUnmarshalDataMember(*d, fixId((*d)->name(), DotNet::ICloneable, true), ns);
        }
    }
    for (DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
    {
        writeUnmarshalDataMember(*d, fixId((*d)->name(), DotNet::ICloneable, true), ns);
    }
    _out << nl << "istr_.endSlice();";
    if (base)
    {
        _out << nl << "base.iceReadImpl(istr_);";
    }
    _out << eb;

    _out << sp << nl << "#endregion"; // marshaling support
}

string
Slice::CsVisitor::getParamAttributes(const ParamDeclPtr& p)
{
    string result;
    StringList metaData = p->getMetaData();
    for (StringList::const_iterator i = metaData.begin(); i != metaData.end(); ++i)
    {
        static const string prefix = "cs:attribute:";
        if (i->find(prefix) == 0)
        {
            result += "[" + i->substr(prefix.size()) + "] ";
        }
    }
    return result;
}

vector<string>
Slice::CsVisitor::getParams(const OperationPtr& op, const string& ns)
{
    vector<string> params;
    ParamDeclList paramList = op->parameters();
    InterfaceDefPtr interface = op->interface();
    for (ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        string param = getParamAttributes(*q);
        if ((*q)->isOutParam())
        {
            param += "out ";
        }
        param += typeToString((*q)->type(), ns, (*q)->optional()) + " " + fixId((*q)->name());
        params.push_back(param);
    }
    return params;
}

vector<string>
Slice::CsVisitor::getInParams(const OperationPtr& op, const string& ns, bool internal)
{
    vector<string> params;

    string name = fixId(op->name());
    InterfaceDefPtr interface = op->interface();
    ParamDeclList paramList = op->inParameters();
    for (ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        params.push_back(
            getParamAttributes(*q) + typeToString((*q)->type(), ns, (*q)->optional()) + " " +
            (internal ? "iceP_" + (*q)->name() : fixId((*q)->name())));
    }
    return params;
}

vector<string>
Slice::CsVisitor::getOutParams(const OperationPtr& op, const string& ns, bool returnParam, bool outKeyword)
{
    vector<string> params;
    if (returnParam)
    {
        TypePtr ret = op->returnType();
        if (ret)
        {
            params.push_back(typeToString(ret, ns, op->returnIsOptional()) + " ret");
        }
    }

    ParamDeclList paramList = op->outParameters();
    for (ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        string s = getParamAttributes(*q);
        if (outKeyword)
        {
            s += "out ";
        }
        s += typeToString((*q)->type(), ns, (*q)->optional()) + ' ' + fixId((*q)->name());
        params.push_back(s);
    }

    return params;
}

vector<string>
Slice::CsVisitor::getArgs(const OperationPtr& op)
{
    vector<string> args;
    ParamDeclList paramList = op->parameters();
    for (ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        string arg = fixId((*q)->name());
        if ((*q)->isOutParam())
        {
            arg = "out " + arg;
        }
        args.push_back(arg);
    }
    return args;
}

vector<string>
Slice::CsVisitor::getInArgs(const OperationPtr& op, bool internal)
{
    vector<string> args;
    ParamDeclList paramList = op->parameters();
    for (ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if (!(*q)->isOutParam())
        {
            args.push_back(internal ? "iceP_" + (*q)->name() : fixId((*q)->name()));
        }
    }
    return args;
}

string
Slice::CsVisitor::getDispatchParams(
    const OperationPtr& op,
    string& retS,
    vector<string>& params,
    vector<string>& args,
    const string& ns)
{
    string name;
    InterfaceDefPtr interface = op->interface();
    ParamDeclList paramDecls;

    if (interface->hasMetaData("amd") || op->hasMetaData("amd"))
    {
        name = op->name() + "Async";
        params = getInParams(op, ns);
        args = getInArgs(op);
        paramDecls = op->inParameters();
        retS = taskResultType(op, ns, true);
    }
    else if (op->hasMarshaledResult())
    {
        name = fixId(op->name(), DotNet::Object, true);
        params = getInParams(op, ns);
        args = getInArgs(op);
        paramDecls = op->inParameters();
        retS = resultType(op, ns, true);
    }
    else
    {
        name = fixId(op->name(), DotNet::Object, true);
        params = getParams(op, ns);
        args = getArgs(op);
        paramDecls = op->parameters();
        retS = typeToString(op->returnType(), ns, op->returnIsOptional());
    }

    string currentParamName = getEscapedParamName(op, "current");
    params.push_back(getUnqualified("Ice.Current", ns) + " " + currentParamName);
    args.push_back(currentParamName);
    return name;
}

void
Slice::CsVisitor::emitAttributes(const ContainedPtr& p)
{
    StringList metaData = p->getMetaData();
    for (StringList::const_iterator i = metaData.begin(); i != metaData.end(); ++i)
    {
        static const string prefix = "cs:attribute:";
        if (i->find(prefix) == 0)
        {
            _out << nl << '[' << i->substr(prefix.size()) << ']';
        }
    }
}

void
Slice::CsVisitor::emitComVisibleAttribute()
{
    _out << nl << "[global::System.Runtime.InteropServices.ComVisible(false)]";
}

void
Slice::CsVisitor::emitGeneratedCodeAttribute()
{
    _out << nl << "[global::System.CodeDom.Compiler.GeneratedCodeAttribute(\"slice2cs\", \"" << ICE_STRING_VERSION
         << "\")]";
}

void
Slice::CsVisitor::emitNonBrowsableAttribute()
{
    _out << nl
         << "[global::System.ComponentModel.EditorBrowsable(global::System.ComponentModel.EditorBrowsableState.Never)]";
}

void
Slice::CsVisitor::emitPartialTypeAttributes()
{
    //
    // We are not supposed to mark an entire partial type with GeneratedCodeAttribute, therefore
    // FxCop may complain about naming convention violations. These attributes suppress those
    // warnings, but only when the generated code is compiled with /define:CODE_ANALYSIS.
    //
    _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1704\")]";
    _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1707\")]";
    _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1709\")]";
    _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1710\")]";
    _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1711\")]";
    _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1715\")]";
    _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1716\")]";
    _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1720\")]";
    _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1722\")]";
    _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1724\")]";
}

string
Slice::CsVisitor::writeValue(const TypePtr& type, const string& ns)
{
    assert(type);

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindBool:
            {
                return "false";
                break;
            }
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindInt:
            case Builtin::KindLong:
            {
                return "0";
                break;
            }
            case Builtin::KindFloat:
            {
                return "0.0f";
                break;
            }
            case Builtin::KindDouble:
            {
                return "0.0";
                break;
            }
            default:
            {
                return "null";
                break;
            }
        }
    }

    EnumPtr en = dynamic_pointer_cast<Enum>(type);
    if (en)
    {
        return typeToString(type, ns) + "." + fixId((*en->enumerators().begin())->name());
    }

    StructPtr st = dynamic_pointer_cast<Struct>(type);
    if (st && !isMappedToClass(st))
    {
        return "default";
    }

    return "null";
}

void
Slice::CsVisitor::writeConstantValue(const TypePtr& type, const SyntaxTreeBasePtr& valueType, const string& value)
{
    ConstPtr constant = dynamic_pointer_cast<Const>(valueType);
    if (constant)
    {
        _out << fixId(constant->scoped()) << ".value";
    }
    else
    {
        BuiltinPtr bp = dynamic_pointer_cast<Builtin>(type);
        if (bp && bp->kind() == Builtin::KindString)
        {
            _out << "\"" << toStringLiteral(value, "\a\b\f\n\r\t\v\0", "", UCN, 0) << "\"";
        }
        else if (bp && bp->kind() == Builtin::KindLong)
        {
            _out << value << "L";
        }
        else if (bp && bp->kind() == Builtin::KindFloat)
        {
            _out << value << "F";
        }
        else if (dynamic_pointer_cast<Enum>(type))
        {
            EnumeratorPtr lte = dynamic_pointer_cast<Enumerator>(valueType);
            assert(lte);
            _out << fixId(lte->scoped());
        }
        else
        {
            _out << value;
        }
    }
}

void
Slice::CsVisitor::writeDataMemberInitializers(const DataMemberList& dataMembers, unsigned int baseTypes)
{
    // Generates "= null!" for each non-optional collection and struct-mapped-to-a-class field.
    for (const auto& q : dataMembers)
    {
        if (!q->optional())
        {
            StructPtr st = dynamic_pointer_cast<Struct>(q->type());

            if (dynamic_pointer_cast<Sequence>(q->type()) || dynamic_pointer_cast<Dictionary>(q->type()) ||
                (st && isMappedToClass(st)))
            {
                _out << nl << "this." << fixId(q->name(), baseTypes) << " = null!;";
            }
        }
    }
}

string
Slice::CsVisitor::toCsIdent(const string& s)
{
    string::size_type pos = s.find('#');
    if (pos == string::npos)
    {
        return s;
    }

    string result = s;
    if (pos == 0)
    {
        return result.erase(0, 1);
    }

    result[pos] = '.';
    return result;
}

string
Slice::CsVisitor::editMarkup(const string& s)
{
    //
    // Strip HTML markup and javadoc links--VS doesn't display them.
    //
    string result = s;
    string::size_type pos = 0;
    do
    {
        pos = result.find('<', pos);
        if (pos != string::npos)
        {
            string::size_type endpos = result.find('>', pos);
            string::size_type wspos = result.find(' ', pos);
            if (endpos == string::npos && wspos == string::npos)
            {
                break;
            }

            if (wspos < endpos)
            {
                // If we found a whitespace before the end tag marker '>', The '<' char doesn't correspond to the start
                // of a HTML tag, and we replace it with &lt; escape code.
                result.replace(pos, 1, "&lt;", 4);
            }
            else
            {
                result.erase(pos, endpos - pos + 1);
            }
        }
    } while (pos != string::npos);

    // replace remaining '>' chars with '&gt;' escape code, tags have been already strip above.
    pos = 0;
    do
    {
        pos = result.find('>', pos);
        if (pos != string::npos)
        {
            result.replace(pos, 1, "&gt;", 4);
        }
    } while (pos != string::npos);

    const string link = "{@link";
    pos = 0;
    do
    {
        pos = result.find(link, pos);
        if (pos != string::npos)
        {
            result.erase(pos, link.size() + 1); // erase following white space too
            string::size_type endpos = result.find('}', pos);
            if (endpos != string::npos)
            {
                string ident = result.substr(pos, endpos - pos);
                result.erase(pos, endpos - pos + 1);
                result.insert(pos, toCsIdent(ident));
            }
        }
    } while (pos != string::npos);

    //
    // Strip @see sections because VS does not display them.
    //
    static const string seeTag = "@see";
    pos = 0;
    do
    {
        //
        // Look for the next @ and delete up to that, or
        // to the end of the string, if not found.
        //
        pos = result.find(seeTag, pos);
        if (pos != string::npos)
        {
            string::size_type next = result.find('@', pos + seeTag.size());
            if (next != string::npos)
            {
                result.erase(pos, next - pos);
            }
            else
            {
                result.erase(pos, string::npos);
            }
        }
    } while (pos != string::npos);

    //
    // Replace @param, @return, and @throws with corresponding <param>, <returns>, and <exception> tags.
    //
    static const string paramTag = "@param";
    pos = 0;
    do
    {
        pos = result.find(paramTag, pos);
        if (pos != string::npos)
        {
            result.erase(pos, paramTag.size() + 1);

            string::size_type startIdent = result.find_first_not_of(" \t", pos);
            if (startIdent != string::npos)
            {
                string::size_type endIdent = result.find_first_of(" \t", startIdent);
                if (endIdent != string::npos)
                {
                    string ident = result.substr(startIdent, endIdent - startIdent);
                    string::size_type endComment = result.find_first_of("@<", endIdent);
                    string comment = result.substr(
                        endIdent + 1,
                        endComment == string::npos ? endComment : endComment - endIdent - 1);
                    result.erase(startIdent, endComment == string::npos ? string::npos : endComment - startIdent);
                    string newComment = "<param name=\"" + ident + "\">" + comment + "</param>\n";
                    result.insert(startIdent, newComment);
                    pos = startIdent + newComment.size();
                }
            }
            else
            {
                pos += paramTag.size();
            }
        }
    } while (pos != string::npos);

    static const string returnTag = "@return";
    pos = result.find(returnTag);
    if (pos != string::npos)
    {
        result.erase(pos, returnTag.size() + 1);
        string::size_type endComment = result.find_first_of("@<", pos);
        string comment = result.substr(pos, endComment == string::npos ? endComment : endComment - pos);
        result.erase(pos, endComment == string::npos ? string::npos : endComment - pos);
        string newComment = "<returns>" + comment + "</returns>\n";
        result.insert(pos, newComment);
        pos = pos + newComment.size();
    }

    static const string throwsTag = "@throws";
    pos = 0;
    do
    {
        pos = result.find(throwsTag, pos);
        if (pos != string::npos)
        {
            result.erase(pos, throwsTag.size() + 1);

            string::size_type startIdent = result.find_first_not_of(" \t", pos);
            if (startIdent != string::npos)
            {
                string::size_type endIdent = result.find_first_of(" \t", startIdent);
                if (endIdent != string::npos)
                {
                    string ident = result.substr(startIdent, endIdent - startIdent);
                    string::size_type endComment = result.find_first_of("@<", endIdent);
                    string comment = result.substr(
                        endIdent + 1,
                        endComment == string::npos ? endComment : endComment - endIdent - 1);
                    result.erase(startIdent, endComment == string::npos ? string::npos : endComment - startIdent);
                    string newComment = "<exception name=\"" + ident + "\">" + comment + "</exception>\n";
                    result.insert(startIdent, newComment);
                    pos = startIdent + newComment.size();
                }
            }
            else
            {
                pos += throwsTag.size();
            }
        }
    } while (pos != string::npos);

    return result;
}

StringList
Slice::CsVisitor::splitIntoLines(const string& comment)
{
    string s = editMarkup(comment);
    StringList result;
    string::size_type pos = 0;
    string::size_type nextPos;
    while ((nextPos = s.find_first_of('\n', pos)) != string::npos)
    {
        result.push_back(string(s, pos, nextPos - pos));
        pos = nextPos + 1;
    }
    string lastLine = string(s, pos);
    if (lastLine.find_first_not_of(" \t\n\r") != string::npos)
    {
        result.push_back(lastLine);
    }
    return result;
}

void
Slice::CsVisitor::splitComment(const ContainedPtr& p, StringList& summaryLines, StringList& remarksLines)
{
    string s = p->comment();

    const string paramTag = "@param";
    const string throwsTag = "@throws";
    const string exceptionTag = "@exception";
    const string returnTag = "@return";

    unsigned int i;

    for (i = 0; i < s.size(); ++i)
    {
        if (s[i] == '.' && (i + 1 >= s.size() || isspace(static_cast<unsigned char>(s[i + 1]))))
        {
            ++i;
            break;
        }
        else if (
            s[i] == '@' &&
            (s.substr(i, paramTag.size()) == paramTag || s.substr(i, throwsTag.size()) == throwsTag ||
             s.substr(i, exceptionTag.size()) == exceptionTag || s.substr(i, returnTag.size()) == returnTag))
        {
            break;
        }
    }

    summaryLines = splitIntoLines(trim(s.substr(0, i)));
    if (!summaryLines.empty())
    {
        remarksLines = splitIntoLines(trim(s.substr(i)));
    }
}

void
Slice::CsVisitor::writeDocComment(const ContainedPtr& p, const string& deprecateReason, const string& extraParam)
{
    StringList summaryLines;
    StringList remarksLines;
    splitComment(p, summaryLines, remarksLines);

    if (summaryLines.empty())
    {
        if (!deprecateReason.empty())
        {
            _out << nl << "///";
            _out << nl << "/// <summary>" << deprecateReason << "</summary>";
            _out << nl << "///";
        }
        return;
    }

    _out << nl << "/// <summary>";

    for (StringList::const_iterator i = summaryLines.begin(); i != summaryLines.end(); ++i)
    {
        _out << nl << "///";
        if (!(*i).empty())
        {
            _out << " " << *i;
        }
    }

    //
    // We generate everything into the summary tag (despite what the MSDN doc says) because
    // Visual Studio only shows the <summary> text and omits the <remarks> text.
    //
    if (!deprecateReason.empty())
    {
        _out << nl << "///";
        _out << nl << "/// <para>" << deprecateReason << "</para>";
        _out << nl << "///";
    }

    bool summaryClosed = false;

    if (!remarksLines.empty())
    {
        for (StringList::const_iterator i = remarksLines.begin(); i != remarksLines.end(); ++i)
        {
            //
            // The first param, returns, or exception tag ends the description.
            //
            static const string paramTag = "<param";
            static const string returnsTag = "<returns";
            static const string exceptionTag = "<exception";

            if (!summaryClosed && (i->find(paramTag) != string::npos || i->find(returnsTag) != string::npos ||
                                   i->find(exceptionTag) != string::npos))
            {
                _out << nl << "/// </summary>";
                _out << nl << "/// " << *i;
                summaryClosed = true;
            }
            else
            {
                _out << nl << "///";
                if (!(*i).empty())
                {
                    _out << " " << *i;
                }
            }
        }
    }

    if (!summaryClosed)
    {
        _out << nl << "/// </summary>";
    }

    if (!extraParam.empty())
    {
        _out << nl << "/// " << extraParam;
    }

    _out << sp;
}

void
Slice::CsVisitor::writeDocCommentAMI(
    const OperationPtr& p,
    ParamDir paramType,
    const string& deprecateReason,
    const string& extraParam1,
    const string& extraParam2,
    const string& extraParam3)
{
    StringList summaryLines;
    StringList remarksLines;
    splitComment(p, summaryLines, remarksLines);

    if (summaryLines.empty() && deprecateReason.empty())
    {
        return;
    }

    //
    // Output the leading comment block up until the first tag.
    //
    _out << nl << "/// <summary>";
    for (StringList::const_iterator i = summaryLines.begin(); i != summaryLines.end(); ++i)
    {
        _out << nl << "///";
        if (!(*i).empty())
        {
            _out << " " << *i;
        }
    }

    bool done = false;
    for (StringList::const_iterator i = remarksLines.begin(); i != remarksLines.end() && !done; ++i)
    {
        string::size_type pos = i->find('<');
        done = true;
        if (pos != string::npos)
        {
            if (pos != 0)
            {
                _out << nl << "/// " << i->substr(0, pos);
            }
        }
        else
        {
            _out << nl << "///";
            if (!(*i).empty())
            {
                _out << " " << *i;
            }
        }
    }
    _out << nl << "/// </summary>";

    //
    // Write the comments for the parameters.
    //
    writeDocCommentParam(p, paramType, false);

    if (!extraParam1.empty())
    {
        _out << nl << "/// " << extraParam1;
    }

    if (!extraParam2.empty())
    {
        _out << nl << "/// " << extraParam2;
    }

    if (!extraParam3.empty())
    {
        _out << nl << "/// " << extraParam3;
    }

    if (paramType == InParam)
    {
        _out << nl << "/// <returns>An asynchronous result object.</returns>";
    }
    else if (p->returnType())
    {
        //
        // Find the comment for the return value (if any).
        //
        static const string returnsTag = "<returns>";
        static const string returnsCloseTag = "</returns>";
        bool doneReturn = false;
        bool foundReturn = false;
        for (StringList::const_iterator i = remarksLines.begin(); i != remarksLines.end() && !doneReturn; ++i)
        {
            if (!foundReturn)
            {
                string::size_type pos = i->find(returnsTag);
                if (pos != string::npos)
                {
                    foundReturn = true;
                    string::size_type endpos = i->find(returnsCloseTag, pos + 1);
                    if (endpos != string::npos)
                    {
                        _out << nl << "/// " << i->substr(pos, endpos - pos + returnsCloseTag.size());
                        doneReturn = true;
                    }
                    else
                    {
                        _out << nl << "/// " << i->substr(pos);
                    }
                }
            }
            else
            {
                string::size_type pos = i->find(returnsCloseTag);
                if (pos != string::npos)
                {
                    _out << nl << "/// " << i->substr(0, pos + returnsCloseTag.size());
                    doneReturn = true;
                }
                else
                {
                    _out << nl << "///";
                    if (!(*i).empty())
                    {
                        _out << " " << *i;
                    }
                }
            }
        }
        if (foundReturn && !doneReturn)
        {
            _out << returnsCloseTag;
        }
    }

    if (!deprecateReason.empty())
    {
        _out << nl << "/// <para>" << deprecateReason << "</para>";
    }
}

void
Slice::CsVisitor::writeDocCommentTaskAsyncAMI(
    const OperationPtr& p,
    const string& deprecateReason,
    const string& extraParam1,
    const string& extraParam2,
    const string& extraParam3)
{
    StringList summaryLines;
    StringList remarksLines;
    splitComment(p, summaryLines, remarksLines);

    if (summaryLines.empty() && deprecateReason.empty())
    {
        return;
    }

    //
    // Output the leading comment block up until the first tag.
    //
    _out << nl << "/// <summary>";
    for (StringList::const_iterator i = summaryLines.begin(); i != summaryLines.end(); ++i)
    {
        _out << nl << "///";
        if (!(*i).empty())
        {
            _out << " " << *i;
        }
    }

    bool done = false;
    for (StringList::const_iterator i = remarksLines.begin(); i != remarksLines.end() && !done; ++i)
    {
        string::size_type pos = i->find('<');
        done = true;
        if (pos != string::npos)
        {
            if (pos != 0)
            {
                _out << nl << "/// " << i->substr(0, pos);
            }
        }
        else
        {
            _out << nl << "///";
            if (!(*i).empty())
            {
                _out << " " << *i;
            }
        }
    }
    _out << nl << "/// </summary>";

    //
    // Write the comments for the parameters.
    //
    writeDocCommentParam(p, InParam, false);

    if (!extraParam1.empty())
    {
        _out << nl << "/// " << extraParam1;
    }

    if (!extraParam2.empty())
    {
        _out << nl << "/// " << extraParam2;
    }

    if (!extraParam3.empty())
    {
        _out << nl << "/// " << extraParam3;
    }

    _out << nl << "/// <returns>The task object representing the asynchronous operation.</returns>";

    if (!deprecateReason.empty())
    {
        _out << nl << "/// <para>" << deprecateReason << "</para>";
    }
}

void
Slice::CsVisitor::writeDocCommentAMD(const OperationPtr& p, const string& extraParam)
{
    string deprecateReason = getDeprecationMessageForComment(p, "operation");

    StringList summaryLines;
    StringList remarksLines;
    splitComment(p, summaryLines, remarksLines);

    if (summaryLines.empty() && deprecateReason.empty())
    {
        return;
    }

    //
    // Output the leading comment block up until the first tag.
    //
    _out << nl << "/// <summary>";
    for (StringList::const_iterator i = summaryLines.begin(); i != summaryLines.end(); ++i)
    {
        _out << nl << "///";
        if (!(*i).empty())
        {
            _out << " " << *i;
        }
    }

    bool done = false;
    for (StringList::const_iterator i = remarksLines.begin(); i != remarksLines.end() && !done; ++i)
    {
        string::size_type pos = i->find('<');
        done = true;
        if (pos != string::npos)
        {
            if (pos != 0)
            {
                _out << nl << "/// " << i->substr(0, pos);
            }
        }
        else
        {
            _out << nl << "///";
            if (!(*i).empty())
            {
                _out << " " << *i;
            }
        }
    }
    _out << nl << "/// </summary>";

    //
    // Write the comments for the parameters.
    //
    writeDocCommentParam(p, InParam, true);

    if (!extraParam.empty())
    {
        _out << nl << "/// " << extraParam;
    }

    _out << nl << "/// <returns>The task object representing the asynchronous operation.</returns>";

    if (!deprecateReason.empty())
    {
        _out << nl << "/// <para>" << deprecateReason << "</para>";
    }
}

void
Slice::CsVisitor::writeDocCommentParam(const OperationPtr& p, ParamDir paramType, bool /*amd*/)
{
    //
    // Collect the names of the in- or -out parameters to be documented.
    //
    ParamDeclList tmp = p->parameters();
    vector<string> params;
    for (ParamDeclList::const_iterator q = tmp.begin(); q != tmp.end(); ++q)
    {
        if ((*q)->isOutParam() && paramType == OutParam)
        {
            params.push_back((*q)->name());
        }
        else if (!(*q)->isOutParam() && paramType == InParam)
        {
            params.push_back((*q)->name());
        }
    }

    //
    // Print the comments for all the parameters that appear in the parameter list.
    //
    StringList summaryLines;
    StringList remarksLines;
    splitComment(p, summaryLines, remarksLines);

    const string paramTag = "<param";
    StringList::const_iterator i = remarksLines.begin();
    while (i != remarksLines.end())
    {
        string line = *i++;
        if (line.find(paramTag) != string::npos)
        {
            string::size_type paramNamePos = line.find('"', paramTag.length());
            if (paramNamePos != string::npos)
            {
                string::size_type paramNameEndPos = line.find('"', paramNamePos + 1);
                string paramName = line.substr(paramNamePos + 1, paramNameEndPos - paramNamePos - 1);
                if (std::find(params.begin(), params.end(), paramName) != params.end())
                {
                    _out << nl << "/// " << line;
                    StringList::const_iterator j;
                    if (i == remarksLines.end())
                    {
                        break;
                    }
                    j = i++;
                    while (j != remarksLines.end())
                    {
                        string::size_type endpos = j->find("</param>");
                        if (endpos == string::npos)
                        {
                            i = j;
                            string s = *j++;
                            _out << nl << "///";
                            if (!s.empty())
                            {
                                _out << " " << s;
                            }
                        }
                        else
                        {
                            _out << nl << "/// " << *j++;
                            break;
                        }
                    }
                }
            }
        }
    }
}

void
Slice::CsVisitor::moduleStart(const ModulePtr& p)
{
    if (!dynamic_pointer_cast<Contained>(p->container()))
    {
        string ns = getNamespacePrefix(p);
        string name = fixId(p->name());
        if (!ns.empty())
        {
            _out << sp;
            _out << nl << "namespace " << ns;
            _out << sb;
        }
    }
}

void
Slice::CsVisitor::moduleEnd(const ModulePtr& p)
{
    if (!dynamic_pointer_cast<Contained>(p->container()))
    {
        if (!getNamespacePrefix(p).empty())
        {
            _out << eb;
        }
    }
}

Slice::Gen::Gen(const string& base, const vector<string>& includePaths, const string& dir, bool tie)
    : _includePaths(includePaths),
      _tie(tie)
{
    string fileBase = base;
    string::size_type pos = base.find_last_of("/\\");
    if (pos != string::npos)
    {
        fileBase = base.substr(pos + 1);
    }
    string file = fileBase + ".cs";
    string fileImpl = fileBase + "I.cs";

    if (!dir.empty())
    {
        file = dir + '/' + file;
        fileImpl = dir + '/' + fileImpl;
    }

    _out.open(file.c_str());
    if (!_out)
    {
        ostringstream os;
        os << "cannot open `" << file << "': " << IceUtilInternal::errorToString(errno);
        throw FileException(__FILE__, __LINE__, os.str());
    }
    FileTracker::instance()->addFile(file);
    printHeader();

    printGeneratedHeader(_out, fileBase + ".ice");

    _out << nl << "#nullable enable";
    _out << sp << nl << "[assembly:Ice.Slice(\"" << fileBase << ".ice\")]";
    _out << sp << nl << "#pragma warning disable 1591"; // See bug 3654
}

Slice::Gen::~Gen()
{
    if (_out.isOpen())
    {
        _out << '\n';
        _out.close();
    }
}

void
Slice::Gen::generate(const UnitPtr& p)
{
    CsGenerator::validateMetaData(p);

    UnitVisitor unitVisitor(_out);
    p->visit(&unitVisitor, false);

    TypesVisitor typesVisitor(_out);
    p->visit(&typesVisitor, false);

    ResultVisitor resultVisitor(_out);
    p->visit(&resultVisitor, false);

    ProxyVisitor proxyVisitor(_out);
    p->visit(&proxyVisitor, false);

    OpsVisitor opsVisitor(_out);
    p->visit(&opsVisitor, false);

    HelperVisitor helperVisitor(_out);
    p->visit(&helperVisitor, false);

    DispatcherVisitor dispatcherVisitor(_out, _tie);
    p->visit(&dispatcherVisitor, false);
}

void
Slice::Gen::printHeader()
{
    static const char* header = "//\n"
                                "// Copyright (c) ZeroC, Inc. All rights reserved.\n"
                                "//\n";

    _out << header;
    _out << "//\n";
    _out << "// Ice version " << ICE_STRING_VERSION << "\n";
    _out << "//\n";
}

Slice::Gen::UnitVisitor::UnitVisitor(IceUtilInternal::Output& out) : CsVisitor(out) {}

bool
Slice::Gen::UnitVisitor::visitUnitStart(const UnitPtr& p)
{
    DefinitionContextPtr dc = p->findDefinitionContext(p->topLevelFile());
    assert(dc);
    StringList globalMetaData = dc->getMetaData();

    static const string attributePrefix = "cs:attribute:";

    bool sep = false;
    for (StringList::const_iterator q = globalMetaData.begin(); q != globalMetaData.end(); ++q)
    {
        string::size_type pos = q->find(attributePrefix);
        if (pos == 0 && q->size() > attributePrefix.size())
        {
            if (!sep)
            {
                _out << sp;
                sep = true;
            }
            string attrib = q->substr(pos + attributePrefix.size());
            _out << nl << '[' << attrib << ']';
        }
    }
    return false;
}

Slice::Gen::TypesVisitor::TypesVisitor(IceUtilInternal::Output& out) : CsVisitor(out) {}
bool
Slice::Gen::TypesVisitor::visitModuleStart(const ModulePtr& p)
{
    moduleStart(p);
    string name = fixId(p->name());
    _out << sp;
    emitAttributes(p);
    _out << nl << "namespace " << name;

    _out << sb;

    return true;
}

void
Slice::Gen::TypesVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
    moduleEnd(p);
}

bool
Slice::Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = p->name();
    string scoped = fixId(p->scoped());
    string ns = getNamespace(p);
    ClassDefPtr base = p->base();
    StringList baseNames;

    _out << sp;
    emitAttributes(p);

    emitComVisibleAttribute();
    emitPartialTypeAttributes();
    _out << nl << "[Ice.SliceTypeId(\"" << p->scoped() << "\")]";
    if (p->compactId() >= 0)
    {
        _out << nl << "[Ice.CompactSliceTypeId(" << p->compactId() << ")]";
    }
    _out << nl << "public partial class " << fixId(name);

    if (base)
    {
        baseNames.push_back(getUnqualified(base, ns));
    }
    else
    {
        baseNames.push_back(getUnqualified("Ice.Value", ns));
    }

    if (!baseNames.empty())
    {
        _out << " : ";
        for (StringList::const_iterator q = baseNames.begin(); q != baseNames.end(); ++q)
        {
            if (q != baseNames.begin())
            {
                _out << ", ";
            }
            _out << *q;
        }
    }

    _out << sb;

    if (p->hasDataMembers())
    {
        _out << sp << nl << "#region Slice data members";
    }

    return true;
}

void
Slice::Gen::TypesVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string name = fixId(p->name());
    string ns = getNamespace(p);
    DataMemberList classMembers = p->classDataMembers();
    DataMemberList allClassMembers = p->allClassDataMembers();
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList allDataMembers = p->allDataMembers();
    ClassDefPtr base = p->base();

    if (p->hasDataMembers())
    {
        _out << sp << nl << "#endregion";
    }

    _out << sp << nl << "partial void ice_initialize();";
    if (allDataMembers.empty())
    {
        _out << sp << nl << "#region Constructors";
        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public " << name << spar << epar;
        _out << sb;
        _out << nl << "ice_initialize();";
        _out << eb;
        _out << sp << nl << "#endregion"; // Constructors
    }
    else
    {
        _out << sp << nl << "#region Constructors";

        // Primary constructor.
        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public " << name << spar;

        vector<string> paramDecl;
        vector<string> secondaryCtorParams;
        vector<string> secondaryCtorMemberNames;
        vector<string> secondaryCtorBaseParamNames;
        for (const auto& q : allDataMembers)
        {
            string memberName = fixId(q->name(), DotNet::ICloneable);
            string memberType = typeToString(q->type(), ns, q->optional());
            paramDecl.push_back(memberType + " " + memberName);

            // Look for non-nullable fields to be initialized by the secondary constructor.
            if (!q->optional())
            {
                StructPtr st = dynamic_pointer_cast<Struct>(q->type());
                if (dynamic_pointer_cast<Sequence>(q->type()) || dynamic_pointer_cast<Dictionary>(q->type()) ||
                    (st && isMappedToClass(st)))
                {
                    secondaryCtorParams.push_back(memberType + " " + memberName);

                    if (find(dataMembers.begin(), dataMembers.end(), q) != dataMembers.end())
                    {
                        secondaryCtorMemberNames.push_back(memberName);
                    }
                }
            }
        }
        _out << paramDecl << epar;

        if (base && allDataMembers.size() != dataMembers.size())
        {
            _out << " : base" << spar;
            vector<string> baseParamNames;
            DataMemberList baseDataMembers = base->allDataMembers();
            for (const auto& q : baseDataMembers)
            {
                string memberName = fixId(q->name(), DotNet::ICloneable);
                baseParamNames.push_back(memberName);

                // Look for non-nullable fields
                if (!q->optional())
                {
                    StructPtr st = dynamic_pointer_cast<Struct>(q->type());
                    if (dynamic_pointer_cast<Sequence>(q->type()) || dynamic_pointer_cast<Dictionary>(q->type()) ||
                        (st && isMappedToClass(st)))
                    {
                        secondaryCtorBaseParamNames.push_back(memberName);
                    }
                }
            }
            _out << baseParamNames << epar;
        }
        _out << sb;
        for (const auto& q : dataMembers)
        {
            const string memberName = fixId(q->name(), DotNet::ICloneable);
            _out << nl << "this." << memberName << " = " << memberName << ';';
        }
        _out << nl << "ice_initialize();";
        _out << eb;

        // Non-primary constructor. Can be parameterless.
        if (secondaryCtorParams.size() != paramDecl.size())
        {
            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << "public " << name << spar << secondaryCtorParams << epar;
            if (base && secondaryCtorBaseParamNames.size() > 0)
            {
                _out << " : base" << spar << secondaryCtorBaseParamNames << epar;
            }
            _out << sb;
            for (const auto& q : secondaryCtorMemberNames)
            {
                _out << nl << "this." << q << " = " << q << ';';
            }
            _out << nl << "ice_initialize();";
            _out << eb;
        }

        // Parameterless constructor. Required for unmarshaling.
        if (secondaryCtorParams.size() > 0)
        {
            _out << sp;
            emitGeneratedCodeAttribute();
            emitNonBrowsableAttribute();
            _out << nl << "public " << name << "()";
            _out << sb;
            writeDataMemberInitializers(dataMembers, DotNet::ICloneable);
            _out << nl << "ice_initialize();";
            _out << eb;
        }

        _out << sp << nl << "#endregion"; // Constructors
    }

    _out << sp;
    _out << nl << "private const string _id = \"" << p->scoped() << "\";";

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public static new string ice_staticId()";
    _out << sb;
    _out << nl << "return _id;";
    _out << eb;

    emitGeneratedCodeAttribute();
    _out << nl << "public override string ice_id()";
    _out << sb;
    _out << nl << "return _id;";
    _out << eb;

    writeMarshaling(p);

    _out << eb;
}

bool
Slice::Gen::TypesVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    string name = p->name();
    string scoped = fixId(p->scoped());
    string ns = getNamespace(p);
    InterfaceList bases = p->bases();

    StringList baseNames;

    _out << sp;
    emitAttributes(p);

    emitComVisibleAttribute();
    emitPartialTypeAttributes();
    _out << nl << "[Ice.SliceTypeId(\"" << p->scoped() << "\")]";
    _out << nl << "public partial interface " << fixId(name);
    baseNames.push_back(getUnqualified("Ice.Object", ns));
    baseNames.push_back(name + "Operations_");

    for (InterfaceList::const_iterator q = bases.begin(); q != bases.end(); ++q)
    {
        baseNames.push_back(getUnqualified(*q, ns));
    }

    //
    // Check for cs:implements metadata.
    //
    const StringList metaData = p->getMetaData();
    static const string prefix = "cs:implements:";
    for (StringList::const_iterator q = metaData.begin(); q != metaData.end(); ++q)
    {
        if (q->find(prefix) == 0)
        {
            baseNames.push_back(q->substr(prefix.size()));
        }
    }

    if (!baseNames.empty())
    {
        _out << " : ";
        for (StringList::const_iterator q = baseNames.begin(); q != baseNames.end(); ++q)
        {
            if (q != baseNames.begin())
            {
                _out << ", ";
            }
            _out << *q;
        }
    }

    _out << sb;
    _out << eb;

    return false;
}

void
Slice::Gen::TypesVisitor::visitSequence(const SequencePtr&)
{
    //
    // No need to generate anything for sequences.
    //
}

bool
Slice::Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string name = fixId(p->name());
    string ns = getNamespace(p);
    ExceptionPtr base = p->base();

    _out << sp;
    writeDocComment(p, getDeprecationMessageForComment(p, "type"));
    emitObsoleteAttribute(p, _out);
    emitAttributes(p);
    emitComVisibleAttribute();
    //
    // Suppress FxCop diagnostic about a missing constructor MyException(String).
    //
    _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Design\", \"CA1032\")]";

    emitPartialTypeAttributes();
    _out << nl << "[Ice.SliceTypeId(\"" << p->scoped() << "\")]";
    _out << nl << "public partial class " << name << " : ";
    if (base)
    {
        _out << getUnqualified(base, ns);
    }
    else
    {
        _out << getUnqualified("Ice.UserException", ns);
    }
    _out << sb;

    if (!p->dataMembers().empty())
    {
        _out << sp << nl << "#region Slice data members";
    }

    return true;
}

void
Slice::Gen::TypesVisitor::visitExceptionEnd(const ExceptionPtr& p)
{
    string name = fixId(p->name());
    string ns = getNamespace(p);
    DataMemberList allDataMembers = p->allDataMembers();
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList allClassMembers = p->allClassDataMembers();
    DataMemberList classMembers = p->classDataMembers();
    DataMemberList optionalMembers = p->orderedOptionalDataMembers();
    ExceptionPtr base = p->base();

    if (!dataMembers.empty())
    {
        _out << sp << nl << "#endregion"; // Slice data members
    }

    _out << sp << nl << "#region Constructors";

    string exParam = getEscapedParamName(allDataMembers, "innerException");

    // Primary constructor.
    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public " << name << spar;

    vector<string> paramDecl;
    vector<string> secondaryCtorParams;
    vector<string> secondaryCtorMemberNames;
    vector<string> secondaryCtorBaseParamNames;
    bool hideParameterlessCtor = false;

    for (const auto& q : allDataMembers)
    {
        string memberName = fixId(q->name(), DotNet::Exception);
        string memberType = typeToString(q->type(), ns, q->optional());
        paramDecl.push_back(memberType + " " + memberName);

        // Look for non-nullable fields to be initialized by the secondary constructor.
        if (!q->optional())
        {
            StructPtr st = dynamic_pointer_cast<Struct>(q->type());
            if (dynamic_pointer_cast<Sequence>(q->type()) || dynamic_pointer_cast<Dictionary>(q->type()) ||
                (st && isMappedToClass(st)))
            {
                secondaryCtorParams.push_back(memberType + " " + memberName);
                if (find(dataMembers.begin(), dataMembers.end(), q) != dataMembers.end())
                {
                    secondaryCtorMemberNames.push_back(memberName);
                }
                hideParameterlessCtor = true;
            }
        }
    }
    // Add exception for inner exception. It's defaulted to null only if it's not the only parameter.
    if (paramDecl.size() > 0)
    {
        paramDecl.push_back("global::System.Exception? " + exParam + " = null");
    }
    else
    {
        paramDecl.push_back("global::System.Exception? " + exParam);
    }
    if (secondaryCtorParams.size() > 0)
    {
        secondaryCtorParams.push_back("global::System.Exception? " + exParam + " = null");
    }
    else
    {
        secondaryCtorParams.push_back("global::System.Exception? " + exParam);
    }

    _out << paramDecl << epar;
    _out << " : base" << spar;
    vector<string> baseParamNames;
    if (base)
    {
        DataMemberList baseDataMembers = base->allDataMembers();
        for (const auto& q : baseDataMembers)
        {
            string memberName = fixId(q->name(), DotNet::Exception);
            baseParamNames.push_back(memberName);

            // Look for non-nullable fields
            if (!q->optional())
            {
                StructPtr st = dynamic_pointer_cast<Struct>(q->type());
                if (dynamic_pointer_cast<Sequence>(q->type()) || dynamic_pointer_cast<Dictionary>(q->type()) ||
                    (st && isMappedToClass(st)))
                {
                    secondaryCtorBaseParamNames.push_back(memberName);
                }
            }
        }
    }
    baseParamNames.push_back(exParam);
    secondaryCtorBaseParamNames.push_back(exParam);

    _out << baseParamNames << epar;
    _out << sb;
    for (const auto& q : dataMembers)
    {
        const string memberName = fixId(q->name(), DotNet::Exception);
        _out << nl << "this." << memberName << " = " << memberName << ';';
    }
    _out << eb;

    // Non-primary constructor. If generated, it has at least one parameter without a default value.
    if (secondaryCtorParams.size() != paramDecl.size())
    {
        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public " << name << spar << secondaryCtorParams << epar;
        _out << " : base" << spar << secondaryCtorBaseParamNames << epar;
        _out << sb;
        for (const auto& q : secondaryCtorMemberNames)
        {
            _out << nl << "this." << q << " = " << q << ';';
        }
        _out << eb;
    }

    // Parameterless constructor. Required for unmarshaling.
    _out << sp;
    emitGeneratedCodeAttribute();
    if (hideParameterlessCtor)
    {
        emitNonBrowsableAttribute();
    }
    _out << nl << "public " << name << "()";
    _out << sb;
    writeDataMemberInitializers(dataMembers, DotNet::Exception);
    _out << eb;

    _out << sp << nl << "#endregion"; // Constructors

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public override string ice_id()";
    _out << sb;
    _out << nl << "return \"" << p->scoped() << "\";";
    _out << eb;

    _out << sp << nl << "#region Marshaling support";

    string scoped = p->scoped();

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "protected override void iceWriteImpl(" << getUnqualified("Ice.OutputStream", ns) << " ostr_)";
    _out << sb;
    _out << nl << "ostr_.startSlice(\"" << scoped << "\", -1, " << (!base ? "true" : "false") << ");";
    for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        if (!(*q)->optional())
        {
            writeMarshalDataMember(*q, fixId((*q)->name(), DotNet::Exception), ns);
        }
    }

    for (DataMemberList::const_iterator q = optionalMembers.begin(); q != optionalMembers.end(); ++q)
    {
        writeMarshalDataMember(*q, fixId((*q)->name(), DotNet::Exception), ns);
    }

    _out << nl << "ostr_.endSlice();";
    if (base)
    {
        _out << nl << "base.iceWriteImpl(ostr_);";
    }
    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "protected override void iceReadImpl(" << getUnqualified("Ice.InputStream", ns) << " istr_)";
    _out << sb;
    _out << nl << "istr_.startSlice();";

    for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        if (!(*q)->optional())
        {
            writeUnmarshalDataMember(*q, fixId((*q)->name(), DotNet::Exception), ns);
        }
    }

    for (DataMemberList::const_iterator q = optionalMembers.begin(); q != optionalMembers.end(); ++q)
    {
        writeUnmarshalDataMember(*q, fixId((*q)->name(), DotNet::Exception), ns);
    }
    _out << nl << "istr_.endSlice();";
    if (base)
    {
        _out << nl << "base.iceReadImpl(istr_);";
    }
    _out << eb;

    if ((!base || (base && !base->usesClasses(false))) && p->usesClasses(false))
    {
        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public override bool iceUsesClasses()";
        _out << sb;
        _out << nl << "return true;";
        _out << eb;
    }

    _out << sp << nl << "#endregion"; // marshaling support

    _out << eb;
}

bool
Slice::Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    string name = fixId(p->name());
    string ns = getNamespace(p);
    _out << sp;
    const bool classMapping = isMappedToClass(p);

    emitObsoleteAttribute(p, _out);

    emitAttributes(p);
    emitPartialTypeAttributes();
    _out << nl << "public " << (classMapping ? "sealed partial class" : "partial record struct") << ' ' << name;

    StringList baseNames;

    if (classMapping)
    {
        baseNames.push_back("System.IEquatable<" + name + ">");
    }

    //
    // Check for cs:implements metadata.
    //
    const StringList metaData = p->getMetaData();
    static const string prefix = "cs:implements:";
    for (StringList::const_iterator q = metaData.begin(); q != metaData.end(); ++q)
    {
        if (q->find(prefix) == 0)
        {
            baseNames.push_back(q->substr(prefix.size()));
        }
    }

    if (!baseNames.empty())
    {
        _out << " : ";
        for (StringList::const_iterator q = baseNames.begin(); q != baseNames.end(); ++q)
        {
            if (q != baseNames.begin())
            {
                _out << ", ";
            }
            _out << getUnqualified(*q, ns);
        }
    }

    _out << sb;

    _out << sp << nl << "#region Slice data members";

    return true;
}

void
Slice::Gen::TypesVisitor::visitStructEnd(const StructPtr& p)
{
    string name = fixId(p->name());
    string scope = fixId(p->scope());
    string ns = getNamespace(p);
    DataMemberList classMembers = p->classDataMembers();
    DataMemberList dataMembers = p->dataMembers();
    _out << sp << nl << "#endregion"; // Slice data members

    const bool classMapping = isMappedToClass(p);
    _out << sp << nl << "partial void ice_initialize();";
    _out << sp << nl << "#region Constructor";

    if (classMapping)
    {
        // We generate a constructor that initializes all non-nullable fields (collections and structs mapped to
        // classes). It can be parameterless.

        vector<string> ctorParams;
        vector<string> ctorParamNames;
        for (const auto& q : dataMembers)
        {
            StructPtr st = dynamic_pointer_cast<Struct>(q->type());

            if (dynamic_pointer_cast<Sequence>(q->type()) || dynamic_pointer_cast<Dictionary>(q->type()) ||
                (st && isMappedToClass(st)))
            {
                string memberName = fixId(q->name(), DotNet::ICloneable);
                string memberType = typeToString(q->type(), ns, false);
                ctorParams.push_back(memberType + " " + memberName);
                ctorParamNames.push_back(memberName);
            }
        }

        // We only generate this ctor if it's different from the primary constructor.
        if (ctorParams.size() != dataMembers.size())
        {
            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << "public " << name << spar << ctorParams << epar;
            _out << sb;
            for (const auto& q : ctorParamNames)
            {
                _out << nl << "this." << q << " = " << q << ';';
            }
            // All the other fields keep their default values (explicit or implicit).
            _out << nl << "ice_initialize();";
            _out << eb;
        }
    }

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public " << name << spar;
    vector<string> paramDecl;
    for (const auto& q : dataMembers)
    {
        string memberName = fixId(q->name(), classMapping ? DotNet::ICloneable : 0);
        string memberType = typeToString(q->type(), ns, false);
        paramDecl.push_back(memberType + " " + memberName);
    }
    _out << paramDecl << epar;
    _out << sb;
    for (const auto& q : dataMembers)
    {
        string paramName = fixId(q->name(), classMapping ? DotNet::ICloneable : 0);
        _out << nl << "this." << paramName << " = " << paramName << ';';
    }
    _out << nl << "ice_initialize();";
    _out << eb;

    // Unmarshaling constructor
    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public " << name << "(" << getUnqualified("Ice.InputStream", ns) << " istr)";
    _out << sb;
    for (const auto& q : dataMembers)
    {
        writeUnmarshalDataMember(q, fixId(q->name(), classMapping ? DotNet::ICloneable : 0), ns, true);
    }
    _out << nl << "ice_initialize();";
    _out << eb;

    _out << sp << nl << "#endregion"; // Constructor(s)

    if (classMapping)
    {
        _out << sp << nl << "#region Clone method";

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public " << name << " Clone() => (" << name << ")MemberwiseClone();";

        _out << sp << nl << "#endregion"; // ICloneable members

        _out << sp << nl << "#region Object members";

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public override int GetHashCode()";
        _out << sb;
        writeMemberHashCode(dataMembers);
        _out << eb;

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public override bool Equals(object? other) => Equals(other as " << name << ");";

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public bool Equals(" << name << "? other)";
        _out << sb;
        writeMemberEquals(dataMembers);
        _out << eb;

        _out << sp << nl << "#endregion"; // Object members

        _out << sp << nl << "#region Comparison members";

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public static bool operator ==(" << name << "? lhs, " << name << "? rhs) => ";
        _out << "lhs is not null ? lhs.Equals(rhs) : rhs is null;";

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public static bool operator !=(" << name << "? lhs, " << name << "? rhs) => !(lhs == rhs);";
        _out << sp << nl << "#endregion"; // Comparison members
    }

    _out << sp << nl << "#region Marshaling support";

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public void ice_writeMembers(" << getUnqualified("Ice.OutputStream", ns) << " ostr)";
    _out << sb;
    for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        writeMarshalDataMember(*q, fixId((*q)->name(), classMapping ? DotNet::ICloneable : 0), ns, true);
    }
    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public static void ice_write(" << getUnqualified("Ice.OutputStream", ns) << " ostr, " << name
         << " v)";
    _out << sb;
    _out << nl << "v.ice_writeMembers(ostr);";
    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public static " << name << " ice_read(" << getUnqualified("Ice.InputStream", ns)
         << " istr) => new(istr);";

    _out << sp << nl << "#endregion"; // marshaling support

    _out << eb;
}

void
Slice::Gen::TypesVisitor::visitDictionary(const DictionaryPtr&)
{
}

void
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    string name = fixId(p->name());
    string ns = getNamespace(p);
    string scoped = fixId(p->scoped());
    EnumeratorList enumerators = p->enumerators();
    const bool explicitValue = p->explicitValue();

    _out << sp;
    emitObsoleteAttribute(p, _out);
    writeDocComment(p, getDeprecationMessageForComment(p, "type"));
    emitAttributes(p);
    emitGeneratedCodeAttribute();
    _out << nl << "public enum " << name;
    _out << sb;
    for (EnumeratorList::const_iterator en = enumerators.begin(); en != enumerators.end(); ++en)
    {
        if (en != enumerators.begin())
        {
            _out << ',';
        }
        writeDocComment(*en, "");
        _out << nl << fixId((*en)->name());
        if (explicitValue)
        {
            _out << " = " << (*en)->value();
        }
    }
    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public sealed class " << p->name() << "Helper";
    _out << sb;
    _out << sp;
    _out << nl << "public static void write(" << getUnqualified("Ice.OutputStream", ns) << " ostr, " << name << " v)";
    _out << sb;
    writeMarshalUnmarshalCode(_out, p, ns, "v", true);
    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public static " << name << " read(" << getUnqualified("Ice.InputStream", ns) << " istr)";
    _out << sb;
    _out << nl << name << " v;";
    writeMarshalUnmarshalCode(_out, p, ns, "v", false);
    _out << nl << "return v;";
    _out << eb;

    _out << eb;
}

void
Slice::Gen::TypesVisitor::visitConst(const ConstPtr& p)
{
    string name = fixId(p->name());
    _out << sp;
    emitAttributes(p);
    emitGeneratedCodeAttribute();
    _out << nl << "public abstract class " << name;
    _out << sb;
    _out << sp << nl << "public const " << typeToString(p->type(), "") << " value = ";
    writeConstantValue(p->type(), p->valueType(), p->value());
    _out << ";";
    _out << eb;
}

void
Slice::Gen::TypesVisitor::visitDataMember(const DataMemberPtr& p)
{
    unsigned int baseTypes = 0;
    bool isClass = false;
    bool isProtected = false;
    const bool isOptional = p->optional();

    ContainedPtr cont = dynamic_pointer_cast<Contained>(p->container());
    assert(cont);
    bool isProperty = cont->hasMetaData("cs:property");

    StructPtr st = dynamic_pointer_cast<Struct>(cont);
    ExceptionPtr ex = dynamic_pointer_cast<Exception>(cont);
    ClassDefPtr cl = dynamic_pointer_cast<ClassDef>(cont);
    string ns = getNamespace(cont);
    if (st)
    {
        if (isMappedToClass(st))
        {
            baseTypes = DotNet::ICloneable;
        }
    }
    else if (ex)
    {
        baseTypes = DotNet::Exception;
    }
    else
    {
        assert(cl);
        baseTypes = DotNet::ICloneable;
        isClass = true;
        isProtected = cont->hasMetaData("protected") || p->hasMetaData("protected");
    }

    _out << sp;

    emitObsoleteAttribute(p, _out);

    string type = typeToString(p->type(), ns, isOptional);
    string dataMemberName = fixId(p->name(), baseTypes, isClass);

    emitAttributes(p);
    emitGeneratedCodeAttribute();
    if (isProtected)
    {
        _out << nl << "protected";
    }
    else
    {
        _out << nl << "public";
    }
    _out << ' ' << type << ' ' << dataMemberName;

    bool addSemicolon = true;
    if (isProperty)
    {
        _out << " { get; set; }";
        addSemicolon = false;
    }

    // Generate the default value for this field unless the enclosing type is a struct.
    if (!st || isMappedToClass(st))
    {
        if (p->defaultValueType())
        {
            _out << " = ";
            writeConstantValue(p->type(), p->defaultValueType(), p->defaultValue());
            addSemicolon = true;
        }
        else if (!p->optional())
        {
            BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(p->type());
            if (builtin && builtin->kind() == Builtin::KindString)
            {
                // This behavior is unfortunate but kept for backwards compatibility.
                _out << " = \"\"";
                addSemicolon = true;
            }
        }
    }

    if (addSemicolon)
    {
        _out << ';';
    }
}

void
Slice::Gen::TypesVisitor::writeMemberHashCode(const DataMemberList& dataMembers)
{
    _out << nl << "var hash = new global::System.HashCode();";
    assert(dataMembers.size() > 0); // a Slice struct must have at least one field.

    for (const auto& q : dataMembers)
    {
        TypePtr memberType = q->type();
        string memberName = fixId(q->name(), DotNet::ICloneable);

        if (dynamic_pointer_cast<Sequence>(memberType) || dynamic_pointer_cast<Dictionary>(memberType))
        {
            _out << nl << "Ice.UtilInternal.Collections.HashCodeAdd(ref hash, this." << memberName << ");";
        }
        else
        {
            _out << nl << "hash.Add(this." << memberName << ");";
        }
    }
    _out << nl << "return hash.ToHashCode();";
}

void
Slice::Gen::TypesVisitor::writeMemberEquals(const DataMemberList& dataMembers)
{
    _out << nl << "if (ReferenceEquals(this, other))";
    _out << sb;
    _out << nl << "return true;";
    _out << eb;
    _out << nl << "return other is not null";

    assert(!dataMembers.empty()); // a Slice struct must have at least one field.

    _out.inc();

    for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        assert(!(*q)->optional()); // a Slice struct does not have optional fields.

        _out << " && " << nl;

        string memberName = fixId((*q)->name(), DotNet::ICloneable);
        TypePtr memberType = (*q)->type();

        if (SequencePtr seq = dynamic_pointer_cast<Sequence>(memberType))
        {
            _out << "Ice.UtilInternal.Collections.NullableSequenceEqual(this." << memberName << ", other." << memberName
                 << ")";
        }
        else if (DictionaryPtr dict = dynamic_pointer_cast<Dictionary>(memberType))
        {
            // Equals() for generic types does not have value semantics.
            _out << "Ice.UtilInternal.Collections.DictionaryEquals(this." << memberName << ", other." << memberName
                 << ")";
        }
        else if (isProxyType(memberType))
        {
            // We need to cast it to the base concrete type to get ==
            _out << "(Ice.ObjectPrxHelperBase?)this." << memberName << " == (Ice.ObjectPrxHelperBase?)other."
                 << memberName;
        }
        else
        {
            _out << "this." << memberName << " == other." << memberName;
        }
    }
    _out.dec();
    _out << ";";
}

Slice::Gen::ResultVisitor::ResultVisitor(::IceUtilInternal::Output& out) : CsVisitor(out) {}

namespace
{
    bool hasResultType(const ModulePtr& p)
    {
        InterfaceList interfaces = p->interfaces();
        for (InterfaceList::const_iterator i = interfaces.begin(); i != interfaces.end(); ++i)
        {
            InterfaceDefPtr interface = *i;
            OperationList operations = interface->operations();
            for (OperationList::const_iterator j = operations.begin(); j != operations.end(); ++j)
            {
                OperationPtr op = *j;
                ParamDeclList outParams = op->outParameters();
                TypePtr ret = op->returnType();
                if (outParams.size() > 1 || (ret && outParams.size() > 0))
                {
                    return true;
                }
            }
        }

        ModuleList modules = p->modules();
        for (ModuleList::const_iterator i = modules.begin(); i != modules.end(); ++i)
        {
            if (hasResultType(*i))
            {
                return true;
            }
        }

        return false;
    }
}

bool
Slice::Gen::ResultVisitor::visitModuleStart(const ModulePtr& p)
{
    if (hasResultType(p))
    {
        moduleStart(p);
        _out << sp << nl << "namespace " << fixId(p->name());
        _out << sb;
        return true;
    }
    return false;
}

void
Slice::Gen::ResultVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
    moduleEnd(p);
}

void
Slice::Gen::ResultVisitor::visitOperation(const OperationPtr& p)
{
    InterfaceDefPtr interface = p->interface();
    string ns = getNamespace(interface);
    ParamDeclList outParams = p->outParameters();
    TypePtr ret = p->returnType();

    if (outParams.size() > 1 || (ret && outParams.size() > 0))
    {
        string name = resultStructName(interface->name(), p->name());

        string retS;
        string retSName;
        if (ret)
        {
            retS = typeToString(ret, ns, p->returnIsOptional());
            retSName = resultStructReturnValueName(outParams);
        }

        _out << sp;
        _out << nl << "public record struct " << name;
        _out << spar;
        if (ret)
        {
            _out << (retS + " " + retSName);
        }

        for (const auto& q : outParams)
        {
            _out << (typeToString(q->type(), ns, q->optional()) + " " + fixId(q->name()));
        }
        _out << epar;
        _out << ";";
    }

    if (p->hasMarshaledResult())
    {
        string name = resultStructName(interface->name(), p->name(), true);

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public readonly record struct " << name << " : " << getUnqualified("Ice.MarshaledResult", ns);
        _out << sb;

        //
        // Marshaling constructor
        //
        _out << nl << "public " << name << spar << getOutParams(p, ns, true, false)
             << getUnqualified("Ice.Current", ns) + " current" << epar;
        _out << sb;
        _out << nl << "_ostr = global::Ice.Internal.Incoming.createResponseOutputStream(current);";
        _out << nl << "_ostr.startEncapsulation(current.encoding, " << opFormatTypeToString(p, ns) << ");";
        writeMarshalUnmarshalParams(outParams, p, true, ns, false, true, "_ostr");
        if (p->returnsClasses(false))
        {
            _out << nl << "_ostr.writePendingValues();";
        }
        _out << nl << "_ostr.endEncapsulation();";
        _out << eb;
        _out << sp;
        _out << nl << "public " << getUnqualified("Ice.OutputStream", ns) << " outputStream => _ostr;";
        _out << sp;
        _out << nl << "private readonly " << getUnqualified("Ice.OutputStream", ns) << " _ostr;";
        _out << eb;
    }
}

Slice::Gen::ProxyVisitor::ProxyVisitor(IceUtilInternal::Output& out) : CsVisitor(out) {}

bool
Slice::Gen::ProxyVisitor::visitModuleStart(const ModulePtr& p)
{
    if (!p->hasInterfaceDefs())
    {
        return false;
    }

    moduleStart(p);
    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::ProxyVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
    moduleEnd(p);
}

bool
Slice::Gen::ProxyVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    string name = p->name();
    string ns = getNamespace(p);
    InterfaceList bases = p->bases();

    _out << sp;
    writeDocComment(p, getDeprecationMessageForComment(p, "interface"));
    emitGeneratedCodeAttribute();
    _out << nl << "public interface " << name << "Prx : ";

    vector<string> baseInterfaces;
    for (InterfaceList::const_iterator q = bases.begin(); q != bases.end(); ++q)
    {
        baseInterfaces.push_back(getUnqualified(*q, ns, "", "Prx"));
    }

    if (baseInterfaces.empty())
    {
        baseInterfaces.push_back(getUnqualified("Ice.ObjectPrx", ns));
    }

    for (vector<string>::const_iterator q = baseInterfaces.begin(); q != baseInterfaces.end();)
    {
        _out << *q;
        if (++q != baseInterfaces.end())
        {
            _out << ", ";
        }
    }
    _out << sb;

    return true;
}

void
Slice::Gen::ProxyVisitor::visitInterfaceDefEnd(const InterfaceDefPtr&)
{
    _out << eb;
}

void
Slice::Gen::ProxyVisitor::visitOperation(const OperationPtr& p)
{
    InterfaceDefPtr interface = p->interface();
    string ns = getNamespace(interface);
    string name = fixId(p->name(), DotNet::ICloneable, true);
    vector<string> inParams = getInParams(p, ns);
    ParamDeclList inParamDecls = p->inParameters();
    string retS = typeToString(p->returnType(), ns, p->returnIsOptional());
    string deprecateReason = getDeprecationMessageForComment(p, "operation");

    {
        //
        // Write the synchronous version of the operation.
        //
        string context = getEscapedParamName(p, "context");
        _out << sp;
        writeDocComment(
            p,
            deprecateReason,
            "<param name=\"" + context + "\">The Context map to send with the invocation.</param>");
        emitObsoleteAttribute(p, _out);
        _out << nl << retS << " " << name << spar << getParams(p, ns)
             << ("global::System.Collections.Generic.Dictionary<string, string>? " + context + " = null") << epar
             << ';';
    }

    {
        //
        // Write the async version of the operation (using Async Task API)
        //
        string context = getEscapedParamName(p, "context");
        string cancel = getEscapedParamName(p, "cancel");
        string progress = getEscapedParamName(p, "progress");

        _out << sp;
        writeDocCommentTaskAsyncAMI(
            p,
            deprecateReason,
            "<param name=\"" + context + "\">Context map to send with the invocation.</param>",
            "<param name=\"" + progress + "\">Sent progress provider.</param>",
            "<param name=\"" + cancel + "\">A cancellation token that receives the cancellation requests.</param>");
        emitObsoleteAttribute(p, _out);
        _out << nl << taskResultType(p, ns);
        _out << " " << p->name() << "Async" << spar << inParams
             << ("global::System.Collections.Generic.Dictionary<string, string>? " + context + " = null")
             << ("global::System.IProgress<bool>? " + progress + " = null")
             << ("global::System.Threading.CancellationToken " + cancel + " = default") << epar << ";";
    }
}

Slice::Gen::OpsVisitor::OpsVisitor(IceUtilInternal::Output& out) : CsVisitor(out) {}

bool
Slice::Gen::OpsVisitor::visitModuleStart(const ModulePtr& p)
{
    if (!p->hasInterfaceDefs())
    {
        return false;
    }
    moduleStart(p);
    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::OpsVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
    moduleEnd(p);
}

bool
Slice::Gen::OpsVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    string name = p->name();
    string ns = getNamespace(p);
    string scoped = fixId(p->scoped());
    InterfaceList bases = p->bases();
    string opIntfName = "Operations";

    _out << sp;
    writeDocComment(p, getDeprecationMessageForComment(p, "interface"));
    emitGeneratedCodeAttribute();
    _out << nl << "public interface " << name << opIntfName << '_';
    if (bases.size() > 0)
    {
        _out << " : ";
        InterfaceList::const_iterator q = bases.begin();
        bool first = true;
        while (q != bases.end())
        {
            if (!first)
            {
                _out << ", ";
            }
            else
            {
                first = false;
            }
            _out << getUnqualified(*q, ns, "", "Operations_");
            ++q;
        }
    }
    _out << sb;

    OperationList ops = p->operations();
    for (OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        bool amd = p->hasMetaData("amd") || op->hasMetaData("amd");
        string retS;
        vector<string> params, args;
        string opName = getDispatchParams(op, retS, params, args, ns);
        _out << sp;
        if (amd)
        {
            writeDocCommentAMD(op, "<param name=\"" + args.back() + "\">The Current object for the dispatch.</param>");
        }
        else
        {
            writeDocComment(
                op,
                getDeprecationMessageForComment(op, "operation"),
                "<param name=\"" + args.back() + "\">The Current object for the dispatch.</param>");
        }
        emitAttributes(op);
        emitObsoleteAttribute(op, _out);
        emitGeneratedCodeAttribute();
        _out << nl << retS << " " << opName << spar << params << epar << ";";
    }

    _out << eb;
    return false;
}

Slice::Gen::HelperVisitor::HelperVisitor(IceUtilInternal::Output& out) : CsVisitor(out) {}

bool
Slice::Gen::HelperVisitor::visitModuleStart(const ModulePtr& p)
{
    if (!p->hasInterfaceDefs() && !p->hasSequences() && !p->hasDictionaries())
    {
        return false;
    }

    moduleStart(p);
    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::HelperVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
    moduleEnd(p);
}

bool
Slice::Gen::HelperVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    string name = p->name();
    string ns = getNamespace(p);
    InterfaceList bases = p->bases();

    _out << sp;
    emitComVisibleAttribute();
    emitGeneratedCodeAttribute();
    _out << nl << "public sealed class " << name << "PrxHelper : " << getUnqualified("Ice.ObjectPrxHelperBase", ns)
         << ", " << name << "Prx";
    _out << sb;

    _out << sp;
    _out << nl << "public " << name << "PrxHelper()";
    _out << sb;
    _out << eb;

    OperationList ops = p->allOperations();

    if (!ops.empty())
    {
        _out << sp << nl << "#region Synchronous operations";
    }

    for (OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        InterfaceDefPtr interface = op->interface();
        string opName = fixId(op->name(), DotNet::ICloneable, true);
        TypePtr ret = op->returnType();
        string retS = typeToString(ret, ns, op->returnIsOptional());

        vector<string> params = getParams(op, ns);
        vector<string> args = getArgs(op);
        vector<string> argsAMI = getInArgs(op);

        string deprecateReason = getDeprecationMessageForComment(op, "operation");

        ParamDeclList inParams = op->inParameters();
        ParamDeclList outParams = op->outParameters();

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();

        //
        // Arrange exceptions into most-derived to least-derived order. If we don't
        // do this, a base exception handler can appear before a derived exception
        // handler, causing compiler warnings and resulting in the base exception
        // being marshaled instead of the derived exception.
        //
        throws.sort(Slice::DerivedToBaseCompare());

        string context = getEscapedParamName(op, "context");

        _out << sp;
        _out << nl << "public " << retS << " " << opName << spar << params
             << ("global::System.Collections.Generic.Dictionary<string, string>? " + context + " = null") << epar;
        _out << sb;
        _out << nl << "try";
        _out << sb;

        _out << nl;

        if (ret || !outParams.empty())
        {
            if (outParams.empty())
            {
                _out << "return ";
            }
            else if (ret || outParams.size() > 1)
            {
                _out << "var result_ = ";
            }
            else
            {
                _out << fixId(outParams.front()->name()) << " = ";
            }
        }
        _out << "_iceI_" << op->name() << "Async" << spar << argsAMI << context << "null"
             << "global::System.Threading.CancellationToken.None"
             << "true" << epar;

        if (ret || outParams.size() > 0)
        {
            _out << ".Result;";
        }
        else
        {
            _out << ".Wait();";
        }

        if ((ret && outParams.size() > 0) || outParams.size() > 1)
        {
            for (ParamDeclList::const_iterator i = outParams.begin(); i != outParams.end(); ++i)
            {
                ParamDeclPtr param = *i;
                _out << nl << fixId(param->name()) << " = result_." << fixId(param->name()) << ";";
            }

            if (ret)
            {
                _out << nl << "return result_." << resultStructReturnValueName(outParams) << ";";
            }
        }
        _out << eb;
        _out << nl << "catch (global::System.AggregateException ex_)";
        _out << sb;
        _out << nl << "throw ex_.InnerException!;";
        _out << eb;
        _out << eb;
    }

    if (!ops.empty())
    {
        _out << sp << nl << "#endregion"; // Synchronous operations
    }

    //
    // Async Task AMI mapping.
    //
    _out << sp << nl << "#region Async Task operations";
    for (OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;

        InterfaceDefPtr interface = op->interface();
        vector<string> paramsAMI = getInParams(op, ns);
        vector<string> argsAMI = getInArgs(op);

        string opName = op->name();

        ParamDeclList inParams = op->inParameters();
        ParamDeclList outParams = op->outParameters();

        string context = getEscapedParamName(op, "context");
        string cancel = getEscapedParamName(op, "cancel");
        string progress = getEscapedParamName(op, "progress");

        TypePtr ret = op->returnType();

        string retS = typeToString(ret, ns, op->returnIsOptional());

        string returnTypeS = resultType(op, ns);

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();

        //
        // Arrange exceptions into most-derived to least-derived order. If we don't
        // do this, a base exception handler can appear before a derived exception
        // handler, causing compiler warnings and resulting in the base exception
        // being marshaled instead of the derived exception.
        //
        throws.sort(Slice::DerivedToBaseCompare());

        //
        // Write the public Async method.
        //
        _out << sp;
        _out << nl << "public global::System.Threading.Tasks.Task";
        if (!returnTypeS.empty())
        {
            _out << "<" << returnTypeS << ">";
        }
        _out << " " << opName << "Async" << spar << paramsAMI
             << ("global::System.Collections.Generic.Dictionary<string, string>? " + context + " = null")
             << ("global::System.IProgress<bool>? " + progress + " = null")
             << ("global::System.Threading.CancellationToken " + cancel + " = default") << epar;

        _out << sb;
        _out << nl << "return _iceI_" << opName << "Async" << spar << argsAMI << context << progress << cancel
             << "false" << epar << ";";
        _out << eb;

        //
        // Write the Async method implementation.
        //
        _out << sp;
        _out << nl << "private global::System.Threading.Tasks.Task";
        if (!returnTypeS.empty())
        {
            _out << "<" << returnTypeS << ">";
        }
        _out << " _iceI_" << opName << "Async" << spar << getInParams(op, ns, true)
             << "global::System.Collections.Generic.Dictionary<string, string>? context"
             << "global::System.IProgress<bool>? progress"
             << "global::System.Threading.CancellationToken cancel"
             << "bool synchronous" << epar;
        _out << sb;

        string flatName = "_" + opName + "_name";
        if (op->returnsData())
        {
            _out << nl << "iceCheckTwowayOnly(" << flatName << ");";
        }
        if (returnTypeS.empty())
        {
            _out << nl << "var completed = "
                 << "new global::Ice.Internal.OperationTaskCompletionCallback<object>(progress, cancel);";
        }
        else
        {
            _out << nl << "var completed = "
                 << "new global::Ice.Internal.OperationTaskCompletionCallback<" << returnTypeS
                 << ">(progress, cancel);";
        }

        _out << nl << "_iceI_" << opName << spar << getInArgs(op, true) << "context"
             << "synchronous"
             << "completed" << epar << ";";
        _out << nl << "return completed.Task;";

        _out << eb;

        _out << sp << nl << "private const string " << flatName << " = \"" << op->name() << "\";";

        //
        // Write the common invoke method
        //
        _out << sp << nl;
        _out << "private void _iceI_" << op->name() << spar << getInParams(op, ns, true)
             << "global::System.Collections.Generic.Dictionary<string, string>? context"
             << "bool synchronous"
             << "global::Ice.Internal.OutgoingAsyncCompletionCallback completed" << epar;
        _out << sb;

        if (returnTypeS.empty())
        {
            _out << nl << "var outAsync = getOutgoingAsync<object>(completed);";
        }
        else
        {
            _out << nl << "var outAsync = getOutgoingAsync<" << returnTypeS << ">(completed);";
        }

        _out << nl << "outAsync.invoke(";
        _out.inc();
        _out << nl << flatName << ",";
        _out << nl << sliceModeToIceMode(op->mode(), ns) << ",";
        _out << nl << opFormatTypeToString(op, ns) << ",";
        _out << nl << "context,";
        _out << nl << "synchronous";
        if (!inParams.empty())
        {
            _out << ",";
            _out << nl << "write: (" << getUnqualified("Ice.OutputStream", ns) << " ostr) =>";
            _out << sb;
            writeMarshalUnmarshalParams(inParams, 0, true, ns);
            if (op->sendsClasses(false))
            {
                _out << nl << "ostr.writePendingValues();";
            }
            _out << eb;
        }

        if (!throws.empty())
        {
            _out << ",";
            _out << nl << "userException: (" << getUnqualified("Ice.UserException", ns) << " ex) =>";
            _out << sb;
            _out << nl << "try";
            _out << sb;
            _out << nl << "throw ex;";
            _out << eb;

            //
            // Generate a catch block for each legal user exception.
            //
            for (ExceptionList::const_iterator i = throws.begin(); i != throws.end(); ++i)
            {
                _out << nl << "catch(" << getUnqualified(*i, ns) << ")";
                _out << sb;
                _out << nl << "throw;";
                _out << eb;
            }

            _out << nl << "catch(" << getUnqualified("Ice.UserException", ns) << ")";
            _out << sb;
            _out << eb;

            _out << eb;
        }

        if (ret || !outParams.empty())
        {
            _out << ",";
            _out << nl << "read: (" << getUnqualified("Ice.InputStream", ns) << " istr) =>";
            _out << sb;
            if (outParams.empty())
            {
                _out << nl << returnTypeS << " ret" << (isClassType(ret) ? " = null;" : ";");
            }
            else if (ret || outParams.size() > 1)
            {
                // Generated OpResult struct
                _out << nl << "var ret = new " << returnTypeS << "();";
            }
            else
            {
                TypePtr t = outParams.front()->type();
                _out << nl << typeToString(t, ns, (outParams.front()->optional())) << " iceP_"
                     << outParams.front()->name() << (isClassType(t) ? " = null;" : ";");
            }

            writeMarshalUnmarshalParams(outParams, op, false, ns, true);
            if (op->returnsClasses(false))
            {
                _out << nl << "istr.readPendingValues();";
            }

            if (!ret && outParams.size() == 1)
            {
                _out << nl << "return iceP_" << outParams.front()->name() << ";";
            }
            else
            {
                _out << nl << "return ret;";
            }
            _out << eb;
        }
        _out << ");";
        _out.dec();
        _out << eb;
    }

    _out << sp << nl << "#endregion"; // Asynchronous Task operations

    _out << sp << nl << "#region Factory operations";

    _out << sp << nl << "public static " << name << "Prx createProxy(" << getUnqualified("Ice.Communicator", ns)
         << " communicator, string proxyString) =>";
    _out.inc();
    _out << nl << "uncheckedCast(" << getUnqualified("Ice.ObjectPrxHelper", ns)
         << ".createProxy(communicator, proxyString));";
    _out.dec();

    _out << sp << nl << "public static " << name << "Prx? checkedCast(" << getUnqualified("Ice.ObjectPrx", ns)
         << " b, global::System.Collections.Generic.Dictionary<string, string>? ctx = null)";
    _out << sb;
    _out << nl << "if (b is not null && b.ice_isA(ice_staticId(), ctx))";
    _out << sb;
    _out << nl << name << "PrxHelper prx = new " << name << "PrxHelper();";
    _out << nl << "prx.iceCopyFrom(b);";
    _out << nl << "return prx;";
    _out << eb;
    _out << nl << "return null;";
    _out << eb;

    _out << sp << nl << "public static " << name << "Prx? checkedCast(" << getUnqualified("Ice.ObjectPrx", ns)
         << " b, string f, "
         << "global::System.Collections.Generic.Dictionary<string, string>? ctx = null)";
    _out << sb;
    _out << nl << getUnqualified("Ice.ObjectPrx?", ns) << " bb = b?.ice_facet(f);";
    _out << nl << "try";
    _out << sb;
    _out << nl << "if (bb is not null && bb.ice_isA(ice_staticId(), ctx))";
    _out << sb;
    _out << nl << name << "PrxHelper prx = new " << name << "PrxHelper();";
    _out << nl << "prx.iceCopyFrom(bb);";
    _out << nl << "return prx;";
    _out << eb;
    _out << eb;
    _out << nl << "catch (" << getUnqualified("Ice.FacetNotExistException", ns) << ")";
    _out << sb;
    _out << eb;
    _out << nl << "return null;";
    _out << eb;

    _out << sp << nl << "[return: global::System.Diagnostics.CodeAnalysis.NotNullIfNotNull(nameof(b))]";
    _out << sp << nl << "public static " << name << "Prx? uncheckedCast(" << getUnqualified("Ice.ObjectPrx?", ns)
         << " b)";
    _out << sb;
    _out << nl << "if (b is not null)";
    _out << sb;
    _out << nl << "var prx = new " << name << "PrxHelper();";
    _out << nl << "prx.iceCopyFrom(b);";
    _out << nl << "return prx;";
    _out << eb;
    _out << nl << "return null;";
    _out << eb;

    _out << sp << nl << "[return: global::System.Diagnostics.CodeAnalysis.NotNullIfNotNull(nameof(b))]";
    _out << sp << nl << "public static " << name << "Prx? uncheckedCast(" << getUnqualified("Ice.ObjectPrx?", ns)
         << " b, string f)";
    _out << sb;
    _out << nl << "if (b is not null)";
    _out << sb;
    _out << nl << getUnqualified("Ice.ObjectPrx?", ns) << " bb = b.ice_facet(f);";
    _out << nl << "var prx = new " << name << "PrxHelper();";
    _out << nl << "prx.iceCopyFrom(bb);";
    _out << nl << "return prx;";
    _out << eb;
    _out << nl << "return null;";
    _out << eb;

    string scoped = p->scoped();
    StringList ids = p->ids();

    //
    // Need static-readonly for arrays in C# (not const)
    //
    _out << sp << nl << "private static readonly string[] _ids =";
    _out << sb;

    {
        StringList::const_iterator q = ids.begin();
        while (q != ids.end())
        {
            _out << nl << '"' << *q << '"';
            if (++q != ids.end())
            {
                _out << ',';
            }
        }
    }
    _out << eb << ";";

    _out << sp << nl << "public static string ice_staticId() => \"" << scoped << "\";";

    _out << sp << nl << "#endregion"; // Factory operations

    _out << sp << nl << "#region Marshaling support";

    _out << sp << nl << "public static void write(" << getUnqualified("Ice.OutputStream", ns) << " ostr, " << name
         << "Prx? v)";
    _out << sb;
    _out << nl << "ostr.writeProxy(v);";
    _out << eb;

    _out << sp << nl << "public static " << name << "Prx? read(" << getUnqualified("Ice.InputStream", ns) << " istr)";
    _out << sb;
    _out << nl << getUnqualified("Ice.ObjectPrx?", ns) << " proxy = istr.readProxy();";
    _out << nl << "if (proxy is not null)";
    _out << sb;
    _out << nl << " var result = new " << name << "PrxHelper();";
    _out << nl << "result.iceCopyFrom(proxy);";
    _out << nl << "return result;";
    _out << eb;
    _out << nl << "return null;";
    _out << eb;

    _out << sp << nl << "#endregion"; // Marshaling support

    return true;
}

void
Slice::Gen::HelperVisitor::visitInterfaceDefEnd(const InterfaceDefPtr&)
{
    _out << eb;
}

void
Slice::Gen::HelperVisitor::visitSequence(const SequencePtr& p)
{
    string ns = getNamespace(p);
    string typeS = typeToString(p, ns);
    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public sealed class " << p->name() << "Helper";
    _out << sb;

    _out << sp << nl << "public static void write(" << getUnqualified("Ice.OutputStream", ns) << " ostr, " << typeS
         << " v)";
    _out << sb;
    writeSequenceMarshalUnmarshalCode(_out, p, ns, "v", true, false);
    _out << eb;

    _out << sp << nl << "public static " << typeS << " read(" << getUnqualified("Ice.InputStream", ns) << " istr)";
    _out << sb;
    _out << nl << typeS << " v;";
    writeSequenceMarshalUnmarshalCode(_out, p, ns, "v", false, false);
    _out << nl << "return v;";
    _out << eb;
    _out << eb;

    string prefix = "cs:generic:";
    string meta;
    if (p->findMetaData(prefix, meta))
    {
        string type = meta.substr(prefix.size());
        if (type == "List" || type == "LinkedList" || type == "Queue" || type == "Stack")
        {
            return;
        }

        if (!isClassType(p->type()))
        {
            return;
        }

        //
        // The sequence is a custom sequence with elements of class type.
        // Emit a dummy class that causes a compile-time error if the
        // custom sequence type does not implement an indexer.
        //
        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public class " << p->name() << "_Tester";
        _out << sb;
        _out << nl << p->name() << "_Tester()";
        _out << sb;
        _out << nl << typeS << " test = new " << typeS << "();";
        _out << nl << "test[0] = null;";
        _out << eb;
        _out << eb;
    }
}

void
Slice::Gen::HelperVisitor::visitDictionary(const DictionaryPtr& p)
{
    TypePtr key = p->keyType();
    TypePtr value = p->valueType();

    string meta;

    string prefix = "cs:generic:";
    string genericType;
    if (!p->findMetaData(prefix, meta))
    {
        genericType = "Dictionary";
    }
    else
    {
        genericType = meta.substr(prefix.size());
    }

    string ns = getNamespace(p);
    string keyS = typeToString(key, ns);
    string valueS = typeToString(value, ns);
    string name = "global::System.Collections.Generic." + genericType + "<" + keyS + ", " + valueS + ">";

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public sealed class " << p->name() << "Helper";
    _out << sb;

    _out << sp << nl << "public static void write(";
    _out.useCurrentPosAsIndent();
    _out << getUnqualified("Ice.OutputStream", ns) << " ostr,";
    _out << nl << name << " v)";
    _out.restoreIndent();
    _out << sb;
    _out << nl << "if(v == null)";
    _out << sb;
    _out << nl << "ostr.writeSize(0);";
    _out << eb;
    _out << nl << "else";
    _out << sb;
    _out << nl << "ostr.writeSize(v.Count);";
    _out << nl << "foreach(global::System.Collections.";
    _out << "Generic.KeyValuePair<" << keyS << ", " << valueS << ">";
    _out << " e in v)";
    _out << sb;
    writeMarshalUnmarshalCode(_out, key, ns, "e.Key", true);
    writeMarshalUnmarshalCode(_out, value, ns, "e.Value", true);
    _out << eb;
    _out << eb;
    _out << eb;

    _out << sp << nl << "public static " << name << " read(" << getUnqualified("Ice.InputStream", ns) << " istr)";
    _out << sb;
    _out << nl << "int sz = istr.readSize();";
    _out << nl << name << " r = new " << name << "();";
    _out << nl << "for(int i = 0; i < sz; ++i)";
    _out << sb;
    _out << nl << keyS << " k;";
    writeMarshalUnmarshalCode(_out, key, ns, "k", false);

    if (isClassType(value))
    {
        ostringstream os;
        os << '(' << typeToString(value, ns) << " v) => { r[k] = v; }";
        writeMarshalUnmarshalCode(_out, value, ns, os.str(), false);
    }
    else
    {
        _out << nl << valueS << " v;";
        writeMarshalUnmarshalCode(_out, value, ns, "v", false);
        _out << nl << "r[k] = v;";
    }
    _out << eb;
    _out << nl << "return r;";
    _out << eb;

    _out << eb;
}

Slice::Gen::DispatcherVisitor::DispatcherVisitor(::IceUtilInternal::Output& out, bool tie) : CsVisitor(out), _tie(tie)
{
}

bool
Slice::Gen::DispatcherVisitor::visitModuleStart(const ModulePtr& p)
{
    if (!p->hasInterfaceDefs())
    {
        return false;
    }

    moduleStart(p);
    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::DispatcherVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
    moduleEnd(p);
}

bool
Slice::Gen::DispatcherVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    InterfaceList bases = p->bases();
    string name = p->name();
    string ns = getNamespace(p);
    string baseClass = getUnqualified("Ice.ObjectImpl", ns);

    _out << sp;
    emitComVisibleAttribute();
    emitGeneratedCodeAttribute();
    _out << nl << "public abstract class " << name << "Disp_ : " << baseClass << ", ";

    _out << fixId(name);

    _out << sb;

    OperationList ops = p->operations();
    if (!ops.empty())
    {
        _out << sp << nl << "#region Slice operations";
    }

    for (OperationList::const_iterator i = ops.begin(); i != ops.end(); ++i)
    {
        string retS;
        vector<string> params, args;
        string opName = getDispatchParams(*i, retS, params, args, ns);
        _out << sp << nl << "public abstract " << retS << " " << opName << spar << params << epar << ';';
    }

    if (!ops.empty())
    {
        _out << sp << nl << "#endregion"; // Slice operations
    }

    writeInheritedOperations(p);
    writeDispatch(p);

    if ((_tie || p->hasMetaData("cs:tie")))
    {
        // Need to generate tie

        // close previous class
        _out << eb;

        string opIntfName = "Operations";

        _out << sp;
        emitComVisibleAttribute();
        emitGeneratedCodeAttribute();
        _out << nl << "public class " << name << "Tie_ : " << name << "Disp_, " << getUnqualified("Ice.TieBase", ns);

        _out << sb;

        _out << sp << nl << "public " << name << "Tie_()";
        _out << sb;
        _out << eb;

        _out << sp << nl << "public " << name << "Tie_(" << name << opIntfName << "_ del)";
        _out << sb;
        _out << nl << "_ice_delegate = del;";
        _out << eb;

        _out << sp << nl << "public object? ice_delegate()";
        _out << sb;
        _out << nl << "return _ice_delegate;";
        _out << eb;

        _out << sp << nl << "public void ice_delegate(object del)";
        _out << sb;
        _out << nl << "_ice_delegate = (" << name << opIntfName << "_)del;";
        _out << eb;

        _out << sp << nl << "public override int GetHashCode()";
        _out << sb;
        _out << nl << "return _ice_delegate == null ? 0 : _ice_delegate.GetHashCode();";
        _out << eb;

        _out << sp << nl << "public override bool Equals(object? rhs)";
        _out << sb;
        _out << nl << "if (object.ReferenceEquals(this, rhs))";
        _out << sb;
        _out << nl << "return true;";
        _out << eb;
        _out << nl << "if (!(rhs is " << name << "Tie_))";
        _out << sb;
        _out << nl << "return false;";
        _out << eb;
        _out << nl << "if (_ice_delegate == null)";
        _out << sb;
        _out << nl << "return ((" << name << "Tie_)rhs)._ice_delegate == null;";
        _out << eb;
        _out << nl << "return _ice_delegate.Equals(((" << name << "Tie_)rhs)._ice_delegate);";
        _out << eb;

        writeTieOperations(p);

        _out << sp << nl << "private " << name << opIntfName << "_? _ice_delegate;";
    }

    return true;
}
void
Slice::Gen::DispatcherVisitor::visitInterfaceDefEnd(const InterfaceDefPtr&)
{
    _out << eb;
}

void
Slice::Gen::DispatcherVisitor::writeTieOperations(const InterfaceDefPtr& p, NameSet* opNames)
{
    string ns = getNamespace(p);
    OperationList ops = p->operations();
    for (OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        string retS;
        vector<string> params;
        vector<string> args;
        string opName = getDispatchParams(*r, retS, params, args, ns);
        if (opNames)
        {
            if (opNames->find(opName) != opNames->end())
            {
                continue;
            }
            opNames->insert(opName);
        }

        _out << sp << nl << "public override " << retS << ' ' << opName << spar << params << epar;
        _out << sb;
        _out << nl;
        if (retS != "void")
        {
            _out << "return ";
        }
        _out << "_ice_delegate!." << opName << spar << args << epar << ';';
        _out << eb;
    }

    if (!opNames)
    {
        NameSet opNamesTmp;
        InterfaceList bases = p->bases();
        for (InterfaceList::const_iterator i = bases.begin(); i != bases.end(); ++i)
        {
            writeTieOperations(*i, &opNamesTmp);
        }
    }
    else
    {
        InterfaceList bases = p->bases();
        for (InterfaceList::const_iterator i = bases.begin(); i != bases.end(); ++i)
        {
            writeTieOperations(*i, opNames);
        }
    }
}
