// **********************************************************************
//
// Copyright (c) 2004
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <FreezeScript/DumpDescriptors.h>
#include <FreezeScript/AssignVisitor.h>
#include <FreezeScript/Print.h>
#include <FreezeScript/Functions.h>
#include <FreezeScript/Exception.h>
#include <FreezeScript/Util.h>
#include <db_cxx.h>

using namespace std;

namespace FreezeScript
{

class SymbolTableI;
typedef IceUtil::Handle<SymbolTableI> SymbolTableIPtr;

class SymbolTableI : public SymbolTable
{
public:

    SymbolTableI(const DataFactoryPtr&, const Slice::UnitPtr&, const ErrorReporterPtr&, ExecuteInfo*,
                 const SymbolTablePtr& = SymbolTablePtr());

    virtual void add(const string&, const DataPtr&);

    virtual DataPtr getValue(const EntityNodePtr&) const;

    virtual DataPtr getConstantValue(const string&) const;

    virtual SymbolTablePtr createChild();

    virtual Slice::TypePtr lookupType(const string&);

    virtual DataPtr invokeFunction(const string&, const DataPtr&, const DataList&);

private:

    DataPtr findValue(const string&) const;

    class EntityVisitor : public EntityNodeVisitor
    {
    public:

        EntityVisitor(SymbolTableI*);

        virtual void visitIdentifier(const string&);
        virtual void visitElement(const NodePtr&);

        DataPtr getCurrent() const;

    private:

        SymbolTableI* _table;
        DataPtr _current;
        bool _error;
    };

    friend class EntityVisitor;

    DataFactoryPtr _factory;
    Slice::UnitPtr _unit;
    ErrorReporterPtr _errorReporter;
    ExecuteInfo* _info;
    SymbolTablePtr _parent;
    typedef map<string, DataPtr> DataMap;
    DataMap _dataMap;
    DataMap _constantCache;
};

typedef map<string, DumpDescriptorPtr> DumpMap;

struct ExecuteInfo
{
    Ice::CommunicatorPtr communicator;
    Db* db;
    DbTxn* txn;
    SymbolTablePtr symbolTable;
    DumpMap dumpMap;
    Slice::TypePtr keyType;
    Slice::TypePtr valueType;
};

class DumpVisitor : public DataVisitor
{
public:

    DumpVisitor(const DataFactoryPtr&, const Slice::UnitPtr&, const ErrorReporterPtr&, ExecuteInfo*);

    virtual void visitBoolean(const BooleanDataPtr&);
    virtual void visitInteger(const IntegerDataPtr&);
    virtual void visitDouble(const DoubleDataPtr&);
    virtual void visitString(const StringDataPtr&);
    virtual void visitProxy(const ProxyDataPtr&);
    virtual void visitStruct(const StructDataPtr&);
    virtual void visitSequence(const SequenceDataPtr&);
    virtual void visitEnum(const EnumDataPtr&);
    virtual void visitDictionary(const DictionaryDataPtr&);
    virtual void visitObject(const ObjectRefPtr&);

private:

    void dump(const DataPtr&);

    DataFactoryPtr _factory;
    Slice::UnitPtr _unit;
    ErrorReporterPtr _errorReporter;
    ExecuteInfo* _info;
};

} // End of namespace FreezeScript

//
// Descriptor
//
FreezeScript::Descriptor::Descriptor(const DescriptorPtr& parent, int line, const DataFactoryPtr& factory,
                                     const ErrorReporterPtr& errorReporter) :
    _parent(parent), _line(line), _factory(factory), _errorReporter(errorReporter)
{
}

FreezeScript::Descriptor::~Descriptor()
{
}

FreezeScript::DescriptorPtr
FreezeScript::Descriptor::parent() const
{
    return _parent;
}

FreezeScript::NodePtr
FreezeScript::Descriptor::parse(const string& expr) const
{
    return parseExpression(expr, _factory, _errorReporter);
}

Slice::TypePtr
FreezeScript::Descriptor::findType(const Slice::UnitPtr& u, const string& type)
{
    Slice::TypeList l;

    l = u->lookupType(type, false);
    if(l.empty())
    {
        _errorReporter->error("unknown type `" + type + "'");
    }

    return l.front();
}

//
// SetDescriptor
//
FreezeScript::SetDescriptor::SetDescriptor(const DescriptorPtr& parent, int line, const DataFactoryPtr& factory,
                                           const ErrorReporterPtr& errorReporter,
                                           const IceXML::Attributes& attributes) :
    Descriptor(parent, line, factory, errorReporter)
{
    DescriptorErrorContext ctx(_errorReporter, "set", _line);

    IceXML::Attributes::const_iterator p;

    p = attributes.find("target");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `target' is missing");
    }
    NodePtr node = parse(p->second);
    _target = EntityNodePtr::dynamicCast(node);
    if(!_target)
    {
        _errorReporter->error("`target' attribute is not an entity: `" + p->second + "'");
    }

    p = attributes.find("value");
    if(p != attributes.end())
    {
        _valueStr = p->second;
    }

    p = attributes.find("length");
    if(p != attributes.end())
    {
        _lengthStr = p->second;
    }

    p = attributes.find("convert");
    if(p != attributes.end())
    {
        _convert = p->second == "true";
    }

    if(_valueStr.empty() && _lengthStr.empty())
    {
        _errorReporter->error("requires a value for attributes `value' or 'length'");
    }

    if(!_valueStr.empty())
    {
        _value = parse(_valueStr);
    }

    if(!_lengthStr.empty())
    {
        _length = parse(_lengthStr);
    }
}

