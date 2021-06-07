#define LEN(arr) (sizeof(arr) / sizeof(arr[0]))

#define MI 10
#define CLK 11
#define RI 12
#define EEPROM_CE 13
#define RST A0

#define MASK(pin) (1 << (pin - 8))

const byte instructions[] = {
  0b00100100,
  0b11100000,
  0b01100000,
  0b11110000,
  0b00000001
};

void setup() {
  // Set pins to output
  DDRD |= 0b11111100;                                                     // D2 - D7
  DDRB = 0b00000011 | MASK(EEPROM_CE) | MASK(MI) | MASK(RI) | MASK(CLK);  // D8 - D9, RI, CLK, MI, EEPROM_CE

  // Disable ucode EEPROMs
  digitalWrite(EEPROM_CE, HIGH);

  for (int i = 0; i < LEN(instructions); i++) {
    setAddress(i);
    writeRAM(instructions[i]);
  }

  // Set pins to floating(input)
  DDRD &= 0b00000011;
  DDRB &= MASK(EEPROM_CE);

  // Reset the computer
  pinMode(RST, OUTPUT);
  pulse(RST, HIGH);

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
  digitalWrite(MI, HIGH);
  pulse(CLK, HIGH);
  digitalWrite(MI, LOW);
}

void writeRAM(byte value) {
  PORTD = value << 2;
  PORTB = (value & 0b11000000) >> 6 | MASK(EEPROM_CE);

  digitalWrite(RI, HIGH);
  pulse(CLK, HIGH);
  digitalWrite(RI, LOW);
}
