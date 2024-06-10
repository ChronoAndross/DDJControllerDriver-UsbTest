#include "libusb.h"
class UsbActions {
public:
	UsbActions();
	double getDDJWheelPosition();
	virtual ~UsbActions();
protected:
	void initializeDDJ400Device();
	void initializeDDJ400WheelEndpoint(const libusb_device_descriptor& deviceDesc);
	void printBuffer(unsigned char* buffer, int bufferSize);
private:
	libusb_context* context = nullptr;
	libusb_device** devices = nullptr;
	libusb_device* deviceDDJ400 = nullptr;
	libusb_device_handle* hDeviceDDJ400 = nullptr;
	libusb_config_descriptor* deviceConfigDDJ400 = nullptr;
	libusb_endpoint_descriptor* deviceDDJ400EndpointWheel = nullptr;
	int deviceInterfaceNumber = -1;
	const uint8_t ENDPOINT_ADDRESS_WHEEL = 84; // discovered by looking at wireshark data
};