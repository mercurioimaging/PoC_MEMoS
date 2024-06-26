/*
 * PoC_MEMOS - Proof of Concept
 * 
 * Description :
 * Ce projet implémente un générateur d'identifiants uniques universels (UUID) utilisant un ESP32 et un écran e-ink. 
 * Le dispositif permet de générer des UUID aléatoires qui sont ensuite affichés sous forme de QR codes sur l'écran.
 * L'état de la batterie, la présence de la carte SD et d'autres informations systèmes sont également affichés à l'aide d'icônes sur l'écran.
 *
 * Fonctionnalités :
 * - Génération d'UUID conformes à la norme RFC 4122.
 * - Affichage de QR codes pour les UUID générés.
 * - Surveillance de la batterie et indication de l'état de charge.
 * - Détection de la carte SD et indication de sa présence.
 * - Mode veille automatique après une minute d'inactivité ou par pression d'un bouton.
 * - Réveil du dispositif par pression d'un bouton dédié.
 * 
 * Matériel Requis :
 * - Écran e-ink PAPERD.ink Classic
 * - Carte microSD (optionnelle, pour mémoire étendue)
 * 
 * 
 * Auteur : Eloi Gattet - Mercurio Imaging
 * Date : 05/2024
 * 

 */

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
#include "PCF8574.h"
#include "fonts.h"

// PCF8574 GPIO extender
PCF8574 pcf8574(PCF_I2C_ADDR, SDA, SCL);


const char* version = "PoC_MEMoS";


UUID uuid;
QRCode qrcode;
PAPERDINK_DEVICE Paperdink;

GxEPD2_GFX& display = Paperdink.epd;
int screenWidth = 400;  // La largeur de l'écran
int charWidth = 6;      // Largeur estimée d'un caractère en pixels

std::vector<String> mots;  // Vecteur global pour stocker les mots
String combinedWords;
bool sdPresent = false;  // Variable globale pour suivre l'état de la carte SD
bool sleepStatus = false;
bool errorStatus = false;
long lastInteractionTime;

void setup() {

  char pcf_failed[] = "PCF8574 Failed";
  char pcf_success[] = "PCF8574 Success";


  Serial.begin(115200);
  Serial.print("Générateur d'UUID sur QrCode:");
  Serial.println(version);
  Serial.println("Codé par Eloi Gattet - Mercurio Imaging");
  Serial.println("05/2024");
  Serial.println("https://mercurioimaging.com");

  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(BUTTON_2_PIN, INPUT_PULLUP);
  pinMode(BUTTON_3_PIN, INPUT_PULLUP);
  pinMode(BUTTON_4_PIN, INPUT_PULLUP);
  // Board Init
  pinMode(EPD_EN, OUTPUT);
  pinMode(EPD_RST, OUTPUT);
  pinMode(SD_EN, OUTPUT);
  pinMode(BATT_EN, OUTPUT);
  pinMode(PCF_INT, OUTPUT);  // Required to lower power consumption
  pinMode(CHARGING_PIN, INPUT_PULLUP);
  // Power up EPD
  digitalWrite(EPD_EN, LOW);
  digitalWrite(SD_EN, LOW);
  digitalWrite(EPD_RST, LOW);
  delay(50);
  digitalWrite(EPD_RST, HIGH);
  delay(50);

  // Initialisation de l'écran
  Paperdink.begin();
  Paperdink.enable_display();
  //Test de l'extention de GPIO (pour détecter si la carte SD est présente)

  pcf8574.pinMode(SD_CD, INPUT);
  if (pcf8574.begin()) {
    Serial.println(pcf_success);
  } else {
    Serial.println(pcf_failed);
  }

  homescreen();  // Afficher l'écran d'accueil
  lastInteractionTime = millis();
}


