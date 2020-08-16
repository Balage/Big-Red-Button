/*
  VbsKeyboard.cpp
  
  Copyright (c) 2020, Balazs Vecsey, www.vbstudio.hu
  
  This is a merged, modified and extended version of the Keyboard.cpp
  and HID.cpp from the official Arduino library, version 1.8.12.
  Original copyright is as follows:

  Copyright (c) 2015, Arduino LLC
  Original code (pre-library): Copyright (c) 2011, Peter Barrett

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
*/

#include "VbsKeyboard.h"

#if defined(USBCON)

#define HID_REPORTID_KEYBOARD       0x02
#define HID_REPORTID_GENERICDESKTOP 0x04

static const uint8_t _hidReportDescriptorPage1[] PROGMEM = {
	0x05, 0x01,									// USAGE_PAGE (Generic Desktop)
	0x09, 0x06, 								// USAGE (Keyboard)
	0xA1, 0x01, 								// COLLECTION (application)
	0x85, HID_REPORTID_GENERICDESKTOP, 		    // REPORT_ID (HID_REPORTID_CONSUMERCONTROL)
	
	// 4 Media Keys
	0x15, 0x00, 								// LOGICAL_MINIMUM
	0x26, 0xFF, 0x03, 							// LOGICAL_MAXIMUM (3ff)
	0x19, 0x00, 								// USAGE_MINIMUM (0)
	0x2A, 0xFF, 0x03, 							// USAGE_MAXIMUM (3ff)
	0x95, 0x04, 								// REPORT_COUNT (4)
	0x75, 0x10, 								// REPORT_SIZE (16)
	0x81, 0x00, 								// INPUT (Data,Ary,Abs)
	0xC0 // END_COLLECTION
};

static const uint8_t _hidReportDescriptorPage7[] PROGMEM = {
	0x05, 0x01, // USAGE_PAGE (Generic Desktop)
	0x09, 0x06, // USAGE (Keyboard)
	0xa1, 0x01, // COLLECTION (Application)
	0x85, HID_REPORTID_KEYBOARD, // REPORT_ID (2)
	
	0x05, 0x07, //   USAGE_PAGE (Keyboard)
	
	// Keyboard Modifiers (shift, alt, ...)
	0x19, 0xe0, //   USAGE_MINIMUM (Keyboard LeftControl)
	0x29, 0xe7, //   USAGE_MAXIMUM (Keyboard Right GUI)
	0x15, 0x00, //   LOGICAL_MINIMUM (0)
	0x25, 0x01, //   LOGICAL_MAXIMUM (1)
	0x75, 0x01, //   REPORT_SIZE (1)
	0x95, 0x08, //   REPORT_COUNT (8)
	0x81, 0x02, //   INPUT (Data,Var,Abs)
	
	0x95, 0x01, //   REPORT_COUNT (1)
	0x75, 0x08, //   REPORT_SIZE (8)
	
	0x81, 0x03, //   INPUT (Cnst,Var,Abs)
	0x95, 0x05, //   REPORT_COUNT (5)
	0x75, 0x01, //   REPORT_SIZE (1)
	
	// 3 LEDs
	0x05, 0x08, //   USAGE_PAGE (LEDs)
	0x19, 0x01, //   USAGE_MINIMUM (1)
	0x29, 0x05, //   USAGE_MAXIMUM (5)
	0x91, 0x02, //   OUTPUT (Data,Var,Abs) // LED report
	0x95, 0x01, //   REPORT_COUNT (1)
	0x75, 0x03, //   REPORT_SIZE (3)
	
	0x91, 0x01, //   OUTPUT (Constant) // padding 
	0x95, 0x06, //   REPORT_COUNT (6)
	// END of LEDs
	
	// 6 Keyboard keys
	0x95, 0x06, //   REPORT_COUNT (6)
	0x75, 0x08, //   REPORT_SIZE (8)
	0x15, 0x00, //   LOGICAL_MINIMUM (0)
	0x25, 0x73, //   LOGICAL_MAXIMUM (115)
	0x05, 0x07, //   USAGE_PAGE (Keyboard)
	0x19, 0x00, //   USAGE_MINIMUM (Reserved (no event indicated))
	0x29, 0x73, //   USAGE_MAXIMUM (Keyboard Application)
	0x81, 0x00, //   INPUT (Data,Ary,Abs)
	0xc0,       // END_COLLECTION
};

// Implementation of PluggableUSBModule
int VbsKeyboard_::getInterface(uint8_t* interfaceCount)
{
	*interfaceCount += 1; // uses 1
	HIDDescriptor hidInterface = {
		D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
		D_HIDREPORT(_descriptorSize),
		D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
	};
	return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
}

// Implementation of PluggableUSBModule
int VbsKeyboard_::getDescriptor(USBSetup& setup)
{
	// Check if this is a HID Class Descriptor request
	if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE) return 0;
	if (setup.wValueH != HID_REPORT_DESCRIPTOR_TYPE) return 0;
	
	// In a HID Class Descriptor wIndex cointains the interface number
	if (setup.wIndex != pluggedInterface) return 0;
	
	int total = 0;
	HIDSubDescriptor* node;
	for (node = _rootNode; node; node = node->next)
	{
		int res = USB_SendControl(TRANSFER_PGM, node->data, node->length);
		if (res == -1) return -1;
		total += res;
	}
	
	// Reset the protocol on reenumeration. Normally the host should not assume the state of the protocol
	// due to the USB specs, but Windows and Linux just assumes its in report mode.
	_protocol = HID_REPORT_PROTOCOL;
	
	return total;
}

