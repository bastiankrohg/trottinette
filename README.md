# Rapport
# Bureau d'étude - Trottinette électronique
Bureau d'étude Trottinette à l'INSA de Toulouse en 4ème année
De Bastian KROHG et Nicolas SIARD
Automatique Electronique - Systèmes Embarqués
2022-2023

<img width="549" alt="trott" src="https://user-images.githubusercontent.com/98895859/214147761-d3fc3845-a2ed-4167-9a2e-7dd3c775d7a3.png">

## Sommaire
Introduction\
Objectifs et compétences développées\
1 - Asservissement de couple\
&nbsp;1.1 - Premier approche et prise en main du système\
&nbsp;1.2 - Asservissement dans le domaine continu\
&nbsp;1.3 - Passage au discret - Asservissement dans le domaine discret\
&nbsp;1.4 - Asservissement avec correcteur numérique par µcontrôleur\
&nbsp;&nbsp;1.4.1 - De la fonction de transfert en z vers l’équation récurrente du correcteur\
&nbsp;&nbsp;1.4.2 - Étapes de la conception du correcteur numérique\
&nbsp;&nbsp;1.4.3 - Vérifier et débugger le code - mauvais calcul des macros\
&nbsp;&nbsp;1.4.4 - Vérification en simulation - comparaison Simulink/Keil\
&nbsp;&nbsp;1.4.5 - Implémentation et essais du correcteur\
2 - Régulation de vitesse\
3 - Autres évolutions du projet\
Conclusion

## Introduction

## Objectifs et compétences développées
L’objectif de ce BE Trottinette est d'arriver à comprendre et à analyser un système électronique complexe dans le but d’implémenter une commande en couple et en vitesse d’une trottinette électrique. 

Pour cela nous allons devoir mettre en application les connaissances que nous avons en architecture électronique mais aussi celles que nous avons en automatique, notamment pour la stabilité du système et la conception d’un correcteur. Nous allons également devoir apprendre à commander et comprendre un élément de puissance qui sera pour nous le hacheur et le moteur électrique, dans le but d’implémenter un asservissement. Enfin il faudra aller d’une vision continue à une vision discrète et implémenter le correcteur dans un microcontrôleur grâce à la transformée bilinéaire.

Ce BE comprend beaucoup de concepts qu’il nous faut comprendre pour pouvoir réaliser le meilleur asservissement possible. Pour cela nous avons accès à toutes les documentation techniques et à tous les schémas électroniques du système.


## 1 - Asservissement de couple

### 1.1 - Premier approche et prise en main du système
Tout d’abord il faut avoir une vue d’ensemble du système et comprendre de quels éléments il est composé et comment ces derniers interagissent entre eux. Pour cela nous avons le dossier technique à notre disposition. Nous pouvons commencer par analyser le diagramme en bloc qui indique comment les blocs de la commande électronique sont reliés les uns aux autres. Cela nous permet d’identifier les différents blocs que nous aurons dans le schéma bloc du système.
<img width="616" alt="diagramme_cmd_elec" src="https://user-images.githubusercontent.com/98895859/214151608-f5abf345-e0fd-42fd-9824-a49e8d0ae07c.png">\
Figure 1.1.1 - Diagramme en bloc de la commande électronique\
Il a donc les différents blocs:\
Le hacheur : H(p)\
Le moteur M(p)\
Le capteur de courant : N(p)\
Le conditionnement et les filtres : F(p)

Ensuite, grâce à la documentation technique nous pouvons trouver quelles sont les grandeurs physiques que nous devons prendre en compte, quels sont les points de repos et quelles sont les amplitudes de chacun des signaux. Nous arrivons au schéma bloc suivant: 
<img width="564" alt="schema_bloc" src="https://user-images.githubusercontent.com/98895859/214151944-ac00c694-b379-4b94-b9c6-717add630fa8.png">\
Figure 1.1.2 - Schéma bloc qui simplifie et fusionne entièrement le système de régulation de couple, avec unités physiques ainsi que les plages de valeurs admises.

On notera C(p), le microcontrôleur dans lequel on implémentera notre correcteur discret.

Il suffit maintenant d’exprimer les fonctions de transfert des différents blocs.

