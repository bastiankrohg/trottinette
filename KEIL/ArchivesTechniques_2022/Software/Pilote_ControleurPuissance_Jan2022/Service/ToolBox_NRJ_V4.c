//*********************************************************************************************************
//
//					TOOLBOX_NRJ Boîte à outils pour la carte de commande de puissance
//
//*********************************************************************************************************

/*

MATERIEL :
Carte "Contrôleur de Puissance" + "Nucléo STM32F103"
Configuration de la carte Nucléo version C02 :
					SB62, SB63 (USART) = ON (PA3 et PA2 connectées au connecteur Arduino)
					SB13, SB14 (ST-LINK-USART) = Off déconnection USART débug
					SB21 = off (déconnection User LED du PA5)
Configuration de la carte Nucléo version C01 = conf C02 et :
					SB16, SB50 = On (Osc 8MHz externe en provenance du débugger ST link)


LOGICIEL (compilateur ARM) :
Ce fichier contient des fonctions simples qui permettent d'utiliser les ressources de la carte.
Les ressources sont classées en 5 ensembles :

INITIALISATION GENERALE DES IO (fonction d'init générale et fonction spécifique pour choisir analog/num sur l'entrée
																0 à 3V3)
LED (fonctions de mise à 1 ou à 0 de toutes les LED de  visu)
INTERRUPTIONS (fonction de configuration de l'IT principale cadencée par le Systick et fonction d'IT externe pour l'entrée
							0/3V3. Elle sera utilisée par exemple pour l'onduleur triphasée pour faire avancer le réseau de tension tri)
CODEUR INCREMENTAL (fonction de configuration du Timer 2 en mode codeur + lecture état du codeur)
PWM (fonction de configurations et d'utilisation de la PWM)
(PWM_Aux)

Nécessite la lib Drivers_STM32F103_107_Juin2017a.lib  pour fonctionner.

*/


#include "Clock.h"
#include "stm32f10x.h"
#include "Timer_1234.h"
#include "GPIO.h"
#include "Timer_Systick.h"
#include "NVIC_IT_Ext.h"
#include "ADC_DMA.h"
#include "Toolbox_NRJ_V4.h"

vu16 Resolution;
vu16 Resolution_1;
vu16 Resolution_2_3;

/*===================================================================
					INITIALISATION GENERALE DES IO
=====================================================================*/

