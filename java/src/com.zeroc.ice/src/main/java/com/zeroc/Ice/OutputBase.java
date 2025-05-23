// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.LinkedList;

final class OutputBase {
    public OutputBase() {
        _out = null;
        _pos = 0;
        _indent = 0;
        _indentSize = 4;
        _useTab = true;
        _separator = true;
    }

    public OutputBase(PrintWriter out) {
        _out = out;
        _pos = 0;
        _indent = 0;
        _indentSize = 4;
        _useTab = true;
        _separator = true;
    }

    public OutputBase(String s) {
        _out = null;
        _pos = 0;
        _indent = 0;
        _indentSize = 4;
        _useTab = true;
        _separator = true;

        open(s);
    }

    public void setIndent(int indentSize) {
        _indentSize = indentSize;
    }

    public void setUseTab(boolean useTab) {
        _useTab = useTab;
    }

    public void open(String s) {
        try {
            FileWriter fw = new FileWriter(s);
            BufferedWriter bw = new BufferedWriter(fw);
            _out = new PrintWriter(bw);
        } catch (IOException ex) {}
    }

    public void print(String s) {
        final char[] arr = s.toCharArray();
        for (int i = 0; i < arr.length; i++) {
            if (arr[i] == '\n') {
                _pos = 0;
            } else {
                ++_pos;
            }
        }

        _out.print(s);
    }

    public void inc() {
        _indent += _indentSize;
    }

    public void dec() {
        assert (_indent >= _indentSize);
        _indent -= _indentSize;
    }

    public void useCurrentPosAsIndent() {
        _indentSave.addFirst(_indent);
        _indent = _pos;
    }

    public void zeroIndent() {
        _indentSave.addFirst(_indent);
        _indent = 0;
    }

    public void restoreIndent() {
        assert (!_indentSave.isEmpty());
        _indent = _indentSave.removeFirst().intValue();
    }

    public void nl() {
        _out.println();
        _pos = 0;
        _separator = true;

        int indent = _indent;

        if (_useTab) {
            while (indent >= 8) {
                indent -= 8;
                _out.print('\t');
                _pos += 8;
            }
        } else {
            while (indent >= _indentSize) {
                indent -= _indentSize;
                _out.print("    ");
                _pos += _indentSize;
            }
        }

        while (indent > 0) {
            --indent;
            _out.print(' ');
            ++_pos;
        }

        _out.flush();
    }

    public void sp() {
        if (_separator) {
            _out.println();
        }
    }

    public boolean valid() {
        return _out != null;
    }

    protected PrintWriter _out;
    protected int _pos;
    protected int _indent;
    protected int _indentSize;
    protected LinkedList<Integer> _indentSave = new LinkedList<>();
    protected boolean _useTab;
    protected boolean _separator;
}