- Le hacheur peut juste se traduire par un gain Khacheur  qui vaut 48 d’après la fiche technique. Cet élément permet à partir d’un rapport cyclique alpha, de fournir une tension continue. Cependant, si on analyse le spectre fréquentiel du signal généré entre +/- 24 Volts, la tension de la batterie, on peut voir la fondamental mais aussi des harmoniques (de 20kHz). Or, on aimerait garder uniquement la composante continue à 0Hz. Il faudrait donc rajouter un filtre passe bas pour éliminer les harmoniques. Cela n’est pas nécessaire dans notre cas car on attaque un moteur à courant continu en sortie qui est semblable à un filtre LR, un filtre passe bas avec une fréquence de coupure à 80Hz, ce qui permet de supprimer les harmoniques. La tension de sortie du hacheur UM est donc une tension continue avec une valeur de tension moyenne induite par alpha.
- La fonction de transfert du moteur se traduit par un fonction de transfert du premier ordre : 
<img width="495" alt="moteur" src="https://user-images.githubusercontent.com/98895859/214152198-3a25050f-af0e-4e92-8f3a-d91deb238811.png">
Figure 1.1.3 - Schéma simplifié et calcul des caractéristiques du moteur\
On ne prend pas en compte la perturbation E(p) car on E(p) à une dynamique lente par rapport au moteur. Donc on considérera que les variables sont indépendantes. Pour étudier la stabilité on peut ne pas prendre en compte cette perturbation. C’est donc pour cela qu’elle n'apparaît pas dans les calculs.\
- Le capteur de courant, selon le document technique, correspond à un gain de K_courant = 0.1042. 
- La fonction de transfert du bloc de conditionnement peut se calculer à partir du schéma électronique dans la documentation technique:
<img width="615" alt="Schema_elec " src="https://user-images.githubusercontent.com/98895859/214152431-1a18ccbc-a2a6-400a-9eac-6cced1587b62.png">
Figure 1.1.4 - Schéma électronique du filtre F(p) de conditionnement\
Pour analyser ce schéma électronique on peut d’abord analyser la partie statique et ensuite la partie dynamique.

La partie statique servira à contrôler le gain statique du montage et la partie dynamique à calculer la fonction de transfert dynamique du système. Après calcul, on obtient la fonction de transfert finale suivante:\
<img width="266" alt="formule_filtre" src="https://user-images.githubusercontent.com/98895859/214153544-092a6e5e-53e8-44b7-bbc1-9b4b9c190e85.png">


### 1.2 - Asservissement dans le domaine continu
Pour asservir notre système nous devons respecter le cahier des charges suivant:
- Marge de phase supérieure ou égale à 45° dans le pire des cas
- Fréquence de transition en boucle ouverte 300 et 500Hz
- Erreur statique nulle en boucle fermée.

Par la suite on notera G(p) le bloc qui comprend le hacheur et le moteur et F(p) le bloc qui comprend le capteur de courant et le conditionnement.

On cherche donc à asservir le système en boucle ouverte suivant : G(p)F(p).

Il faut d’abord analyser le système en Boucle Ouverte, car si le système n’est pas stable ici, il ne le sera pas non plus en Boucle Fermée. L’enjeu est donc d’avoir une marche de phase positive à la fréquence de transition pour que le système soit stable. Si la marge de phase est négative, le système oscille et on perd la stabiité. Regarder la Boucle Ouverte peut également nous renseigner sur la vivacité du système.

On comprend donc que si on assure une marche de phase supérieure à 45 degrés en Boucle Ouverte on assure que le système soit stable en Boucle Fermée.

Maintenant, il nous faut aussi une erreur statique nulle en boucle fermée. Quel correcteur choisir ? Nous allons essayer plusieurs correcteurs, du plus simple au plus complexe pour trouver celui qui nous convient et qui nous permet de respecter le cahier des changes.

- Le correcteur proportionnel: 
$$C(p) = k $$

Si on a par exemple une marge de phase de 45 degrés à 200Hz on peut trouver un gain k tel que on arrive à avoir cette marge de phase pour la fréquence de transition que l’on veut, ici 400Hz. En effet un gain élevé shiftera la courbe de gain vers le haut et un gain faible à l’inverse la shiftera vers le bas. Cependant l'erreur statique ne sera pas nulle et ne pourra jamais l’être avec ce type de correcteur à moins d’avoir un gain immense, ce qui entraînerait des instabilités et ce n’est pas ce que l’on recherche. Ce correcteur n’est donc pas le bon.

- L’intégrateur pur: 
$$C(p) = \frac{1}{p}$$

Ce correcteur ajoute -20dB/décade à la courbe de gain. Cela permet d’avoir une erreur statique nulle, cependant cela fait descendre la phase de 90 degrés pour toute la courbe de phase et notre marge de phase est donc maintenant de 0 degré, ce qui n’est pas satisfaisant.  On ne peut pas non plus prendre ce correcteur pour notre système.

