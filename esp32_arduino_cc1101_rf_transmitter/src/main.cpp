/**
 * @file        main.cpp
 * @brief       Entry point and main program for CC1101 RF transmitter
 * @author      Copyright (C) Peter Ivanov, 2026
 *
 * Created      2026-05-04 11:30:53
 * Last modify: 2026-10-02 19:32:49 ivanovp {Time-stamp}
 * Licence:     GPL
 */
#include <Arduino.h>
#include <SPI.h>
#include <ctype.h>

// Fixed ESP32-WROOM-32U <-> CC1101 wiring.
constexpr uint8_t PIN_SCK = 18;
constexpr uint8_t PIN_MISO = 19;
constexpr uint8_t PIN_MOSI = 23;
constexpr uint8_t PIN_CSN = 5;
constexpr uint8_t PIN_GDO0 = 25; // Asynchronous TX data input.
constexpr uint8_t PIN_GDO2 = 27; // Reserved for a later RX test.

// A-OK remote IDs are 24-bit values. Profile 1 clones the existing remote;
// profile 2 is reserved for pairing the window opener that has no remote.
constexpr uint32_t EXISTING_REMOTE_ID = 0x000984;
constexpr uint32_t NEW_REMOTE_ID = 0x123456;
constexpr uint16_t ADDRESS = 0x0001;

constexpr uint32_t ID_SEARCH_OBSERVE_MS = 100;
uint32_t activeRemoteId = EXISTING_REMOTE_ID;
uint32_t lastTriedRemoteId = 0;
uint32_t nextSearchRemoteId = 0;
bool idSearchHasTried = false;
bool idSearchComplete = false;

// Timings measured from the physical A-OK remote, in microseconds.
// Keep them together so they can easily be tuned after an RTL-SDR capture.
constexpr uint32_t ZERO_HIGH_US = 255;
constexpr uint32_t ZERO_LOW_US = 605;
constexpr uint32_t ONE_HIGH_US = 605;
constexpr uint32_t ONE_LOW_US = 255;
constexpr uint32_t SYNC_HIGH_US = 4910;
constexpr uint32_t SYNC_LOW_US = 1915;
constexpr uint32_t INTER_FRAME_LOW_US = 5100;
constexpr uint8_t PREAMBLE_BITS = 7;

constexpr uint8_t AOK_START = 0xA3;
constexpr uint8_t CMD_OPEN = 0x0B;
constexpr uint8_t CMD_STOP = 0x23;
constexpr uint8_t CMD_CLOSE = 0x43;
constexpr uint8_t CMD_AFTER = 0x24;
constexpr uint8_t CMD_PROGRAM = 0x53;

namespace cc1101
{
    constexpr uint8_t WRITE_BURST = 0x40;
    constexpr uint8_t READ_SINGLE = 0x80;
    constexpr uint8_t READ_BURST = 0xC0;

    constexpr uint8_t IOCFG2 = 0x00;
    constexpr uint8_t IOCFG1 = 0x01;
    constexpr uint8_t IOCFG0 = 0x02;
    constexpr uint8_t FIFOTHR = 0x03;
    constexpr uint8_t PKTCTRL1 = 0x07;
    constexpr uint8_t PKTCTRL0 = 0x08;
    constexpr uint8_t FSCTRL1 = 0x0B;
    constexpr uint8_t FSCTRL0 = 0x0C;
    constexpr uint8_t FREQ2 = 0x0D;
    constexpr uint8_t FREQ1 = 0x0E;
    constexpr uint8_t FREQ0 = 0x0F;
    constexpr uint8_t MDMCFG4 = 0x10;
    constexpr uint8_t MDMCFG3 = 0x11;
    constexpr uint8_t MDMCFG2 = 0x12;
    constexpr uint8_t DEVIATN = 0x15;
    constexpr uint8_t MCSM0 = 0x18;
    constexpr uint8_t FOCCFG = 0x19;
    constexpr uint8_t BSCFG = 0x1A;
    constexpr uint8_t AGCCTRL2 = 0x1B;
    constexpr uint8_t AGCCTRL1 = 0x1C;
    constexpr uint8_t AGCCTRL0 = 0x1D;
    constexpr uint8_t FREND1 = 0x21;
    constexpr uint8_t FREND0 = 0x22;
    constexpr uint8_t FSCAL3 = 0x23;
    constexpr uint8_t FSCAL2 = 0x24;
    constexpr uint8_t FSCAL1 = 0x25;
    constexpr uint8_t FSCAL0 = 0x26;
    constexpr uint8_t TEST2 = 0x2C;
    constexpr uint8_t TEST1 = 0x2D;
    constexpr uint8_t TEST0 = 0x2E;
    constexpr uint8_t PARTNUM = 0x30;
    constexpr uint8_t VERSION = 0x31;
    constexpr uint8_t MARCSTATE = 0x35;
    constexpr uint8_t PATABLE = 0x3E;

