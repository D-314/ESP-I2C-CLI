#include <Wire.h>

// Default I2C address and register address
uint8_t defaultI2CAddress = 0x00;
uint8_t defaultRegAddress = 0x00;

// Output format flags
bool hexOutput = true;
bool binOutput = false;

// Arguments array to store parsed arguments from the command
uint8_t argsArray[10]; // The first element is the number of arguments

void setup() {
  Serial.begin(115200); // Initialize serial communication at 115200 bits per second
  Wire.begin(); // Initialize the I2C protocol
  Serial.println("I2C CLI Ready"); // Print a ready message to the serial console
}

void loop() {
  // Check if data has been sent to the serial port
  if (Serial.available()) {
    String commandLine = Serial.readStringUntil('\n'); // Read the incoming string until newline
    commandLine.trim(); // Remove any whitespace
    processCommand(commandLine); // Process the command line input
  }
}

// Process and execute commands based on the input
void processCommand(String commandLine) {
  commandLine.trim(); // Trim any leading or trailing whitespace
  int firstSpace = commandLine.indexOf(' '); // Find the index of the first space
  String command = commandLine.substring(0, firstSpace); // Extract the command from the input
  String args = "";
  if (firstSpace != -1) {
    args = commandLine.substring(firstSpace + 1); // Extract arguments if any
    args.trim(); // Trim arguments
  }

  parseArgs(args); // Parse arguments into the argsArray

  // Conditional statements to handle different commands
  if (command == "addr") {
    // If no arguments are provided, read the current I2C address variable
    if (argsArray[0] == 0) {
      readAddr();
    // If at least one argument is provided, set the I2C address variable
    } else if (argsArray[0] >= 1) {
      setAddr(argsArray[1]);
    }
  } else if (command == "reg") {
    // If no arguments are provided, read the current register address variable
    if (argsArray[0] == 0) {
      readReg();
    // If at least one argument is provided, set the register address variable
    } else if (argsArray[0] >= 1) {
      setReg(argsArray[1]);
    }
  } else if (command == "bit") {
    // For setting or reading a single bit, check the number of arguments
    if (argsArray[0] == 1) {
      // If one argument is provided, read the bit at position
      readBit(defaultI2CAddress, defaultRegAddress, argsArray[1]);
    } else if (argsArray[0] >= 2) {
      // If two arguments are provided, set the bit at position to value
      setBit(defaultI2CAddress, defaultRegAddress, argsArray[1], argsArray[2]);
    }
  } else if (command == "byte") {
    // Handle byte read/write commands based on the number of arguments
    if (argsArray[0] == 0) {
      readByte(defaultI2CAddress, defaultRegAddress);
    } else if (argsArray[0] >= 1) {
      setByte(defaultI2CAddress, defaultRegAddress, argsArray[1]);
    }
  } else if (command == "map") {
    // Handle mapping commands to read a range of registers
    if (argsArray[0] == 0) {
      readMap(defaultI2CAddress, 0x00, 0xFF);  // Read all registers if no arguments are provided
    } else if (argsArray[0] == 1) {
      readMap(defaultI2CAddress, 0x00, argsArray[1]);  // Read registers up to the specified end if one argument is provided
    } else if (argsArray[0] >= 2) {
      readMap(defaultI2CAddress, argsArray[1], argsArray[2]);  // Read a specific range of registers if two arguments are provided
    }
  } else if (command == "scan") {
    // Scan the I2C bus for devices
    command_scan();
  } else if (command == "hex") {
    // Set the output format to hexadecimal
    hexOutput = true;
    binOutput = false;
    Serial.println("Output format set to HEX.");
  } else if (command == "bin") {
    // Set the output format to binary
    hexOutput = false;
    binOutput = true;
    Serial.println("Output format set to BIN.");
  } else if (command == "bits") {
    // Handle multi-bit read/write commands based on the number of arguments
    if (argsArray[0] == 3) {
      // If three arguments are provided, set multiple bits from start to end positions to value
      setBits(defaultI2CAddress, defaultRegAddress, argsArray[1], argsArray[2], argsArray[3]);
    } else if (argsArray[0] == 2) {
      // If two arguments are provided, read multiple bits from start to end positions
      readBits(defaultI2CAddress, defaultRegAddress, argsArray[1], argsArray[2]);
    } else {
      Serial.println("Invalid number of arguments for bits command.");
    }
  } else if (command == "help") {
    // Display help information
    printHelp();
  } else {
    Serial.println("Unknown command. Type 'help' for list of commands.");
  }
}