- Le correcteur proportionnel intégral :
$$C(p) = K_0 + \frac{K}{p} = \frac{1 + tau\' p}{tau_i p} $$

Ce correcteur nous permet d’avoir une erreur statique nulle grâce à l’intégrateur et le gain K0 permet de faire remonter la phase. On peut illustrer ce correcteur grâce au schéma de bode suivant pour plus de compréhension:
<img width="429" alt="bode_a_la_main" src="https://user-images.githubusercontent.com/98895859/214154314-f8902655-98b2-444b-81db-0b6921609bcd.png">\
Figure 1.2.1 - Tracé de bode du gain et de la phase pour montrer le comportement du correcteur PI

Ce correcteur est donc parfait pour nos besoins. Il suffit de trouver les différents paramètres pour le calibrer au plus proche de ce que l’on veut. Le calcul dépendra de ce que l’on veut pour le système.
- La première approche consiste à compenser le pôle du moteur (pôle dominant), ce qui simplifie l’équation. De plus, comme on se place aux alentours de la fréquence de transition qui vaut 400Hz, on peut négliger le pôle qui se trouve à 30kHz car sa dynamique n’entre pas en jeu ici. Il nous reste une expression simple dans laquelle il nous suffit de calculer K, le gain total du système et de trouver tau_i.
- La deuxième est celle sans compensation de pôle. Dans ce cas l’équation est bien plus fastidieuse et bien plus compliquée à résoudre pour trouver tau_i. Cependant, nous pouvons faire des simplifications car on travaille autour d’une fréquence particulière et ainsi l’équation est bien plus rapide à résoudre.

Dans ce BE, nous avons choisi d’appliquer la compensation de pôle. Nous allons donc compenser le pôle du moteur (tau' = 0.0020) et nous avons donc cette équation à résoudre:\
<img width="553" alt="formule_K_glob" src="https://user-images.githubusercontent.com/98895859/214154885-238db919-0137-4096-bd9f-d7431f168ee5.png">\
Ainsi, on peut facilement calculer tau_i:\
<img width="467" alt="calcul_tau_i" src="https://user-images.githubusercontent.com/98895859/214155051-01e8e633-ddf9-4d5f-883c-4e8ad0f84828.png">\

Nous avons donc tous les éléments pour calculer notre correcteur et faire les simulations sur Matlab et simulink !
#### Version 1 : 
Dans notre première version du système bouclée avec simulink, on a la configuration suivante: 
![simulink_v1](https://user-images.githubusercontent.com/98895859/214155114-105d7a14-5a81-4e8c-afc0-a8962d262b8f.PNG)\
Figure 1.2.2 - Simulink système asservi version 1\
![reponse_echelon_v1](https://user-images.githubusercontent.com/98895859/214155154-caa3018c-f12e-4ddd-8cfb-2f9564ec0781.PNG)\
Figure 1.2.3 - Simulink - réponse à un échelon d’amplitude 1.65 
![erreur_v1](https://user-images.githubusercontent.com/98895859/214155225-bc41ce87-02f1-4660-b169-6833c3dd5168.PNG)\
Figure 1.2.4 - Simulink - erreur du système - on voit bien qu’on a une erreur nulle après un certain temps\
![alpha_not_real_v1](https://user-images.githubusercontent.com/98895859/214155244-e284043b-6ad6-4229-8d06-e20aca19f88c.PNG)\
Figure 1.2.5 - Simulink - Le signal alpha - le signal pwm qui est envoyé à l’issue du bloc correcteur du système. Ici on voit bien que le alpha dépasse à un moment les valeurs admises, car normalement alpha est censé être compris dans l’intervalle [-0.5; +0.5]. Le système essaie d’être commandé si rapidement que le correcteur essaie d’envoyer un signal pwm de rapport cyclique > 100% (à alpha=+0.5) qui n’a pas de sens physique.

#### Version 2 : 
![simulink_v2_saturateur](https://user-images.githubusercontent.com/98895859/214155324-2243e825-0b6e-4b16-9112-925e26087405.PNG)
Figure 1.2.6 - Version 2 du système sous simulink - on fait évoluer notre système avec un bloc saturateur qui fait que l’alpha est à nouveau compris dans l’intervalle [-0.5; +0.5] afin d’assurer que le signal PWM de alpha ait un sens physique, qu’il ne dépasse pas ±100%. 
![reponse_echelon_depassement_de_saturation_v2](https://user-images.githubusercontent.com/98895859/214155359-d350e298-7887-4f5d-8a25-e6d32d837271.PNG)\
Figure 1.2.7 - La réponse à un échelon de 1.65 du système avec saturateur. On voit l’apparition du phénomène de dépassement lié à la saturation lorsque l’on essaie d’exciter un système plus rapidement que son slew rate.\
![erreur_avec_saturateur_v2](https://user-images.githubusercontent.com/98895859/214155365-9b7e009d-6fa9-4a61-9fba-e27a95a47e96.PNG)\
Figure 1.2.8 - L’erreur pour la version 2 du système (avec saturateur)
![alpha_sature_v2](https://user-images.githubusercontent.com/98895859/214155374-c7617d3a-6216-4171-b35f-1e0c02fe7aef.PNG)\
Figure 1.2.9 - L’alpha quand on ajoute un bloc de saturation à ±0.5. 

### 1.3 - Passage au discret - Asservissement dans le domaine discret
Lorsque l'on passe du continu vers le discret, on passe d'une représentation en continu comme ceci : 
$$C(p) = \frac{1 + tau_c p}{tau_i p} = \frac{K}{p} + \frac{tau_c}{tau_i}$$
À une représentation discrète comme ceci : 
$$C(z) = \frac{a_0 z - a_1}{z - 1}$$
On fait cette passage en gardant tous les propriétés du système comme les pôles et les zéros grâce à la méthode de Tustin, autrement dit la transformée bilinéaire. Pour cette transformée, on pose : 
$$p = \frac{2}{T_e} \frac{z - 1}{z + 1}$$
et on insère l'expression de p dans l'expression de C(p) afin de retrouver la fonction de transfert de C discrétisée, maintenant avec le variable z : 

- On pose a0 et a1 tels que :  
$$a_0 = \frac{T_e+2\tau_c}{2\tau_i}$$
$$a_1 = \frac{T_e-2\tau_c}{2\tau_i}$$

Donc on obtient la fonction de transfert suivante pour C(z) :  
$$C(z) = \frac{a_0 z - a_1}{z - 1}$$

#### Version 3 : 
![simulink_v3_discret_w_color](https://user-images.githubusercontent.com/98895859/214155591-e20297d7-783b-4ab1-86af-8590f05813ed.PNG)
Figure 1.3.1 - Simulink - Version 3 du système, on a enlevé le bloc correcteur continu de la version 2 et on a ajouté un bloc correcteur discret. Ici les couleurs correspondent aux différents domaines des signaux, c’est-à-dire que le noir est en continu, le rouge est en discret, et le bloc jaune correspond à un bloc qui convertit un signal discret en continu. On a choisi de rajouter le bloc échantillonneur Te en amont du bloc C_z afin de discrétiser avec la bonne période d’échantillonnage Te. 
![reponse_echelon_plus_petit_discret_v3](https://user-images.githubusercontent.com/98895859/214155651-dd6fbe22-cbde-428d-b6a5-ff83fea56cb5.PNG)\
Figure 1.3.2 - Réponse à un échelon de 1.65 avec le correcteur discret. On remarque que le phénomène de dépassement lié à la saturation y est toujours.  
![erreur_continue_avec_C_z_v3](https://user-images.githubusercontent.com/98895859/214155686-727b3ebb-618d-42db-9d96-8b45034d32f0.PNG)\
Figure 1.3.3 - L’erreur lorsque l’on passe en discret.\ 
![alpha_discret_v3](https://user-images.githubusercontent.com/98895859/214155739-be04db83-4f66-48a6-ad53-e1f572024de2.PNG)\
Figure 1.3.4 - Le signal PWM de alpha lorsque le correcteur est discrétisé.\
#### Version 4 et 5 : 
![simulink_corr_seul_v5](https://user-images.githubusercontent.com/98895859/214155832-5f484c40-5854-45e9-9032-9b2973900720.PNG)
Figure 1.3.5 - Simulink quand on souhaitait vérifier le bon comportement du correcteur discret avec le correcteur codé avec Keil. On excite le bloc correcteur avec un petit échelon d’entrée afin de mettre en évidence le K du premier pas de la réponse, qui est une des caractéristiques de notre correcteur PI.  
![corr_seul_reponse_verif_keil_v5](https://user-images.githubusercontent.com/98895859/214155863-7c65dc53-de1d-461f-989d-4e9000559881.PNG)\
Figure 1.3.6 - Réponse à un échelon du bloc correcteur seul pour vérifier le comportement dans keil par rapport à celui dans simulink. Ici on voit un premier pas de K=0.077, avant de monter jusqu’à +0.5 après 12.5ms. On s’arrête à alpha=0.5 grâce au bloc saturateur en aval du correcteur. Ce comportement va servir de test pour notre correcteur numérique afin qu’on puisse tester le bon fonctionnement du correcteur avant de le câbler physiquement. Cette vérification évite d’asservir le banc de trottinette avec un comportement qui n’est pas prévu qui peut engendrer des conséquences sur le matériel ou peut mettre l’utilisateur en risque.


### 1.4 - Asservissement avec correcteur numérique avec µcontrôleur
Jusqu’ici, on a vu comment on peut modéliser et asservir ce couple en continu puis en discret avec des correcteurs théoriques (analogiques) sur papier et avec Matlab et Simulink. Dans cette partie, on va voir comment on peut faire cette même conception, de manière numérique, avec un STM32/carte nucléo. De plus, le fait d’attaquer l’asservissement en numérique permet d’éviter quelques phénomènes qui se montrent lorsque l’on modélise le système analogiquement avec Simulink, comme le dépassement lié à la saturation(figure 1.2.7) lorsque le slew rate du système n’est pas “respecté”, mais aussi de faire en sorte que la configuration du correcteur soit plus finement réglée pour chaque système physique en recalculant les caractéristiques de ce dernier pour chaque trottinette. 

Pour ce travail de bureau d’étude, on nous a fourni un “Toolbox”, une boîte à outils contenant toute la couche service et driver pour configurer et mettre en œuvre le correcteur numérique avec un STM32. Lors de notre réalisation, on a tout simplement défini des macros (#define …) contenant les grandeurs physiques et les gains dont on avait besoin, issues de la modélisation théorique à la main et sur matlab et simulink. Cela nous permettait ensuite de définir des formules qui calculent automatiquement les nouveaux coefficients du correcteur spécifique lorsque les valeurs de temps d’échantillonnage “Te”/fréquence d’échantillonnage Fe, différents gains, résistances etc. changent. Cela permettait également de plus facilement voir et comprendre comment le comportement du système pourrait changer lorsque ces grandeurs physiques changent, surtout le temps d’échantillonnage qui a un grand impact sur l’asservissement numérique.  

Pour rentrer plus dans les détails de l’implémentation du correcteur numérique: 
Afin de libérer les ressources du microcontrôleur, ainsi de faire intervenir périodiquement les actions du correcteur chaque période d’échantillonnage “Te”, on implémente le correcteur avec une interruption principale basée sur un Timer. Cette interruption est configurée afin d’être levée chaque fois qu’un temps Te s’écoule. Le bon choix de cette période est impératif afin d’assurer le bon fonctionnement du correcteur numérique. Regardons cela de plus près: 
- Si on choisit Te trop petit, alors : 
  - On va avoir des énergies trop élevées lorsque la fréquence augmente, ce qui dissipe davantage la puissance et la chaleur. De plus, on peut avoir un phénomène d'interruptions qui ne s'effectuent pas assez rapidement, et la même interruption va être pending et la commande ne s'effectuera plus en temps réel.
- et si on choisit Te trop grand, alors : 
   - On va voir l’apparition d’instabilités parce qu'on n'a pas assez de points pour bien discrétiser/échantillonner le signal en entrée du bloc correcteur de notre système.

#### 1.4.1 - De la fonction de transfert en z vers l’équation récurrente du correcteur
On a retrouvé l’équation récurrente pour le calcul de l’alpha de la manière suivante : 
$$C(z) = \frac{Y(z)}{U(z)} = \frac{a_0 z - a_1}{z - 1}$$ 
$$Y(z)(z - 1) = U(z)(a_0 z - a_1) $$
Avec la transformée inverse en z, on obtient:\
<div align="center">y<sub>n+1</sub> - y<sub>n</sub> = a<sub>0</sub> e<sub>n+1</sub> - a<sub>1</sub> e<sub>n</sub></div>
\
On pose n=n+1, et on obtient l’équation récurrente :\
<div align="center">y<sub>n</sub> = y<sub>n-1</sub> + a<sub>0</sub> e<sub>n</sub> - a<sub>1</sub> e<sub>n-1</sub></div>
\
Avec alpha = y et l’erreur = e\
Cette fonction récurrente nous permet de calculer le nouveau alpha / nouveau rapport cyclique avec lequel on va commander le système à la sortie du bloc correcteur C(z) numérique. 
#### 1.4.2 - Étapes de la conception du correcteur numérique
L’utilisation du périphérique ADC fait que l’on doit réfléchir à ce à quoi correspondent les différents signaux et leurs plages de valeurs. Par exemple, pour le calcul de l’erreur où on prend l’écart entre le courant et la consigne, il faut penser à diviser ce résultat avant de l’utiliser dans le calcul de alpha. Sinon, il faut bien garder en tête ce que ces valeurs représentent. Par défaut, ce que renvoie la fonction Entree_3V3() de l’ADC est compris entre [0;4096]. Pour cela, il faut penser à ramener ces valeurs à la plage [0;3.3] si on souhaite avoir un alpha qui est bien compris entre [-0.5;+0.5]. Cela permet de faire le bon calcul de l’erreur, qui est utilisée pour le calcul de l’équation récurrente des nouvelles valeurs de alpha.  
Afin de coder l’interruption principale de notre correcteur en C on a suivi les étapes suivants: 
1. Initialisation\
On initialise les valeurs de l’alpha et de l’erreur à n=0 à 0.0, la valeur de repos. Celles-ci sont importantes pour pouvoir commencer le calcul de l’équation récurrente 
2. Acquisition de la consigne\
On fait cela grâce à la fonction fournie dans le toolbox Entree_3V3().
3. Acquisition du courant\
On retrouve la mesure du courant grâce à la fonction I1(), également fournie.
4. Calcul de la valeur courante d’epsilon, l’erreur entre le courant et la consigne\
On commence par faire le calcul de l’écart : epsilon = (Cons_In - Courant_1)\
On ramene cette valeur à la plage de valeurs correspondante : Il faut multiplier par 3.3 et diviser par 4096 pour avoir une valeur qui correspond à la plage de valeurs de notre erreur afin de pouvoir calculer un alpha compris entre [-0.5;+0.5]. La valeur issue de cette opération est une float : epsilon = 3.3*(Cons_In - Courant_1)/4096
5. Calcul de la nouvelle valeur analogique de alpha avec l’expression déduite de l’équation récurrente. L’alpha analogique signifie la sortie comprise entre [-0.5;+0.5].
On utilise la formule déduite de l’équation récurrente:

alphaAnalogique<sub>n</sub> = alphaAnalogique<sub>n-1</sub> + a<sub>0</sub> erreur<sub>n</sub> + a<sub>1</sub> erreur<sub>n-1</sub>

Ici, a0 et a1 correspondent aux valeurs des coefficients que l’on a calculés pour la fonction de transfert en z, et sont exprimés ci-dessous : 

$$a_0 = \frac{T_e+2\tau_c}{2\tau_i}$$
$$a_1 = \frac{T_e-2\tau_c}{2\tau_i}$$
$$C(z) = \frac{a_0 z - a_1}{z - 1}$$

6. Saturation\
On sature la sortie alpha pour s'assurer qu’il est bien compris dans la plage [-0.5;+0.5]. Cela est fait pour être sûr que la sortie du bloc correcteur, qui est sous forme de signal PWM - pulse width modulation, a un sens physique. Quand alpha atteint (resp.) +0.5 et -0.5, cela correspond à, respectivement, un signal PWM de 100% (toute la période en état haut) et un signal PWM de 0% (toute la période en état bas). Cette saturation est faite tout simplement avec un if else if qui redéfinit la valeur de alpha à la borne supérieure ou inférieure lorsqu’il dépasse.    
7. Mise à jour de l’ancienne valeur d’alpha et d’erreur\
Le fait de les affecter après la saturation fait que l’on arrive à éviter le phénomène de dépassement lié à la saturation que l’on a vu lors de la simulation avec Simulink en discret (avec C(Z)). Faire réf à l’ancienne courbe/figure avec lien.
8. Recalcule la valeur numérique de alpha, qui est comprise dans la plage [0;4096] permettant de commander la fonction R_Cyc_1 / R_Cyc_2 qui mettent à jour les signaux PWM en sortie. 

#### 1.4.3 - Vérifier et débugger le code - mauvais calcul des macros
Après le premier essai lorsque l’on a lancé la simulation du correcteur numérique on a constaté la présence d’erreurs qui faisaient que l’on n’obtenait pas le bon comportement en sortie. Au début, notre alpha n’avait pas l’air d’augmenter, mais c’était tout simplement dû au fait que les macros que l’on avait définis pour les coefficients du contrôleur ne donnaient pas la bonne valeur. On se rendait compte après quelques essais-erreurs que c’était pas bon, mais on a réussi à résoudre le problème en décomposant les calculs et en regardant les résultats que cela donnait. On avait bien utilisé la bonne expression, néanmoins les macros ne calculaient pas le résultat que l’on avait prévu. Ce problème a été résolu en reformulant les expressions de ces coefficients (a0, a1), ainsi que de définir la valeur de la période d’échantillonnage Te directement, au lieu de l’exprimer comme l’inverse de la fréquence d’échantillonnage. 

Suite à cela, on a à nouveau regardé la sortie du bloc correcteur. Cette fois-ci, on avait presque le bon résultat qui colle avec la simulation en Simulink. Cependant, on a vu l’apparition d'une espèce de bâtonnets quand alpha saturait à 0.5 qui allaient au-delà.
On peut voir ces bâtonnets sur la figure 1.4.4.3 lorsque alpha = 0.5
On n’a pas encore compris d’où ils viennent. 

#### 1.4.4 - Vérification en simulation - comparaison Simulink/Keil
Après avoir codé le correcteur numérique discret, c’était important de vérifier que l’on obtient bien le comportement souhaité de ce bloc. Sous Simulink on a donc isolé le bloc C(z) avec seulement un petit échelon en entrée d’amplitude 0.1, et un bloc saturateur et un scope en aval.\ 
<img width="467" alt="corr_seul_simulink" src="https://user-images.githubusercontent.com/98895859/214157584-6565b0ba-fcc6-40b4-8675-bbae1b74e7dd.png"> \
Figure 1.4.4.1 - Schéma bloc du correcteur seul sous simulink lors de la vérification du comportement du simulink vs celui obtenu avec Keil. 
Le comportement prévu était le suivant:\ 
![corr_seul_reponse_verif_keil_v5](https://user-images.githubusercontent.com/98895859/214157643-020cdf28-73b7-4ad5-8861-b46d40e02b7d.PNG)
Figure 1.4.4.2 - Tracé de la réponse à un échelon de 0.1 du bloc correcteur sous Simulink, qui met en évidence le comportement d’intégrateur souhaité.

Dans ce schéma on constate que le comportement observé était composé d’un premier step égale à K=tau_c/tau_i=0.077, suivi de petits pas en escalier qui monte (pense: intégrateur) jusqu’à l’obtention de alpha=0.5. Le temps que met l’intégrateur à monter jusqu’à alpha=0.5 depuis le premier pas K est ici égale au delta ΔT=12.5ms. 
Il fallait vérifier que ces valeurs, et donc le comportement du bloc correcteur (si on excite avec la même entrée) était exactement la même sous Keil et Simulink. On a commencé par réécrire l’expression de C(p), qui nous donnait une idée de ce à quoi on s’attendait, avant de faire une application numérique. En regardant les courbes obtenues en simulation avec Keil et Simulink, les valeurs de K et tau collent bien avec notre calcul théorique. On obtient bien le même K=0.077 (premier pas) et temps de réponse de alpha=K=0.077 jusqu’à alpha=0.5, ΔT=12.5ms.

Réécriture de l’expression du correcteur PI : 
$$C(p) = \frac{1 + tau_c p}{tau_i p} = \frac{K}{p} + \frac{tau_c}{tau_i}$$
<img width="632" alt="keil_simu" src="https://user-images.githubusercontent.com/98895859/214158176-e94e6e2d-6a89-4652-95c5-05220d4160f5.png">\
Figure 1.4.4.3 - Tracé de la réponse à un échelon de 0.1 du bloc correcteur sous keil
#### 1.4.5 - Implémentation et essais du correcteur
Avant de tester la carte de puissance que l’on a conçue, on a vérifié que le banc trottinette fonctionnait. Pour cela on a branché le banc à une carte de puissance analogique qui a été conçue préalablement. Quand on a vu que cette carte et donc le correcteur fonctionnait comme prévu, c’est-à-dire qu’on réussissait à commander la trottinette vers l’avant, l’arrière, et arriver à maintenir une couple nulle. On pouvait donc passer à l’étape de test du correcteur numérique en réel.
Lors de l’implémentation du correcteur, on a commencé par configurer la carte de contrôle de puissance contenant la carte STM32. Ensuite, on a branché un générateur basse-fréquence à l’entrée In de la carte. De plus, on a relié les deux PWM en opposition de phase au banc trottinette avec deux fils blancs. On a également récupéré la tension Vcourant, venant du banc trottinette (à l’issue du capteur courant après conditionnement par le bloc de filtre F(p)), et l’a branché à la carte de contrôle de puissance à l’entrée du courant. On a relié la carte à la masse du banc trottinette, et on a repéré la commande du potentiomètre au niveau de l’entrée 3V3 (même endroit que le GBF) permettant de régler la consigne d’entrée.\
<img width="484" alt="banc_cable" src="https://user-images.githubusercontent.com/98895859/214158448-a79b73e9-ee80-4133-a023-6c5b4527e1d4.png">\
Figure 1.4.5.1 - Banc trottinette câblé avec la carte de puissance codée. 

Le système récupérateur d’énergie est relié au banc trottinette avec un système de contrôle de tension avec un transistor et une partie dissipatrice permettant de dissiper de la chaleur lorsqu’il y a trop de récupération d’énergie afin d’éviter une augmentation/accumulation de tension. La trottinette est alimentée à travers le système de récupération d’énergie qui sert de mécanisme de protection entre l’alimentation et la trottinette. 
 
Finalement, on a relié deux sondes de l'oscilloscope pour visualiser les signaux que l’on envoie et l’on reçoit du système. Cela nous permettait de comprendre le comportement du système quand on appliquait des entrées différentes (sinus / créneaux / basse fréquence / haute fréquence)  
<img width="597" alt="creneaux_sortie_degrad" src="https://user-images.githubusercontent.com/98895859/214158580-781ff2fb-260c-495f-93d1-50a1b95fb1a3.png">\
Figure 1.4.5.2 - Observations sur l’oscilloscope lorsque l’on a mis un signal créneaux en entrée, basse fréquence (f=450mHz). On observe le phénomène de dégradation de la sortie à cause du long temps que met le système pour répondre. Le système n’arrive plus à accélérer, et on voit une dégradation du signal en sortie vers la fin du créneau/carré.\ 
<img width="597" alt="creneaux_sortie_normale" src="https://user-images.githubusercontent.com/98895859/214158683-df0af708-6c94-4d80-b695-47fc588052b3.png">
Figure 1.4.5.3 - Observations sur l’oscilloscope lorsque l’on a mis un signal créneaux en entrée, à ≈ 1Hz, on voit que la sortie a à peu près la même amplitude que l’entrée, c'est-à-dire que l’on retrouve le gain de 0dB en basse fréquence.  
<img width="597" alt="sinus_0707" src="https://user-images.githubusercontent.com/98895859/214158824-1bb18c96-53de-4b3c-b519-8bc5bb839c3d.png">\
Figure 1.4.5.4 - Observations sur l’oscilloscope lorsque l’on a mis un sinus en entrée, sortie à 0,74 (presque 0,707…) lorsque l’on est près de la fréquence de coupure à 730Hz.  
On n’a pas pris de photo lors de la fréquence de coupure exacte, mais on l’a mesurée à f ≈ 750Hz.
Pour tester la trottinette lors de l’implémentation en réel, on veut vérifier trois choses : 
- La fréquence de coupure
- L’erreur statique nulle
- La marge de phase

Comme on l’a vu ci-dessus, on a trouvé une fréquence de coupure égale à ≈ 750 Hz. Dans nos simulations, on s’attendait à une fréquence de coupure à 500Hz en boucle fermée, donc on n’est pas loin. Cette différence peut être dûe à des petites différences de gain dans la modélisation par rapport au système réel. Le gain du système réel est donc à un facteur ≈1,5 de gain par rapport au système modélisé, qui apparaît à cause des différences physiques des systèmes. Par rapport au comportement global du système, on constate que cette différence ne change pas beaucoup le comportement observé, car avec cette fréquence de coupure de même ordre de grandeur que celle prévue, on ne perd pas la stabilité du système.   
Pour vérifier l’erreur statique nulle, on essaie de commander le système avec une consigne nulle qui maintient le système à un point de repos où la roue reste immobile. On arrive bien à avoir un tel comportement, et on peut donc constater que l’erreur statique est nulle car le système ne se met pas en route tout seul si on arrive à garder ce niveau de consigne. De plus, on voit en relativement basse fréquence, on arrive à avoir une sortie qui est l’image de l’entrée. En boucle fermée à basse fréquence, on sait que le comportement prévu est un gain en dB de 0dB, autrement dit un gain de 1. Cela veut dire que l’erreur est ≈ 0 car on obtient le même signal en sortie qu’en entrée.

Pour vérifier la marge de phase, il faut estimer la stabilité en boucle fermée, car on n’a pas de possibilité de mesurer la marge de phase à ce point. Vu que la marge de phase correspond à une grandeur physique en boucle ouverte, on ne peut pas le vérifier en réel, cependant on peut tester la stabilité en boucle fermée qui nous indique la marge de phase qui conduit à un tel comportement. On a déjà constaté que l’on arrive à maintenir une consigne nulle qui nous donne une sortie nulle, c’est-à-dire une roue immobile. En réglant une entrée créneaux, on peut regarder la réponse pour voir si on voit de dépassement lorsque le système va répondre à sa commande de couple. Sur la figure 1.4.5 on voit qu’il n’y a pas de dépassement lors de la réponse à un échelon (une période du signal créneaux). On a donc gardé le comportement stable souhaité avec ≈60 degrés de marge de phase, car on sait qu’avec 45 degrés de marge, la réponse à un échelon contient un premier dépassement de 5%. Vu que l’on n’a pas de dépassement, cela veut dire que l’on a bien >45 degrés, et probablement vers 60+ degrés vu que l’on n’a pas du tout de dépassement en sortie du système.  


## 2 - Régulation de vitesse
On n’a pas eu le temps de passer à l’étape de la conception de la boucle de vitesse, mais on va expliquer l’idée et comment on aurait fait si on avait le temps de le faire pour la trottinette. 
La boucle de vitesse est une boucle extérieure qui va commander la consigne donnée en entrée de la boucle interne qui est celle que l’on a mise en œuvre préalablement.
Pour l’étape de conception du correcteur de la boucle de vitesse, on va pouvoir simplifier la boucle interne. Cela est dû au fait qu'entre la sortie et l’entrée, puisque la régulation du courant en sortie du système est très rapide devant la régulation de vitesse nécessaire, on constate le comportement d’un simple bloc de gain qui permet de passer une consigne comprise entre [0;3.3] volts à un courant compris entre [-10;+10] ampères. De ce fait, on peut modéliser le système de régulation de vitesse de la trottinette de la manière suivante : 
<img width="597" alt="schema_bloc_vitesse" src="https://user-images.githubusercontent.com/98895859/214159160-6de2c5dc-7b1c-4217-98d4-1c445ea401d6.png">\
Figure 2.1.1 - Schéma bloc simplifié permettant de plus facilement déterminer la fonction de transfert du correcteur de vitesse de la boucle de régulation de vitesse.  

## 3 - Autres évolutions du projet
Le projet de la trottinette peut évoluer encore plus, car il serait possible d’implémenter une courte phase d’initialisation lorsque l’utilisateur allume la trottinette, qui permet de configurer plus finement l’asservissement (numérique) du système. Pendant cette initialisation, le microcontrôleur STM32 pourrait exciter le système pendant quelques millisecondes afin d’analyser et calculer les grandeurs physiques de chaque trottinette en analysant la réponse à un échelon, permettant de mettre en évidence quelques caractéristiques comme le temps de réponse à 5%/2% / temps de montée etc. Vu qu’il y a de légères différences physiques entre chaque trottinette, comme par exemple les valeurs exactes des résistances du moteur ou les valeurs des inductances du système, on pourrait donc les calculer pour avoir des valeurs plus exactes par rapport à la vraie vie, permettant ainsi à concevoir un correcteur encore plus performant, avec des coefficients optimisés pour chaque trottinette. 

## Conclusion




