#define PAPERDINK_DEVICE Paperdink_Classic  // Définir avant toute inclusion
#include <Paperdink.h>
#include <TimeLib.h>
#include "UUID.h"
#include "qrcode_gen.h"


UUID uuid;
QRCode qrcode;
PAPERDINK_DEVICE Paperdink;

// Définition des pins des boutons
#define BUTTON_1_PIN 14

void setup() {

  pinMode(BUTTON_1_PIN, INPUT_PULLUP);  // Configure le bouton avec une résistance de rappel interne

  // Initialisation de l'écran
  Paperdink.begin();
  Paperdink.enable_display();
  Paperdink.epd.fillScreen(GxEPD_WHITE);
  Paperdink.epd.setTextColor(GxEPD_BLACK);
  GxEPD2_GFX& display = Paperdink.epd;

  display.setCursor(50, display.height() / 2);
  display.println("Pret a generer UUID");
  Paperdink.epd.display();
}


void loop() {
  if (digitalRead(BUTTON_1_PIN) == LOW) {
    generateAndDisplayUUID();
    delay(200);  // Anti-rebond
    while(digitalRead(BUTTON_1_PIN) == LOW);  // Attendre que le bouton soit relâché
  }
}



void generateAndDisplayUUID() {
  char uuidString[37];  // Buffer pour stocker l'UUID généré
  uuid.generate();  // Générer un nouvel UUID
  uuid.toString(uuidString);  // Convertir l'UUID en chaîne de caractères
  GxEPD2_GFX& display = Paperdink.epd;
  display.setCursor(0, 50);
  display.println("UUID:");
  display.println(uuid);
  Paperdink.epd.display();
  delay(500);
  generateQRCode(uuidString);  // Générer et afficher le QR Code
}



void generateQRCode(const char *text) {
    uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
    uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
    bool ok = qrcodegen_encodeText(text, tempBuffer, qrcode,
        qrcodegen_Ecc_MEDIUM, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
    if (ok) {
        printQR(qrcode);  // Fonction à définir pour l'affichage
    }
}
void printQR(const uint8_t qrcode[]) {
    int size = qrcodegen_getSize(qrcode);
    int top = (display.height() - size * 2) / 2;  // Centrer le QR code
    int left = (display.width() - size * 2) / 2;
    display.fillScreen(GxEPD_WHITE);
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            if (qrcodegen_getModule(qrcode, x, y)) {
                display.fillRect(left + x * 2, top + y * 2, 2, 2, GxEPD_BLACK);
            }
        }
    }
    display.display();
}
