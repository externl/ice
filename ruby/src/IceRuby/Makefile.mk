# Copyright (c) ZeroC, Inc.

$(project)_libraries := IceRuby

IceRuby_target          := ruby-module
IceRuby_targetname      := IceRuby
IceRuby_targetdir       := $(lang_srcdir)/ruby
IceRuby_installdir      := $(install_rubylibdir)
IceRuby_cppflags        := $(ice_cpp_cppflags) -I$(top_srcdir)/cpp/src $(ruby_cppflags)
IceRuby_system_libs     := $(ruby_ldflags)
IceRuby_dependencies     := IceDiscovery IceLocatorDiscovery Ice
IceRuby_libs            := mcpp
IceRuby_extra_sources   := $(wildcard $(top_srcdir)/cpp/src/Slice/*.cpp) \
                           $(top_srcdir)/cpp/src/slice2rb/RubyUtil.cpp   \
                           (top_srcdir)/cpp/src/slice2rb/Ruby.cpp        \

#
# On the default platform, always write the module in the ruby directory.
#
$(foreach p,$(supported-platforms),$(eval $$p_targetdir[IceRuby] := /$$p))
$(firstword $(supported-platforms))_targetdir[IceRuby] :=
$(firstword $(supported-platforms))_installdir[IceRuby] :=

projects += $(project)
srcs:: $(project)
