#!/usr/bin/env python3
#
# Copyright (c) ZeroC, Inc.
#

import os
import sys
import shutil
import signal

from enum import StrEnum, auto
from typing import override

from xml.sax import make_parser
from xml.sax.handler import feature_namespaces
from xml.sax.handler import ContentHandler
from xml.dom.minidom import parse

progname = os.path.basename(sys.argv[0])
propertyClasses = {}


class Language(StrEnum):
    ALL = auto()
    CPP = auto()
    JAVA = auto()
    CSHARP = auto()
    JS = auto()


commonPreamble = """\
// Copyright (c) ZeroC, Inc.

// Generated by makeprops.py from PropertyNames.xml

// IMPORTANT: Do not edit this file -- any edits made here will be lost!
"""

cppHeaderPreamble = (
    commonPreamble
    + """
#ifndef ICE_INTERNAL_PROPERTY_NAMES_H
#define ICE_INTERNAL_PROPERTY_NAMES_H

#include "Ice/Config.h"

#include <string>

namespace IceInternal
{
    struct PropertyArray;
    struct Property
    {
        const char* pattern;
        const char* defaultValue;
        bool usesRegex;
        bool deprecated;
        const PropertyArray* propertyClass;
        bool prefixOnly;
    };

    struct PropertyArray
    {
        const char* name;
        const Property* properties;
        const int length;
    };

    class ICE_API PropertyNames
    {
    public:

"""
)

cppHeaderPostamble = """
        static const PropertyArray validProps[];
        static const char* clPropNames[];
    };
}

#endif
"""

cppSrcPreamble = (
    commonPreamble
    + """
#include "PropertyNames.h"

using namespace IceInternal;

"""
)

javaPreamble = (
    commonPreamble
    + """
package com.zeroc.Ice;

final class PropertyNames
{
"""
)

csPreamble = (
    commonPreamble
    + """
namespace Ice.Internal;

public sealed class PropertyNames
{
"""
)

jsPreamble = (
    commonPreamble
    + """
import { Property } from "./Property.js";
export const PropertyNames = {};
"""
)


#
# Currently the processing of PropertyNames.xml is going to take place
# in two parts. One is using DOM to extract the property 'classes' such
# as 'proxy', 'objectadapter', etc. The other part uses SAX to create
# the language mapping source code.
#


# TODO: delete once all languages support class props
class PropertyClass:
    def __init__(self, prefixOnly, childProperties):
        self.prefixOnly = prefixOnly
        self.childProperties = childProperties

    def getChildren(self):
        return self.childProperties

    def isPrefixOnly(self):
        return self.prefixOnly

    def __repr__(self):
        return repr((repr(self.prefixOnly), repr(self.childProperties)))


def initPropertyClasses(filename):
    doc = parse(filename)
    propertyClassNodes = doc.getElementsByTagName("class")
    global propertyClasses
    propertyClasses = {}
    for n in propertyClassNodes:
        className = n.attributes["name"].nodeValue
        classType = n.attributes["prefix-only"].nodeValue
        properties = []
        for a in n.childNodes:
            if a.localName == "property" and a.hasAttributes():
                """Convert minidom maps to hash tables """
                attributeMap = {}
                for i in range(0, a.attributes.length):
                    attributeMap[a.attributes.item(i).name] = a.attributes.item(i).value
                properties.append(attributeMap)

        propertyClasses[className] = PropertyClass(
            classType.lower() == "true", properties
        )


#
# SAX part.
#