void
FreezeScript::SetDescriptor::addChild(const DescriptorPtr&)
{
    DescriptorErrorContext ctx(_errorReporter, "set", _line);
    _errorReporter->error("child elements are not supported");
}

void
FreezeScript::SetDescriptor::validate()
{
}

void
FreezeScript::SetDescriptor::execute(const SymbolTablePtr& sym, ExecuteInfo*)
{
    DescriptorErrorContext ctx(_errorReporter, "set", _line);

    DataPtr data = sym->getValue(_target);
    if(data->readOnly())
    {
        ostringstream ostr;
        ostr << _target;
        _errorReporter->error("target `" + ostr.str() + "' cannot be modified");
    }

    DataPtr value;
    if(_value)
    {
        try
        {
            value = _value->evaluate(sym);
        }
        catch(const EvaluateException& ex)
        {
            _errorReporter->error("evaluation of value `" + _valueStr + "' failed:\n" + ex.reason());
        }
    }

    DataPtr length;
    if(_length)
    {
        SequenceDataPtr seq = SequenceDataPtr::dynamicCast(data);
        if(!seq)
        {
            ostringstream ostr;
            ostr << _target;
            _errorReporter->error("value `" + ostr.str() + "' is not a sequence");
        }

        try
        {
            length = _length->evaluate(sym);
        }
        catch(const EvaluateException& ex)
        {
            _errorReporter->error("evaluation of length `" + _lengthStr + "' failed:\n" + ex.reason());
        }

        DataList& elements = seq->getElements();
        Ice::Long l = length->integerValue();
        if(l < 0 || l > INT_MAX)
        {
            _errorReporter->error("sequence length " + length->toString() + " is out of range");
        }

        DataList::size_type len = static_cast<DataList::size_type>(l);
        if(len < elements.size())
        {
            for(DataList::size_type i = len; i < elements.size(); ++i)
            {
                elements[i]->destroy();
            }
            elements.resize(len);
        }
        else if(len > elements.size())
        {
            Slice::SequencePtr seqType = Slice::SequencePtr::dynamicCast(seq->getType());
            assert(seqType);
            Slice::TypePtr elemType = seqType->type();
            for(DataList::size_type i = elements.size(); i < len; ++i)
            {
                DataPtr v = _factory->create(elemType, false);
                if(value)
                {
                    AssignVisitor visitor(value, _factory, _errorReporter, _convert);
                    v->visit(visitor);
                }
                elements.push_back(v);
            }
        }
    }
    else
    {
        AssignVisitor visitor(value, _factory, _errorReporter, _convert);
        data->visit(visitor);
    }
}

//
// DefineDescriptor
//
FreezeScript::DefineDescriptor::DefineDescriptor(const DescriptorPtr& parent, int line, const DataFactoryPtr& factory,
                                                 const ErrorReporterPtr& errorReporter,
                                                 const IceXML::Attributes& attributes, const Slice::UnitPtr& unit) :
    Descriptor(parent, line, factory, errorReporter)
{
    DescriptorErrorContext ctx(_errorReporter, "define", _line);

    IceXML::Attributes::const_iterator p;

    p = attributes.find("name");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `name' is missing");
    }
    _name = p->second;

    p = attributes.find("type");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `type' is missing");
    }
    _type = findType(unit, p->second);

    p = attributes.find("value");
    if(p != attributes.end())
    {
        _valueStr = p->second;
    }

    p = attributes.find("convert");
    if(p != attributes.end())
    {
        _convert = p->second == "true";
    }

    if(!_valueStr.empty())
    {
        _value = parse(_valueStr);
    }
}

void
FreezeScript::DefineDescriptor::addChild(const DescriptorPtr&)
{
    DescriptorErrorContext ctx(_errorReporter, "define", _line);
    _errorReporter->error("child elements are not supported");
}

void
FreezeScript::DefineDescriptor::validate()
{
}

void
FreezeScript::DefineDescriptor::execute(const SymbolTablePtr& sym, ExecuteInfo*)
{
    DescriptorErrorContext ctx(_errorReporter, "define", _line);

    DataPtr data = _factory->create(_type, false);

    DataPtr value;
    if(_value)
    {
        try
        {
            value = _value->evaluate(sym);
        }
        catch(const EvaluateException& ex)
        {
            _errorReporter->error("evaluation of value `" + _valueStr + "' failed:\n" + ex.reason());
        }
    }

    if(value)
    {
        AssignVisitor visitor(value, _factory, _errorReporter, _convert);
        data->visit(visitor);
    }

    sym->add(_name, data);
}

//
// AddDescriptor
//
FreezeScript::AddDescriptor::AddDescriptor(const DescriptorPtr& parent, int line, const DataFactoryPtr& factory,
                                           const ErrorReporterPtr& errorReporter,
                                           const IceXML::Attributes& attributes) :
    Descriptor(parent, line, factory, errorReporter)
{
    DescriptorErrorContext ctx(_errorReporter, "add", _line);

    IceXML::Attributes::const_iterator p;

    string target;
    p = attributes.find("target");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `target' is missing");
    }
    target = p->second;

    p = attributes.find("key");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `key' is missing");
    }
    _keyStr = p->second;

    p = attributes.find("value");
    if(p != attributes.end())
    {
        _valueStr = p->second;
    }

    p = attributes.find("convert");
    if(p != attributes.end())
    {
        _convert = p->second == "true";
    }

    NodePtr node = parse(target);
    _target = EntityNodePtr::dynamicCast(node);
    if(!_target)
    {
        _errorReporter->error("`target' attribute is not an entity: `" + target + "'");
    }

    assert(!_keyStr.empty());
    _key = parse(_keyStr);

    if(!_valueStr.empty())
    {
        _value = parse(_valueStr);
    }
}

