
/*
	!!!! NB : ALIMENTER LA CARTE AVANT DE CONNECTER L'USB !!!

*/


/*
STRUCTURE DES FICHIERS

COUCHE APPLI : 
Main_User.c : programme principal à modifier. Par défaut hacheur sur entrée +/-10V, sortie 1 PWM

COUCHE SERVICE :
Toolbox_NRJ.c : contient toutes les fonctions utilisables, qui "parlent" à un électrotechnicien
Table_Cosinus_Tri.c : contient 3 tables de cosinus déphasés de 120°, 36 pts (10° de résolution)
Table_Cosinus_Tri_120pts.c : idem mais 120 pts par cosinus

COUCHE DRIVER :
clock.c : contient LA fonction Clock_Configure() qui prépare le STM32. Lancée automatiquement à l'init IO
lib : bibliothèque qui gère les périphériques du STM : Drivers_STM32F103_107_Jan_2015_b
*/



/*
VOIR LE FICHIER TOOLBOX_NRJ.H POUR TOUTES INFOS SUR LES E/S
*/




#include "ToolBox_NRJ_v3.h"
#include "Table_Cosinus_Tri.h"
#include "math.h"


//=================================================================================================================
// 					USER DEFINE
//=================================================================================================================

// Choix de la fréquence PWM (en kHz)
#define FPWM_Khz 20.0
// Choix de la fréquence d'échantillonnage de l'ADC (en kHz)
#define Fe_Khz 3.6
// Choix de la fréquence d'échantillonnage de l'ADC (en Hz)
#define Fe_Hz 3600
// Choix de la fréquence de transition (en Hz)
#define Ft_Hz 400.0

//Choix des paramètres des composants
#define pi 3.14
#define L 0.002 
#define R 1.0
#define Ubatt 24.0
#define Kfiltre 1.45
#define F2 2.15e3
#define S 0.104



//==========END USER DEFINE========================================================================================

// ========= Variable globales indispensables et déclarations fct d'IT ============================================
#define Te_us (1000.0/Fe_Khz) // Te_us est la période d'échantillonnage en float, exprimée en µs
void IT_Principale(void);
void IT_Ext_3V3(void);
//=================================================================================================================


/*=================================================================================================================
 					FONCTION MAIN : 
					NB : On veillera à allumer les diodes au niveau des E/S utilisée par le progamme. 
					
					EXEMPLE: Ce progamme permet de générer une PWM (Voie 1) à 20kHz dont le rapport cyclique se règle
					par le potentiomètre de "l'entrée Analogique +/-10V"
					Placer le cavalier sur la position "Pot."
					La mise à jour du rapport cyclique se fait à la fréquence 1kHz.

//=================================================================================================================*/

//variables
float tau_2 ;
float Kg ;
float tau_i;
float tau_0 ;
float wt; 
float w2 ;
float b0;
float b1;
float a0;
float a1;
float A0;
float A1;
float Te; 

float Sn,Sn_prec, U_prec; 

int main (void)
{
Sn_prec =0.0 ;
U_prec = 0.0; 
Te = 1.0/Fe_Hz; 
w2=2.0*pi*F2;
tau_2=(1.0)/(w2) ;
Kg=Kfiltre*S*2.0*Ubatt ;
wt = 2.0*pi*Ft_Hz;
tau_i=Kg/((wt*sqrt(1.0+(tau_2*wt*tau_2*wt))));
tau_0=L/R ;

b0=(Te)-(2.0*tau_0);
b1=(Te)+(2.0*tau_0);
a0=-2.0*tau_i;
a1=2.0*tau_i;



	
// !OBLIGATOIRE! //		
Conf_Generale_IO_Carte();

//______________ Ecrire ici toutes les CONFIGURATIONS des périphériques ________________________________

	
// Paramétrage ADC pour entrée analogique
Conf_ADC();
	
// Conf IT
Conf_IT_Principale_Systick(IT_Principale, Te_us);
	
// Configuration de la PWM avec une porteuse Triangle 
Triangle (FPWM_Khz);
Active_Voie_PWM(1);
Active_Voie_PWM(2);
Inv_Voie(2); 
Start_PWM;
R_Cyc_1(2048);  // positionnement à 50% par défaut de la PWM
R_Cyc_2(2048) ;


// Activation LED
LED_Courant_On;
LED_PWM_On;
LED_PWM_Aux_On;
LED_Entree_10V_On;
LED_Entree_3V3_Off;
LED_Codeur_Off;

	while(1)
	{
		//boucle infinie
	}

}





//=================================================================================================================
// 					FONCTION D'INTERRUPTION PRINCIPALE SYSTICK
//=================================================================================================================
int Cy_1, Cy_2, Cy_3;
float Courant_1,Courant_2,Courant_3;
float In_V, In2_V;
float epsilon; 
float Sortie; 
void IT_Principale(void)

{
	
A1 = (b1/a1) ;
A0=(b0/a1);
	
	//ACQUISITION
 
 epsilon = (3.3/4095.0)*((float)(Entree_3V3()-I1())); 
	
 //CORRECTION 
	Sn= A1*epsilon +A0*U_prec + Sn_prec; 
	
 
	//SORTIE
	//saturateur 
		if (Sn> 0.95) 
		{
			Sn = 0.95; 
		}
		else if (Sn< 0.05) 
		{
			Sn = 0.05; 
		}
		
	Sn_prec = Sn; 
	U_prec= epsilon;
		
	Sortie = Sn*4095.0; 
		
  R_Cyc_1((int)Sortie);
	R_Cyc_2((int)Sortie); 
}

//=================================================================================================================
// 					FONCTION D'INTERRUPTION EXTERNE (Entrée 0/3V3) 
//=================================================================================================================

void IT_Ext_3V3(void)
{
}