    constexpr uint8_t SRES = 0x30;
    constexpr uint8_t STX = 0x35;
    constexpr uint8_t SIDLE = 0x36;
    constexpr uint8_t SFTX = 0x3B;
    constexpr uint8_t STATE_IDLE = 0x01;
    constexpr uint8_t STATE_TX = 0x13;
} // namespace cc1101

struct RegisterValue
{
    uint8_t address;
    uint8_t value;
};

// 433.92 MHz, ASK/OOK, about 1.162 kbaud, asynchronous serial mode.
// PKT_FORMAT=3 bypasses the packet engine and makes GDO0 the TX data input.
constexpr RegisterValue RADIO_CONFIG[] = {
    {cc1101::IOCFG2, 0x2E},
    {cc1101::IOCFG1, 0x2E},
    {cc1101::IOCFG0, 0x2E},
    {cc1101::FIFOTHR, 0x47},
    {cc1101::PKTCTRL1, 0x04},
    {cc1101::PKTCTRL0, 0x32},
    {cc1101::FSCTRL1, 0x06},
    {cc1101::FSCTRL0, 0x00},
    {cc1101::FREQ2, 0x10},
    {cc1101::FREQ1, 0xB0},
    {cc1101::FREQ0, 0x71},
    {cc1101::MDMCFG4, 0xF5},
    {cc1101::MDMCFG3, 0x77},
    {cc1101::MDMCFG2, 0x30},
    {cc1101::DEVIATN, 0x00},
    {cc1101::MCSM0, 0x18},
    {cc1101::FOCCFG, 0x16},
    {cc1101::BSCFG, 0x6C},
    {cc1101::AGCCTRL2, 0x03},
    {cc1101::AGCCTRL1, 0x40},
    {cc1101::AGCCTRL0, 0x91},
    {cc1101::FREND1, 0x56},
    {cc1101::FREND0, 0x11},
    {cc1101::FSCAL3, 0xE9},
    {cc1101::FSCAL2, 0x2A},
    {cc1101::FSCAL1, 0x00},
    {cc1101::FSCAL0, 0x1F},
    {cc1101::TEST2, 0x81},
    {cc1101::TEST1, 0x35},
    {cc1101::TEST0, 0x09},
};

SPISettings radioSpiSettings(4000000, MSBFIRST, SPI_MODE0);
bool radioReady = false;

bool selectRadio()
{
    SPI.beginTransaction(radioSpiSettings);
    digitalWrite(PIN_CSN, LOW);
    const uint32_t started = micros();
    while (digitalRead(PIN_MISO) == HIGH)
    {
        if (micros() - started > 5000)
        {
            digitalWrite(PIN_CSN, HIGH);
            SPI.endTransaction();
            return false;
        }
    }
    return true;
}

void deselectRadio()
{
    digitalWrite(PIN_CSN, HIGH);
    SPI.endTransaction();
}

