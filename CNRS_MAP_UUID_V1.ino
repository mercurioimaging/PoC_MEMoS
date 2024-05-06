#define PAPERDINK_DEVICE Paperdink_Classic  // Définir avant toute inclusion
#include <Paperdink.h>
#include "UUID.h"
#include "qrcode_gen.h"
#include <SPI.h>
#include <SD.h>
#include "Button.h"
#include "config.h"


const char* version = "V1.1 - Test SD";


UUID uuid;
QRCode qrcode;
PAPERDINK_DEVICE Paperdink;

GxEPD2_GFX& display = Paperdink.epd;
int screenWidth = 400;  // La largeur de l'écran
int charWidth = 6;      // Largeur estimée d'un caractère en pixels


void setup() {
  Serial.begin(115200);

  //REGLER L'HEURE :
  pinMode(BUTTON_1_PIN, INPUT_PULLUP);  // Configure le bouton avec une résistance de rappel interne
  pinMode(SD_EN, OUTPUT);

  // Initialisation de l'écran
  Paperdink.begin();
  Paperdink.enable_display();
  Paperdink.epd.fillScreen(GxEPD_WHITE);
  Paperdink.epd.setTextColor(GxEPD_BLACK);

  if (!SD.begin(SD_CS)) {
    Serial.println("Initialisation de la carte SD échouée!");
    return;  // Arrêter ici si échec
  }
  Serial.println("Initialisation de la carte SD réussie.");


  int textlenght = strlen(version) * charWidth;
  int uuidTextX = (screenWidth - textlenght) / 2;
  display.setCursor(uuidTextX, display.height() / 2);
  display.println(version);
  Paperdink.epd.display();
}


void loop() {
  if (digitalRead(BUTTON_1_PIN) == LOW) {
    Paperdink.epd.fillScreen(GxEPD_WHITE);
    generateAndDisplayUUID();
    delay(200);  // Anti-rebond
    while (digitalRead(BUTTON_1_PIN) == LOW)
      ;  // Attendre que le bouton soit relâché
  }
}


void generateAndDisplayUUID() {
  uuid.generate();                        // Générer un nouvel UUID
  char* uuidString = uuid.toCharArray();  // Obtenir la chaîne UUID
  int uuidTextWidth = strlen("UUID:") * charWidth;
  int uuidValueWidth = strlen(uuidString) * charWidth;

  // Calculer la position x pour centrer le texte "UUID:"
  int uuidTextX = (screenWidth - uuidTextWidth) / 2;
  // Calculer la position x pour centrer la chaîne UUID
  int uuidValueX = (screenWidth - uuidValueWidth) / 2;


  // Positionner et afficher la valeur UUID
  display.setCursor(uuidValueX, 20);
  display.println(uuidString);
  digitalWrite(SD_EN, LOW);

  File file = SD.open("/historique.txt", FILE_APPEND);
  if (file) {
    file.println(uuidString);  // Écrire l'UUID suivi d'un saut de ligne
    file.close();              // Fermer le fichier après écriture
    Serial.print("UUID ajouté à 'historique.txt': ");
    Serial.println(uuidString);
  } else {
    Serial.println("Erreur : Impossible d'ouvrir 'historique.txt' en mode ajout.");
  }

  // Générer et afficher le QR Code
  generateQRCode(uuidString);
  digitalWrite(SD_EN, HIGH);
}


void generateQRCode(const char* text) {
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, text);
  displayQRCode(qrcode);
}

void displayQRCode(QRCode& qrcode) {
  int size = qrcode.size;
  int scale = 8;  // Ajuster la taille des modules pour un meilleur affichage
  int top = (display.height() - size * scale) / 2;
  int left = (display.width() - size * scale) / 2;
  for (int y = 0; y < size; y++) {
    for (int x = 0; x < size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        display.fillRect(left + x * scale, top + y * scale, scale, scale, GxEPD_BLACK);
      }
    }
  }
  display.display();  // Mettre à jour l'affichage
}
