#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <limits.h>

// Deklarasi enum dan tipe data
enum SystemMode {MODE_IDLE, MODE_A, MODE_B, MODE_C, MODE_D};

// Deklarasi semua fungsi
void showSplashScreen();
void showMainScreen();
void handleSwitchPress();
void runSystem();
void startMode(SystemMode mode);
void determineNextMode();
void updateRunningDisplay();
void startInputMode(char mode);
void processDigitInput(char key);
void saveInput();
void resetAll();
void readSwitch();
void handleKeypadInput(char key);
char getModeChar(SystemMode mode);
unsigned int getModeTime(SystemMode mode);
void printSeconds(unsigned int milliseconds);

// Inisialisasi LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2); // Ganti ke 0x3F jika LCD tidak bekerja

// Konfigurasi Keypad
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {22, 24, 26, 28};
byte colPins[COLS] = {30, 32, 34, 36};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Pin Relay dan Saklar
const int relayPin = 2;
const int switchPin = 3;

// Variabel waktu dalam milidetik
unsigned int timeA = 1000;  // Default 1.0 detik
unsigned int timeB = 1300;  // Default 1.3 detik
unsigned int timeC = 900;   // Default 0.9 detik
unsigned int timeD = 0;     // Default 0 ms (dilewati)

// Variabel mode
SystemMode currentMode = MODE_IDLE;
SystemMode nextMode = MODE_A;
bool systemRunning = false;
bool inputMode = false;
char currentInput = ' ';

// Variabel input waktu
String inputValue = "";
unsigned int tempValue = 0;

// Timing
unsigned long previousMillis = 0;
unsigned long interval = 0;

// Variabel debounce saklar
int switchState = HIGH;
int lastSwitchState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// Fungsi untuk menghitung selisih waktu dengan aman
unsigned long safeMillisDiff(unsigned long now, unsigned long previous) {
  return (now >= previous) ? (now - previous) : (ULONG_MAX - previous + now + 1);
}

void setup() {
  Serial.begin(9600);
  
  // Inisialisasi relay
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH); // Relay mati awal
  
  // Saklar dengan pullup internal
  pinMode(switchPin, INPUT_PULLUP);
  
  // Inisialisasi LCD
  lcd.init();
  lcd.backlight();
  lcd.begin(16, 2);
  
  // Tampilkan splash screen
  showSplashScreen();
  
  // Tampilkan layar utama
  showMainScreen();
}

void loop() {
  static unsigned long lastKeypadTime = 0;
  const unsigned long keypadInterval = 50; // Interval baca keypad
  
  // Baca keypad dengan interval
  if (millis() - lastKeypadTime >= keypadInterval) {
    lastKeypadTime = millis();
    char key = keypad.getKey();
    
    if (key) {
      handleKeypadInput(key);
    }
  }
  
  // Baca saklar dengan debounce
  readSwitch();
  
  // Jalankan sistem jika aktif
  if (systemRunning) {
    runSystem();
  }
}

void showSplashScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  Timer Multi");
  lcd.setCursor(0, 1);
  lcd.print("   CONTROLLER");
  delay(2000);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("    BY AGUS F");
  lcd.setCursor(0, 1);
  lcd.print("  10 JULI 2025");
  delay(2000);
}

void showMainScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  
  if (systemRunning) {
    updateRunningDisplay();
  } else {
    lcd.print("Sistem Ready");
    lcd.setCursor(0, 1);
    lcd.print("Tekan Saklar");
  }
}

void readSwitch() {
  int reading = digitalRead(switchPin);
  
  if (reading != lastSwitchState) {
    lastDebounceTime = millis();
  }
  
  if (safeMillisDiff(millis(), lastDebounceTime) > debounceDelay) {
    if (reading != switchState) {
      switchState = reading;
      
      if (switchState == LOW) {
        handleSwitchPress();
      }
    }
  }
  lastSwitchState = reading;
}

void handleSwitchPress() {
  if (!systemRunning) {
    // Tentukan mode berikutnya
    switch (currentMode) {
      case MODE_IDLE: nextMode = MODE_A; break;
      case MODE_A: nextMode = MODE_B; break;
      case MODE_B: nextMode = MODE_C; break;
      case MODE_C: nextMode = MODE_D; break;
      case MODE_D: nextMode = MODE_A; break;
    }
    
    // Skip mode dengan waktu 0 ms
    while (true) {
      if (nextMode == MODE_A && timeA > 0) break;
      if (nextMode == MODE_B && timeB > 0) break;
      if (nextMode == MODE_C && timeC > 0) break;
      if (nextMode == MODE_D && timeD > 0) break;
      
      // Jika semua mode 0, set minimal A = 100 ms
      if (timeA == 0 && timeB == 0 && timeC == 0 && timeD == 0) {
        timeA = 100;
        nextMode = MODE_A;
        break;
      }
      
      // Pindah ke mode berikutnya
      nextMode = (SystemMode)((nextMode + 1) % (MODE_D + 1));
      if (nextMode == MODE_IDLE) nextMode = MODE_A;
    }
    
    startMode(nextMode);
  }
}

