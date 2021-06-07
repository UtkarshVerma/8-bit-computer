#define LEN(arr) (sizeof(arr) / sizeof(arr[0]))

// Pin definitions
#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_EN 13

// Control signals
#define HLT 1 << 15  // Halt clock
#define MI 1 << 14   // Memory address register in
#define RI 1 << 13   // RAM data in
#define RO 1 << 12   // RAM data out
#define IO 1 << 11   // Instruction register out
#define II 1 << 10   // Instruction register in
#define AI 1 << 9    // A register in
#define AO 1 << 8    // A register out
#define EO 1 << 7    // ALU out
#define SUB 1 << 6   // ALU subtract
#define BI 1 << 5    // B register in
#define OI 1 << 4    // Output register in
#define CE 1 << 3    // Program counter enable
#define CO 1 << 2    // Program counter out
#define J 1 << 1     // Jump (program counter in)
#define FI 1         // Flag in

const byte dataMask = 1 << SHIFT_DATA;
const byte clkMask = 1 << SHIFT_CLK;
const byte latchMask = 1 << SHIFT_LATCH;

const PROGMEM word ucodeTemplate[16][8] = {
  { MI | CO, RO | II | CE, 0 },                                        // 0000 - NOP
  { MI | CO, RO | II | CE, IO | MI, RO | AI, 0 },                      // 0001 - LDA
  { MI | CO, RO | II | CE, IO | MI, RO | BI, EO | AI | FI, 0 },        // 0010 - ADD
  { MI | CO, RO | II | CE, IO | MI, RO | BI, EO | AI | SUB | FI, 0 },  // 0011 - SUB
  { MI | CO, RO | II | CE, IO | MI, AO | RI, 0 },                      // 0100 - STA
  { MI | CO, RO | II | CE, IO | AI, 0 },                               // 0101 - LDI
  { MI | CO, RO | II | CE, IO | J, 0 },                                // 0110 - JMP
  { MI | CO, RO | II | CE, 0 },                                        // 0111 - JC
  { MI | CO, RO | II | CE, 0 },                                        // 1000 - JZ
  { MI | CO, RO | II | CE, 0 },                                        // 1001
  { MI | CO, RO | II | CE, 0 },                                        // 1010
  { MI | CO, RO | II | CE, 0 },                                        // 1011
  { MI | CO, RO | II | CE, 0 },                                        // 1100
  { MI | CO, RO | II | CE, 0 },                                        // 1101
  { MI | CO, RO | II | CE, AO | OI, 0 },                               // 1110 - OUT
  { MI | CO, RO | II | CE, HLT, 0 },                                   // 1111 - HLT
};

word ucode[4][16][8];

void setup() {
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  disableWrite();
  pinMode(WRITE_EN, OUTPUT);
  Serial.begin(57600);
  initUcode();

  Serial.print("Disabling EEPROM Software Data Protection(SDP)...");
  disableSoftwareWriteProtect();
  Serial.println(" done");

  Serial.print("Programming EEPROM");
  for (int address = 0; address < 1024; address++) {
    int flags = (address & 0b1100000000) >> 8;
    int byteSel = (address & 0b0010000000) >> 7;
    int instruction = (address & 0b0001111000) >> 3;
    int step = address & 0b0000000111;

    writeEEPROM(address, ucode[flags][instruction][step] >> (!byteSel * 8));
    if (address % 64 == 0) Serial.print(".");
  }
  Serial.println(" done");

  Serial.println("Reading EEPROM");
  dumpEEPROM(0, 1024);
}

void loop() {}


void initUcode() {
  for (byte flags = 0; flags < LEN(ucode); flags++) {
    byte CF = flags & 0b01;
    byte ZF = flags >> 1;

    memcpy_P(ucode[flags], ucodeTemplate, sizeof(ucodeTemplate));

    if (CF) ucode[flags][0b0111][2] = IO | J;
    if (ZF) ucode[flags][0b1000][2] = IO | J;
  }
}

