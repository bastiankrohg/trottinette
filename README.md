# Bureau d'étude - trottinette électronique
Bureau d'étude Trottinette à l'INSA de Toulouse en 4ème année
De Bastian KROHG et Nicolas SIARD
Automatique Electronique - Systèmes Embarqués
2022-2023

## Introduction

## Objectifs et compétences développées

## Mission n°1 - Asservissement de couple

### Premier approche et prise en main du système

### Asservissement dans le domaine continu

### Passage au discret - Asservissement dans le domaine discret

### Asservissement avec correcteur numérique avec µcontrôleur

En continu : 
$$C(p) = \frac{1 + tau_c p}{tau_i p} = \frac{K}{p} + \frac{tau_c}{tau_i}$$

Avec la transformée bilinéaire (Méthode de Tustin) on retrouve la fonction de transfert de C correspondante discrétisée
$$p = \frac{2}{T_e} \frac{z - 1}{z + 1}$$

En discret : 
On pose a0 et a1 tels que :  
$$a_0 = \frac{Te+2\tau_c}{2\tau_i}$$
$$a_1 = \frac{Te-2\tau_c}{2\tau_i}$$
On a donc la fonction de transfert de C en Z suivante :  
$$C(z) = \frac{a_0 z - a_1}{z - 1}$$

#### Vérification en simulation - comparaison Simulink/Keil

#### Implémentation du correcteur


## (Mission n°2 - Implémentation de la régulation de couple)
## (Autres evolutions du projet)
### (Détermination des grandeurs physiques de chaque système de trottinette)

## Conclusion