class PropertyHandler(ContentHandler):
    def __init__(self, language):
        # The language we are generating properties for
        self.language = language
        # The section section we are currently parsing
        self.parentNodeName = None
        # Dictionary of node names to properties
        self.nodeProperties = dict()
        # The current section type (class or section)
        self.parentNodeType = None
        # The list of section names
        self.sectionNames = []

    def cleanup(self):
        # Needs to be overridden in derived class
        pass

    def openFiles(self):
        # Needs to be overridden in derived class
        pass

    def closeFiles(self):
        # Needs to be overridden in derived class
        pass

    def createProperty(
        self,
        propertyName,
        usesRegex,
        defaultValue,
        deprecated,
        propertyClass,
        prefixOnly,
    ):
        # Needs to be overridden in derived class
        pass

    def writeProperty(self, property):
        # Needs to be overridden in derived class
        pass

    def openSection(self, sectionName):
        # Needs to be overridden in derived class
        pass

    def writeProperties(self):
        self.openFiles()

        for section in self.nodeProperties.keys():
            self.openSection(section)
            for prop in self.nodeProperties[section]:
                self.writeProperty(prop)
            self.closeSection(section)

        self.closeFiles()

    def moveFiles(self, location):
        # Needs to be overridden in derived class
        pass

    # The list of sections that have properties generated for them
    def generatedSections(self):
        return [
            name for name in self.sectionNames if name in self.nodeProperties.keys()
        ]

    def parseProperty(self, propertyPrefix, attrs):
        name = attrs.get("name")
        usesRegex = "[any]" in name
        deprecated = attrs.get("deprecated", "false").lower() == "true"
        defaultValue = attrs.get("default", None)
        propertyClass = attrs.get("class", None)
        prefixOnly = attrs.get("prefix-only", "false").lower() == "true"

        if self.language == Language.CPP or self.language == Language.CSHARP:
            propertyName = name
        else:
            propertyName = (
                f"{propertyPrefix}.{name}" if self.parentNodeType == "section" else name
            )

        if not propertyClass and prefixOnly:
            print(
                sys.stderr,
                f"Property '{propertyName}' cannot have prefix-only without a class",
            )

        if propertyClass and defaultValue:
            print(
                sys.stderr,
                f"Property '{propertyName}' cannot have both a class and a default value",
            )

        if propertyClass and usesRegex:
            print(
                sys.stderr,
                f"Property '{propertyName}' cannot have both a class and use a regex",
            )

        return self.createProperty(
            propertyName,
            usesRegex,
            defaultValue or "",
            deprecated,
            propertyClass,
            prefixOnly,
        )

    def validateKnownAttributes(self, validAttrs, attrs):
        if "name" not in attrs:
            print(sys.stderr, "missing name attribute")

        for a in attrs.keys():
            if a not in validAttrs:
                print(sys.stderr, "invalid attribute '%s'" % a)

    def validateLanguages(self, attrs):
        languageAttr = attrs.get("languages", None)
        # If no language attribute is specified issue a warning and skip code generation for this element
        if languageAttr is None:
            print(
                sys.stderr,
                "missing languages attribute in property element %s"
                % attrs.get("name"),
            )
            return False

        # languages="cpp,java" -> ["cpp", "java"]
        languages = [lang.strip() for lang in languageAttr.split(",")]
        for lang in languages:
            if lang not in [lang.value for lang in Language]:
                print(
                    sys.stderr,
                    "invalid language '%s' in property element %s"
                    % (lang, attrs.get("name")),
                )
                return False

        # All should be by itself. Issue a warning but continue generation for this element.
        if Language.ALL in languages and len(languages) > 1:
            print(
                sys.stderr,
                "Invalid languages attribute in property element: %s. 'all' must be specified alone."
                % attrs.get("name"),
            )

        # True if the current language is in the list of languages or if the list contains 'all'
        return Language.ALL in languages or self.language in languages

    def startElement(self, name, attrs):
        match name:
            case "properties":
                pass
            case "class":
                self.validateKnownAttributes(["name", "prefix-only"], attrs)
                self.parentNodeType = "class"
                self.parentNodeName = f"{attrs.get("name")}"
            case "section":
                self.validateKnownAttributes(["name"], attrs)
                self.parentNodeType = "section"
                self.parentNodeName = attrs.get("name")
                self.sectionNames.append(self.parentNodeName)

            case "property":
                self.validateKnownAttributes(
                    ["name", "class", "default", "deprecated", "languages"], attrs
                )

                if self.validateLanguages(attrs) is False:
                    return

                # TODO: temporary until all languages support class props

                if self.language != Language.CPP and self.language != Language.CSHARP:
                    if self.parentNodeType == "class":
                        return

                    propertyName = attrs.get("name", None)
                    if "class" in attrs:
                        c = propertyClasses[attrs["class"]]
                        assert c is not None
                        for p in c.getChildren():
                            t = dict(p)
                            t["name"] = "%s.%s" % (propertyName, p["name"])
                            self.startElement(name, t)
                        if c.isPrefixOnly():
                            return

                property = self.parseProperty(self.parentNodeName, attrs)
                self.nodeProperties.setdefault(self.parentNodeName, []).append(property)
            case _:
                print(sys.stderr, "unknown element '%s'" % name)

    def endElement(self, name):
        print("ending element", self.parentNodeName)
        if name == "section" or name == "class":
            self.parentNodeName = None
            self.parentNodeType = None


