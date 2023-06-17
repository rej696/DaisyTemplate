# Daisy Template
This repo is a template project for working with the Simple-Fix board and [Daisy Seed](https://www.electro-smith.com/daisy/daisy)

It implements a [Drone Synth](https://github.com/Synthux-Academy/simple-examples/tree/main/daisyduino/simple-multivoice-drone) from [this Livestream](https://youtu.be/aPIAkeeGB0Q)

## Setup
I use WSL2 which requires some [configuration](https://blog.golioth.io/program-mcu-from-wsl2-with-usb-support/).
To flash the board, first pass through the USB to WSL using [usbipd](https://github.com/dorssel/usbipd-win) in powershell as follows:
```
sudo usbipd wsl list
sudo usbipd wsl attach --busid 2-2
ss -com:10
```
ss is a serial console for windows

### Dependencies
Information about the [toolchain is here](https://github.com/electro-smith/DaisyWiki/wiki/1d.-Installing-the-Toolchain-on-Linux)

You also need to clone [libDaisy](https://github.com/electro-smith/libDaisy) and [DaisySP](https://github.com/electro-smith/DaisySP)

- [libDaisy Docs](https://electro-smith.github.io/libDaisy/index.html)
- [DaisySP Docs](https://electro-smith.github.io/DaisySP/index.html)
- [Simple Synth Guide](https://www.youtube.com/watch?v=qIpW3MuebBE&list=PLBrCYHmNvufXhqGnm0eNpsfLcktGQkS-q&index=2)


### Build and Flash
```
make -C ../DaisySP
make -C ../libDaisy
make
make program-dfu
```

### Information
A0, A1 and A2 are setup as ADC's to read potentiometers, D18 is setup as a GPIO switch/button.
```
////////////// SIMPLE X DAISY PINOUT CHEATSHEET ///////////////

// 3v3           29  |       |   20    AGND
// D15 / A0      30  |       |   19    OUT 01
// D16 / A1      31  |       |   18    OUT 00
// D17 / A2      32  |       |   17    IN 01
// D18 / A3      33  |       |   16    IN 00
// D19 / A4      34  |       |   15    D14
// D20 / A5      35  |       |   14    D13
// D21 / A6      36  |       |   13    D12
// D22 / A7      37  |       |   12    D11
// D23 / A8      38  |       |   11    D10
// D24 / A9      39  |       |   10    D9
// D25 / A10     40  |       |   09    D8
// D26           41  |       |   08    D7
// D27           42  |       |   07    D6
// D28 / A11     43  |       |   06    D5
// D29           44  |       |   05    D4
// D30           45  |       |   04    D3
// 3v3 Digital   46  |       |   03    D2
// VIN           47  |       |   02    D1
// DGND          48  |       |   01    D0
```

### TODO
- Make a `make help` command
