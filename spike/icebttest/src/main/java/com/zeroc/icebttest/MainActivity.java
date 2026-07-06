// Spike: exercise the REAL IceBT Java plug-in between two Android emulators over Netsim/Rootcanal.
//   server: Ice ObjectAdapter on a `bt` endpoint + a servant, activate.
//   client: pre-bond to the server (IceBT uses secure RFCOMM), then ice_ping() a `bt` proxy.
// Launch via `adb am start`, read the result from logcat tag ICEBTTEST.
package com.zeroc.icebttest;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.ObjectPrx;

import java.util.Collections;

public class MainActivity extends Activity {
    static final String TAG = "ICEBTTEST";

    // Auto-confirm pairing (needs BLUETOOTH_PRIVILEGED, granted via the priv-app allowlist).
    private final BroadcastReceiver pairingReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context ctx, Intent intent) {
            BluetoothDevice dev = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
            Log.i(TAG, "PAIRING_REQUEST from " + (dev == null ? "?" : dev.getAddress()));
            try {
                dev.setPairingConfirmation(true);
                Log.i(TAG, "pairing confirmed");
            } catch (Throwable t) {
                Log.w(TAG, "setPairingConfirmation failed: " + t);
            }
        }
    };

    @Override
    protected void onCreate(Bundle b) {
        super.onCreate(b);
        registerReceiver(pairingReceiver, new IntentFilter(BluetoothDevice.ACTION_PAIRING_REQUEST));
        Intent it = getIntent();
        final String mode = it.getStringExtra("mode");
        final String peer = it.getStringExtra("peer");
        final String uuid = it.getStringExtra("uuid");
        Log.i(TAG, "start mode=" + mode + " peer=" + peer + " uuid=" + uuid);
        new Thread(() -> run(mode, peer, uuid)).start();
    }

    void run(String mode, String peer, String uuid) {
        Communicator communicator = null;
        try {
            InitializationData initData = new InitializationData();
            initData.pluginFactories = Collections.singletonList(new com.zeroc.IceBT.PluginFactory());
            communicator = new Communicator(initData);
            Log.i(TAG, "Ice communicator created with IceBT plug-in");

            if ("server".equals(mode)) {
                ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints("A", "bt -u " + uuid);
                adapter.add(new com.zeroc.Ice.Object() {}, new Identity("test", ""));
                adapter.activate();
                BluetoothAdapter bt = BluetoothAdapter.getDefaultAdapter();
                Log.i(TAG, "RESULT server OK : object adapter active on bt, local addr="
                    + (bt == null ? "?" : bt.getAddress()));
                communicator.waitForShutdown();
            } else if ("client".equals(mode)) {
                // IceBT uses secure RFCOMM, which needs a bond first. Pre-bond (auto-confirmed above).
                BluetoothAdapter bt = BluetoothAdapter.getDefaultAdapter();
                BluetoothDevice dev = bt.getRemoteDevice(peer);
                if (dev.getBondState() != BluetoothDevice.BOND_BONDED) {
                    Log.i(TAG, "bonding to " + peer + " ...");
                    dev.createBond();
                    long deadline = System.currentTimeMillis() + 60000;
                    while (dev.getBondState() != BluetoothDevice.BOND_BONDED
                            && System.currentTimeMillis() < deadline) {
                        Thread.sleep(1000);
                    }
                    Log.i(TAG, "bond state=" + dev.getBondState());
                }
                // Quote the address: ':' is the proxy endpoint separator, so a bare BT MAC gets split.
                ObjectPrx prx = communicator.stringToProxy("test:bt -a \"" + peer + "\" -u " + uuid);
                Log.i(TAG, "ice_ping over IceBT-over-Bluetooth ...");
                prx.ice_ping();
                Log.i(TAG, "RESULT client OK : ice_ping succeeded over IceBT-over-Bluetooth");
            } else {
                Log.i(TAG, "RESULT client FAIL : unknown mode '" + mode + "'");
            }
        } catch (Throwable t) {
            Log.e(TAG, "RESULT " + mode + " FAIL : " + t, t);
        } finally {
            if (communicator != null && "client".equals(mode)) {
                try {
                    communicator.destroy();
                } catch (Throwable ignore) {
                    // ignore
                }
            }
        }
    }
}