class CppPropertyHandler(PropertyHandler):
    def __init__(self):
        super().__init__(Language.CPP)
        self.hFile = None
        self.cppFile = None

    def cleanup(self):
        if self.hFile is not None:
            self.hFile.close()
            if os.path.exists("PropertyNames.h"):
                os.remove("PropertyNames.h")
        if self.cppFile is not None:
            self.cppFile.close()
            if os.path.exists("PropertyNames.cpp"):
                os.remove("PropertyNames.cpp")

    def openFiles(self):
        self.hFile = open("PropertyNames.h", "w")
        self.cppFile = open("PropertyNames.cpp", "w")
        self.hFile.write(cppHeaderPreamble)
        self.cppFile.write(cppSrcPreamble)

    def closeFiles(self):
        self.hFile.write(cppHeaderPostamble)
        self.cppFile.write("const PropertyArray PropertyNames::validProps[] =\n")

        self.cppFile.write("{\n")
        for s in self.generatedSections():
            self.cppFile.write("    %sProps,\n" % s)
        self.cppFile.write("    PropertyArray{nullptr, nullptr ,0}\n")
        self.cppFile.write("};\n\n")

        self.cppFile.write("const char* PropertyNames::clPropNames[] =\n")
        self.cppFile.write("{\n")
        for s in self.sectionNames:
            self.cppFile.write('    "%s",\n' % s)
        self.cppFile.write("    nullptr\n")
        self.cppFile.write("};\n")
        self.hFile.close()
        self.cppFile.close()

    def fix(self, propertyName):
        return propertyName.replace("[any]", "*")

    @override
    def writeProperty(self, property):
        self.cppFile.write("    %s,\n" % property)

    @override
    def createProperty(
        self,
        propertyName,
        usesRegex,
        defaultValue,
        deprecated,
        propertyClass,
        prefixOnly,
    ):
        propertyLine = 'Property{{"{pattern}", {defaultValue}, {usesRegex}, {deprecated}, {propertyClass}, {prefixOnly}}}'.format(
            pattern=self.fix(propertyName) if usesRegex else propertyName,
            defaultValue=f'"{defaultValue}"',
            usesRegex="true" if usesRegex else "false",
            deprecated="true" if deprecated else "false",
            propertyClass=f"&PropertyNames::{propertyClass}Props"
            if propertyClass
            else "nullptr",
            prefixOnly="true" if prefixOnly else "false",
        )

        return propertyLine

    def openSection(self, sectionName):
        self.hFile.write("        static const PropertyArray %sProps;\n" % sectionName)
        self.cppFile.write("const Property %sPropsData[] =\n" % sectionName)
        self.cppFile.write("{\n")

    def closeSection(self, sectionName):
        self.cppFile.write("};\n")
        self.cppFile.write(
            f"""
const PropertyArray
    PropertyNames::{sectionName}Props{{"{sectionName}", {sectionName}PropsData,
        sizeof({sectionName}PropsData)/sizeof({sectionName}PropsData[0])}};

"""
        )

    def moveFiles(self, location):
        dest = os.path.join(location, "cpp", "src", "Ice")
        if os.path.exists(os.path.join(dest, "PropertyNames.h")):
            os.remove(os.path.join(dest, "PropertyNames.h"))
        if os.path.exists(os.path.join(dest, "PropertyNames.cpp")):
            os.remove(os.path.join(dest, "PropertyNames.cpp"))
        shutil.move("PropertyNames.h", dest)
        shutil.move("PropertyNames.cpp", dest)