void
FreezeScript::AddDescriptor::addChild(const DescriptorPtr&)
{
    DescriptorErrorContext ctx(_errorReporter, "add", _line);
    _errorReporter->error("child elements are not supported");
}

void
FreezeScript::AddDescriptor::validate()
{
}

void
FreezeScript::AddDescriptor::execute(const SymbolTablePtr& sym, ExecuteInfo*)
{
    DescriptorErrorContext ctx(_errorReporter, "add", _line);

    DataPtr data = sym->getValue(_target);
    if(data->readOnly())
    {
        ostringstream ostr;
        ostr << _target;
        _errorReporter->error("target `" + ostr.str() + "' cannot be modified");
    }

    DictionaryDataPtr dict = DictionaryDataPtr::dynamicCast(data);
    if(!dict)
    {
        ostringstream ostr;
        ostr << _target;
        _errorReporter->error("target `" + ostr.str() + "' is not a dictionary");
    }

    Slice::DictionaryPtr type = Slice::DictionaryPtr::dynamicCast(dict->getType());
    assert(type);

    DataPtr key;
    Destroyer<DataPtr> keyDestroyer;
    try
    {
        DataPtr v = _key->evaluate(sym);
        key = _factory->create(type->keyType(), false);
        keyDestroyer.set(key);
        AssignVisitor visitor(v, _factory, _errorReporter, _convert);
        key->visit(visitor);
    }
    catch(const EvaluateException& ex)
    {
        _errorReporter->error("evaluation of key `" + _keyStr + "' failed:\n" + ex.reason());
    }

    if(dict->getElement(key))
    {
        ostringstream ostr;
        printData(key, ostr);
        _errorReporter->error("key " + ostr.str() + " already exists in dictionary");
    }

    DataPtr elem = _factory->create(type->valueType(), false);
    Destroyer<DataPtr> elemDestroyer(elem);

    DataPtr value;
    if(_value)
    {
        try
        {
            value = _value->evaluate(sym);
        }
        catch(const EvaluateException& ex)
        {
            _errorReporter->error("evaluation of value `" + _valueStr + "' failed:\n" + ex.reason());
        }
    }

    if(value)
    {
        AssignVisitor visitor(value, _factory, _errorReporter, _convert);
        elem->visit(visitor);
    }
    DataMap& map = dict->getElements();
    map.insert(DataMap::value_type(key, elem));
    keyDestroyer.release();
    elemDestroyer.release();
}

//
// RemoveDescriptor
//
FreezeScript::RemoveDescriptor::RemoveDescriptor(const DescriptorPtr& parent, int line, const DataFactoryPtr& factory,
                                                 const ErrorReporterPtr& errorReporter,
                                                 const IceXML::Attributes& attributes) :
    Descriptor(parent, line, factory, errorReporter)
{
    DescriptorErrorContext ctx(_errorReporter, "remove", _line);

    IceXML::Attributes::const_iterator p;

    string target;
    p = attributes.find("target");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `target' is missing");
    }
    target = p->second;

    p = attributes.find("key");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `key' is missing");
    }
    _keyStr = p->second;

    NodePtr node = parse(target);
    _target = EntityNodePtr::dynamicCast(node);
    if(!_target)
    {
        _errorReporter->error("`target' attribute is not an entity: `" + target + "'");
    }

    _key = parse(_keyStr);
}

void
FreezeScript::RemoveDescriptor::addChild(const DescriptorPtr&)
{
    DescriptorErrorContext ctx(_errorReporter, "remove", _line);
    _errorReporter->error("child elements are not supported");
}

void
FreezeScript::RemoveDescriptor::validate()
{
}

void
FreezeScript::RemoveDescriptor::execute(const SymbolTablePtr& sym, ExecuteInfo*)
{
    DescriptorErrorContext ctx(_errorReporter, "remove", _line);

    DataPtr key;
    try
    {
        key = _key->evaluate(sym);
    }
    catch(const EvaluateException& ex)
    {
        _errorReporter->error("evaluation of key `" + _keyStr + "' failed:\n" + ex.reason());
    }

    DataPtr data = sym->getValue(_target);
    if(data->readOnly())
    {
        ostringstream ostr;
        ostr << _target;
        _errorReporter->error("target `" + ostr.str() + "' cannot be modified");
    }

    DictionaryDataPtr dict = DictionaryDataPtr::dynamicCast(data);
    if(!dict)
    {
        ostringstream ostr;
        ostr << _target;
        _errorReporter->error("target `" + ostr.str() + "' is not a dictionary");
    }

    DataMap& map = dict->getElements();
    DataMap::iterator p = map.find(key);
    if(p != map.end())
    {
        p->first->destroy();
        p->second->destroy();
        map.erase(p);
    }
}

//
// FailDescriptor
//
FreezeScript::FailDescriptor::FailDescriptor(const DescriptorPtr& parent, int line, const DataFactoryPtr& factory,
                                             const ErrorReporterPtr& errorReporter,
                                             const IceXML::Attributes& attributes) :
    Descriptor(parent, line, factory, errorReporter)
{
    IceXML::Attributes::const_iterator p;

    p = attributes.find("test");
    if(p != attributes.end())
    {
        _testStr = p->second;
    }

    p = attributes.find("message");
    if(p != attributes.end())
    {
        _message = p->second;
    }

    if(!_testStr.empty())
    {
        _test = parse(_testStr);
    }

    if(_message.empty())
    {
        ostringstream ostr;
        ostr << "<fail> executed at line " << line << endl;
        _message = ostr.str();
    }
}

