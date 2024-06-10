#include "UsbActions.hpp"
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <format>

UsbActions::UsbActions() {
	 // assume there's only one user of this lib at a time
	int success = libusb_init_context(/*ctx=*/NULL, /*options=*/NULL, /*num_options=*/0);
#if _DEBUG
	libusb_set_debug(NULL, 3);
#endif
	libusb_get_device_list(NULL, &devices) >= 0 ? initializeDDJ400Device() : libusb_exit(NULL);
}

void UsbActions::initializeDDJ400Device() {
	libusb_device_descriptor deviceDesc;
	libusb_device_handle* deviceHandle;
	int success = 0;
	for (int i = 0; devices[i]; i++) {
		auto device = devices[i];
		success = libusb_get_device_descriptor(devices[i], &deviceDesc);
		if (success >= 0) {
			libusb_open(device, &deviceHandle);
			if (deviceHandle && deviceDesc.iProduct) {
				unsigned char string[256];
				success = libusb_get_string_descriptor_ascii(deviceHandle, deviceDesc.iProduct, string, sizeof(string));
				if (success > 0 && strcmp(reinterpret_cast<const char*> (string), "DDJ-400") == 0) {
					std::cout << "Found DDJ:" << string << std::endl;
					this->deviceDDJ400 = device;
					this->hDeviceDDJ400 = deviceHandle;
					initializeDDJ400WheelEndpoint(deviceDesc);
					break;
				}
			}
		}
	}

	if (success < 1) {
		std::cout << "DDJ-400 not found, please reconnect and try initializing again." << std::endl;
	}
}

void UsbActions::initializeDDJ400WheelEndpoint(const libusb_device_descriptor& deviceDesc) {
	for (int i = 0; i < deviceDesc.bNumConfigurations && !this->deviceDDJ400EndpointWheel; i++) {

		if (libusb_get_config_descriptor(deviceDDJ400, i, &deviceConfigDDJ400) != LIBUSB_SUCCESS || deviceConfigDDJ400 == nullptr) {
			std::cout << "Couldn't retrieve descriptors" << std::endl;
			continue;
		}

		bool foundWheel = false;
		for (int j = 0; j < deviceConfigDDJ400->bNumInterfaces && !foundWheel; j++) {
			auto configInterface = deviceConfigDDJ400->interface[j];
			for (int k = 0; k < configInterface.num_altsetting && !foundWheel; k++) {
				auto altSetting = configInterface.altsetting[k];
				for (int l = 0; l < altSetting.bNumEndpoints && !foundWheel; l++) {
					auto endpoint = std::format("{:x}", altSetting.endpoint[l].bEndpointAddress);
					if (endpoint._Equal("84")) {
						std::cout << "Found DDJ Wheel Endpoint" << std::endl;
						this->deviceDDJ400EndpointWheel = const_cast<libusb_endpoint_descriptor *>(&altSetting.endpoint[l]);
						int errorUsbClaim = libusb_claim_interface(this->hDeviceDDJ400, altSetting.iInterface);
						if (errorUsbClaim == 0) {
							std::cout << "claim successful of interfaceNumber=" << std::format("{:d}", altSetting.iInterface) << std::endl;
							this->deviceInterfaceNumber = altSetting.iInterface;
						}
						else {
							std::cout << "claim unsuccessful of interfaceNumber=" << std::format("{:d}", altSetting.iInterface) << std::endl;
						}
						foundWheel = true;
					}
				}
			}
		}
	}
}

// test method. will probably go away after a while.
void UsbActions::printBuffer(unsigned char* buffer, int bufferSize) {
	std::cout << "buffer=";
	for (int i = 0; i < bufferSize; i++) {
		std::cout << std::format("{:x}", buffer[i]);
	}
	std::cout << std::endl;
}

double UsbActions::getDDJWheelPosition() {
	double outWheelPosition = 0.0;
	if (this->deviceDDJ400EndpointWheel) {
		uint8_t buffer[91];
		memset(buffer, 0x00, sizeof(buffer));
		int actualLength = 0;
		auto endpointIn = deviceDDJ400EndpointWheel->bEndpointAddress & LIBUSB_ENDPOINT_IN;
		// for this endpoint we always get failed transfers, I'm assuming this has to do with how libusb cannot intercept non generic usb feedback (this is using the usbaudio driver from pioneer)
		int errorCode = libusb_interrupt_transfer(this->hDeviceDDJ400, deviceDDJ400EndpointWheel->bEndpointAddress, (unsigned char*) &buffer, 91, &actualLength, 100);
		std::cout << "errorCode=" << errorCode << ", actualLength=" << actualLength << std::endl;
		printBuffer(&buffer[0], actualLength);
	}
	else {
		std::cout << "Did not find DDJ Wheel endpoint, skipping for now" << std::endl;
	}
	return outWheelPosition;
}

UsbActions::~UsbActions() {
	if (this->deviceConfigDDJ400) {
		libusb_free_config_descriptor(this->deviceConfigDDJ400);
	}
	if (this->hDeviceDDJ400) {
		if (this->deviceInterfaceNumber > -1) {
			libusb_release_interface(this->hDeviceDDJ400, this->deviceInterfaceNumber);
		}
		libusb_close(this->hDeviceDDJ400);
	}
	if (this->devices) {
		libusb_free_device_list(this->devices, 1);
		libusb_exit(NULL);
	}
}