// Implementation of PluggableUSBModule
uint8_t VbsKeyboard_::getShortName(char *name)
{
	name[0] = 'H';
	name[1] = 'I';
	name[2] = 'D';
	name[3] = 'A' + (_descriptorSize & 0x0F);
	name[4] = 'A' + ((_descriptorSize >> 4) & 0x0F);
	return 5;
}

// Implementation of PluggableUSBModule
bool VbsKeyboard_::setup(USBSetup& setup)
{
	if (pluggedInterface != setup.wIndex) return false;
	
	uint8_t request = setup.bRequest;
	uint8_t requestType = setup.bmRequestType;
	
	if (requestType == REQUEST_DEVICETOHOST_CLASS_INTERFACE)
	{
		if (request == HID_GET_REPORT)
		{
			// TODO: HID_GetReport();
			return true;
		}
		else if (request == HID_GET_PROTOCOL)
		{
			// TODO: Send8(_protocol);
			return true;
		}
		else if (request == HID_GET_IDLE)
		{
			// TODO: Send8(_idle);
		}
	}
	
	if (requestType == REQUEST_HOSTTODEVICE_CLASS_INTERFACE)
	{
		if (request == HID_SET_PROTOCOL)
		{
			// The USB Host tells us if we are in boot or report mode.
			// This only works with a real boot compatible device.
			_protocol = setup.wValueL;
			return true;
		}
		else if (request == HID_SET_IDLE)
		{
			_idle = setup.wValueL;
			return true;
		}
		else if (request == HID_SET_REPORT)
		{
			if (setup.wLength == 2) 
			{
				uint8_t data[2];
				if (USB_RecvControl(data, 2) == 2) 
				{
					_ledsState = data[1];
				}
			}
		}
	}
	
	return false;
}

VbsKeyboard_::VbsKeyboard_(void) :
	PluggableUSBModule(1, 1, _epType),
	_rootNode(NULL), _descriptorSize(0),
	_protocol(HID_REPORT_PROTOCOL), _idle(1)
{
	_epType[0] = EP_TYPE_INTERRUPT_IN;
	PluggableUSB().plug(this);
	
	// Append generic keyboard descriptor
	static HIDSubDescriptor nodeKeyboard(_hidReportDescriptorPage7, sizeof(_hidReportDescriptorPage7));
	AppendDescriptor(&nodeKeyboard);
	
	// Append system keyboard descriptor
	static HIDSubDescriptor nodeConsumer(_hidReportDescriptorPage1, sizeof(_hidReportDescriptorPage1));
	AppendDescriptor(&nodeConsumer);
}

void VbsKeyboard_::AppendDescriptor(HIDSubDescriptor* node)
{
	if (!_rootNode)
	{
		_rootNode = node;
	}
	else
	{
		HIDSubDescriptor *current = _rootNode;
		while (current->next)
		{
			current = current->next;
		}
		current->next = node;
	}
	_descriptorSize += node->length;
}

#define SendReportPage1() { SendReport(HID_REPORTID_GENERICDESKTOP, &_keyReportPage1, sizeof(KeyReportPage1)); }
#define SendReportPage7() { SendReport(HID_REPORTID_KEYBOARD, &_keyReportPage7, sizeof(KeyReportPage7)); }

void VbsKeyboard_::SendReport(uint8_t id, void* data, int len)
{
	if (USB_Send(pluggedEndpoint, &id, 1) < 0) return;
	if (USB_Send(pluggedEndpoint | TRANSFER_RELEASE, data, len) < 0) return;
}

void VbsKeyboard_::PressKeyPage1(uint16_t key) 
{
	// Press
	_keyReportPage1.keys[0] = key;
	_keyReportPage1.keys[1] = 0;
	_keyReportPage1.keys[2] = 0;
	_keyReportPage1.keys[3] = 0;
	SendReportPage1();
	
	// Release
	_keyReportPage1.keys[0] = 0;
	SendReportPage1();
}
	
void VbsKeyboard_::PressKey(uint8_t key, uint8_t modifier)
{
	HoldKey(key, modifier);
	ReleaseKey();
}

void VbsKeyboard_::HoldKey(uint8_t key, uint8_t modifier)
{
	_keyReportPage7.keys[0] = key;
	_keyReportPage7.keys[1] = 0;	
	_keyReportPage7.keys[2] = 0;
	_keyReportPage7.keys[3] = 0;
	_keyReportPage7.keys[4] = 0;
	_keyReportPage7.keys[5] = 0;	
	_keyReportPage7.modifiers = modifier;
	SendReportPage7();
}

bool VbsKeyboard_::GetLedState(uint8_t mask) const
{
	return _ledsState & KB_LED_SCROLL;
}


VbsKeyboard_ Keyboard;

#endif /* if defined(USBCON) */