void
FreezeScript::FailDescriptor::addChild(const DescriptorPtr&)
{
    DescriptorErrorContext ctx(_errorReporter, "fail", _line);
    _errorReporter->error("child elements are not supported");
}

void
FreezeScript::FailDescriptor::validate()
{
}

void
FreezeScript::FailDescriptor::execute(const SymbolTablePtr& sym, ExecuteInfo*)
{
    DescriptorErrorContext ctx(_errorReporter, "fail", _line);

    if(_test)
    {
        try
        {
            DataPtr b = _test->evaluate(sym);
            BooleanDataPtr bd = BooleanDataPtr::dynamicCast(b);
            if(!bd)
            {
                _errorReporter->error("expression `" + _testStr + "' does not evaluate to a boolean");
            }
            if(!bd->booleanValue())
            {
                return;
            }
        }
        catch(const EvaluateException& ex)
        {
            _errorReporter->error("evaluation of expression `" + _testStr + "' failed:\n" + ex.reason());
        }
    }

    throw Exception(__FILE__, __LINE__, _message);
}

//
// EchoDescriptor
//
FreezeScript::EchoDescriptor::EchoDescriptor(const DescriptorPtr& parent, int line, const DataFactoryPtr& factory,
                                             const ErrorReporterPtr& errorReporter,
                                             const IceXML::Attributes& attributes) :
    Descriptor(parent, line, factory, errorReporter)
{
    IceXML::Attributes::const_iterator p;

    p = attributes.find("message");
    if(p != attributes.end())
    {
        _message = p->second;
    }

    p = attributes.find("value");
    if(p != attributes.end())
    {
        _valueStr = p->second;
    }

    if(!_valueStr.empty())
    {
        _value = parse(_valueStr);
    }
}

void
FreezeScript::EchoDescriptor::addChild(const DescriptorPtr&)
{
    DescriptorErrorContext ctx(_errorReporter, "echo", _line);
    _errorReporter->error("child elements are not supported");
}

void
FreezeScript::EchoDescriptor::validate()
{
}

void
FreezeScript::EchoDescriptor::execute(const SymbolTablePtr& sym, ExecuteInfo*)
{
    DescriptorErrorContext ctx(_errorReporter, "echo", _line);

    ostream& out = _errorReporter->stream();

    if(!_message.empty())
    {
        out << _message;
    }

    if(_value)
    {
        DataPtr v;
        try
        {
            v = _value->evaluate(sym);
        }
        catch(const EvaluateException& ex)
        {
            _errorReporter->error("evaluation of value `" + _valueStr + "' failed:\n" + ex.reason());
        }
        printData(v, out);
    }

    out << endl;
}

//
// ExecutableContainerDescriptor
//
FreezeScript::ExecutableContainerDescriptor::ExecutableContainerDescriptor(const DescriptorPtr& parent, int line,
                                                                           const DataFactoryPtr& factory,
                                                                           const ErrorReporterPtr& errorReporter,
                                                                           const IceXML::Attributes&,
                                                                           const string& name) :
    Descriptor(parent, line, factory, errorReporter), _name(name)
{
}

void
FreezeScript::ExecutableContainerDescriptor::addChild(const DescriptorPtr& child)
{
    _children.push_back(child);
}

void
FreezeScript::ExecutableContainerDescriptor::validate()
{
    for(vector<DescriptorPtr>::iterator p = _children.begin(); p != _children.end(); ++p)
    {
        (*p)->validate();
    }
}

void
FreezeScript::ExecutableContainerDescriptor::execute(const SymbolTablePtr& sym, ExecuteInfo* info)
{
    for(vector<DescriptorPtr>::iterator p = _children.begin(); p != _children.end(); ++p)
    {
        (*p)->execute(sym, info);
    }
}

//
// IfDescriptor
//
FreezeScript::IfDescriptor::IfDescriptor(const DescriptorPtr& parent, int line, const DataFactoryPtr& factory,
                                         const ErrorReporterPtr& errorReporter,
                                         const IceXML::Attributes& attributes) :
    ExecutableContainerDescriptor(parent, line, factory, errorReporter, attributes, "if"),
    Descriptor(parent, line, factory, errorReporter)
{
    DescriptorErrorContext ctx(_errorReporter, "if", _line);

    IceXML::Attributes::const_iterator p = attributes.find("test");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `test' is missing");
    }
    _testStr = p->second;

    _test = parse(_testStr);
}

void
FreezeScript::IfDescriptor::execute(const SymbolTablePtr& sym, ExecuteInfo* info)
{
    DescriptorErrorContext ctx(_errorReporter, "if", _line);

    try
    {
        DataPtr b = _test->evaluate(sym);
        BooleanDataPtr bd = BooleanDataPtr::dynamicCast(b);
        if(!bd)
        {
            _errorReporter->error("expression `" + _testStr + "' does not evaluate to a boolean");
        }
        if(bd->booleanValue())
        {
            ExecutableContainerDescriptor::execute(sym, info);
        }
    }
    catch(const EvaluateException& ex)
    {
        _errorReporter->error("evaluation of conditional expression `" + _testStr + "' failed:\n" + ex.reason());
    }
}