void Conf_Generale_IO_Carte (void)
{
/*
DESCRIPTION DES E/S  
La PWM est en output Ppull alternate
La PWM_Aux est en output Ppull alternate
*/ 

/* Liste des E/S par Port:

PA0 - Ch A Codeur
PA1 - Ch B Codeur
PA2 - I1
PA3 - I2
PA4 - index I Codeur
PA5 - I3
PA6 - PWM_1						
PA7 - PWM_2
PA8 - PWM_1_Aux
PA9 - LED_3.3V					
PA10-	LED_CODEUR : PA10
					
PB0	-	PWM_3					
PB5 -	LED_Courant 
PB6 -	LED_+/-10V 
PB8 - PWM_2_Aux
PB9 - PWM_3_Aux
PB10-	LED_PWM 
					
PC7	- LED_PWM_Aux 

*/

CLOCK_Configure();

// *********** ENTREES -/+ 10 V et 0/3V3 *******
//L'initialisation générale permet de :
//- configurer l'entrée -10V / +10V (entrée onduleur ou retour tachnymètre) : PC1 - Voie ADC 11
GPIO_Configure(GPIOC, 11, INPUT, ANALOG);
//- configurer l'entrée 0/3V3   (entrée de consigne potentiomètre sur carte ou externe) : PC0 - Voie ADC 10
GPIO_Configure(GPIOC, 10, INPUT, ANALOG);	
	
// *********** ENTREES COURANT *******	
//configurer les entrées courants (2.5V +/- S.I) => 
//					I1 : PA2 - Voie ADC 2
GPIO_Configure(GPIOA, 2, INPUT, ANALOG);
//					I2 : PA3 - Voie ADC 3
GPIO_Configure(GPIOA, 3, INPUT, ANALOG);
//					I3 : PA5 - Voie ADC 5
GPIO_Configure(GPIOA, 5, INPUT, ANALOG);
		
// *********** CODEUR INCREMENTAL *******		
// configurer les entrées du codeur incrémental simplement entrée (pull up à faire sur le capteur !)
//					A : PA0 - ch1 de TIM2
GPIO_Configure(GPIOA, 0, INPUT, INPUT_FLOATING);
//					B : PA1 - ch2 de TIM2
GPIO_Configure(GPIOA, 1, INPUT, INPUT_FLOATING);
//					  I : PA4 - index en remise à 0 TIM2 sous interruption ou pas selon #define
GPIO_Configure(GPIOA, 4, INPUT, INPUT_FLOATING);

// *********** LED VISU *******	
// configurer les LED de visualisation (par défaut éteinte)
//LED_Courant : PB3
GPIO_Configure(GPIOB, 5, OUTPUT, OUTPUT_PPULL);
GPIO_Clear(GPIOB,3);
//LED_PWM : PB10
GPIO_Configure(GPIOB, 10, OUTPUT, OUTPUT_PPULL);
GPIO_Clear(GPIOB,10);
//LED_PWM_Aux : PC7
GPIO_Configure(GPIOC, 7, OUTPUT, OUTPUT_PPULL);
GPIO_Clear(GPIOC,7);
//LED_+/-10V : PB6
GPIO_Configure(GPIOB, 6, OUTPUT, OUTPUT_PPULL);
GPIO_Clear(GPIOB,6);
//LED_3.3V : PA9
GPIO_Configure(GPIOA, 9, OUTPUT, OUTPUT_PPULL);
GPIO_Clear(GPIOA,9);
//LED_CODEUR : PA10
GPIO_Configure(GPIOA, 10, OUTPUT, OUTPUT_PPULL);
GPIO_Clear(GPIOA,10);

// *********** SORTIES PWMS *******	
//				TIM3_Ch1 : PA6
GPIO_Configure(GPIOA, 6, OUTPUT, ALT_PPULL );
//				TIM3_Ch2 : PA7
GPIO_Configure(GPIOA, 7, OUTPUT, ALT_PPULL );
//					TIM3_Ch3 : PB0
GPIO_Configure(GPIOB, 0, OUTPUT, ALT_PPULL );


// *********** SORTIES PWM NON GEREES*******					
//					TIM1_Ch1 : PA8
GPIO_Configure(GPIOA, 8, OUTPUT, ALT_PPULL );					
//					TIM4_Ch3 : PB8
GPIO_Configure(GPIOB, 8, OUTPUT, ALT_PPULL );
//					TIM4_Ch4 : PB9
GPIO_Configure(GPIOB, 9, OUTPUT, ALT_PPULL );				

}

// .......................................................
// 				 new .V4 	 														
// .......................................................
void Conf_Sorties_PWM_Aux_Mode_PushPull_Idpt(void)
{
/*
La fontion permet de configurer les sorties PWM_Aux en sorties push-pull.
Ces sorties sont les commande /SHDN des bras
*/
//					/SHDN 1 -> TIM1_Ch1 : PA8
GPIO_Configure(GPIOA, 8, OUTPUT, OUTPUT_PPULL );					
//					/SHDN 2 -> TIM4_Ch3 : PB8
GPIO_Configure(GPIOB, 8, OUTPUT, OUTPUT_PPULL );
//					/SHDN 3  ->TIM4_Ch4 : PB9
GPIO_Configure(GPIOB, 9, OUTPUT, OUTPUT_PPULL );

	
}
// .......................................................

/*===================================================================
					INTERRUPTION
=====================================================================*/
void Conf_IT_Principale_Systick(void (*IT_function) (void),float Periode_IT_us )
{
	Systick_Prio_IT(Prio_Systick,IT_function);
	Systick_Period(Periode_IT_us);
	SysTick_Enable_IT;
	SysTick_On;
}

void Conf_IT_Externe_3V3(void (*IT_function) (void))
{
NVIC_Ext_IT (GPIOC, 0, RISING_EDGE, INPUT_FLOATING,Prio_IT_3V3,IT_function);	
}

