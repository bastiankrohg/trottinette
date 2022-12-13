//________________________________________________________________________
//
//  COMPATIBLE avec "stm32f10x.h" et "Clock.h" (lors du passage KEIL4.53) 
//________________________________________________________________________
//

//============== ** MODIF TR 15/12/2021 **  ==================
// dans les fonctions d'init PWM, ajout de ARPE = 1
// ce qui permet de synchroniser les actions de chargement CCR donc modif rcy.


//=======================================================================================

//============== MODIF TR 01 Oct 2015==================
// Modif TR : version lib : f.
// 18 jan 2015
// Modi Timer : .c : Affectation des ptr de fct dès le début
// 									dans les fct d'intit IT. Sinon, l'IT se déclenche tout de suite
//									et le handler saute dans l'inconnue.
//                Une seule fonction affectée : Active_IT_Debordement_Timer

//============== Fin MODIF TR 01 Oct 2015==============


//============== MODIF TR 16 Sept 2015==================
// Modif TR : version lib : d.
// 18 jan 2015
// Modi Timer : .c : IT compare débuggeé : l'IT voie 1 ne lançait pas la fct d'IT depuis le handler
//============== Fin MODIF TR 16 Sept 2015==============

// Modif TR :

// 03 Sept 2015
// Modification de la fonction fondamentale :
//Timer_1234_Init(TIM_TypeDef *Timer, float Duree_us );
// Elle est maintenant configurée pour que le Timer tourne en mode Up.
// NOM LIB :  Drivers_STM32F103_107_Jan_2015_c



// 21 Décembre 2014
// Ajout de la fonction :
// Ajout de la macro qui permet de donner une polarité négative à un channel de PWM
// PWM_Polarite_Neg(Timer,Voie)


#include "stm32f10x.h"
  
/******************************************************************************
* File Name          : TIMER_1234.h
* Author             : T Rocacher
* Version            : revu 21/12/2014
* Date               : 01/03/2011
* Description        : Module qui configure les TIMER 1,2,3 et 4
*  MISE A JOUR		 : Commentaire revus 18/01/2015                     
*******************************************************************************/



#ifndef _TIMER_1234_H__
#define _TIMER_1234_H__

//=======================================================================================
// Rappel sur les ressources 
//
//
//  3 Timers "general Purpose", TIM2, TIM3 et TIM4 + TIM1
//  Chacun d'entre eux dispose de 4 voies de sorties numérotées de 1 à 4
//  Mapping des IO:
//  	TIM1_CH1 - PA08       TM2_CH1_ETR - PA0 TM3_CH1 - PA6		TIM4_CH1 - PB6	
//    TIM1_CH2 - PA09       TM2_CH2 - PA1		  TM3_CH2 - PA7		TIM4_CH2 - PB7
// 		TIM1_CH3 - PA10       TM2_CH3 - PA2			TM3_CH3 - PB0		TIM4_CH3 - PB8
// 		TIM1_CH4 - PA11		    TM2_CH4 - PA3			TM3_CH4 - PB1		TIM4_CH4 - PB9
//	  TIM1_CH1N - PB13
//	  TIM1_CH2N - PB14
//	  TIM1_CH3N - PB15
//=======================================================================================



//=======================================================================================
// Fonctions basiques des timers
//=======================================================================================

//______________________________________________________________________________

float Timer_1234_Init(TIM_TypeDef *Timer, float Duree_us );
//______________________________________________________________________________

/*
Timer : de TIM1 à TIM4
Duree_us : c'est l'intervalle de temps exprimé en µs entre deux débordements successifs
Renvoie la durée véritable, au plus proche de la valeur souhaitée
Le prescaler est calculé le plus petit possible pour une précision la plus 
fine possible). Le Timer est lancé en mode UP et non down (modif 2015).
NB: Lancer avant la fonction d'initialisétion du système d'hrologe. 
Elle permet de régler toutes les horloges par défaut.

Exemple :
Init_Clock_System(); // lance l'initialisétion du système d'horloge de la puce
Timer_1234_Init(TIM2, 10.0 );   // Lance le timer avec une périodicité de 10.0 uS. 
*/
//______________________________________________________________________________




//______________________________________________________________________________