// Parses the command line arguments and stores them in a global array
void parseArgs(String args) {
  argsArray[0] = 0;  // Reset the count of arguments
  int argCount = 0;  // Initialize argument count

  // Only parse if there are arguments present
  if (args.length() > 0) {
    int lastSpace = -1;  // Track the position of the last space character
    // Loop through all spaces to find and parse each argument
    while (lastSpace < (int)args.length()) {
      int spaceIndex = args.indexOf(' ', lastSpace + 1); // Find the next space character
      if (spaceIndex == -1) spaceIndex = args.length();  // If no more spaces, set to the end of the string
      // Check if there is a valid argument between spaces
      if (lastSpace + 1 < spaceIndex) {
        // Extract the argument as a substring
        String argStr = args.substring(lastSpace + 1, spaceIndex);
        // Convert the argument string to a number
        uint8_t arg = parseNumber(argStr);
        // Store the argument in the array if within limits
        if (argCount < 9) {
          argsArray[argCount + 1] = arg;
        }
        argCount++; // Increment the argument count
      }
      lastSpace = spaceIndex; // Move to the next position
    }
  }
  argsArray[0] = argCount; // Store the count of parsed arguments in the first element

  // Warn if more than 8 arguments are provided
  if (argCount > 9) {
    Serial.println("Warning: More than 8 arguments are not supported.");
  }
}

// Parses a string to a number, supporting decimal, hexadecimal, and binary formats
uint8_t parseNumber(String str) {
  // Check if the string is in hexadecimal format
  if (str.startsWith("0x")) {
    return (uint8_t)strtol(str.c_str(), NULL, 16); // Parse as hexadecimal
  } else if (str.startsWith("0b")) {
    // Check if the string is in binary format
    return (uint8_t)strtol(str.c_str() + 2, NULL, 2); // Parse as binary
  } else {
    // Otherwise, parse as decimal
    return (uint8_t)str.toInt();
  }
}

// Prints a value with a prefix message in the specified output format
void printValue(String prefix, uint8_t value) {
  Serial.print(prefix); // Print the prefix message
  if (hexOutput) {
    // If hex output is enabled, print the value in hexadecimal format
    Serial.print("0x");
    if (value < 16) Serial.print("0"); // Add leading zero for values less than 16
    Serial.print(value, HEX);
  } else if (binOutput) {
    // If binary output is enabled, print the value in binary format
    Serial.print("0b");
    // Iterate over each bit to construct the binary string
    for (int i = 7; i >= 0; i--) {
      Serial.print((value & (1 << i)) ? '1' : '0');
    }
  } else {
    // Fallback to decimal format if neither hex nor bin is set
    Serial.print
    (value);
  }
  Serial.println();
}

void requestByte (uint8_t i2cAddress, uint8_t regAddress) {
  Wire.beginTransmission(i2cAddress);
  Wire.write(regAddress);
  Wire.endTransmission(false);
  Wire.requestFrom((int)i2cAddress, 1);
}

void sendByte (uint8_t i2cAddress, uint8_t regAddress, uint8_t regValue) {
    Wire.beginTransmission(i2cAddress);
    Wire.write(regAddress);
    Wire.write(regValue);
    Wire.endTransmission();
}

