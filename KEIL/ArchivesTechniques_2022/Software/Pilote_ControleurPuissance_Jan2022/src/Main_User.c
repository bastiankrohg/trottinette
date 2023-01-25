/*
* BE Trottinette
* Bastian Krohg et Nicolas Siard
* 4A AE-SE Groupe 1 
* Partie correcteur numérique avec u-controleur
*/

/*
	!!!! NB : ALIMENTER LA CARTE AVANT DE CONNECTER L'USB !!!

VERSION 16/12/2021 :
- ToolboxNRJ V4
- Driver version 2021b (synchronisation de la mise à jour Rcy -CCR- avec la rampe)
- Validé Décembre 2021

*/


/*
STRUCTURE DES FICHIERS

COUCHE APPLI = Main_User.c : 
programme principal à modifier. Par défaut hacheur sur entrée +/-10V, sortie 1 PWM
Attention, sur la trottinette réelle, l'entrée se fait sur 3V3.
Attention, l'entrée se fait avec la poignée d'accélération qui va de 0.6V à 2.7V !

COUCHE SERVICE = Toolbox_NRJ_V4.c
Middleware qui configure tous les périphériques nécessaires, avec API "friendly"

COUCHE DRIVER =
clock.c : contient la fonction Clock_Configure() qui prépare le STM32. Lancée automatiquement à l'init IO
lib : bibliothèque qui gère les périphériques du STM : Drivers_STM32F103_107_Jan_2015_b
*/

#include "ToolBox_NRJ_v4.h"


//=================================================================================================================
// 					USER DEFINE
//=================================================================================================================
//pour faciliter les notations des calculs de frequence/cste de temps
#define pi 3.1415

// Choix de la fréquence PWM (en kHz)
#define FPWM_Khz 20.0

//Frequence et temps d'échantillonnage, Fe > 2fmax OK
#define Fe 2000
#define Te_s 1/Fe

//Resistance
#define R 1 
//Gain moteur
#define Km 1/R
//Gain Hacheur
#define Khach 48
//Gain capteur de courant
#define Kcourant 0.10416
//Gain Filtre
#define Kf 1.4569

//Frequence de transition souhaitée
#define ft 400 
//Frequence de coupure moteur
#define fm 80
//Cste de temps du moteur
#define tau_m 1/(2*pi*fm)

/*
//Gain Systeme global
#define K Km*Khach*Kcourant*Kf
//Frequence de coupure du filtre
#define ff 2000
//Cste de temps du filtre
#define tau_f 1/(2*pi*ff)
*/


//Cste de temps pour partie integrateur du correcteur
#define tau_i Km*Khach*Kcourant*Kf/(2*pi*ft)
//Frequence de coupure pour partie integrateur
#define fi 1/(2*pi*tau_i)
//Deuxieme cste de temps pour correcteur
#define tau_c tau_m 

//Coeffs de l'equation recurrente du correcteur C
//Verification avec matlab c2d et valeurs numériques OK
//C(z) = (C_a0*z - C_a1)/(z - 1)
#define C_a0 (Te_s/(2*tau_i)+tau_c/tau_i)
#define C_a1 (Te_s/(2*tau_i)-tau_c/tau_i)

//==========END USER DEFINE========================================================================================

// ========= Variable globales indispensables et déclarations fct d'IT ============================================

void IT_Principale(void);
//=================================================================================================================


/*=================================================================================================================
 					FONCTION MAIN : 
					NB : On veillera à allumer les diodes au niveau des E/S utilisée par le progamme. 
					
					EXEMPLE: Ce progamme permet de générer une PWM (Voie 1) à 20kHz dont le rapport cyclique se règle
					par le potentiomètre de "l'entrée Analogique +/-10V"
					Placer le cavalier sur la position "Pot."
					La mise à jour du rapport cyclique se fait à la fréquence 1kHz.

//=================================================================================================================*/


float Te,Te_us;
int alpha_new, alpha_old, epsilon_new, epsilon_old; 

int main (void)
{
// !OBLIGATOIRE! //	
Conf_Generale_IO_Carte();	

	
// ------------- Discret, choix de Te -------------------	
Te=	1/Fe; // en seconde
Te_us=Te*1000000.0; // conversion en µs pour utilisation dans la fonction d'init d'interruption
	

//______________ Ecrire ici toutes les CONFIGURATIONS des périphériques ________________________________	
// Paramétrage ADC pour entrée analogique
Conf_ADC();
// Configuration de la PWM avec une porteuse Triangle, voie 1 & 2 activée, inversion voie 2
Triangle (FPWM_Khz);
Active_Voie_PWM(1);	
Active_Voie_PWM(2);	
Inv_Voie(2);

Start_PWM;
R_Cyc_1(2048);  // positionnement à 50% par défaut de la PWM
R_Cyc_2(2048);

// Activation LED
LED_Courant_On;
LED_PWM_On;
LED_PWM_Aux_Off;
LED_Entree_10V_On;
LED_Entree_3V3_Off; 
LED_Codeur_Off;

// Conf IT
Conf_IT_Principale_Systick(IT_Principale, Te_us);

//Init valeur de alpha avant calcul recurrent
alpha_old = 0;
epsilon_old = 0; //la valeur d'avant de la consigne

	while(1)
	{
	//appliquer une entree 0.1V pour tester et verifier comportement avec celui du simulink
	}

}





//=================================================================================================================
// 					FONCTION D'INTERRUPTION PRINCIPALE SYSTICK
//=================================================================================================================
int Courant_1,Cons_In, epsilon;


void IT_Principale(void)
{
	//acq consigne
	Cons_In=Entree_10V(); 
	//Entree comprise entre 0 et 3v3 physiquement pour notre système
	//4096 <=> 100% rapport cyclique, par defaut ils sont à 50%
	R_Cyc_1(Cons_In);
	R_Cyc_2(Cons_In);
	
	//acq courant
	Courant_1 = I1();
	
	//Calculer l'erreur
	epsilon_new = Cons_In - Courant_1;
	
	//Calcul alpha_new en fonction de la nouvelle consigne (Signal S_n en fonction de S_n-1, entree_n et entree_n-1)
	alpha_new = alpha_old - C_a0*epsilon_old + C_a1*epsilon_new;
	
	//Saturation - Rapport cyclique par defaut à 50% (R_Cyc_1(2048), R_Cyc_2(2048))
	/* Pt de repos: (Logique: "2048"/"0.5"/"50%" => alpha=0) 
	donc si alpha dépasse 4096 (Logique: "4096"/"1.0"/"100%" => alpha=0.5)
	ou si alpha descend plus que 0 (Logique: "0"/"0.0"/"0%" => alpha=-0.5)
	
	On veut donc saturer le alpha lorsqu'il depasse 4096/100% ou lorsqu'il descend plus que 0/0% car
	les valeurs hors l'intervalle [0;4096], donc des rapports cycliques de < 0 ou > 100% 
	n'ont pas de sens physique pour notre commande/système. 
	*/
	if (alpha_new > 4096) {
		alpha_new = 4096; 
	} else if (alpha_new < 0) {
		alpha_new = 0;
	}
		
	//Mise a jour des alpha old / alpha new / consigne 
	alpha_old = alpha_new;
	epsilon_old = epsilon_new;
	
	//Mise a jour rapport cyclique du PWM?
	//R_Cyc_1(alpha_new);
	//R_Cyc_2(alpha_new);
}

