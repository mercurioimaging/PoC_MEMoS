#define PAPERDINK_DEVICE Paperdink_Classic  // Définir avant toute inclusion
#include <Paperdink.h>
#include <TimeLib.h>
#include "UUID.h"
#include "qrcode_gen.h"


UUID uuid;
QRCode qrcode;
PAPERDINK_DEVICE Paperdink;

GxEPD2_GFX& display = Paperdink.epd;

// Définition des pins des boutons
#define BUTTON_1_PIN 14

void setup() {

  pinMode(BUTTON_1_PIN, INPUT_PULLUP);  // Configure le bouton avec une résistance de rappel interne

  // Initialisation de l'écran
  Paperdink.begin();
  Paperdink.enable_display();
  Paperdink.epd.fillScreen(GxEPD_WHITE);
  Paperdink.epd.setTextColor(GxEPD_BLACK);

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
  uuid.generate();  // Générer un nouvel UUID
  char* uuidString = uuid.toCharArray();  // Obtenir la chaîne UUID

  display.setCursor(0, 50);
  display.println("UUID:");
  display.println(uuidString);  // Affichez la chaîne correctement obtenue
  Paperdink.epd.display();
  delay(500);
  generateQRCode(uuidString);  // Passez cette chaîne au générateur de QR Code
}


void generateQRCode(const char *text) {
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, text);

  // Appeler la fonction d'affichage du QR Code
  displayQRCode(qrcode);
}

void displayQRCode(QRCode &qrcode) {
  int size = qrcode.size;
  int scale = 2;  // Ajuster la taille des modules pour un meilleur affichage
  int top = (display.height() - size * scale) / 2;
  int left = (display.width() - size * scale) / 2;

  display.fillScreen(GxEPD_WHITE);  // Effacer l'écran
  for (int y = 0; y < size; y++) {
    for (int x = 0; x < size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        display.fillRect(left + x * scale, top + y * scale, scale, scale, GxEPD_BLACK);
      }
    }
  }
  display.display();  // Mettre à jour l'affichage
}
