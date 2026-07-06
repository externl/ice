# Bluetooth Testing Guide

This document describes how to run the Ice test suite over Bluetooth connections between different language mappings.

## Prerequisites

- C++ and Java source builds, refer to the [cpp/BUILDING.md] and [java/BUILDING.md] for details
- An Android device with Bluetooth capability
- The [Android test controller] application installed and running on the device
- Bluetooth adapters on both client and server machines
- Paired Bluetooth devices (the Android device and the machine running the C++ tests)

## Run C++ Client Against Android Server Over Bluetooth

First, start the Android test controller application on your device.

From the `java` directory, start the Python controller to manage the Android server:

```bash
python ../scripts/Controller.py --host-bt="A4:FF:9F:A6:48:C1" --host=192.168.1.51 --id=server --android
```

Where:
- `--host-bt` is the Bluetooth address of the Android device running the server
- `--host` is the IP address of the Android device running the server
- `--id=server` identifies this controller as managing servers
- `--android` indicates this is controlling an Android device

Then from the `cpp` directory, start the C++ test suite to run against the Android servers:

```bash
python ./allTests.py --server=server --protocol=bt --cross=java
```

Where:
- `--server=server` indicates to use the remote server controller
- `--protocol=bt` specifies to use Bluetooth transport, use `--protocol=bts` to test Bluetooth over SSL
- `--cross=java` indicates cross-language testing with Java servers

## Run Android Client Against C++ Server Over Bluetooth

First, start the Android test controller application on your device.

From the `cpp` directory, start the Python controller to manage the C++ server:

```bash
python ../scripts/Controller.py --host-bt="00:15:83:ED:D7:29" --host=192.168.1.48 --id=server
```

Where:
- `--host-bt` is the Bluetooth address of the machine running the C++ server
- `--host` is the IP address of the machine running the C++ server
- `--id=server` identifies this controller as managing servers

Then from the `java` directory, start the Java/Android test suite to run against the C++ servers:

```bash
python ./allTests.py --server=server --protocol=bt --cross=cpp --android
```

Where:
- `--server=server` indicates to use the remote server controller
- `--protocol=bt` specifies to use Bluetooth transport, use `--protocol=bts` to test Bluetooth over SSL
- `--cross=cpp` indicates cross-language testing with C++ servers
- `--android` runs the Android client tests

## Run Android Client Against Android Server Over Bluetooth (two emulators)

Both sides can run on Android emulators, using the emulator's virtual Bluetooth controller
(Netsim/Rootcanal) — no radios required. This uses two independent controller processes, each
managing one emulator via `--device`.

Prerequisites:
- Two running emulators (API 32+ `google_apis` images) launched with `-packet-streamer-endpoint
  default` so both attach to the shared Netsim BT network.
- The two emulators **bonded** beforehand (IceBT uses secure RFCOMM). Bonding is a one-time step;
  headless (CI) it can be automated by installing a small helper as a privileged system app that
  auto-confirms pairing (`setPairingConfirmation`, which needs `BLUETOOTH_PRIVILEGED`).

Start the server controller, bound to the server emulator:

```bash
# from the java directory
python ../scripts/Controller.py --id=server --android --device=emulator-5556 --host-bt="<emulator-5556 BT address>"
```

Then run the Android client test suite against it, bound to the client emulator:

```bash
# from the java directory
python ./allTests.py --server=server --protocol=bt --cross=java --android --device=emulator-5554 --host-bt="<emulator-5556 BT address>" Ice/operations
```

Where:
- `--device=emulator-<port>` selects which emulator each process drives (the harness forwards a
  per-device host port so the two controllers don't collide).
- `--host-bt` is the **server** emulator's Bluetooth address (find it with
  `adb -s emulator-5556 shell settings get secure bluetooth_address`).
- `--cross=java` runs a Java server against the Java client.

## Finding Bluetooth Addresses

On Linux, you can find the Bluetooth address of your machine using:

```bash
hciconfig
```

On Android, go to Settings → About phone → Status (or Settings → System → About phone) to find the Bluetooth address.

## Troubleshooting

- Ensure Bluetooth is enabled on all devices
- Verify devices are paired before running tests
- Check that the Android test controller application is running and visible
- Ensure IP addresses and Bluetooth addresses are correct
- Use `--debug` flag for verbose output to diagnose connection issues

[Android test controller]: java/test//android/controller/
[cpp/BUILDING.md]: cpp/BUILDING.md
[java/BUILDING.md]: java/BUILDING.md