//
// IterateDescriptor
//
FreezeScript::IterateDescriptor::IterateDescriptor(const DescriptorPtr& parent, int line,
                                                   const DataFactoryPtr& factory,
                                                   const ErrorReporterPtr& errorReporter,
                                                   const IceXML::Attributes& attributes) :
    ExecutableContainerDescriptor(parent, line, factory, errorReporter, attributes, "iterate"),
    Descriptor(parent, line, factory, errorReporter)
{
    DescriptorErrorContext ctx(_errorReporter, "iterate", _line);

    IceXML::Attributes::const_iterator p;
    string target;

    p = attributes.find("target");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `target' is missing");
    }
    target = p->second;

    p = attributes.find("key");
    if(p != attributes.end())
    {
        _key = p->second;
    }

    p = attributes.find("value");
    if(p != attributes.end())
    {
        _value = p->second;
    }

    p = attributes.find("element");
    if(p != attributes.end())
    {
        _element = p->second;
    }

    p = attributes.find("index");
    if(p != attributes.end())
    {
        _index = p->second;
    }

    NodePtr node = parse(target);
    _target = EntityNodePtr::dynamicCast(node);
    if(!_target)
    {
        _errorReporter->error("`target' attribute is not an entity: `" + target + "'");
    }
}

void
FreezeScript::IterateDescriptor::execute(const SymbolTablePtr& sym, ExecuteInfo* info)
{
    DescriptorErrorContext ctx(_errorReporter, "iterate", _line);

    DataPtr data = sym->getValue(_target);

    DictionaryDataPtr dict = DictionaryDataPtr::dynamicCast(data);
    SequenceDataPtr seq = SequenceDataPtr::dynamicCast(data);
    if(!dict && !seq)
    {
        ostringstream ostr;
        ostr << _target;
        _errorReporter->error("target `" + ostr.str() + "' is not a dictionary or sequence");
    }

    if(dict)
    {
        if(!_element.empty())
        {
            _errorReporter->error("attribute `element' specified for dictionary target");
        }
        if(!_index.empty())
        {
            _errorReporter->error("attribute `index' specified for dictionary target");
        }

        string key = _key;
        if(key.empty())
        {
            key = "key";
        }

        string value = _value;
        if(value.empty())
        {
            value = "value";
        }

        DataMap& map = dict->getElements();
        for(DataMap::iterator p = map.begin(); p != map.end(); ++p)
        {
            SymbolTablePtr elemSym = sym->createChild();
            elemSym->add(key, p->first);
            elemSym->add(value, p->second);
            ExecutableContainerDescriptor::execute(elemSym, info);
        }
    }
    else
    {
        if(!_key.empty())
        {
            _errorReporter->error("attribute `key' specified for sequence target");
        }
        if(!_value.empty())
        {
            _errorReporter->error("attribute `value' specified for sequence target");
        }

        string element = _element;
        if(element.empty())
        {
            element = "elem";
        }

        string index = _index;
        if(index.empty())
        {
            index = "i";
        }

        DataList& l = seq->getElements();
        Ice::Long i = 0;
        for(DataList::iterator p = l.begin(); p != l.end(); ++p, ++i)
        {
            SymbolTablePtr elemSym = sym->createChild();
            elemSym->add(element, *p);
            elemSym->add(index, _factory->createInteger(i, true));
            ExecutableContainerDescriptor::execute(elemSym, info);
        }
    }
}

