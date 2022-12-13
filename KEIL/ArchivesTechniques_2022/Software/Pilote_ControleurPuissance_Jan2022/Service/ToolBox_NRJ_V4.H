
/******************************************************************************
* File Name          : ToolBox_NRJ_v3.h (v3 = ARM sur KEIL5)
* Author             : T Rocacher
* Version            : 
* Date               : 7/11/2017
* Description        : contient les fonctions clé en main pour exploiter la carte de commande de puissance
* Processeur				 : STM32F103, KEIL 5, ARM 
* Documentation			 : Manuel_Toolbox_NRJ.pdf
********************************************************************************/



//=========================================================================================================
//
//					TOOLBOX_NRJ :Boîte à outils pour la carte de commande de puissance
//
//=========================================================================================================
/*

CONFIGURATION MATERIELLe DE LA CARTE NUCLEO :
Carte "Contrôleur de Puissance" + "Nucléo STM32F103"
Configuration de la carte Nucléo version C02 :
					SB62, SB63 (USART) = ON (PA3 et PA2 connectées au connecteur Arduino)
					SB13, SB14 (ST-LINK-USART) = Off déconnection USART débug
					SB21 = off (déconnection User LED du PA5)
Configuration de la carte Nucléo version C01 = conf C02 et :
					SB16, SB50 = On (Osc 8MHz externe en provenance du débugger ST link)


LOGICIEL (compilateur ARM) :
Nécessite la lib Drivers_STM32F103_107_Juin2017a.lib  pour fonctionner.
Permet de configurer et d'utiliser les unités PWM, les entrées ADC, les LEDs ...

!! NB IT ext et IT capteur HALL ne peuvent fonctionner ensemble !! C'est l'un OU l'autre

*/





/*=========================================================================================================
		
				ENTREE ET SORTIE PAR GRANDES FONCTIONS :
				
===========================================================================================================

_____________________________________________________________________________________
ENTREES CONSIGNE:
					entrée -10V / +10V : (entrée onduleur ou retour tachnymètre) : PC1 - Voie ADC 11
					entrée 0/3V3       : (entrée de consigne potentiomètre sur carte ou externe) : PC0 - Voie ADC 10

_____________________________________________________________________________________
ENTREES COURANTS :
					I1 : PA2 - Voie ADC 2
					I2 : PA3 - Voie ADC 3
					I3 : PA5 - Voie ADC 5

_____________________________________________________________________________________
ENTREE CODEUR INCREMENTAL
					A : PA0 - ch1 de TIM2
					B : PA1 - ch2 de TIM2
					I : PA4 - index en remise à 0 TIM2 sous interruption ou pas selon #define

_____________________________________________________________________________________
LED DE VISUALISATION E/S A UTILISER
					LED_Courant : PB5
					LED_PWM : PB10
					LED_PWM_Aux : PC7
					LED_+/-10V : PB6
					LED_3.3V : PA9
					LED_CODEUR : PA10

_____________________________________________________________________________________
LA PWM
					TIM3_Ch1 : PA6
					TIM3_Ch2 : PA7
					TIM3_Ch3 : PB0

______________________________________________________________________________________
LA PWM AUXILIAIRE
					TIM1_Ch1 : PA8
					TIM4_Ch3 : PB8
					TIM4_Ch4 : PB9
					Rem : ces 3 output peuvent être utilisées en simple Output PushPull afin de
					contrôler chacun des 3 bras de l'onduleur (bras on ou off), voir fonction
					void Conf_Optionnelle_Step_Motor_PWM(void)
*/



#ifndef _TOOLBOX_H__
#define _TOOLBOX_H__
#include "stm32f10x.h"

/*===================================================================
					USER Define
=====================================================================*/
// Choisir la priorité d'interruption de 0 à 15, 0 le plus prioritaire.
 #define Prio_Systick 1
 #define Prio_IT_3V3 1
 #define Prio_Index_Codeur 0
// !!! ATTENTION si le systick et l'IT externe ont des prio différentes
// il y a un risque d'accumulation de demande de conv ADC => plantage général
/*===================================================================*/


/*===================================================================
					INITIALISATION GENERALE DES IO
=====================================================================*/

void Conf_Generale_IO_Carte (void);
/*
La fonction permet de :
- configurer l'entrée -10V / +10V (entrée onduleur ou retour tachnymètre) : PC1 - Voie ADC 11
- configurer l'entrée 0/3V3   (entrée de consigne potentiomètre sur carte ou externe) : PC0 - Voie ADC 10
- configurer les entrées courants (2.5V +/- S.I)  
- configurer les entrées du codeur incrémental (selon #define) et régler TIM2 en mode incrémental
- configurer les LED de visualisation (par défaut éteinte)
- configurer la PWM 
- configurer la PWM_Aux 

		
*/