void loop() {
  static unsigned long buttonPressTime = 0;
  static bool buttonPressed = false;
  static unsigned long button3PressTime = 0;
  static bool button3Pressed = false;

  // Vérifier le bouton 1 pour générer un UUID
  if (digitalRead(BUTTON_1_PIN) == LOW) {
    Serial.println("Nouveau UUID");
    CheckSD();
    Paperdink.epd.fillScreen(GxEPD_WHITE);
    generateAndDisplayUUID();
    delay(200);  // Anti-rebond
    while (digitalRead(BUTTON_1_PIN) == LOW)
      ;
    lastInteractionTime = millis();
  }
  // Vérifier le bouton 2 pour afficher l'historique
  else if (digitalRead(BUTTON_2_PIN) == LOW) {
    CheckSD();
    menuHistorique();
  }
  // Vérifier si le bouton 3 est maintenu enfoncé
  else if (digitalRead(BUTTON_3_PIN) == LOW) {
    if (!button3Pressed) {
      button3PressTime = millis();  // Enregistrer le temps du début de la pression
      button3Pressed = true;
    } else if (millis() - button3PressTime > 3000) {  // 3000 ms = 3 secondes
      Bonus();
      button3Pressed = false;  // Réinitialiser le statut du bouton
    }
  } else {
    if (button3Pressed) {
      button3Pressed = false;
      if (millis() - button3PressTime <= 3000) {
        Infos();
      }
    }
    // Vérifier si le bouton 4 est maintenu enfoncé
    if (digitalRead(BUTTON_4_PIN) == LOW) {
      if (!buttonPressed) {
        buttonPressTime = millis();  // Enregistrer le temps du début de la pression
        buttonPressed = true;
      } else if (millis() - buttonPressTime > 1500) {  // 1500 ms = 1.5 secondes
        GoToSleep();
        buttonPressed = false;  // Réinitialiser le statut du bouton
      }
    } else {
      if (buttonPressed) {
        buttonPressed = false;
        if (millis() - buttonPressTime <= 1500) {
          homescreen();
        }
      }
      // Mettre en veille après une période d'inactivité
      if (millis() - lastInteractionTime > 60000) {
        GoToSleep();
      }
    }
  }
  delay(30);
}




void GoToSleep() {
  sleepStatus = true;
  Serial.println("ENTERING DEEPSLEEP");
  HUD();
  MenuSleep();
  //display.display();
  display.displayWindow(0, 0, 35, 10);     //(box_x, box_y, box_w, box_h)
  display.displayWindow(0, 0, 20, 100);    //(box_x, box_y, box_w, box_h)
  display.displayWindow(370, 0, 30, 300);  //(box_x, box_y, box_w, box_h)
  Paperdink.deep_sleep_button_wakeup(BUTTON_4_PIN);
}

void CheckSD() {
  bool previousSdPresent = sdPresent;

  // Vérification physique de la présence de la carte SD
  if (pcf8574.digitalRead(SD_CD) == HIGH) {
    Serial.println("Carte SD absente physiquement!");
    sdPresent = false;
    mots.clear();  // Vider le vecteur des mots
    SD.end();
  } else {
    Serial.println("Carte SD détectée!");
    if (previousSdPresent == false) {
      Serial.println("Carte réinsérée!");
      digitalWrite(SD_EN, HIGH);
      delay(250);
      digitalWrite(SD_EN, LOW);
      delay(300);
    }
    if (SD.begin(SD_CS)) {
      Serial.println("Initialisation de la carte SD réussie.");
      sdPresent = true;
      // Si l'état de la carte SD a changé, mettre à jour la liste des mots
      if (previousSdPresent != sdPresent) {
        CreateWordsList();
      }
    } else {
      Serial.println("Initialisation de la carte SD échouée!");
      sdPresent = false;
    }
  }
}

void CreateWordsList() {
  // Charger les mots du fichier dans le vecteur global
  Serial.println("CreateWordLists()");
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
}

void generateAndDisplayUUID() {

  display.fillScreen(GxEPD_WHITE);
  uint32_t seed1 = random(999999999);
  uint32_t seed2 = random(999999999);
  uuid.seed(seed1, seed2);
  uuid.generate();                        // Générer un nouvel UUID
  char* uuidString = uuid.toCharArray();  // Obtenir la chaîne UUID

  if (mots.size() < 3) {
    Serial.println("Pas assez de mots dans le fichier.");
  }

  // Sélectionner aléatoirement trois mots
  String selectedWords[3];

  if (sdPresent) {

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
      errorStatus = 0;
      Serial.print("UUID et mots enregistrés : ");
      Serial.print(uuidString);
      Serial.print(" ");
      Serial.println(selectedWords[0] + "-" + selectedWords[1] + "-" + selectedWords[2]);
    } else {
      errorStatus = 1;
      Serial.println("Erreur : Impossible d'ouvrir 'historique.txt' en mode ajout.");
    }
  } else {
    Serial.println("Pas carte SD donc pas de sauvegarde.");
  }

  // Générer et afficher le QR Code
  displayUUIDandQRCode(uuidString, selectedWords);
}