//
// DumpDescriptor
//
FreezeScript::DumpDescriptor::DumpDescriptor(const DescriptorPtr& parent, int line,
                                             const DataFactoryPtr& factory,
                                             const ErrorReporterPtr& errorReporter,
                                             const IceXML::Attributes& attributes,
                                             const Slice::UnitPtr& unit) :
    ExecutableContainerDescriptor(parent, line, factory, errorReporter, attributes, "dump"),
    Descriptor(parent, line, factory, errorReporter)
{
    DescriptorErrorContext ctx(_errorReporter, "dump", _line);

    IceXML::Attributes::const_iterator p;

    p = attributes.find("type");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `type' is missing");
    }
    _type = findType(unit, p->second);
}

Slice::TypePtr
FreezeScript::DumpDescriptor::type() const
{
    return _type;
}

string
FreezeScript::DumpDescriptor::typeName() const
{
    return typeToString(_type);
}

//
// RecordDescriptor
//
FreezeScript::RecordDescriptor::RecordDescriptor(const DescriptorPtr& parent, int line,
                                                 const DataFactoryPtr& factory,
                                                 const ErrorReporterPtr& errorReporter,
                                                 const IceXML::Attributes& attributes,
                                                 const Slice::UnitPtr& unit) :
    ExecutableContainerDescriptor(parent, line, factory, errorReporter, attributes, "record"),
    Descriptor(parent, line, factory, errorReporter), _unit(unit)
{
}

void
FreezeScript::RecordDescriptor::execute(const SymbolTablePtr& sym, ExecuteInfo* info)
{
    //
    // We need the Instance in order to use BasicStream.
    //
    IceInternal::InstancePtr instance = IceInternal::getInstance(info->communicator);

    //
    // Temporarily add an object factory.
    //
    info->communicator->addObjectFactory(new FreezeScript::ObjectFactory(_factory, _unit), "");

    //
    // Iterate over the database.
    //
    Dbc* dbc = 0;
    info->db->cursor(info->txn, &dbc, 0);
    try
    {
        Dbt dbKey, dbValue;
        while(dbc->get(&dbKey, &dbValue, DB_NEXT) == 0)
        {
            IceInternal::BasicStream inKey(instance.get());
            inKey.b.resize(dbKey.get_size());
            memcpy(&inKey.b[0], dbKey.get_data(), dbKey.get_size());
            inKey.i = inKey.b.begin();

            IceInternal::BasicStream inValue(instance.get());
            inValue.b.resize(dbValue.get_size());
            memcpy(&inValue.b[0], dbValue.get_data(), dbValue.get_size());
            inValue.i = inValue.b.begin();
            inValue.startReadEncaps();

            //
            // Create data representations of the key and value types.
            //
            DataPtr keyData = _factory->create(info->keyType, true);
            Destroyer<DataPtr> keyDataDestroyer(keyData);
            DataPtr valueData = _factory->create(info->valueType, true);
            Destroyer<DataPtr> valueDataDestroyer(valueData);

            //
            // Unmarshal the key and value.
            //
            keyData->unmarshal(inKey);
            valueData->unmarshal(inValue);
            if(info->valueType->usesClasses())
            {
                inValue.readPendingObjects();
            }

            //
            // Visit the key and value.
            //
            DumpVisitor visitor(_factory, _unit, _errorReporter, info);
            keyData->visit(visitor);
            valueData->visit(visitor);

            if(!_children.empty())
            {
                //
                // Execute the child descriptors.
                //
                SymbolTablePtr st = new SymbolTableI(_factory, _unit, _errorReporter, info, info->symbolTable);
                st->add("key", keyData);
                st->add("value", valueData);
                ExecutableContainerDescriptor::execute(st, info);
            }
        }
    }
    catch(...)
    {
        if(dbc)
        {
            dbc->close();
        }
        info->communicator->removeObjectFactory("");
        throw;
    }

    info->communicator->removeObjectFactory("");

    if(dbc)
    {
        dbc->close();
    }
}

//
// DatabaseDescriptor
//
FreezeScript::DatabaseDescriptor::DatabaseDescriptor(const DescriptorPtr& parent, int line,
                                                     const DataFactoryPtr& factory,
                                                     const ErrorReporterPtr& errorReporter,
                                                     const IceXML::Attributes& attributes,
                                                     const Slice::UnitPtr& unit) :
    ExecutableContainerDescriptor(parent, line, factory, errorReporter, attributes, "database"),
    Descriptor(parent, line, factory, errorReporter), _unit(unit)
{
    DescriptorErrorContext ctx(_errorReporter, "database", _line);

    IceXML::Attributes::const_iterator p = attributes.find("key");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `key' is missing");
    }
    _key = findType(unit, p->second);

    p = attributes.find("value");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `value' is missing");
    }
    _value = findType(unit, p->second);
}

void
FreezeScript::DatabaseDescriptor::addChild(const DescriptorPtr& child)
{
    DescriptorErrorContext ctx(_errorReporter, "database", _line);

    RecordDescriptorPtr rec = RecordDescriptorPtr::dynamicCast(child);
    if(rec)
    {
        if(_record)
        {
            _errorReporter->error("only one <record> element can be specified");
        }
        _record = rec;
    }

    ExecutableContainerDescriptor::addChild(child);
}

void
FreezeScript::DatabaseDescriptor::execute(const SymbolTablePtr&, ExecuteInfo* info)
{
    //
    // Store the key and value types.
    //
    info->keyType = _key;
    info->valueType = _value;

    ExecutableContainerDescriptor::execute(info->symbolTable, info);
}

//
// DumpDBDescriptor
//
FreezeScript::DumpDBDescriptor::DumpDBDescriptor(int line, const DataFactoryPtr& factory,
                                                 const ErrorReporterPtr& errorReporter,
                                                 const IceXML::Attributes& attributes, const Slice::UnitPtr& unit) :
    Descriptor(0, line, factory, errorReporter), _unit(unit), _info(new ExecuteInfo)
{
    _info->symbolTable = new SymbolTableI(factory, unit, errorReporter, _info);
}

FreezeScript::DumpDBDescriptor::~DumpDBDescriptor()
{
    delete _info;
}

void
FreezeScript::DumpDBDescriptor::addChild(const DescriptorPtr& child)
{
    DescriptorErrorContext ctx(_errorReporter, "dumpdb", _line);

    DatabaseDescriptorPtr db = DatabaseDescriptorPtr::dynamicCast(child);
    DumpDescriptorPtr dump = DumpDescriptorPtr::dynamicCast(child);

    if(db)
    {
        if(_database)
        {
            _errorReporter->error("only one <database> element can be specified");
        }
        else
        {
            _database = db;
            _children.push_back(db);
        }
    }
    else if(dump)
    {
        string name = dump->typeName();
        DumpMap::iterator p = _info->dumpMap.find(name);
        if(p != _info->dumpMap.end())
        {
            _errorReporter->error("<dump> descriptor for `" + name + "' specified more than once");
        }
        _info->dumpMap.insert(DumpMap::value_type(name, dump));

        _children.push_back(dump);
    }
    else
    {
        _errorReporter->error("invalid child element");
    }
}

void
FreezeScript::DumpDBDescriptor::validate()
{
    DescriptorErrorContext ctx(_errorReporter, "dumpdb", _line);

    if(!_database)
    {
        _errorReporter->error("no <database> element specified");
    }

    for(vector<DescriptorPtr>::iterator p = _children.begin(); p != _children.end(); ++p)
    {
        (*p)->validate();
    }
}

void
FreezeScript::DumpDBDescriptor::execute(const SymbolTablePtr&, ExecuteInfo*)
{
    assert(false);
}

void
FreezeScript::DumpDBDescriptor::dump(const Ice::CommunicatorPtr& communicator, Db* db, DbTxn* txn)
{
    _info->communicator = communicator;
    _info->db = db;
    _info->txn = txn;

    try
    {
        _database->execute(0, _info);
    }
    catch(...)
    {
        _info->communicator = 0;
        _info->db = 0;
        _info->txn = 0;
        throw;
    }

    _info->communicator = 0;
    _info->db = 0;
    _info->txn = 0;
}