class JavaPropertyHandler(PropertyHandler):
    def __init__(self):
        super().__init__(Language.JAVA)
        self.srcFile = None

    def cleanup(self):
        if self.srcFile is not None:
            self.srcFile.close()
            if os.path.exists("PropertyNames.java"):
                os.remove("PropertyNames.java")

    def openFiles(self):
        self.srcFile = open("PropertyNames.java", "w")
        self.srcFile.write(javaPreamble)

    def closeFiles(self):
        self.srcFile.write("    public static final Property[] validProps[] =\n")

        self.srcFile.write("    {\n")
        for s in self.generatedSections():
            self.srcFile.write("        %sProps,\n" % s)
        self.srcFile.write("        null\n")
        self.srcFile.write("    };\n")

        self.srcFile.write("\n    public static final String clPropNames[] =\n")
        self.srcFile.write("    {\n")
        for s in self.sectionNames:
            self.srcFile.write('        "%s",\n' % s)
        self.srcFile.write("        null\n")
        self.srcFile.write("    };\n")
        self.srcFile.write("}\n")
        self.srcFile.close()

    def fix(self, propertyName):
        #
        # The Java property strings are actually regexp's that will be passed to Java's regexp facility.
        #
        return propertyName.replace(".", r"\\.").replace("[any]", r"[^\\s]+")

    @override
    def writeProperty(self, property):
        self.srcFile.write("    %s,\n" % property)

    def createProperty(
        self,
        propertyName,
        usesRegex,
        defaultValue,
        deprecated,
        propertyClass,
        prefixOnly,
    ):
        line = 'new Property("{pattern}", {usesRegex}, {defaultValue}, {deprecated})'.format(
            pattern=self.fix(propertyName) if usesRegex else propertyName,
            usesRegex="true" if usesRegex else "false",
            defaultValue=f'"{defaultValue}"',
            deprecated="true" if deprecated else "false",
        )

        return line

    def openSection(self, sectionName):
        self.srcFile.write(
            "    public static final Property %sProps[] =\n" % sectionName
        )
        self.srcFile.write("    {\n")

    def closeSection(self, sectionName):
        self.srcFile.write("        null\n")
        self.srcFile.write("    };\n\n")

    def moveFiles(self, location):
        dest = os.path.join(
            location,
            "java",
            "src",
            "Ice",
            "src",
            "main",
            "java",
            "com",
            "zeroc",
            "Ice",
        )
        if os.path.exists(os.path.join(dest, "PropertyNames.java")):
            os.remove(os.path.join(dest, "PropertyNames.java"))
        shutil.move("PropertyNames.java", dest)


class CSPropertyHandler(PropertyHandler):
    def __init__(self):
        super().__init__(Language.CSHARP)
        self.srcFile = None

    def cleanup(self):
        if self.srcFile is not None:
            self.srcFile.close()
            if os.path.exists("PropertyNames.cs"):
                os.remove("PropertyNames.cs")

    def openFiles(self):
        self.srcFile = open("PropertyNames.cs", "w")
        self.srcFile.write(csPreamble)

    def closeFiles(self):
        self.srcFile.write("    internal static PropertyArray[] validProps =\n")

        self.srcFile.write("    [\n")
        for s in self.generatedSections():
            self.srcFile.write("        %sProps,\n" % s)
        self.srcFile.write("    ];\n\n")

        self.srcFile.write("    internal static string[] clPropNames =\n")
        self.srcFile.write("    [\n")
        for s in self.sectionNames:
            self.srcFile.write('        "%s",\n' % s)
        self.srcFile.write("    ];\n")
        self.srcFile.write("}\n")
        self.srcFile.close()

    def fix(self, propertyName):
        return propertyName.replace(".", r"\.").replace("[any]", r"[^\s]+")

    @override
    def writeProperty(self, property):
        self.srcFile.write("            %s,\n" % property)

    @override
    def createProperty(
        self,
        propertyName,
        usesRegex,
        defaultValue,
        deprecated,
        propertyClass,
        prefixOnly,
    ):
        line = 'new(pattern: @"{pattern}", usesRegex: {usesRegex}, defaultValue: {defaultValue}, deprecated: {deprecated}, propertyClass: {propertyClass}, prefixOnly: {prefixOnly})'.format(
            pattern=f"^{self.fix(propertyName)}$" if usesRegex else propertyName,
            usesRegex="true" if usesRegex else "false",
            defaultValue=f'"{defaultValue}"',
            deprecated="true" if deprecated else "false",
            propertyClass=f"{propertyClass}Props" if propertyClass else "null",
            prefixOnly="true" if prefixOnly else "false",
        )
        return line

    def openSection(self, sectionName):
        self.srcFile.write(
            f'    internal static PropertyArray {sectionName}Props = new(\n        "{sectionName}",\n'
        )
        self.srcFile.write("        [\n")

    def closeSection(self, sectionName):
        self.srcFile.write("        ]);\n")
        self.srcFile.write("\n")

    def moveFiles(self, location):
        dest = os.path.join(location, "csharp", "src", "Ice", "Internal")
        if os.path.exists(os.path.join(dest, "PropertyNames.cs")):
            os.remove(os.path.join(dest, "PropertyNames.cs"))
        shutil.move("PropertyNames.cs", dest)


