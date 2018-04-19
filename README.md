# libserialport-example
An example showing use of libserialport on a Raspberry Pi.  This example assumes the
use of Raspbian or other Debian/Ubuntu-based distribution running on the Pi.  Cross
compiling is out of scope for this example, although that is certainly possible.

To test, connect an Arduino or other board that appears as a serial port
to the Pi via USB.  The Arduino sketch should just periodically print
something via Serial.println.

This example will wait for a serial device to be available, then read 4 lines from it
and exit.  If the Arduino is unplugged before this program receives 4 lines, it will
wait for the device to reappear.

### Prerequisites
You will need a Raspberry Pi with development tools installed.  The following should do it:
```
sudo apt install build-essential
```

You also need to install libserialport:
```
sudo apt install libserial-dev
```

### Building
Make sure the build script is executable.  If not:
```
chmod +x makeit.sh
```

Build it:
```
./makeit.sh
```

or

```
bash makeit.sh
```

### Running
```
./list-ports
```