// .......................................................
// 				 new .V2 	 														
// .......................................................
void Init_IT_Ext_ABI_Rise_Fall (void (*IT_Fct_A) (void),void (*IT_Fct_B) (void),void (*IT_Fct_I) (void),char Prio )
{
/*PA0 - Ch A Codeur
  PA1 - Ch B Codeur
  PA4 - index I Codeur*/
	
	NVIC_Ext_IT (GPIOA, 0, FALLING_RISING_EDGE, INPUT_FLOATING,Prio,IT_Fct_A);
	NVIC_Ext_IT (GPIOA, 1, FALLING_RISING_EDGE, INPUT_FLOATING,Prio,IT_Fct_B);
	NVIC_Ext_IT (GPIOA, 4, FALLING_RISING_EDGE, INPUT_FLOATING,Prio,IT_Fct_I);
}



/*===================================================================
					ENTREE ANALOGIQUE
					Entrée +/-10V - Voie ADC 11
					Entrée 0/3V3  - Voie ADC 10
					I1 : PA2 - Voie ADC 2
					I2 : PA3 - Voie ADC 3
					I3 : PA5 - Voie ADC 5
=====================================================================*/
void Conf_ADC(void) // par défaut sur l'entrée +/-10V
{
	Init_TimingADC_ActiveADC( ADC1, 0.0);
	Single_Channel_ADC(ADC1, 11);
}

//_______________________________________
int Entree_3V3(void)
{
// Specif de la voie ADC
ADC1->SQR3=(ADC1->SQR3) & ~0x001F;
ADC1->SQR3=(ADC1->SQR3) | 10;
Start_ADC_Conversion(ADC1);
// tant que EOC=0 rien
while (((ADC1->SR)& 0x2) == 0)
	{}
ADC1->SR=(ADC1->SR)&~(0x1<<1); // mise à 0 du flag
return (ADC1->DR);
}
//_______________________________________
int Entree_10V(void)
{
// Specif de la voie ADC
ADC1->SQR3=(ADC1->SQR3) & ~0x001F;
ADC1->SQR3=(ADC1->SQR3) | 11;
Start_ADC_Conversion(ADC1);
// tant que EOC=0 rien
while (((ADC1->SR)& 0x2) == 0)
	{}
ADC1->SR=(ADC1->SR)&~(0x1<<1); // mise à 0 du flag
return (ADC1->DR);
}
//_______________________________________
int I1(void)
{
// Specif de la voie ADC
ADC1->SQR3=(ADC1->SQR3) & ~0x001F;
ADC1->SQR3=(ADC1->SQR3) | 2;
Start_ADC_Conversion(ADC1);
// tant que EOC=0 rien
while (((ADC1->SR)& 0x2) == 0)
	{}
ADC1->SR=(ADC1->SR)&~(0x1<<1); // mise à 0 du flag
return (ADC1->DR);
}
//_______________________________________
int I2(void)
{
// Specif de la voie ADC
ADC1->SQR3=(ADC1->SQR3) & ~0x001F;
ADC1->SQR3=(ADC1->SQR3) | 3;
Start_ADC_Conversion(ADC1);
// tant que EOC=0 rien
while (((ADC1->SR)& 0x2) == 0)
	{}
ADC1->SR=(ADC1->SR)&~(0x1<<1); // mise à 0 du flag
return (ADC1->DR);
}
//_______________________________________
int I3(void)
{
// Specif de la voie ADC
ADC1->SQR3=(ADC1->SQR3) & ~0x001F;
ADC1->SQR3=(ADC1->SQR3) | 5;
Start_ADC_Conversion(ADC1);
// tant que EOC=0 rien
while (((ADC1->SR)& 0x2) == 0)
	{}
ADC1->SR=(ADC1->SR)&~(0x1<<1); // mise à 0 du flag
return (ADC1->DR);
}


	
/*===================================================================
					CODEUR INCREMENTAL
=====================================================================*/
void IT_Index(void)
{
	CNT(TIM2)=0;
}

void Conf_Codeur_Inc(char Pas, unsigned int Pleine_Echelle, char IT_Passage_a_O)
{
if (	IT_Passage_a_O==IT_On) NVIC_Ext_IT (GPIOA, 4, RISING_EDGE, INPUT_FLOATING,Prio_Index_Codeur,IT_Index);	
	
Timer_Inc_Init(TIM2, Reso_Quart_Per);
Bloque_Timer(TIM2);
Reset_Timer(TIM2);	
ARR(TIM2)=Pleine_Echelle-1;
Run_Timer(TIM2);	
}




/*===================================================================
					PWM  !! Rapport cyclique de 0 à 4096  !!
=====================================================================*/

