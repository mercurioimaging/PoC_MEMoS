## PoC_MEMOS

## Description

PoC_MEMOS est une preuve de concept de dispositif  basé sur ESP32 et un écran e-ink, conçu pour générer des UUID et les afficher sous forme de QR codes. Le dispositif gère également les informations système telles que l'état de la batterie et la présence de la carte SD.
Chaque UUID est également associé à un triplet de mots aléatoires, choisis dans un fichier de vocabulaire "Nom.txt" placé à la raçine de la carte SD.

## Fonctionnalités

- **Génération d'UUID** : Génère des UUID conformes à la norme RFC 4122.
- **Génération de triplet aléatoire**: Chaque UUID est suivi de trois mots séparés par des tirets, par exemple : `123e4567-e89b-12d3-a456-426614174000, mot1-mot2-mot3`.
- **Affichage de QR Code** : Affiche les UUID générés sous forme de QR codes. Le triplet est simplement affiché sous la forme de texte.
- **Surveillance de la batterie** : Affiche le niveau actuel de la batterie et son état de charge.
- **Détection de carte SD** : Indique la présence et le fonctionnement de la carte SD. L'abcense de carte SD n'empêche pas le fonctionnement de l'appareil.
- **Mode veille** : Passe en mode veille après une minute d'inactivité ou par pression longue du bouton "Maison"
- **Réveil** : Réveille l'appareil du mode veille par pression du bouton "Power"

## Matériel Requis

- Écran e-ink PAPERD.ink Classic
- Carte SD (optionnelle, pour mémoire étendue)

## Installation Logicielle

1. **Configuration de l'IDE Arduino** :
   - Installez l'IDE Arduino et configurez-le pour le développement avec ESP32 (Carte : ESP32 Dev Module).
   - Suivez le guide : [Démarrage avec Paperd.ink](https://docs.paperd.ink/docs/software/getting-started/) pour installer les librairies nécéssaires. La librairie QR_Code 

2. **Installation des bibliothèques** :
   - Installez les bibliothèques nécessaires (QRCode, GxEPD, etc.) via le gestionnaire de bibliothèques de l'IDE Arduino.

3. **Configuration du projet** :
   - Clonez le dépôt ou téléchargez les fichiers sources.
   - Ouvrez le fichier principal du projet dans l'IDE Arduino.
   - Téléversez le code sur votre carte ESP32.

## Instructions d'Utilisation

- Allumez l'appareil en appuyant sur le bouton en bas à droite.
- Utilisez les boutons de navigation pour générer des UUID et naviguer dans les menus.
- Surveillez l'état de la batterie et la présence de la carte SD via les icônes affichées.

Pour toute question ou contribution, veuillez consulter le [dépôt GitHub](https://github.com/votre-repo-lien).