bool strobe(uint8_t command)
{
    if (!selectRadio())
        return false;
    SPI.transfer(command);
    deselectRadio();
    return true;
}

bool writeRegister(uint8_t address, uint8_t value)
{
    if (!selectRadio())
        return false;
    SPI.transfer(address);
    SPI.transfer(value);
    deselectRadio();
    return true;
}

bool writeBurst(uint8_t address, const uint8_t *values, size_t length)
{
    if (!selectRadio())
        return false;
    SPI.transfer(address | cc1101::WRITE_BURST);
    for (size_t i = 0; i < length; ++i)
        SPI.transfer(values[i]);
    deselectRadio();
    return true;
}

bool readRegister(uint8_t address, uint8_t &value, bool status = false)
{
    if (!selectRadio())
        return false;
    SPI.transfer(address | (status ? cc1101::READ_BURST : cc1101::READ_SINGLE));
    value = SPI.transfer(0x00);
    deselectRadio();
    return true;
}

bool resetRadio()
{
    digitalWrite(PIN_CSN, HIGH);
    delayMicroseconds(5);
    digitalWrite(PIN_CSN, LOW);
    delayMicroseconds(10);
    digitalWrite(PIN_CSN, HIGH);
    delayMicroseconds(45);
    return strobe(cc1101::SRES);
}

bool readMarcState(uint8_t &state)
{
    if (!readRegister(cc1101::MARCSTATE, state, true))
        return false;
    state &= 0x1F;
    return true;
}

bool waitForState(uint8_t expected, uint32_t timeoutUs)
{
    const uint32_t started = micros();
    uint8_t state = 0xFF;
    do
    {
        if (readMarcState(state) && state == expected)
            return true;
    } while (micros() - started < timeoutUs);
    Serial.printf("CC1101 state timeout: expected=0x%02X actual=0x%02X\r\n",
                  expected, state);
    return false;
}

void printRadioInfo()
{
    uint8_t part = 0xFF, version = 0xFF, state = 0xFF;
    uint8_t pktctrl0 = 0xFF, mdmcfg2 = 0xFF;
    uint8_t freq2 = 0xFF, freq1 = 0xFF, freq0 = 0xFF;
    const bool ok =
        readRegister(cc1101::PARTNUM, part, true) &&
        readRegister(cc1101::VERSION, version, true) && readMarcState(state) &&
        readRegister(cc1101::PKTCTRL0, pktctrl0) &&
        readRegister(cc1101::MDMCFG2, mdmcfg2) &&
        readRegister(cc1101::FREQ2, freq2) &&
        readRegister(cc1101::FREQ1, freq1) && readRegister(cc1101::FREQ0, freq0);

    if (!ok)
    {
        Serial.println("CC1101 status read FAILED (MISO ready timeout)");
        return;
    }
    Serial.printf("CC1101 PARTNUM=0x%02X VERSION=0x%02X MARCSTATE=0x%02X\r\n",
                  part, version, state);
    Serial.printf(
        "Registers: FREQ=%02X %02X %02X PKTCTRL0=0x%02X MDMCFG2=0x%02X\r\n",
        freq2, freq1, freq0, pktctrl0, mdmcfg2);
    Serial.println(
        "RF: 433.92 MHz, ASK/OOK, asynchronous serial TX on GDO0/GPIO25");
    Serial.printf(
        "Active remote: %s (0x%06lX)\r\n",
        activeRemoteId == EXISTING_REMOTE_ID
            ? "existing"
            : (activeRemoteId == NEW_REMOTE_ID ? "new" : "manual"),
        static_cast<unsigned long>(activeRemoteId));
}