#define PWM_Edge_Aligned(Timer) Timer->CR1=Timer->CR1&~(3<<5)
// porteuse PWM en triangle CMS bits in TIMx_CR1 à 11
#define PWM_Center_Aligned(Timer) Timer->CR1=Timer->CR1|(3<<5)

// FONCTIONS (au choix) A LANCER OBLIGATOIREMENT ET AVANT TOUT AUTRE FONCTION PWM
void Triangle ( float F_PWM_KHZ_Val)
{
// Réglage de la porteuse en triangle symétrique :
Resolution=PWM_Init(TIM3, 1, 2*F_PWM_KHZ_Val);
Bloque_Timer(TIM3);
CNT(TIM3)=0;	
CptUp(TIM3);
PWM_Center_Aligned(TIM3);
}



void UpRamp ( float F_PWM_KHZ_Val)
{
Resolution=PWM_Init(TIM3, 1, F_PWM_KHZ_Val);
Bloque_Timer(TIM3);
CNT(TIM3)=0;
CptUp(TIM3);
PWM_Edge_Aligned(TIM3);
}



// Fonctions permettant d'activer chacune des voies PWM
void Active_Voie_PWM(char Voie)
{
(RCC->APB1ENR)=(RCC->APB1ENR) | RCC_APB1ENR_TIM3EN;
//Config OCy en sortie, implicite mais bon :
if ((Voie==1) || (Voie==2)) 
{
//Config OCy en sortie, implicite mais bon :
TIM3->CCMR1 = (TIM3->CCMR1)& ~(0x03<<((Voie-1)*8));	//00 ds le champs CCyS
//PWM en mode Edge aligned OCyM=110
TIM3->CCMR1 = (TIM3->CCMR1)& ~(0x07<<((Voie-1)*8+4));
TIM3->CCMR1 = (TIM3->CCMR1)|(0x06<<((Voie-1)*8+4));	
}
if ((Voie==3) || (Voie==4)) 
{
//Config OCy en sortie, implicite mais bon :
TIM3->CCMR2 = (TIM3->CCMR2)& ~(0x03<<((Voie-3)*8));	//00 ds le champs CCyS
//PWM en mode Edge aligned OCyM=110
TIM3->CCMR2 = (TIM3->CCMR2)& ~(0x07<<((Voie-3)*8+4));
TIM3->CCMR2 = (TIM3->CCMR2)|(0x06<<((Voie-3)*8+4));	
}
// polarité positive bit CCyP = 0, reg CCER
TIM3->CCER = (TIM3->CCER)& ~(0x01<<((Voie-1)*4+1));
// validation du canal y, bit CCyE, reg CCER
TIM3->CCER = (TIM3->CCER)|(0x01<<((Voie-1)*4));
}

void Desactive_Voie_PWM(char Voie)
{
(RCC->APB1ENR)=(RCC->APB1ENR) | RCC_APB1ENR_TIM3EN;
// validation du canal y, bit CCyE, reg CCER
TIM3->CCER = (TIM3->CCER)&~(0x01<<((Voie-1)*4));
}

void Inv_Voie(char Voie)
{
	(RCC->APB1ENR)=(RCC->APB1ENR) | RCC_APB1ENR_TIM3EN;
	// Inversion des voies
	if (Voie==1) TIM3->CCER = (TIM3->CCER)|(0x01<<1);
	if (Voie==2) TIM3->CCER = (TIM3->CCER)|(0x01<<5);
	if (Voie==3) TIM3->CCER = (TIM3->CCER)|(0x01<<9);	
	
}

/*===================================================================
					PWM AUXILIAIRE

					TIM1_Ch1 : PA8
					TIM4_Ch3 : PB8
					TIM4_Ch4 : PB9
=====================================================================*/
// FONCTIONS (au choix) A LANCER OBLIGATOIREMENT ET AVANT TOUT AUTRE FONCTION PWM
void Triangle_Aux_Voie_1 ( float F_PWM_KHZ_Val)
{
// Réglage de la porteuse en triangle symétrique :
Resolution_1=PWM_Init(TIM1, 1, 2*F_PWM_KHZ_Val);
Bloque_Timer(TIM1);
CNT(TIM1)=0;	
CptUp(TIM1);
PWM_Center_Aligned(TIM1);
}


