#include "USBComposite.h"
#include "USBMIDI.hpp"

#include "usb_generic.h"
#include "usb_midi_device.h"
#include <libmaple/usb.h>
#include <wirish.h>

BEGIN_CS_NAMESPACE

namespace USBMIDI {

#define USB_TIMEOUT 50

uint8_t isConnected() {
    return usb_is_connected(USBLIB) && usb_is_configured(USBLIB);
}

MIDIUSBPacket_t read() {

    if (!usb_midi_data_available() || !isConnected()) {
        return MIDIUSBPacket_t{{0, 0, 0, 0}};
    }

    uint32 packetBuffer;
    MIDIUSBPacket_t packet;

    uint32 readBytes = 0;
    uint32 start = millis();

    while (readBytes == 0) {
        if (millis() - start > USB_TIMEOUT) {
            return MIDIUSBPacket_t{{0, 0, 0, 0}};
        }

        readBytes = usb_midi_rx(&packetBuffer, 1);
    }

    memcpy(packet.data, &packetBuffer, sizeof(packet.data));

    return packet;
}

void write(uint8_t cn, uint8_t cin, uint8_t d0, uint8_t d1, uint8_t d2) {
    if (!isConnected()) {
        return;
    }

    uint32 packet = (cn << 4) | cin | // CN|CIN
                    (d0 << 8) |       // status
                    (d1 << 16) |      // data 1
                    (d2 << 24);       // data 2

    uint32 sent = usb_midi_tx(&packet, 1);

    while (usb_midi_is_transmitting() != 0) {
        delay(0);
    }

    usb_midi_tx(NULL, 0);
}

void flush() { 
    usb_midi_tx(NULL, 0); 
}

void begin() {

    USBComposite.clear();
    USBComposite.add(&usbMIDIPart, NULL,
                     (USBPartInitializer)&CS::USBMIDI::init);
    USBComposite.begin();

    while (!USBComposite) {
        delay(0);
    };
}

bool init() {
    usb_midi_setTXEPSize(64);
    usb_midi_setRXEPSize(64);

    digitalWrite(33, 1); // For now, debug Test LED here
    return true;
}

} // namespace USBMIDI

// namespace USBMIDI

END_CS_NAMESPACE