void displayUUIDandQRCode(const char* uuidString, const String words[]) {

  int textWidth, textX, textY;
  // Génération du QR Code
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, uuidString);


  /// AFFICHER LE TEXTE DE L'UUID
  /*############################################################*/
  int uuidValueWidth = strlen(uuidString) * charWidth;
  int uuidValueX = (screenWidth - uuidValueWidth) / 2;
  // Positionner et afficher la valeur UUID
  display.setCursor(uuidValueX, 10);
  display.println(uuidString);


  /// AFFICHER LE QR CODE
  /*############################################################*/
  int size = qrcode.size;
  int scale = 9;  // Ajuster la taille des modules pour un meilleur affichage
  int top = (display.height() - size * scale) / 2 + 1;
  int left = (display.width() - size * scale) / 2;
  for (int y = 0; y < size; y++) {
    for (int x = 0; x < size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        display.fillRect(left + x * scale, top + y * scale, scale, scale, GxEPD_BLACK);
      }
    }
  }

  /// AFFICHER LE TRIPTYQUE
  /*############################################################*/
  // Préparation à l'affichage des mots
  if (sdPresent) {
    combinedWords = words[0] + " - " + words[1] + " - " + words[2];
    textWidth = combinedWords.length() * charWidth;
    textX = (screenWidth - textWidth) / 2;
    textY = top + size * scale + 5;  // 20 pixels sous le QR code
  } else {
    combinedWords = "Ajoutez \"Noms.txt\" a la racine de la SD. ";
    textWidth = combinedWords.length() * charWidth;
    textX = (screenWidth - textWidth) / 2;
    textY = top + size * scale + 20;  // 20 pixels sous le QR code
  }
  // Mise en place du curseur
  display.setCursor(textX, textY);
  display.println(combinedWords);

  //On appelle le HUD
  HUD();
  Menu();
  display.display();  // Mettre à jour l'affichage
}


void displayError(const char* msg) {
  display.fillScreen(GxEPD_WHITE);
  int textlenght = strlen(msg) * charWidth;
  int TextX = (screenWidth - textlenght) / 2;
  display.setCursor(TextX, display.height() / 2);
  display.println(msg);
  display.display();
  Serial.println(msg);
  delay(2000);
  display.fillScreen(GxEPD_WHITE);
}



