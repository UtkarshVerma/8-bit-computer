#define LEN(arr) (sizeof(arr) / sizeof(arr[0]))

#define MI 10
#define CLK 11
#define RI 12
#define HLT 13
#define RST A0
#define EEPROM_CE A1

#define MASK(pin) (1 << (pin - 8))

// Fibonacci program
const byte instructions[] = {
  0x50,
  0x4e,
  0x51,
  0x4f,
  0x1e,
  0x4d,
  0x1f,
  0x4e,
  0x2d,
  0x4f,
  0x70,
  0xe0,
  0x64,
  0xf0
};

void setup() {
  // Set pins to output
  DDRD |= 0b11111100;                                               // D2 - D7
  DDRB = 0b00000011 | MASK(HLT) | MASK(MI) | MASK(RI) | MASK(CLK);  // D8 - D9, RI, CLK, MI, HLT
  pinMode(RST, OUTPUT);
  pinMode(EEPROM_CE, OUTPUT);

  // Halt the computer
  digitalWrite(HLT, HIGH);

  // Disable ucode EEPROMs
  digitalWrite(EEPROM_CE, HIGH);

  // Write instructions to RAM
  for (int i = 0; i < LEN(instructions); i++) {
    setAddress(i);
    writeRAM(instructions[i]);
  }

  // Reset the computer
  pulse(RST, HIGH);

  // Unhalt the computer
  digitalWrite(HLT, LOW);

  // Set pins to input/floating
  DDRD &= 0b00000011;  // D2 - D7
  DDRB = 0;            // D8 - D13
  pinMode(RST, INPUT);

  // Enable ucode EEPROMs
  digitalWrite(EEPROM_CE, LOW);
}

void loop() {}

void pulse(int pin, bool signal) {
  digitalWrite(pin, signal);
  delay(100);
  digitalWrite(pin, !signal);
}

void setAddress(byte address) {
  PORTD = address << 2;
  PORTB &= MASK(HLT);

  digitalWrite(MI, HIGH);
  pulse(CLK, HIGH);
  digitalWrite(MI, LOW);
}

void writeRAM(byte value) {
  PORTD = value << 2;
  PORTB = (value >> 6) | (PORTB & MASK(HLT));

  digitalWrite(RI, HIGH);
  pulse(CLK, HIGH);
  digitalWrite(RI, LOW);
}