//
// SymbolTableI
//
FreezeScript::SymbolTableI::SymbolTableI(const DataFactoryPtr& factory, const Slice::UnitPtr& unit,
                                         const ErrorReporterPtr& errorReporter, ExecuteInfo* info,
                                         const SymbolTablePtr& parent) :
    _factory(factory), _unit(unit), _errorReporter(errorReporter), _info(info), _parent(parent)
{
}

void
FreezeScript::SymbolTableI::add(const string& name, const DataPtr& data)
{
    DataPtr d = findValue(name);
    if(d)
    {
        _errorReporter->error("`" + name + "' is already defined");
    }

    _dataMap.insert(DataMap::value_type(name, data));
}

FreezeScript::DataPtr
FreezeScript::SymbolTableI::getValue(const EntityNodePtr& entity) const
{
    EntityVisitor visitor(const_cast<SymbolTableI*>(this));
    entity->visit(visitor);
    DataPtr result = visitor.getCurrent();
    if(!result)
    {
        ostringstream ostr;
        ostr << "invalid entity `" << entity << "'";
        _errorReporter->error(ostr.str());
    }

    return result;
}

FreezeScript::DataPtr
FreezeScript::SymbolTableI::getConstantValue(const string& name) const
{
    DataMap::const_iterator p = _constantCache.find(name);
    if(p != _constantCache.end())
    {
        return p->second;
    }

    if(_parent)
    {
        return _parent->getConstantValue(name);
    }

    Slice::ContainedList l = _unit->findContents(name);
    if(l.empty())
    {
        _errorReporter->error("unknown constant `" + name + "'");
    }

    Slice::EnumeratorPtr e = Slice::EnumeratorPtr::dynamicCast(l.front());
    Slice::ConstPtr c = Slice::ConstPtr::dynamicCast(l.front());
    if(!e && !c)
    {
        _errorReporter->error("`" + name + "' does not refer to a Slice constant or enumerator");
    }

    DataPtr result;

    if(c)
    {
        Slice::TypePtr type = c->type();
        string value = c->value();
        Slice::BuiltinPtr b = Slice::BuiltinPtr::dynamicCast(type);
        if(b)
        {
            switch(b->kind())
            {
            case Slice::Builtin::KindByte:
            case Slice::Builtin::KindShort:
            case Slice::Builtin::KindInt:
            case Slice::Builtin::KindLong:
            {
                string::size_type end;
                Ice::Long n;
                if(!IceUtil::stringToInt64(value, n, end))
                {
                    assert(false);
                }
                result = _factory->createInteger(n, true);
                break;
            }

            case Slice::Builtin::KindBool:
            {
                result = _factory->createBoolean(value == "true", true);
                break;
            }

            case Slice::Builtin::KindFloat:
            case Slice::Builtin::KindDouble:
            {
                double v = strtod(value.c_str(), 0);
                result = _factory->createDouble(v, true);
                break;
            }

            case Slice::Builtin::KindString:
            {
                result = _factory->createString(value, true);
                break;
            }

            case Slice::Builtin::KindObject:
            case Slice::Builtin::KindObjectProxy:
            case Slice::Builtin::KindLocalObject:
                assert(false);
            }
        }
        else
        {
            Slice::EnumPtr en = Slice::EnumPtr::dynamicCast(type);
            assert(en);
            Slice::EnumeratorList el = en->getEnumerators();
            for(Slice::EnumeratorList::iterator q = el.begin(); q != el.end(); ++q)
            {
                if((*q)->name() == value)
                {
                    e = *q;
                    break;
                }
            }
            assert(e);
        }
    }

    if(!result)
    {
        assert(e);
        result = _factory->create(e->type(), true);
        EnumDataPtr ed = EnumDataPtr::dynamicCast(result);
        assert(ed);
        ed->setValueAsString(e->name());
    }

    //
    // Cache the value.
    //
    const_cast<DataMap&>(_constantCache).insert(DataMap::value_type(name, result));

    return result;
}

FreezeScript::SymbolTablePtr
FreezeScript::SymbolTableI::createChild()
{
    return new SymbolTableI(_factory, _unit, _errorReporter, _info, this);
}

Slice::TypePtr
FreezeScript::SymbolTableI::lookupType(const string& name)
{
    Slice::TypeList l = _unit->lookupType(name, false);
    Slice::TypePtr result;
    if(!l.empty())
    {
        result = l.front();
    }
    return result;
}

FreezeScript::DataPtr
FreezeScript::SymbolTableI::invokeFunction(const string& name, const DataPtr& target, const DataList& args)
{
    if(target)
    {
        DictionaryDataPtr targetDict = DictionaryDataPtr::dynamicCast(target);
        if(targetDict && name == "containsKey")
        {
            if(args.size() != 1)
            {
                _errorReporter->error("containsKey() requires one argument");
            }
            Slice::DictionaryPtr dictType = Slice::DictionaryPtr::dynamicCast(targetDict->getType());
            assert(dictType);
            DataPtr key = _factory->create(dictType->keyType(), false);
            AssignVisitor visitor(args[0], _factory, _errorReporter, false);
            key->visit(visitor);
            DataPtr value = targetDict->getElement(key);
            return _factory->createBoolean(value ? true : false, false);
        }

        DataPtr result;
        if(invokeMemberFunction(name, target, args, result, _factory, _errorReporter))
        {
            return result;
        }

        _errorReporter->error("unknown function `" + name + "' invoked on type " + typeToString(target->getType()));
    }
    else
    {
        //
        // Global function.
        //
        DataPtr result;
        if(invokeGlobalFunction(name, args, result, _factory, _errorReporter))
        {
            return result;
        }
        else
        {
            _errorReporter->error("unknown global function `" + name + "'");
        }
    }

    return 0;
}