// .......................................................
// 				 new .V4	 														
// .......................................................
void Conf_Sorties_PWM_Aux_Mode_PushPull_Idpt(void);
/*
La fontion permet de configurer les sorties PWM_Aux en sorties push-pull.
Ces sorties sont les /ShDn des IRF2104
La PWM aux n'est donc plus active
 !!! A lancer APRES Conf_IO_Generale !!!
 
 Les macros à utiliser sont donnée ci-dessous :
*/

// forçage 0/1 des sorties /ShDn pour IR 2104 (PWM aux)
//					 1 -> TIM1_Ch1 : PA8				
//					 2 -> TIM4_Ch3 : PB8
//					 3  ->TIM4_Ch4 : PB9
#define Bras_1_On	(GPIOA->BSRR=(0x01<<8))
#define Bras_1_Off	(GPIOA->BRR=(0x01<<8))
#define Bras_2_On	(GPIOB->BSRR=(0x01<<8))
#define Bras_2_Off	(GPIOB->BRR=(0x01<<8))
#define Bras_3_On	(GPIOB->BSRR=(0x01<<9))
#define Bras_3_Off	(GPIOB->BRR=(0x01<<9))


// .......................................................

/*===================================================================
					LED
=====================================================================*/
#define LED_Courant_On 				GPIOB->BSRR=(0x01<<5)
#define LED_Courant_Off 			GPIOB->BRR=(0x01<<5)
#define LED_PWM_On 						GPIOB->BSRR=(0x01<<10)
#define LED_PWM_Off 					GPIOB-->BRR=(0x01<<10)
#define LED_PWM_Aux_On 				GPIOC->BSRR=(0x01<<7)
#define LED_PWM_Aux_Off 			GPIOC->BRR=(0x01<<7)
#define LED_Entree_10V_On 		GPIOB->BSRR=(0x01<<6)
#define LED_Entree_10V_Off 		GPIOB->BRR=(0x01<<6)
#define LED_Entree_3V3_On 		GPIOA->BSRR=(0x01<<9)
#define LED_Entree_3V3_Off 		GPIOA->BRR=(0x01<<9)
#define LED_Codeur_On 				GPIOA->BSRR=(0x01<<10)
#define LED_Codeur_Off 				GPIOA->BRR=(0x01<<10)
// Exemple :
//LED_PWM_On; // allume le LED PWM
		


/*===================================================================
					INTERRUPTIONS
=====================================================================*/
void Conf_IT_Principale_Systick(void (*IT_function) (void),float Periode_IT_us );
// Priorité : Voir user define en début de ce fichier

void Conf_IT_Externe_3V3(void (*IT_function) (void));
// Priorité : Voir user define en début de ce fichier
// Exemple 1:
// Conf_IT_Externe_3V3(IT_Ext_3V3);  
// lance la fonction void IT_Ext_3V3() sur IT externe
// Exemple 2:
// Conf_IT_Principale_Systick(IT_Principale,120.0);
// Lance la fonction void IT_Principale() toute les 120us

// .......................................................
// 				 new .V2 	 														
// .......................................................
void Init_IT_Ext_ABI_Rise_Fall (void (*IT_Fct_A) (void),void (*IT_Fct_B) (void),void (*IT_Fct_I) (void),char Prio );
// Configure les entrées codeur inc A,B, I en IT externe sur front montant et descendant
// Ces 3 entrées sont alors destinées aux 3 capteurs effet hall du moteur brushless
// A -> Hall 1 A : PA0 - ch1 de TIM2		
// B -> Hall 2 B : PA1 - ch2 de TIM2
// I -> Hall 3 I : PA4 - index 
// la priorité est la même pour toutes les voies
// Une fonction d'IT par voie
// NB : L'IT externe 3V3 et les IT hall ne peuvent pas fonctionner ensemble


/*===================================================================
					CODEUR INCREMENTAL
=====================================================================*/
// Les define à utiliser
#define IT_On 1
#define IT_Off 0

#define Demi_Pas_ChA 2
#define Demi_Pas_ChB 1
#define Quart_Pas 3

void Conf_Codeur_Inc(char Pas, unsigned int Pleine_Echelle, char IT_Passage_a_0);
// configure Timer 2 en codeur incrémental
// Priorité (si IT_On): Voir user define en début de ce fichier

/*

Le pas du codeur: 3 options (En réalité 2, 1/2 pas ou 1/4 de pas)
- Demi_Pas_ChA
- Demi_Pas_ChB
- Quart_Pas 
Pleine_Echelle : donner le nombre de pas possible. Pour le codeur inc 360°, donner 
720 pour un 1/2 pas, 1440 pour un 1/4 de pas

IT_Passage_a_O : 2 options on ou off
- IT_On : une IT est déclenchée sur l'index (passage à 0), le compteur est alors remis à 0
- IT_Off : aucune IT déclenchée.

Exemple :
Conf_Codeur_Inc(Quart_Per, 1440, IT_Off);
Configure le codeur en 1/4 de pas, résolution 1440 (de 0 à 1439), et ne remet pas à 0 le timer sur passage index

*/
#define Codeur_Val 		TIM2->CNT 
// exemple :
//Toto = Codeur_Val; renvoie la valeur courante du codeur