bool configureRadio()
{
    Serial.println("Initializing SPI: SCK=18 MISO=19 MOSI=23 CSN=5");
    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CSN);
    if (!resetRadio())
    {
        Serial.println("CC1101 communication FAILED (reset timeout)");
        return false;
    }

    uint8_t part = 0xFF, version = 0xFF;
    if (!readRegister(cc1101::PARTNUM, part, true) ||
        !readRegister(cc1101::VERSION, version, true) || version == 0x00 ||
        version == 0xFF)
    {
        Serial.printf("CC1101 PARTNUM=0x%02X VERSION=0x%02X\r\n", part, version);
        Serial.println("CC1101 communication FAILED");
        return false;
    }
    Serial.printf("CC1101 PARTNUM=0x%02X VERSION=0x%02X\r\n", part, version);
    Serial.println("CC1101 communication OK");

    for (const RegisterValue &setting : RADIO_CONFIG)
    {
        if (!writeRegister(setting.address, setting.value))
        {
            Serial.printf("CC1101 configuration FAILED at register 0x%02X\r\n",
                          setting.address);
            return false;
        }
    }

    // ASK/OOK uses PATABLE[0] for carrier OFF and PATABLE[1] for carrier ON.
    // 0x60 is approximately 0 dBm at 433 MHz.
    const uint8_t paTable[8] = {0x00, 0x60, 0, 0, 0, 0, 0, 0};
    if (!writeBurst(cc1101::PATABLE, paTable, sizeof(paTable)))
        return false;

    uint8_t pktctrl0 = 0, mdmcfg2 = 0, freq2 = 0, freq1 = 0, freq0 = 0;
    const bool verified = readRegister(cc1101::PKTCTRL0, pktctrl0) &&
                          readRegister(cc1101::MDMCFG2, mdmcfg2) &&
                          readRegister(cc1101::FREQ2, freq2) &&
                          readRegister(cc1101::FREQ1, freq1) &&
                          readRegister(cc1101::FREQ0, freq0) &&
                          pktctrl0 == 0x32 && mdmcfg2 == 0x30 && freq2 == 0x10 &&
                          freq1 == 0xB0 && freq0 == 0x71;
    if (!verified)
    {
        Serial.println("CC1101 configuration read-back FAILED");
        return false;
    }

    strobe(cc1101::SIDLE);
    Serial.println("CC1101 configured: 433.92 MHz, ASK/OOK, raw asynchronous TX");
    return true;
}

uint8_t checksum(uint32_t remoteId, uint16_t address, uint8_t command)
{
    return static_cast<uint8_t>((remoteId >> 16) + (remoteId >> 8) + remoteId +
                                (address >> 8) + address + command);
}

void sendPulse(uint32_t highUs, uint32_t lowUs)
{
    digitalWrite(PIN_GDO0, HIGH); // Normal polarity: HIGH = RF carrier ON.
    delayMicroseconds(highUs);
    digitalWrite(PIN_GDO0, LOW);
    delayMicroseconds(lowUs);
}

void sendBit(bool value, uint32_t lowOverrideUs = 0)
{
    const uint32_t highUs = value ? ONE_HIGH_US : ZERO_HIGH_US;
    const uint32_t lowUs =
        lowOverrideUs ? lowOverrideUs : (value ? ONE_LOW_US : ZERO_LOW_US);
    sendPulse(highUs, lowUs);
}

void sendFrame(const uint8_t frame[8], bool preamble, bool lastFrame)
{
    if (preamble)
    {
        for (uint8_t i = 0; i < PREAMBLE_BITS; ++i)
            sendBit(false);
    }
    sendPulse(SYNC_HIGH_US, SYNC_LOW_US);
    for (uint8_t byteIndex = 0; byteIndex < 8; ++byteIndex)
    {
        for (int8_t bit = 7; bit >= 0; --bit)
        {
            sendBit(frame[byteIndex] & (1U << bit));
        }
    }
    sendBit(true, lastFrame ? ONE_LOW_US : INTER_FRAME_LOW_US);
}