void enableWrite() {
  digitalWrite(WRITE_EN, LOW);
}

void disableWrite() {
  digitalWrite(WRITE_EN, HIGH);
}

// Set an address and data value and toggle the write control. This is used
// to write control sequences, like the software write protect. This is not a
// complete byte write function because it does not set the chip enable or the
// mode of the data bus.
void setByte(byte value, word address) {
  setAddress(address, false);
  writeDataBus(value);

  delayMicroseconds(1);
  enableWrite();
  delayMicroseconds(1);
  disableWrite();
}

// Set the I/O state of the data bus.
// The 8 bits data bus are is on pins D5..D12.
void setDataBusMode(uint8_t mode) {
  // On the Uno and Nano, D5..D12 maps to the upper 3 bits of port D and the
  // lower 5 bits of port B.
  if (mode == OUTPUT) {
    DDRB |= 0x1f;
    DDRD |= 0xe0;
  } else {
    DDRB &= 0xe0;
    DDRD &= 0x1f;
  }
}

// Read a byte from the data bus. The caller must set the bus to input_mode
// before calling this or no useful data will be returned.
byte readDataBus() {
  return (PINB << 3) | (PIND >> 5);
}

// Write a byte to the data bus. The caller must set the bus to output_mode
// before calling this or no data will be written.
void writeDataBus(byte data) {
  PORTB = (PORTB & 0xe0) | (data >> 3);
  PORTD = (PORTD & 0x1f) | (data << 5);
}

void disableSoftwareWriteProtect() {
  disableWrite();
  setDataBusMode(OUTPUT);

  setByte(0xaa, 0x1555);
  setByte(0x55, 0x0aaa);
  setByte(0x80, 0x1555);
  setByte(0xaa, 0x1555);
  setByte(0x55, 0x0aaa);
  setByte(0x20, 0x1555);

  setDataBusMode(INPUT);
  delay(10);
}

void enableSoftwareWriteProtect() {
  disableWrite();
  setDataBusMode(OUTPUT);

  setByte(0xaa, 0x1555);
  setByte(0x55, 0x0aaa);
  setByte(0xa0, 0x1555);

  setDataBusMode(INPUT);
  delay(10);
}

// Output the address bits and outputEnable signal using shift registers
void setAddress(int address, bool outputEnable) {
  address |= outputEnable ? 0 : 0x8000;

  // Make sure the clock is low to start.
  PORTD &= ~clkMask;

  // Shift 16 bits in, starting with the MSB.
  for (byte i = 0; i < 16; i++) {
    // Set the data bit
    if (address & 0x8000) {
      PORTD |= dataMask;
    } else {
      PORTD &= ~dataMask;
    }

    // Toggle the clock high then low
    PORTD |= clkMask;
    delayMicroseconds(3);
    PORTD &= ~clkMask;
    address <<= 1;
  }

  // Latch the shift register contents into the output register by pulsing SHIFT_LATCH high
  PORTD &= ~latchMask;
  delayMicroseconds(1);
  PORTD |= latchMask;
  delayMicroseconds(1);
  PORTD &= ~latchMask;
}

byte readEEPROM(word address) {
  setDataBusMode(INPUT);
  setAddress(address, true);
  return readDataBus();
}

void writeEEPROM(word address, byte data) {
  setAddress(address, false);
  setDataBusMode(OUTPUT);
  writeDataBus(data);
  enableWrite();
  delayMicroseconds(1);
  disableWrite();

  const byte msb = data >> 7;
  while ((readEEPROM(address) >> 7) != msb) delay(1);
}

void dumpEEPROM(unsigned int start, unsigned int length) {
  for (int base = start; base < length; base += 16) {
    byte data[16];
    for (int offset = 0; offset < 16; offset++)
      data[offset] = readEEPROM(base + offset);
    char buf[80];
    sprintf(buf, "%03x: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
            base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
            data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);

    Serial.println(buf);
  }
}
