#define PAPERDINK_DEVICE Paperdink_Classic  // Définir avant toute inclusion
#include <Paperdink.h>
#include "UUID.h"
#include "qrcode_gen.h"
#include <SPI.h>
#include <SD.h>
#include "Button.h"
#include "config.h"
#include "icons.h"
#include <vector>



const char* version = "V1.2 - Icones";


UUID uuid;
QRCode qrcode;
PAPERDINK_DEVICE Paperdink;

GxEPD2_GFX& display = Paperdink.epd;
int screenWidth = 400;  // La largeur de l'écran
int charWidth = 6;      // Largeur estimée d'un caractère en pixels

std::vector<String> mots;  // Vecteur global pour stocker les mots
bool sdPresent = false; // Variable globale pour suivre l'état de la carte SD



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
        sdPresent = false;
    } else {
        Serial.println("Initialisation de la carte SD réussie.");
        sdPresent = true;
    }
  // Charger les mots du fichier dans le vecteur global
  File file = SD.open("/Noms.txt", FILE_READ);
  if (!file) {
    Serial.println("Erreur : Impossible d'ouvrir le fichier de noms.");
    return;
  }

  while (file.available()) {
    String word = file.readStringUntil('\n');
    word.trim();  // Enlever les espaces blancs ou les retours à la ligne
    if (word.length() > 0) {
      mots.push_back(word);
    }
  }

  file.close();
  Serial.println("Mots chargés en mémoire.");

  int textlenght = strlen(version) * charWidth;
  int uuidTextX = (screenWidth - textlenght) / 2;
  display.setCursor(uuidTextX, display.height() / 2);
  display.println(version);
  displayIcon();
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
  

  digitalWrite(SD_EN, LOW);

  if (mots.size() < 3) {
    Serial.println("Pas assez de mots dans le fichier.");
    return;
  }

  // Sélectionner aléatoirement trois mots
  String selectedWords[3];/Users/eloi/Documents/Arduino/CNRS_MAP_UUID_V1/icons.h
  for (int i = 0; i < 3; i++) {
    int index = random(mots.size());  // Choix aléatoire d'un index
    selectedWords[i] = mots[index];
  }

  // Écrire l'UUID et les mots choisis dans "historique.txt"
  File file = SD.open("/historique.txt", FILE_APPEND);
  if (file) {
    file.print(uuidString);
    file.println("," + selectedWords[0] + "-" + selectedWords[1] + "-" + selectedWords[2]);
    file.close();

    Serial.print("UUID et mots enregistrés : ");
    Serial.print(uuidString);
    Serial.print(" ");
    Serial.println(selectedWords[0] + "-" + selectedWords[1] + "-" + selectedWords[2]);
  } else {
    Serial.println("Erreur : Impossible d'ouvrir 'historique.txt' en mode ajout.");
  }

  // Générer et afficher le QR Code
  displayUUIDandQRCode(uuidString,selectedWords);
  digitalWrite(SD_EN, HIGH);
}


void displayUUIDandQRCode(const char* uuidString, const String words[]) {
  // Génération du QR Code
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, uuidString);


  /// AFFICHER LE TEXTE DE L'UUID
  int uuidValueWidth = strlen(uuidString) * charWidth;
  int uuidValueX = (screenWidth - uuidValueWidth) / 2;
  // Positionner et afficher la valeur UUID
  display.setCursor(uuidValueX, 20);
  display.println(uuidString);


  /// AFFICHER LE QR CODE
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

/// AFFICHER LE TRIPTYQUE
  // Préparation à l'affichage des mots
  String combinedWords = words[0] + " - " + words[1] + " - " + words[2];
  int textWidth = combinedWords.length() * charWidth;
  int textX = (screenWidth - textWidth) / 2;
  int textY = top + size * scale + 20; // 20 pixels sous le QR code

  display.setCursor(textX, textY);
  display.println(combinedWords);

  display.display();  // Mettre à jour l'affichage
}

void displayIcon() {
    // Détermine le coin en haut à droite pour l'icône
    int x = display.width() - sd_width;
    int y = 0;

    if (sdPresent) {
        display.drawBitmap(x, y, (const uint8_t*)sd_bits, sd_width, sd_height, GxEPD_BLACK);
    } else {
        display.drawBitmap(x, y, (const uint8_t*)nosd_bits, nosd_width, nosd_height, GxEPD_BLACK);
    }
}