const char *commandName(uint8_t command)
{
    switch (command)
    {
    case CMD_OPEN:
        return "OPEN";
    case CMD_STOP:
        return "STOP";
    case CMD_CLOSE:
        return "CLOSE";
    case CMD_PROGRAM:
        return "PROGRAM";
    case CMD_AFTER:
        return "AFTER";
    default:
        return "UNKNOWN";
    }
}

bool transmitCommand(uint8_t command, uint32_t remoteId, bool verbose)
{
    if (!radioReady)
    {
        Serial.println("TX blocked: CC1101 is not ready");
        return false;
    }

    const uint8_t crc = checksum(remoteId, ADDRESS, command);
    const uint8_t frame[8] = {
        AOK_START,
        static_cast<uint8_t>(remoteId >> 16),
        static_cast<uint8_t>(remoteId >> 8),
        static_cast<uint8_t>(remoteId),
        static_cast<uint8_t>(ADDRESS >> 8),
        static_cast<uint8_t>(ADDRESS),
        command,
        crc,
    };

    if (verbose)
    {
        Serial.println();
        Serial.println("A-OK TX");
        Serial.printf("Remote ID: 0x%06lX\r\n",
                      static_cast<unsigned long>(remoteId));
        Serial.printf("Address:   0x%04X\r\n", ADDRESS);
        Serial.printf("Command:   0x%02X (%s)\r\n", command, commandName(command));
        Serial.printf("Checksum:  0x%02X\r\n", crc);
        Serial.printf("Frame:     %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
                      frame[0], frame[1], frame[2], frame[3], frame[4], frame[5],
                      frame[6], frame[7]);
        Serial.println("TX state: entering TX; sending 3 frames...");
    }

    digitalWrite(PIN_GDO0, LOW);
    strobe(cc1101::SIDLE);
    waitForState(cc1101::STATE_IDLE, 10000);
    strobe(cc1101::SFTX);
    if (!strobe(cc1101::STX) || !waitForState(cc1101::STATE_TX, 20000))
    {
        digitalWrite(PIN_GDO0, LOW);
        strobe(cc1101::SIDLE);
        Serial.println("TX FAILED: CC1101 did not enter TX state");
        return false;
    }

    sendFrame(frame, true, false);
    sendFrame(frame, false, false);
    sendFrame(frame, false, true);

    digitalWrite(PIN_GDO0, LOW);
    strobe(cc1101::SIDLE);
    if (!waitForState(cc1101::STATE_IDLE, 10000))
    {
        Serial.println("Warning: CC1101 did not report IDLE after TX");
    }
    if (verbose)
    {
        Serial.println("TX done; CC1101 is IDLE");
    }
    return true;
}

void sendCommand(uint8_t command)
{
    transmitCommand(command, activeRemoteId, true);
}

bool consumeEnter()
{
    bool enterReceived = false;
    while (Serial.available())
    {
        const char value = static_cast<char>(Serial.read());
        if (value == '\r' || value == '\n')
        {
            enterReceived = true;
        }
    }
    return enterReceived;
}

int8_t hexDigitValue(char value)
{
    if (value >= '0' && value <= '9')
        return value - '0';
    value = static_cast<char>(tolower(static_cast<unsigned char>(value)));
    if (value >= 'a' && value <= 'f')
        return value - 'a' + 10;
    return -1;
}