//########################################################################################################################################################################
void menuHistorique() {

  int yPosition, index;
  //Test avant menu:
  // Si la carte est absente, afficher une erreur
  if (!sdPresent) {
    const char* msg = "Erreur : Carte SD absente.";
    displayError(msg);
    // Réafficher le dernier UUID généré
    String words[3] = { "", "", "" };  // Dummy words array
    displayUUIDandQRCode(uuid.toCharArray(), words);
    return;
  }

  // Lire le fichier historique
  File file = SD.open("/historique.txt", FILE_READ);
  if (!file) {
    const char* msg = "Erreur : Impossible d'ouvrir 'historique.txt'.";
    displayError(msg);
    // Réafficher le dernier UUID généré
    String words[3] = { "", "", "" };  // Dummy words array
    displayUUIDandQRCode(uuid.toCharArray(), words);
    return;
  }
  //Si tout va bien, on lit le fichier
  // Lire les lignes du fichier dans un vecteur
  std::vector<String> historique;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      historique.push_back(line);
    }
  }
  file.close();

  int totalEntries = historique.size();
  int entriesToShow = min(totalEntries, 13);

  // Afficher les 10 derniers UUID
  //On met du blanc partout:

  int cursorPosition = 0;
  bool selectionConfirmed = false;
  //On affiche la liste:
  display.fillScreen(GxEPD_WHITE);
  display.setCursor(120, 20);
  display.setFont(&PAPERDINK_FONT_MED_BOLD);
  display.print("Historique");
  //display.setFont(&PAPERDINK_FONT_SML);
  display.setFont(NULL);
  for (int i = 0; i < entriesToShow; i++) {
    index = totalEntries - 1 - i;
    yPosition = 40 + i * 20;  // Espacement vertical

    display.setCursor(13, yPosition);
    String truncatedEntry = historique[index].substring(0, 60);
    display.print(truncatedEntry);
  }
  MenuHistory();
  display.display();

  //GoToSleep
  unsigned long lastInteractionTime = millis();

  while (!selectionConfirmed) {

    display.fillRect(0, 20, 12, 280, GxEPD_WHITE);
    display.drawXBitmap(0, 39 + cursorPosition * 20, (const uint8_t*)fleche_7x12_bits, fleche_7x12_width, fleche_7x12_height, GxEPD_BLACK);
    display.displayWindow(0, 0, 12, 300);  //(box_x, box_y, box_w, box_h)
    //display.display();
    // Gestion des boutons
    while (true) {
      if (millis() - lastInteractionTime > 60000) {
        GoToSleep();
        return;
      }
      if (digitalRead(BUTTON_4_PIN) == LOW) {
        // Sortir du menu
        /* CODE POUR AFFICHER LE QRCODE PRECEDENT:
        Paperdink.epd.fillScreen(GxEPD_WHITE);
        String words[3] = { "", "", "" };  // Dummy words array
        displayUUIDandQRCode(uuid.toCharArray(), words);
        */
        homescreen();
        return;
      } else if (digitalRead(BUTTON_1_PIN) == LOW) {
        // Déplacer le curseur vers le haut
        lastInteractionTime = millis();
        cursorPosition = (cursorPosition > 0) ? cursorPosition - 1 : entriesToShow - 1;
        break;
      } else if (digitalRead(BUTTON_2_PIN) == LOW) {
        // Déplacer le curseur vers le bas
        lastInteractionTime = millis();
        cursorPosition = (cursorPosition < entriesToShow - 1) ? cursorPosition + 1 : 0;
        break;
      } else if (digitalRead(BUTTON_3_PIN) == LOW) {
        // Valider la sélection
        selectionConfirmed = true;
        break;
      }
      delay(100);  // Anti-rebond
    }
  }

  // Afficher le QR code de l'UUID sélectionné
  if (selectionConfirmed) {
    int selectedIndex = totalEntries - 1 - cursorPosition;
    String selectedEntry = historique[selectedIndex];
    int commaIndex = selectedEntry.indexOf(',');
    String selectedUUID = selectedEntry.substring(0, commaIndex);
    String selectedWords = selectedEntry.substring(commaIndex + 1);
    String selectedWordArray[3];
    int wordIndex = 0;

    // Séparer les mots
    int startIndex = 0;
    for (int i = 0; i < selectedWords.length(); i++) {
      if (selectedWords.charAt(i) == '-') {
        selectedWordArray[wordIndex++] = selectedWords.substring(startIndex, i);
        startIndex = i + 1;
      }
    }
    selectedWordArray[wordIndex] = selectedWords.substring(startIndex);
    display.fillScreen(GxEPD_WHITE);
    displayUUIDandQRCode(selectedUUID.c_str(), selectedWordArray);
  }
}

void homescreen() {
  Paperdink.epd.fillScreen(GxEPD_WHITE);
  Paperdink.epd.setTextColor(GxEPD_BLACK);
  //Paperdink.epd.setFont(&PAPERDINK_FONT_SML);
  display.setFont(&PAPERDINK_FONT_MED);  // Tester :@PAPERDINK_FONT_MED_BOLD et &PAPERDINK_FONT_SML et &PAPERDINK_FONT_MED et &PAPERDINK_FONT_LRG
  int textlenght = strlen(version) * 15;
  int uuidTextX = (screenWidth - textlenght) / 2;
  display.setCursor(uuidTextX, 50);
  display.print(version);
  display.setFont(NULL);
  display.drawXBitmap(100, 80, (const uint8_t*)logo_bits, logo_width, logo_height, GxEPD_BLACK);
  CheckSD();
  HUD();
  Menu();
  display.display();
}