FreezeScript::DataPtr
FreezeScript::SymbolTableI::findValue(const string& name) const
{
    DataMap::const_iterator p = _dataMap.find(name);
    if(p != _dataMap.end())
    {
        return p->second;
    }

    if(_parent)
    {
        SymbolTableIPtr parentI = SymbolTableIPtr::dynamicCast(_parent);
        assert(parentI);
        return parentI->findValue(name);
    }

    return 0;
}

FreezeScript::SymbolTableI::EntityVisitor::EntityVisitor(SymbolTableI* table) :
    _table(table), _error(false)
{
}

void
FreezeScript::SymbolTableI::EntityVisitor::visitIdentifier(const string& name)
{
    if(!_error)
    {
        if(!_current)
        {
            _current = _table->findValue(name);
        }
        else
        {
            _current = _current->getMember(name);
        }

        if(!_current)
        {
            _error = true;
        }
    }
}

void
FreezeScript::SymbolTableI::EntityVisitor::visitElement(const NodePtr& value)
{
    if(!_error)
    {
        assert(_current);

        DataPtr val = value->evaluate(_table);
        _current = _current->getElement(val);

        if(!_current)
        {
            _error = true;
        }
    }
}

FreezeScript::DataPtr
FreezeScript::SymbolTableI::EntityVisitor::getCurrent() const
{
    return _current;
}

//
// DumpVisitor
//
FreezeScript::DumpVisitor::DumpVisitor(const DataFactoryPtr& factory, const Slice::UnitPtr& unit,
                                       const ErrorReporterPtr& errorReporter, ExecuteInfo* info) :
    _factory(factory), _unit(unit), _errorReporter(errorReporter), _info(info)
{
}

void
FreezeScript::DumpVisitor::visitBoolean(const BooleanDataPtr& data)
{
    dump(data);
}

void
FreezeScript::DumpVisitor::visitInteger(const IntegerDataPtr& data)
{
    dump(data);
}

void
FreezeScript::DumpVisitor::visitDouble(const DoubleDataPtr& data)
{
    dump(data);
}

void
FreezeScript::DumpVisitor::visitString(const StringDataPtr& data)
{
    dump(data);
}

void
FreezeScript::DumpVisitor::visitProxy(const ProxyDataPtr& data)
{
    dump(data);
}

void
FreezeScript::DumpVisitor::visitStruct(const StructDataPtr& data)
{
    dump(data);
    DataMemberMap& members = data->getMembers();
    for(DataMemberMap::iterator p = members.begin(); p != members.end(); ++p)
    {
        p->second->visit(*this);
    }
}

void
FreezeScript::DumpVisitor::visitSequence(const SequenceDataPtr& data)
{
    dump(data);
}

void
FreezeScript::DumpVisitor::visitEnum(const EnumDataPtr& data)
{
    dump(data);
}

void
FreezeScript::DumpVisitor::visitDictionary(const DictionaryDataPtr& data)
{
    dump(data);
}

void
FreezeScript::DumpVisitor::visitObject(const ObjectRefPtr& data)
{
    dump(data);
    ObjectDataPtr value = data->getValue();
    if(value)
    {
        DataMemberMap& members = value->getMembers();
        for(DataMemberMap::iterator p = members.begin(); p != members.end(); ++p)
        {
            p->second->visit(*this);
        }
    }
}

void
FreezeScript::DumpVisitor::dump(const DataPtr& data)
{
    ObjectRefPtr obj = ObjectRefPtr::dynamicCast(data);
    if(obj && obj->getValue())
    {
        //
        // When the value is an object, attempt to find a <dump> descriptor for the
        // object's actual type. If no descriptor is found, attempt to find a
        // descriptor for the object's base types (including Ice::Object).
        //
        ObjectDataPtr data = obj->getValue();
        Slice::TypePtr cls = data->getType(); // Actual type
        while(cls)
        {
            string type = typeToString(cls);
            cls = 0;
            DumpMap::const_iterator p = _info->dumpMap.find(type);
            if(p != _info->dumpMap.end())
            {
                SymbolTablePtr sym = new SymbolTableI(_factory, _unit, _errorReporter, _info, _info->symbolTable);
                sym->add("value", data);
                p->second->execute(sym, _info);
            }
            else
            {
                Slice::ClassDeclPtr decl = Slice::ClassDeclPtr::dynamicCast(cls);
                if(decl)
                {
                    Slice::ClassDefPtr def = decl->definition();
                    assert(def);
                    Slice::ClassList bases = def->bases();
                    if(!bases.empty() && !bases.front()->isInterface())
                    {
                        cls = bases.front()->declaration();
                    }
                    else
                    {
                        cls = _unit->builtin(Slice::Builtin::KindObject);
                    }
                }
            }
        }
    }
    else
    {
        string typeName = typeToString(data->getType());
        DumpMap::iterator p = _info->dumpMap.find(typeName);
        if(p != _info->dumpMap.end())
        {
            SymbolTablePtr st = new SymbolTableI(_factory, _unit, _errorReporter, _info, _info->symbolTable);
            st->add("value", data);
            p->second->execute(st, _info);
        }

    }
}
