# XFormat Description
Format de donnée pour le stockage des musiques au format XEN lisible par le PIC32.

* Taille de chaque secteur: `512 octets`
* Limite de musiques: `64`
* Limite de caractère pour le nom des musiques: `59 + \0`
* Taille maximale d'une musique: `4096 octets`.
* Les octets sont toujours stockés avec le bit de poids fort en premier.
* Les octets indiqués dans les structures sont toujours inclus. (`0-3` signifie les octets `0, 1, 2 et 3`)
* Tout les octets sont non-signé.

## Header / En-tête
Le header indique au périphérique de lecture les information de la carte mémoire.

* Le header utilise le `secteur 0`

#### Structure du header
| Octets    | Valeur | Description                                      |
| --------- | ------ | ------------------------------------------------ |
| `0-7`     | 0x42   | Header (Read only)                               |
| `8-11`    | 0x0    | Header (Read only)                               |
| `12-15`   | 0x42   | Header (Read only)                               |
| `16`      |        | Version du format sous 8 bits.                   |

## FAT (File allocation table)
La FAT stocke les informations des musiques (Nom, taille, emplacement dans la mémoire).

* La FAT utilise les `secteurs 1-8` (4096 octets)
* Une entrée dans la FAT fait `64 octets`
* Les entrées dans la FAT sont ordonnées par le secteur cible.
* Les entrées dans la FAT peuvent être fragmentées. Cela signifie que l'entrée 1 peut être vide mais pas l'entrée 2.
* La lecture de l'integralité de la FAT (secteur 1-8) et nescessaire pour récupérer la totalité des données.
* Une entrée vide contient les valeurs 0x0 dans les octets `2-3`.
* La FAT peut stocker au maximum 64 musiques.

#### Structure d'une musique dans la FAT
| Octets    | Description                                      |
| --------- | ------------------------------------------------ |
| `0-1`     | Secteur cible sous 16 bits. (Read only)          |
| `2-3`     | Taille de la musique en octets sous 16 bits.     |
| `4-63`    | Nom de la musique sous 59 caractère + le `\0`    |

## Secteur Musique
Le secteur musique contient les données musicales (notes, durée, tempo).

* Chaque musique utilise `4 secteurs consécutif`
* La taille d'une musique ne doit pas dépasser 4096 octets