void Active_IT_Debordement_Timer( TIM_TypeDef *Timer, char Prio, void (*IT_function) (void));
//______________________________________________________________________________

/*
La fonction initialise le périphérique et le NVIC de manière à générer une interruption 
à chaque débordement du timer précisé.
La fonction passée en paramètre, forcément de type void Toto(void) est lancée sous interruption
Prio fixe la priorité (0 à 15, 0 étant le plus prioritaire)

Exemple :
Active_IT_Debordement_Timer( TIM2, 1, IT_Timer2); // La fonction active une interruption 
lors du débordement du Timer 2. La fonction appelée est IT_Timer2. La priorité associée est	1 
*/

//______________________________________________________________________________
//============== MODIF TR 18 Avril 2012==================
void Active_IT_Compare_Timer( TIM_TypeDef *Timer, char Voie, char Prio, void (*IT_function) (void));
/*
Même principe que la précédente. Permet de lancer une fonction d'IT sur comparaison
du compteur et du CCR (capture compare). Le CCR est donc à régler, grâce à la macro
déjà existante : PWM_Valeur(Timer,Voie) */
//============== FIN MODIF TR 18 Avril 2012==================


//=======================================================================================
// Timers en PWM
//=======================================================================================

//============== ** MODIF TR 15/12/2021 **  ==================
// dans les fonctions d'init PWM, ajout de ARPE = 1
// ce qui permet de synchroniser les actions de chargement CCR donc modif rcy. 
vu16 PWM_Init(TIM_TypeDef *Timer, char Voie, float Frequence_PWM_Khz);
/*
Cette fonction initialise la voie spécifiée du timer spécifié en PWM.
La fréquence souhaitée est passée en paramètre.
La fonction renvoie un entier qui correspond à la résolution de la PWM 
pour pouvoir ensuite régler les rapports cycliques (Resolution = ARR+1)
3 Timer "general Purpose", TIM2, TIM3 et TIM4 + TIM1
Chacun d'entre eux dispose de 4 voies de sorties numérotées de 1 à 4
Mapping des IO, voir début du .h.

!!C'est au user de configurer la sortie correctement, Altenate ppull !

Exemple :
Reso = PWM_Init(TIM2, 2, 20.0);
La fréquence de PWM vaut 20kHz. La PWM est disponible sur la voie 2 du Timer 2
Renvoie la valeur 3600, ce qui veut dire que PWM_Valeur(TIM2,2) = 3600 donne 100% de rapport cyclique
*/


//============== MODIF TR 21 Décembre 2014==================
#define PWM_Polarite_Neg(Timer,Voie) Timer->CCER = (Timer->CCER)|(0x01<<((Voie-1)*4+1))
/*
Par défaut, après l'init de PWM, la polarité est positive. Si besoin est de l'inverser, 
il suffit d'écrire cette ligne, par exemple :
PWM_Polarite_Neg(TIM2,3) aura pour effet d'inverser la polarité de la PWM voie 3 du TIM2
*/
//============== FIN MODIF TR 21 Décembre 2014==================



void PWM_Complementaire_Timer1(char Voie);
/* petite fonction qui configure le timer supposé déjà en PWM
de manière à sortir également la PWM complémentaire sur l'un des 
3 channels ou c'est possible : 1, 2 et 3.
Ce sont donc les channel CH1N, CH2N et CH3N qui s'activent en opposition
de phase avec leur homoloque CH1, CH2, CH3.
NB : les dead time ne sont pas gérés, donc à priori à 0.
*/

#define PWM_Valeur(Timer,Voie) Timer->CCR##Voie
/*
Permet de fixer  la durée à l'état haut de la PWM, dont indirectement son rapport 
cyclique. La grandeur doit être comprise entre 0 et ARR. Ex:
Reso = PWM_Init (TIM3,2,25.0);
PWM_Valeur(TIM3,2) = Reso /4; // arrondi à gerer
*/

#define CNT(Timer) Timer->CNT
/*
Permet un accès direct au compteur du Timer spécifié sans avoir à connaître 
les registres du STM32
*/
#define ARR(Timer) Timer->ARR
/*
Permet un accès direct à l'autoreload du Timer (modulo du timer) spécifié sans 
avoir à connaître les registres du STM32
*/
#define Clear_Flag_Overflow(Timer) 	Timer->SR = ((Timer->SR)&~0x01)

