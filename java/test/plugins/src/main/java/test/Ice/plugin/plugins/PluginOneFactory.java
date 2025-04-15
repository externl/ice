// Copyright (c) ZeroC, Inc.

package test.Ice.plugin.plugins;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Plugin;
import com.zeroc.Ice.PluginFactory;

public class PluginOneFactory implements PluginFactory {
    @Override
    public Plugin create(
            Communicator communicator, String name, String[] args) {
        return new PluginOne(communicator);
    }

    static class PluginOne extends BasePlugin {
        public PluginOne(Communicator communicator) {
            super(communicator);
        }

        @Override
        public void initialize() {
            _other = (BasePlugin) _communicator.getPluginManager().getPlugin("PluginTwo");
            test(!_other.isInitialized());
            _initialized = true;
        }

        @Override
        public void destroy() {
            _destroyed = true;
            test(_other.isDestroyed());
        }
    }
}