void Bonus() {
  display.fillScreen(GxEPD_WHITE);
  String phrases[] = { "jamais", "donner", "toi" };
  displayUUIDandQRCode("https://www.youtube.com/watch?v=dQw4w9WgXcQ", phrases);
}


void Infos() {
  display.setFont(&PAPERDINK_FONT_SML);

  Paperdink.epd.fillScreen(GxEPD_WHITE);
  Paperdink.epd.setTextColor(GxEPD_BLACK);
  CheckSD();
  HUD();
  Menu();
  int x0 = 60;
  int x = x0;
  int y = 30;

  display.setCursor(x, y);
  display.println("Interface developpee par:");


  //LOGO MERCURIO y +30
  y += 15;
  display.drawXBitmap(x, y, (const uint8_t*)logo_mercurio_bits, logo_mercurio_width, logo_mercurio_height, GxEPD_BLACK);

  // Nom de la société Mercurio Imaging
  // On décalle et on centre le texte de mercurio
  y += 22;
  x += 120;

  display.setCursor(x, y);  // Ajuster en fonction de l'emplacement souhaité
  display.println("Mercurio Imaging");

  //On aligne à nouveau à gauche:
  x = x0;
  y += 45;
  display.setCursor(x, y);  // Ajuster en fonction de l'emplacement souhaité
  display.println("Projet MEMos dirige par :");
  y += 20;
  // Afficher le logo du CNRS 60,160
  display.drawXBitmap(x, y, (const uint8_t*)logo_cnrs_bits, logo_cnrs_width, logo_cnrs_height, GxEPD_BLACK);
  x += 60;  //On décale de 60 pixels apres le logo du map
  // Afficher le logo du MAP
  display.drawXBitmap(x, y, (const uint8_t*)logo_map_bits, logo_map_width, logo_map_height, GxEPD_BLACK);

  // Lien vers le site du MAP CNRS
  y += 70;                  // On descend
  x = x0;                   //On réinitialise à gauche
  display.setCursor(x, y);  // Ajuster en fonction de l'emplacement souhaité
  display.println("Laboratoire MAP UPR 2002");
  y += 25;                  // On descend de 30 px
  display.setCursor(x, y);  // Ajuster en fonction de l'emplacement souhaité
  display.print("https://www.map.cnrs.fr/");

  // Lien vers le GitHub du projet
  y += 25;                  //On descend de 40px
  x = x0;                   //On réinitialiser encore la position en x
  display.setCursor(x, y);  // Ajuster en fonction de l'emplacement souhaité
  display.print("Plus d'infos sur:");
  display.setFont(NULL);
  y += 15;                  //on descend encore
  display.setCursor(x, y);  // Ajuster en fonction de l'emplacement souhaité
  display.println("https://github.com/mercurioimaging/PoC_MEMoS");


  display.display();
}