//============== MODIF TR 18 Avril 2012==================
// Gestion de la direction de comptage
#define CptUp(Timer) Timer->CR1=Timer->CR1&~(1<<4)
#define CptDown(Timer) Timer->CR1=Timer->CR1|(1<<4)
//============== FIN MODIF TR 18 Avril 2012==================


//=======================================================================================
// Timers en mode incrémental
//=======================================================================================

//  3 Timer "general Purpose", TIM2, TIM3 et TIM4
//  Chacun d'entre eux dispose de 4 voies de sorties numérotées de 1 à 4
//  Mapping des IO:
//  	TM2_CH1_ETR - PA0 	TM3_CH1 - PA6		TIM4_CH1 - PB6	
//    	TM2_CH2 - PA1		TM3_CH2 - PA7		TIM4_CH2 - PB7
//
//   Dans ce mode, le timerx utilise les 2 entrées 1 et 2
//
char Timer_Inc_Init(TIM_TypeDef *Timer, char Resolution);

#define Reso_Demie_Per_Ch1 2
#define Reso_Demie_Per_Ch2 1
#define Reso_Quart_Per 3

/*
Fonction de configuration du timer en question en mode codeur incrémental
Compte les fronts montant et descendant. Selon la configuration désirée, 
la résolution est de ½ période du channel 1, ou  2, ou encore d' ¼ de période 
en comptant sur les deux voies.

!!C'est au user de configurer des canaux  en entrée !! Le cnt est lancé.
*/
#define Reset_Timer(Timer) Timer->CNT=0
// remet à 0 le compteur 

#define Bloque_Timer(Timer) Timer->CR1=(Timer->CR1)&~(1<<0)
// bloque le timer (CEN=0)


#define Run_Timer(Timer) Timer->CR1=(Timer->CR1)|(1<<0)
// Lance timer (CEN=1)

//=======================================================================================
// Timers en mode capture
//=======================================================================================


vu16 Capture_Init(TIM_TypeDef *Timer, char Voie, float Largeur_Pulse_Max_us, vu16 Resolution, char Prio);
/*
Le Timer considéré est configuré en capture. La fonction permet de mesurer une largeur d'impulsion.
Elle est associée à la fonction vu16  Lire_Duree_Pulse(TIM_TypeDef *Timer,int Voie).
La grandeur renvoyée cette dernière fonction est proportionnelle à la durée de l'impulsion.

Ainsi :
Largeur_Pulse_Max_us est la durée la plus grande mesurable en µs
Resolution  est la valeur maximale souhaitée pour la grandeur de lecture
On a donc la correspondance :

Durée impulsion :  -------->  Grandeur renvoyée par la fonction de lecture
0 µs ---------------------->  0
Largeur_Pulse_Max_us ------>  Resolution

La fonction Capture_Init n'est pas forcément capable de faire la correspondance exacte. C'est pourquoi elle
renvoie la Resolution exacte, afin que l'utilisateur puisse s'adapter. D'autre part, si la configuration voulue 
va au delà des limites physiques, la valeur de résolution renvoyée est la mini/maxi possible.

Exemple :
Val=Capture_Init(TIM2, 2, 300.0, 1000, 1); // prio = 0
Renvoie la valeur 1028.
On a donc :
0 µs ---------------------->  0
100µs ------>  1028 (et non 1000)
(PSC calculé vaut 0x14 (20). Comme Fck = 72MHz, le timer avance à 72/(20+1) = 3 428kHz.
Ainsi, pour avoir 300µs il faut compter 300µs*3 428kHz= 1028


La mesure repose sur une interruption déclenchée sur front montant et sur front descendant. 
Le sens du front est inversé à chaque interruption. 

!!C'est au user de configurer l'IO correspondant  à la voie du timer,  en entrée !!
*/


vu16 Lire_Duree_Pulse(TIM_TypeDef *Timer,int Voie);
/*
Le rôle de la fonction est de retourner le comptage opéré pendant l'impulsion. 
Le nombre évolue de 0 à Largeur_Pulse_Max_us (spécifié lors de l'appel à Capture_Init
*/





#endif
