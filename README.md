This is a project used to test libusb with the DDJ-400. Long story short, this did not work on Windows because libusb can only retreive transfers from generic USB drivers. The DDJ-400 runs via the usbaudio protocol which is entirely different from the normal USB protocol normally used for libusb.