/*===================================================================
					ENTREE ANALOGIQUE
=====================================================================*/
void Conf_ADC(void);
// OBLIGATOIRE. Doit être lancée si on veut se servir d'une des 5 entrées analog.
// A lancer qu'une seule fois
int Entree_3V3(void); // renvoie 0 à 4095 pour 0 à 3V3.
int Entree_10V(void);
int I1(void);
int I2(void);
int I3(void);


/*===================================================================
					PWM  !! Rapport cyclique de 0 à 4096  <=> 0 à 100% !!
=====================================================================*/

extern vu16 Resolution; // doit être visible pour utiliser les macro ToolBox comme R_Cyc_1(Val)
// porteuse PWM en dent de scie (up ou down) CMS bits in TIMx_CR1 à 0
extern vu16 Resolution_1; //idem pour TIM1_ch1
extern vu16 Resolution_2_3; //idem pour TIM4_ch2 et 3


// FONCTIONS (au choix) A LANCER OBLIGATOIREMENT ET AVANT TOUT AUTRE FONCTION PWM
// NB : ces deux fonctions valident automatiquement la voie 1
// NB : la PWM n'est pas lancée
void Triangle ( float F_PWM_KHZ_Val); 
void UpRamp ( float F_PWM_KHZ_Val);

// Demarrage / arrêt PWM
#define Start_PWM 		TIM3->CR1=(TIM3->CR1)|(1<<0)
#define Stop_PWM 			TIM3->CR1=(TIM3->CR1)&~(1<<0)

// Fonctions permettant d'activer chacune des voies PWM
void Active_Voie_PWM(char Voie); // Voie de 1 à 3.
void Desactive_Voie_PWM(char Voie);
void Inv_Voie(char Voie); // permet de donner une logique inverse à la voie PWM (utile pour un pont complet)


// Mise à jour des rapports cycliques des voies PWM
#define R_Cyc_1(Val) 				TIM3->CCR1=((Val)*Resolution)>>12
#define R_Cyc_2(Val) 				TIM3->CCR2=((Val)*Resolution)>>12
#define R_Cyc_3(Val) 				TIM3->CCR3=((Val)*Resolution)>>12


/*===================================================================
					PWM AUXILIAIRE
=====================================================================*/
// FONCTIONS (au choix) A LANCER OBLIGATOIREMENT ET AVANT TOUT AUTRE FONCTION PWM
// NB : les fonctions sont doublées car TIM1 gère la voie 1, TIM4 les voies 2 et 3
void Triangle_Aux_Voie_1 ( float F_PWM_KHZ_Val);
void UpRamp_Aux_Voie_1 ( float F_PWM_KHZ_Val);
void Triangle_Aux_Voie_2_3 ( float F_PWM_KHZ_Val);
void UpRamp_Aux_Voie_2_3 ( float F_PWM_KHZ_Val);
// Demarrage / arrêt PWM_Aux
#define Start_PWM_Aux_1 				TIM1->CR1=(TIM1->CR1)|(1<<0)
#define Stop_PWM_Aux_1 					TIM1->CR1=(TIM1->CR1)&~(1<<0)
#define Start_PWM_Aux_2_3 			TIM4->CR1=(TIM4->CR1)|(1<<0)
#define Stop_PWM_Aux_2_3 				TIM4->CR1=(TIM4->CR1)&~(1<<0)
// Fonctions permettant d'activer chacune des voies PWM
void Active_Voie_PWM_Aux(char Voie); // Voie 1 à 3
//  void Desactive_Voie_PWM_Aux(char Voie); a faire
//  void Inv_Voie_Aux(char Voie); a faire

// Mise à jour des rapports cycliques des voies PWM
#define R_Cyc_Aux_1(Val) 					TIM1->CCR1=((Val)*Resolution_1)>>12
#define R_Cyc_Aux_2(Val)					TIM4->CCR3=((Val)*Resolution_2_3)>>12
#define R_Cyc_Aux_3(Val) 					TIM4->CCR4=((Val)*Resolution_2_3)>>12



// .......................................................
// 				 new .V2a 	 														
// .......................................................

/*===================================================================
					Gestion des sortie pour mode pas à pas 6 états
=====================================================================*/

// test d'état des entrées capteur effet Hall
#define HAll_1_State ((GPIOA->IDR)&(0x01))
#define HAll_2_State (((GPIOA->IDR)>>1)&(0x01))
#define HAll_3_State (((GPIOA->IDR)>>4)&(0x01))
// .......................................................
// 				 new .V2c 	 														
// .......................................................
#define HAll_321_State (((GPIOA->IDR)>>2)&(0x04))|(((GPIOA->IDR))&(0x02))|((GPIOA->IDR)&(0x01))






#endif