void setBit(uint8_t i2cAddress, uint8_t regAddress, uint8_t bitPosition, bool bitState) {
  requestByte (i2cAddress, regAddress);

  if (Wire.available()) {
    uint8_t regValue = Wire.read();
    regValue = bitState ? (regValue | (1 << bitPosition)) : (regValue & ~(1 << bitPosition));
    sendByte (i2cAddress, regAddress, regValue);
    readBit(i2cAddress, regAddress, bitPosition);
  }
}

void readBit(uint8_t i2cAddress, uint8_t regAddress, uint8_t bitPosition) {
  requestByte (i2cAddress, regAddress);
  if (Wire.available()) {
    uint8_t regValue = Wire.read();
    bool bitState = regValue & (1 << bitPosition);
    Serial.println(bitState ? "1" : "0");
  }
}

void setBits(uint8_t i2cAddress, uint8_t regAddress, uint8_t startPos, uint8_t endPos, uint8_t val) {
  requestByte (i2cAddress, regAddress);
  if (Wire.available()) {
    uint8_t regValue = Wire.read();
    uint8_t mask = ((1 << (endPos - startPos + 1)) - 1) << startPos;
    regValue = (regValue & ~mask) | ((val << startPos) & mask);
    sendByte (i2cAddress, regAddress, regValue);
  }
  readBits(i2cAddress, regAddress, startPos, endPos);
}

void readBits(uint8_t i2cAddress, uint8_t regAddress, uint8_t startPos, uint8_t endPos) {
  requestByte (i2cAddress, regAddress);
  if (Wire.available()) {
    uint8_t regValue = Wire.read();
    uint8_t mask = ((1 << (endPos - startPos + 1)) - 1);
    uint8_t bits = (regValue >> startPos) & mask;
    printValue("", bits);
  }
}

void setByte(uint8_t i2cAddress, uint8_t regAddress, uint8_t regValue) {
  sendByte (i2cAddress, regAddress, regValue);
  readByte(i2cAddress, regAddress);
}

void readByte(uint8_t i2cAddress, uint8_t regAddress) {
  requestByte (i2cAddress, regAddress);
  if (Wire.available()) {
    uint8_t regValue = Wire.read();
    printValue("", regValue);
  }
}

void setAddr(uint8_t i2cAddress) {
  defaultI2CAddress = i2cAddress;
  readAddr();
}

void readAddr() {
  printValue("", defaultI2CAddress);
}

void setReg(uint8_t regAddress) {
  defaultRegAddress = regAddress;
  readReg();
}

void readReg() {
  printValue("", defaultRegAddress);
}

void command_scan() {
  uint8_t foundCount = 0;
  for (uint8_t address = 1; address < 128; ++address) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      foundCount++;
      printValue(" ", address);
    }
  }
  Serial.print("Total I2C devices found: ");
  Serial.println(foundCount);
}

void readMap(uint8_t i2cAddress, uint8_t startAddress, uint8_t endAddress) {
  Serial.println("Register map:");
  for (uint16_t regAddress = startAddress; regAddress <= endAddress; regAddress++) {
    requestByte (i2cAddress, regAddress);
    if (Wire.available()) {
      uint8_t value = Wire.read();
      Serial.print("0x");
      if (regAddress < 16) Serial.print("0");
      Serial.print(regAddress, HEX);
      printValue(": ",value);
    }
  }
}

void printHelp() {
  Serial.println("Available commands:");
  Serial.println("  addr [i2c_address] - Get or set the I2C address");
  Serial.println("  reg [register_address] - Get or set the register address");
  Serial.println("  bit [position] [value] - Get or set the bit at the position");
  Serial.println("  byte [value] - Get or set the byte at the current register");
  Serial.println("  bits [start pos] [end pos] [val] - Get or set multiple bits from start to end positions");
  Serial.println("  map [start] [end] - Show a map of registers from start to end");
  Serial.println("  scan - Scan the I2C bus for devices");
  Serial.println("  hex - Set output format to HEX");
  Serial.println("  bin - Set output format to BIN");
  Serial.println("  help - Show this message");
}