void UpRamp_Aux_Voie_1 ( float F_PWM_KHZ_Val)
{
Resolution_1=PWM_Init(TIM1, 1, F_PWM_KHZ_Val);
Bloque_Timer(TIM1);
CNT(TIM1)=0;
CptUp(TIM1);
PWM_Edge_Aligned(TIM1);
}

void Triangle_Aux_Voie_2_3 ( float F_PWM_KHZ_Val)
{
// Réglage de la porteuse en triangle symétrique :
Resolution_2_3=PWM_Init(TIM4, 1, 2*F_PWM_KHZ_Val);
Bloque_Timer(TIM4);
CNT(TIM4)=0;	
CptUp(TIM4);
PWM_Center_Aligned(TIM4);
}

void UpRamp_Aux_Voie_2_3 ( float F_PWM_KHZ_Val)
{
Resolution_2_3=PWM_Init(TIM4, 1, F_PWM_KHZ_Val);
Bloque_Timer(TIM4);
CNT(TIM4)=0;
CptUp(TIM4);
PWM_Edge_Aligned(TIM4);
}



// Fonctions permettant d'activer chacune des voies PWM
		//			TIM1_Ch1 : PA8
		//			TIM4_Ch3 : PB8
		//			TIM4_Ch4 : PB9
void Active_Voie_PWM_Aux(char Voie)
{

if (Voie==1)  
{
(RCC->APB2ENR)=(RCC->APB2ENR) | RCC_APB2ENR_TIM1EN;	
//Config OCy en sortie, implicite mais bon :
TIM1->CCMR1 = (TIM1->CCMR1)& ~(0x03<<((Voie-1)*8));	//00 ds le champs CCyS
//PWM en mode Edge aligned OCyM=110
TIM1->CCMR1 = (TIM1->CCMR1)& ~(0x07<<((Voie-1)*8+4));
TIM1->CCMR1 = (TIM1->CCMR1)|(0x06<<((Voie-1)*8+4));	
// polarité positive bit CCyP = 0, reg CCER
TIM1->CCER = (TIM1->CCER)& ~(0x01<<((Voie-1)*4+1));
// validation du canal y, bit CCyE, reg CCER
TIM1->CCER = (TIM1->CCER)|(0x01<<((Voie-1)*4));
}
if ((Voie==2) || (Voie==3)) 
{
// Modif des voies, ne pas confondre voie PWM pour Timer et voie PWM pour carte
Voie= Voie+1;	
//Config OCy en sortie, implicite mais bon :
TIM4->CCMR2 = (TIM4->CCMR2)& ~(0x03<<((Voie-3)*8));	//00 ds le champs CCyS
//PWM en mode Edge aligned OCyM=110
TIM4->CCMR2 = (TIM4->CCMR2)& ~(0x07<<((Voie-3)*8+4));
TIM4->CCMR2 = (TIM4->CCMR2)|(0x06<<((Voie-3)*8+4));	
// polarité positive bit CCyP = 0, reg CCER
TIM4->CCER = (TIM4->CCER)& ~(0x01<<((Voie-1)*4+1));
// validation du canal y, bit CCyE, reg CCER
TIM4->CCER = (TIM4->CCER)|(0x01<<((Voie-1)*4));
}

}

void Desactive_Voie_PWM_Aux(char Voie);
void Inv_Voie_Aux(char Voie);

//*********************************************************************************************************
//
//					TOOLBOX TESTS EN SIMULATION (Test_Soft_Puissance.ini)
//
//*********************************************************************************************************
/*===================================================================
					INTERRUPTION
INT systick OK
INT externe sur entrée 3V3 OK

=====================================================================*/

/*===================================================================
					PWM  !! Rapport cyclique de 0 à 4096  !!
Active_Voie_PWM validé (voie 2)
void Triangle ( float F_PWM_KHZ_Val) OK
void UpRamp ( float F_PWM_KHZ_Val) OK
R_Cyc_1(Val) OK de 0 à 4096
void Inv_Voie(char Voie) OK pour Voie =2

=====================================================================*/

/*===================================================================
					ENTREE ANALOGIQUE
				
void Conf_ADC(void) validé
int Entree_10V(void) validé sur appli onduleur bipolaire

=====================================================================*/

/*===================================================================
					TESTS GENERAUX
				
Onduleur triphasé OK, réglage d'amplitude signé OK

=====================================================================*/