class JSPropertyHandler(PropertyHandler):
    def __init__(self):
        super().__init__(Language.JS)
        self.srcFile = None
        self.validSections = ["Ice"]

    def cleanup(self):
        if self.srcFile is not None:
            self.srcFile.close()
            if os.path.exists("PropertyNames.js"):
                os.remove("PropertyNames.js")

    def openFiles(self):
        self.srcFile = open("PropertyNames.js", "w")
        self.srcFile.write(jsPreamble)

    def closeFiles(self):
        self.srcFile.write("PropertyNames.validProps = new Map();\n")
        for s in self.generatedSections():
            self.srcFile.write(f'PropertyNames.validProps.set("{s}", {s}Props);\n')

        self.srcFile.close()

    def fix(self, propertyName):
        return propertyName.replace(".", "\\.").replace("[any]", ".")

    @override
    def writeProperty(self, property):
        self.srcFile.write("    %s,\n" % property)

    def createProperty(
        self,
        propertyName,
        usesRegex,
        defaultValue,
        deprecated,
        propertyClass,
        prefixOnly,
    ):
        line = (
            "new Property({pattern}, {usesRegex}, {defaultValue}, {deprecated})".format(
                pattern=f"/^{self.fix(propertyName)}/"
                if usesRegex
                else f'"{propertyName}"',
                usesRegex="true" if usesRegex else "false",
                defaultValue=f'"{defaultValue}"',
                deprecated="true" if deprecated else "false",
            )
        )
        return line

    def openSection(self, sectionName):
        self.srcFile.write(f"const {sectionName}Props =\n")
        self.srcFile.write("[\n")

    def closeSection(self, sectionName):
        self.srcFile.write("];\n")
        self.srcFile.write("\n")

    def moveFiles(self, location):
        dest = os.path.join(location, "js", "src", "Ice")
        if os.path.exists(os.path.join(dest, "PropertyNames.js")):
            os.remove(os.path.join(dest, "PropertyNames.js"))
        shutil.move("PropertyNames.js", dest)


class MultiHandler(ContentHandler):
    def __init__(self, handlers):
        self.handlers = handlers
        super().__init__()

    @override
    def startElement(self, name, attrs):
        for f in self.handlers:
            f.startElement(name, attrs)

    def cleanup(self):
        for f in self.handlers:
            f.cleanup()

    def moveFiles(self, location):
        for f in self.handlers:
            f.moveFiles(location)

    def writeProperties(self):
        for f in self.handlers:
            f.writeProperties()


def main():
    if len(sys.argv) != 1 and len(sys.argv) != 3:
        print(sys.stderr, "makeprops.py does not take any arguments")
        sys.exit(1)

    # Find the top-level directory of the Ice repository
    topLevel = os.popen("git rev-parse --show-toplevel").read().strip()
    propsFile = os.path.join(topLevel, "config", "PropertyNames.xml")

    if not os.path.exists(propsFile):
        print(
            sys.stderr,
            "Cannot find top-level directory. Please run this script from the Ice repository.",
        )
        sys.exit(1)

    contentHandler = MultiHandler(
        [
            CppPropertyHandler(),
            JavaPropertyHandler(),
            CSPropertyHandler(),
            JSPropertyHandler(),
        ]
    )

    # Ignore all signals. This script should not take long to run
    signal.signal(signal.SIGINT, signal.SIG_IGN)
    signal.signal(signal.SIGTERM, signal.SIG_IGN)

    # TODO: delete once all languages support class props
    initPropertyClasses(propsFile)

    parser = make_parser()
    parser.setFeature(feature_namespaces, 0)
    parser.setContentHandler(contentHandler)
    propsFileBuffer = open(propsFile)
    try:
        parser.parse(propsFileBuffer)
        contentHandler.writeProperties()
        contentHandler.moveFiles(topLevel)
    except Exception as ex:
        print(sys.stderr, str(ex))
        contentHandler.cleanup()
        sys.exit(1)


if __name__ == "__main__":
    main()