bool parseRemoteId(const char *text, size_t length, uint32_t &remoteId)
{
    size_t index = 0;
    if (length >= 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X'))
        index = 2;
    if (index == length)
        return false;

    uint32_t value = 0;
    for (; index < length; ++index)
    {
        const int8_t digit = hexDigitValue(text[index]);
        if (digit < 0 || value > (0xFFFFFFUL - digit) / 16)
            return false;
        value = value * 16 + digit;
    }
    remoteId = value;
    return true;
}

bool readRemoteIdFromSerial(uint32_t &remoteId, const char *prompt)
{
    delay(30);
    while (Serial.available())
    {
        const int value = Serial.peek();
        if (value != '\r' && value != '\n')
            break;
        Serial.read();
    }

    Serial.println();
    Serial.println(prompt);
    Serial.println("Use 000000..FFFFFF; the 0x prefix is optional.");
    Serial.println("Empty Enter cancels:");

    char input[9] = {};
    size_t length = 0;
    bool tooLong = false;
    while (true)
    {
        if (!Serial.available())
        {
            delay(5);
            continue;
        }

        const char value = static_cast<char>(Serial.read());
        if (value == '\r' || value == '\n')
        {
            while (Serial.available())
            {
                const int next = Serial.peek();
                if (next != '\r' && next != '\n')
                    break;
                Serial.read();
            }
            Serial.println();
            if (length == 0)
            {
                Serial.println("Remote ID input cancelled");
                return false;
            }
            if (tooLong || !parseRemoteId(input, length, remoteId))
            {
                Serial.println("Invalid ID. Use 000000..FFFFFF, then retry the command.");
                return false;
            }
            return true;
        }

        if (value == '\b' || value == 0x7F)
        {
            if (length > 0)
            {
                --length;
                Serial.print("\b \b");
            }
            continue;
        }

        if (length < sizeof(input) - 1)
        {
            input[length++] = value;
            Serial.write(value);
        }
        else
        {
            tooLong = true;
        }
    }
}

void runIdSearch()
{
    if (!radioReady)
    {
        Serial.println("ID search blocked: CC1101 is not ready");
        return;
    }
    if (idSearchComplete)
    {
        Serial.println("ID SEARCH already tested the entire 24-bit range");
        return;
    }

    // Discard the line ending that may have arrived together with the 'f'
    // command. Subsequent Enter/newline characters stop the search.
    delay(30);
    while (Serial.available())
    {
        Serial.read();
    }

    Serial.println();
    Serial.printf("ID SEARCH starting at 0x%06lX\r\n",
                  static_cast<unsigned long>(nextSearchRemoteId));
    Serial.println("Sending OPEN with each 24-bit remote ID.");
    Serial.println("Press Enter to stop.");

    uint32_t remoteId = nextSearchRemoteId;
    while (true)
    {
        Serial.printf("ID search OPEN: 0x%06lX\r\n",
                      static_cast<unsigned long>(remoteId));

        if (!transmitCommand(CMD_OPEN, remoteId, false))
        {
            Serial.println("ID SEARCH aborted because TX failed");
            return;
        }

        lastTriedRemoteId = remoteId;
        idSearchHasTried = true;
        if (remoteId < 0xFFFFFF)
        {
            nextSearchRemoteId = remoteId + 1;
        }

        const uint32_t waitStarted = millis();
        bool stopRequested = false;
        while (millis() - waitStarted < ID_SEARCH_OBSERVE_MS)
        {
            if (consumeEnter())
            {
                stopRequested = true;
                break;
            }
            delay(5);
        }

        if (stopRequested)
        {
            Serial.printf("ID SEARCH stopped. Last transmitted ID: 0x%06lX\r\n",
                          static_cast<unsigned long>(lastTriedRemoteId));
            Serial.printf("Next search will continue at: 0x%06lX\r\n",
                          static_cast<unsigned long>(nextSearchRemoteId));
            return;
        }

        if (remoteId == 0xFFFFFF)
        {
            idSearchComplete = true;
            Serial.println("ID SEARCH complete: entire 24-bit range tested");
            return;
        }
        remoteId = nextSearchRemoteId;
    }
}

void runIdSearchFromPrompt()
{
    uint32_t startId = 0;
    if (!readRemoteIdFromSerial(
            startId, "Enter ID SEARCH start ID in hexadecimal:"))
        return;

    nextSearchRemoteId = startId;
    idSearchHasTried = false;
    idSearchComplete = false;
    runIdSearch();
}

void setActiveRemoteIdFromPrompt()
{
    uint32_t remoteId = 0;
    if (!readRemoteIdFromSerial(
            remoteId, "Enter active remote ID in hexadecimal:"))
        return;

    activeRemoteId = remoteId;
    Serial.printf("Active remote ID set manually: 0x%06lX\r\n",
                  static_cast<unsigned long>(activeRemoteId));
}

void printHelp()
{
    Serial.println();
    Serial.println("Commands:");
    Serial.printf("  1 = select existing remote (0x%06X)\r\n", EXISTING_REMOTE_ID);
    Serial.printf("  2 = select new window opener (0x%06X)\r\n", NEW_REMOTE_ID);
    Serial.printf(
        "  Active remote: %s (0x%06lX)\r\n",
        activeRemoteId == EXISTING_REMOTE_ID
            ? "existing"
            : (activeRemoteId == NEW_REMOTE_ID ? "new" : "manual"),
        static_cast<unsigned long>(activeRemoteId));
    if (idSearchHasTried)
    {
        Serial.printf("  Last tried search ID: 0x%06lX\r\n",
                      static_cast<unsigned long>(lastTriedRemoteId));
        Serial.printf("  Next search ID:       0x%06lX\r\n",
                      static_cast<unsigned long>(nextSearchRemoteId));
    }
    else
    {
        Serial.println("  Last tried search ID: none");
        Serial.printf("  Next search ID:       0x%06lX\r\n",
                      static_cast<unsigned long>(nextSearchRemoteId));
    }
    Serial.println("  o = OPEN / UP");
    Serial.println("  s = STOP");
    Serial.println("  c = CLOSE / DOWN");
    Serial.println("  p = PROGRAM");
    Serial.println("  a = AFTER (debug only; never sent automatically)");
    Serial.println("  r = set active remote ID manually");
    Serial.println("  f = start/resume ID SEARCH; press Enter to stop");
    Serial.println("  F = enter hexadecimal start ID, then start searching");
    Serial.println("  i = CC1101 information/status");
    Serial.println("  h or ? = help");
}

void setup()
{
    pinMode(PIN_CSN, OUTPUT);
    digitalWrite(PIN_CSN, HIGH);
    pinMode(PIN_GDO0, OUTPUT);
    digitalWrite(PIN_GDO0, LOW);
    pinMode(PIN_GDO2, INPUT);

    Serial.begin(115200);
    delay(1000);
    Serial.println();
    Serial.println("ESP32 + CC1101 A-OK raw OOK transmitter test");

    radioReady = configureRadio();
    printHelp();
}

void loop()
{
    if (!Serial.available())
    {
        delay(10);
        return;
    }

    const char received = static_cast<char>(Serial.read());
    if (received == 'F')
    {
        runIdSearchFromPrompt();
        return;
    }

    const char command =
        static_cast<char>(tolower(static_cast<unsigned char>(received)));
    switch (command)
    {
    case '1':
        activeRemoteId = EXISTING_REMOTE_ID;
        Serial.println("Selected existing remote: 0x000984");
        break;
    case '2':
        activeRemoteId = NEW_REMOTE_ID;
        Serial.println("Selected new window opener remote: 0xDEADBE");
        Serial.println("Put the new receiver in pairing mode, then press p.");
        break;
    case 'o':
        sendCommand(CMD_OPEN);
        break;
    case 's':
        sendCommand(CMD_STOP);
        break;
    case 'c':
        sendCommand(CMD_CLOSE);
        break;
    case 'p':
        sendCommand(CMD_PROGRAM);
        break;
    case 'a':
        sendCommand(CMD_AFTER);
        break;
    case 'r':
        setActiveRemoteIdFromPrompt();
        break;
    case 'f':
        runIdSearch();
        break;
    case 'i':
        printRadioInfo();
        break;
    case 'h':
    case '?':
        printHelp();
        break;
    case '\r':
    case '\n':
        break;
    default:
        Serial.printf("Unknown command '%c'. Press h for help.\r\n", command);
        break;
    }
}