//##########################################################################@ AFFICHAGES GRAPHIQUES
void HUD() {
  display.fillRect(0, 0, 20, 100, GxEPD_WHITE);  //(box_x, box_y, box_w, box_h, GxEPD_WHITE)
  display.fillRect(0, 0, 25, 10, GxEPD_WHITE);   //Efface le logo charge
  int x = 3;
  int y = 3;

  // Déterminer le niveau de la batterie et si elle est en charge
  uint8_t charging = digitalRead(CHARGING_PIN) ^ 1;
  digitalWrite(BATT_EN, LOW);
  delay(10);
  analogReadResolution(12);
  int adc = analogReadMilliVolts(BATTERY_VOLTAGE);
  digitalWrite(BATT_EN, HIGH);
  double vbat = (double(adc) * 1.29375) / 1000;  // Calcul du voltage de la batterie

  // Choix de l'icône de la batterie à afficher
  const uint8_t* batt_icon;
  int batt_width, batt_height;
  if (vbat < 3.5) {
    batt_icon = Batt_Low_bits;
    batt_width = Batt_Low_width;
    batt_height = Batt_Low_height;
  } else if (vbat < 3.7) {
    batt_icon = Batt_33_bits;
    batt_width = Batt_33_width;
    batt_height = Batt_33_height;
  } else if (vbat < 3.9) {
    batt_icon = Batt_66_bits;
    batt_width = Batt_66_width;
    batt_height = Batt_66_height;
  } else {
    batt_icon = Batt_100_bits;
    batt_width = Batt_100_width;
    batt_height = Batt_100_height;
  }
  // Affichage de l'icône de la batterie
  display.drawXBitmap(x, y, (const uint8_t*)batt_icon, batt_width, batt_height, GxEPD_BLACK);

  x += batt_width + 3;  // Décaler x pour placer l'icône de charge à droite de la batterie

  // Si la batterie est en charge, afficher l'icône de charge à droite de l'icône du niveau de batterie
  if (charging) {
    display.drawXBitmap(x, y, (const uint8_t*)charge_bits, charge_width, charge_height, GxEPD_BLACK);
  }
  y += batt_height + 5;  // Ajouter une marge entre les icônes

  x = 3;  //Réinitialise x
  // Affichage de l'icône de la carte SD
  if (sdPresent) {
    display.drawXBitmap(x, y, (const uint8_t*)sd_20px_bits, sd_20px_width, sd_20px_height, GxEPD_BLACK);
  } else {
    display.drawXBitmap(x, y, (const uint8_t*)nosd_20px_bits, nosd_20px_width, nosd_20px_height, GxEPD_BLACK);
  }

  y += sd_20px_height + 5;
  x = 3;  //Réinistialise x
  // Affichage de l'icône de veille si l'appareil est en mode veille
  if (sleepStatus) {
    display.drawXBitmap(x, y, (const uint8_t*)sleep_bits, sleep_width, sleep_height, GxEPD_BLACK);
    y += sleep_height + 5;
  }

  if (errorStatus) {
    display.drawXBitmap(x, y, (const uint8_t*)warning_bits, warning_width, warning_height, GxEPD_BLACK);
  }
}

void Menu() {
  display.fillRect(380, 0, 20, 300, GxEPD_WHITE);
  display.fillRect(360, 280, 20, 20, GxEPD_WHITE);
  // Affichage de l'icône "new"
  display.drawXBitmap(380, 25, (const uint8_t*)new_bits, new_width, new_height, GxEPD_BLACK);
  // Affichage de l'icône "historique"
  display.drawXBitmap(380, 110, (const uint8_t*)history_bits, history_width, history_height, GxEPD_BLACK);
  // Affichage de l'icône "info"
  display.drawXBitmap(380, 195, (const uint8_t*)info_bits, info_width, info_height, GxEPD_BLACK);
  // Affichage de l'icône "home"
  display.drawXBitmap(380, 280, (const uint8_t*)home_bits, home_width, home_height, GxEPD_BLACK);
  // Affichage de l'icône "power"
  //display.drawXBitmap(380, 280, (const uint8_t*)power_bits, power_width, power_height, GxEPD_BLACK);
}

void MenuSleep() {
  display.fillRect(380, 0, 20, 300, GxEPD_WHITE);  //(box_x, box_y, box_w, box_h, GxEPD_WHITE)
  display.fillRect(360, 280, 20, 20, GxEPD_WHITE);
  //TODO : Ajouter flèche à côté de power
  // Affichage de l'icône de "power" à la position spécifiée
  display.drawXBitmap(360, 280, (const uint8_t*)fleche_bits, fleche_width, fleche_height, GxEPD_BLACK);
  display.drawXBitmap(380, 280, (const uint8_t*)power_bits, power_width, power_height, GxEPD_BLACK);
}


void MenuHistory() {
  display.drawXBitmap(389, 25, (const uint8_t*)up_bits, up_width, up_height, GxEPD_BLACK);
  display.drawXBitmap(389, 110, (const uint8_t*)down_bits, down_width, down_height, GxEPD_BLACK);
  display.drawXBitmap(380, 195, (const uint8_t*)check_bits, check_width, check_height, GxEPD_BLACK);
  display.drawXBitmap(380, 280, (const uint8_t*)home_bits, home_width, home_height, GxEPD_BLACK);
}