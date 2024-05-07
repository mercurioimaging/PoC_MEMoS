#MEMoS - Proof of Concept

## Description

Ce projet implémente un générateur d'identifiants uniques universels (UUID) utilisant un ESP32 et un écran e-ink. Le dispositif permet de générer des UUID aléatoires qui sont ensuite affichés sous forme de QRcode sur l'écran. 
L'état de la batterie, la présence de la carte SD et d'autres informations systèmes sont également affichés à l'aide d'icônes sur l'écran.

## Fonctionnalités

- **Génération d'UUID** : Génère des UUID conformes à la norme RFC 4122.
- **Affichage de QR Code** : Affiche l'UUID généré sous forme de QR Code pour une utilisation facile.
- **Surveillance de la batterie** : Affiche le niveau actuel de la batterie et son état de charge.
- **Détection de carte SD** : Indique si une carte SD est présente et active.
- **Mode veille** : Bascule en mode veille après une minute d'inactivité ou par pression d'un bouton pour économiser de l'énergie.
- **Réveil par bouton** : Permet le réveil de l'appareil par pression sur un bouton dédié.

## Matériel Requis

- Écran e-ink PAPERD.ink Classic
- Carte SD (optionnelle pour étendre la mémoire)
- Batterie avec gestion de charge
- Boutons de navigation

## Configuration et Déploiement

### Installation Logicielle

1. Installez l'IDE Arduino et configurez-le pour le développement avec ESP32. (Carte : ESP32 Dev Module). 
2. Téléchargez les bibliothèques nécessaires (QRCode, GxEPD, etc.) via le gestionnaire de bibliothèques de l'IDE Arduino. Suivre le tutoriel https://docs.paperd.ink/docs/software/getting-started/
3. Clonez le dépôt du projet ou téléchargez les fichiers sources.
4. Ouvrez le fichier principal du projet dans l'IDE Arduino et téléversez-le sur votre ESP32.

## Utilisation

- Allumez l'appareil en appuyant sur le bouton de mise en marche (en bas à droite)
- Utilisez les boutons pour naviguer dans les menus et générer des UUID.
- Surveillez l'état de la batterie et la présence de la carte SD via les icônes affichées.