void startMode(SystemMode mode) {
  currentMode = mode;
  systemRunning = true;
  
  determineNextMode();
  
  switch (mode) {
    case MODE_A: interval = timeA; break;
    case MODE_B: interval = timeB; break;
    case MODE_C: interval = timeC; break;
    case MODE_D: interval = timeD; break;
    default: return;
  }
  
  digitalWrite(relayPin, LOW); // Relay aktif
  previousMillis = millis();
  
  updateRunningDisplay();
}

void determineNextMode() {
  switch (currentMode) {
    case MODE_A: 
      nextMode = timeB > 0 ? MODE_B : 
                timeC > 0 ? MODE_C : 
                timeD > 0 ? MODE_D : MODE_A;
      break;
    case MODE_B: 
      nextMode = timeC > 0 ? MODE_C : 
                timeD > 0 ? MODE_D : 
                timeA > 0 ? MODE_A : MODE_B;
      break;
    case MODE_C: 
      nextMode = timeD > 0 ? MODE_D : 
                timeA > 0 ? MODE_A : 
                timeB > 0 ? MODE_B : MODE_C;
      break;
    case MODE_D: 
      nextMode = timeA > 0 ? MODE_A : 
                timeB > 0 ? MODE_B : 
                timeC > 0 ? MODE_C : MODE_D;
      break;
    default:
      nextMode = MODE_A;
  }
}

void runSystem() {
  unsigned long currentMillis = millis();
  unsigned long elapsed = safeMillisDiff(currentMillis, previousMillis);
  
  if (elapsed >= interval) {
    digitalWrite(relayPin, HIGH); // Matikan relay
    
    // Proteksi relay
    unsigned long relayOffTime = millis();
    while(digitalRead(relayPin) != HIGH) {
      if (safeMillisDiff(millis(), relayOffTime) > 1000) {
        resetAll();
        return;
      }
    }
    
    systemRunning = false;
    showModeComplete();
  }
}

void updateRunningDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Running ");
  lcd.print(getModeChar(currentMode));
  
  lcd.setCursor(0, 1);
  lcd.print("Next: ");
  lcd.print(getModeChar(nextMode));
  lcd.print(" (");
  printSeconds(getModeTime(nextMode));
  lcd.print(")");
}

void showModeComplete() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(getModeChar(currentMode));
  lcd.print(" Selesai");
  
  lcd.setCursor(0, 1);
  lcd.print(getModeChar(nextMode));
  lcd.print(" Ready (");
  printSeconds(getModeTime(nextMode));
  lcd.print(")");
}

void printSeconds(unsigned int milliseconds) {
  int seconds = milliseconds / 1000;
  int fraction = (milliseconds % 1000) / 100;
  
  lcd.print(seconds);
  if (fraction > 0) {
    lcd.print(".");
    lcd.print(fraction);
  }
  lcd.print("s");
}

unsigned int getModeTime(SystemMode mode) {
  switch(mode) {
    case MODE_A: return timeA;
    case MODE_B: return timeB;
    case MODE_C: return timeC;
    case MODE_D: return timeD;
    default: return 0;
  }
}

char getModeChar(SystemMode mode) {
  switch(mode) {
    case MODE_A: return 'A';
    case MODE_B: return 'B';
    case MODE_C: return 'C';
    case MODE_D: return 'D';
    default: return ' ';
  }
}

void handleKeypadInput(char key) {
  if (key == '*' && !inputMode) {
    resetAll();
  } 
  else if (key == '#' && inputMode) {
    saveInput();
  } 
  else if (inputMode && isdigit(key)) {
    processDigitInput(key);
  } 
  else if (!inputMode && (key == 'A' || key == 'B' || key == 'C' || key == 'D')) {
    startInputMode(key);
  }
}

void startInputMode(char mode) {
  inputMode = true;
  currentInput = mode;
  inputValue = "";
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set ");
  lcd.print(mode);
  lcd.print(" (ms)");
  lcd.setCursor(0, 1);
  lcd.print("Masukkan angka");
}

void processDigitInput(char key) {
  inputValue += key;
  
  if (inputValue.length() > 4) {
    inputValue = inputValue.substring(0, 4);
  }
  
  tempValue = inputValue.toInt();
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set ");
  lcd.print(currentInput);
  lcd.print(" (ms)");
  lcd.setCursor(0, 1);
  lcd.print(inputValue);
  lcd.print("ms (");
  printSeconds(tempValue);
  lcd.print(")");
}

void saveInput() {
  switch (currentInput) {
    case 'A': timeA = tempValue; break;
    case 'B': timeB = tempValue; break;
    case 'C': timeC = tempValue; break;
    case 'D': timeD = tempValue; break;
  }
  
  inputMode = false;
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(currentInput);
  lcd.print(" = ");
  lcd.print(tempValue);
  lcd.print("ms (");
  printSeconds(tempValue);
  lcd.print(")");
  lcd.setCursor(0, 1);
  lcd.print("Disimpan");
  delay(500);
  
  showMainScreen();
}

void resetAll() {
  timeA = 0;
  timeB = 0;
  timeC = 0;
  timeD = 0;
  inputMode = false;
  currentMode = MODE_IDLE;
  systemRunning = false;
  digitalWrite(relayPin, HIGH);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reset Berhasil");
  lcd.setCursor(0, 1);
  lcd.print("A=0 B=0 C=0");
  delay(500);
  
  showMainScreen();
}
