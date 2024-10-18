#include <stdio.h>
#include <math.h>
#include <string.h>
#include "sys/cm4.h"
#include "sys/devices.h"
#include "sys/init.h"
#include "sys/clock.h"

static volatile char c=0;

static volatile uint32_t var = 0;
static volatile uint32_t limite = 300;


void init_LD2(){
	/* on positionne ce qu'il faut dans les différents
	   registres concernés */
	RCC.AHB1ENR |= 0x01;
	GPIOA.MODER = (GPIOA.MODER & 0xFFFFF3FF) | 0x00000400;
	GPIOA.OTYPER &= ~(0x1<<5);
	GPIOA.OSPEEDR |= 0x03<<10;
	GPIOA.PUPDR &= 0xFFFFF3FF;// GPIOA.ODR &= ~0x1<<5;
}

void init_PB(){
	enable_GPIOC();
	GPIOC.MODER &= (~0x3<<26);
	GPIOC.PUPDR &= (~0x3<<26);
	/* GPIOC.MODER = ... */
}


void init_LDR1(){
	//PA4
	enable_GPIOA();
	GPIOA.MODER = (GPIOA.MODER &(~(0x3<<8)))|(0x1 << 8);
	GPIOA.OTYPER &= ~(0x1<<4);
	GPIOA.OSPEEDR |= 0x03<<8;
	GPIOA.PUPDR &= (~0x3<<8);
	// GPIO_InitStruct.Pin = GPIO_Pin_7;

}

void init_LDJ1(){
	//PB6
	enable_GPIOB();
	GPIOB.MODER = (GPIOB.MODER &(~(0x3<<12)))|(0x1 << 12);
	GPIOB.OTYPER &= ~(0x1<<6);
	GPIOB.OSPEEDR |= 0x03<<12;
	GPIOB.PUPDR &= (~0x3<<12);
}

void init_LDV1(){
	//PA6
	enable_GPIOA();
	GPIOA.MODER = (GPIOA.MODER &(~(0x3<<12)))|(0x1 << 12);
	GPIOA.OTYPER &= ~(0x1<<6);
	GPIOA.OSPEEDR |= 0x03<<12;
	GPIOA.PUPDR &= (~0x3<<12);
}


void init_LDR2(){
	//PB4
	enable_GPIOB();
	GPIOB.MODER = (GPIOB.MODER &(~(0x3<<8)))|(0x1 << 8);
	GPIOB.OTYPER &= ~(0x1<<4);
	GPIOB.OSPEEDR |= 0x03<<8;
	GPIOB.PUPDR &= (~0x3<<8);
}

void init_LDV2(){
	//PB5
	enable_GPIOB();
	GPIOB.MODER = (GPIOB.MODER &(~(0x3<<10)))|(0x1 << 10);
	GPIOB.OTYPER &= ~(0x1<<5);
	GPIOB.OSPEEDR |= 0x03<<10;
	GPIOB.PUPDR &= (~0x3<<10);
}


void init_PBfeu(){
	//PA7
	enable_GPIOA();
	GPIOA.MODER &= (~(0x3<<14));
	GPIOA.PUPDR &= (~(0x3<<14));
}

/* Ne pas optimiser cette fonction : on dépend du nombre
 * approximatif d'instructions qu'elle exécute
 */
__attribute__((optimize("O0")))
void tempo_500ms(){
	volatile uint32_t duree;
	/* estimation, suppose que le compilateur n'optimise pas trop... */
	for (duree = 0; duree < 5600000 ; duree++){
		;
	}

}

void tempo_s(int n){
	for(int i = 0 ;  i<n*2 ; i++){
		tempo_500ms();
	}
}

void init_USART(){
	GPIOA.MODER = (GPIOA.MODER & 0xFFFFFF0F) | 0x000000A0;
	GPIOA.AFRL = (GPIOA.AFRL & 0xFFFF00FF) | 0x00007700;
	USART2.BRR = get_APB1CLK()/9600;
	USART2.CR3 = 0;
	USART2.CR2 = 0;
}

void _putc(const char c){
	while( (USART2.SR & 0x80) == 0);  
	USART2.DR = c;
}

void _puts(const char *c){
	int len = strlen(c);
	for (int i=0;i<len;i++){
		_putc(c[i]);
	}
}

char _getc(){
	/* À compléter */
	return 0;
}

/* Initialisation du timer système (systick) */
/* La fréquence en entrée est en Hz.
 * Elle doit être >= 11 du fait de la taille du registre LOAD */
void systick_init(uint32_t freq){
	uint32_t p = get_SYSCLK()/freq;
	/* ATTENTION: la valeur calculée doit tenir dans le registre sur 24 bits */
	SysTick.LOAD = (p-1) & 0x00FFFFFF;
	SysTick.VAL = 0;
	SysTick.CTRL |= 7;
}

/* Cet attibut n'est plus nécessaire sur l'architecture ARM-Cortex-M4:
 * Les fonctions d'interruption respectent les conventions d'appel
 * de la plateforme.
 * Voir par exemple le paragraphe "Exception Entry & Exit" de
 * https://interrupt.memfault.com/blog/arm-cortex-m-exceptions-and-nvic
 */
// #pragma GCC diagnostic push
// #pragma GCC diagnostic ignored "-Wattributes"GPIOB.ODR |= 0x1<<4;
// __attribute__((interrupt))
void SysTick_Handler(){
// #pragma GCC diagnostic pop
	/* Le fait de définir cette fonction suffit pour
	 * qu'elle soit utilisée comme traitant,
	 * cf les fichiers de compilation et d'édition de lien
	 * pour plus de détails.
	 */
	/* ... */
	if(var>=500){
		var = 0;
	}
	var++;
}

/* Fonction non bloquante envoyant une chaîne par l'UART */
int _async_puts(const char* s) {
	/* Cette fonction doit utiliser un traitant d'interruption
	 * pour gérer l'envoi de la chaîne s (qui doit rester
	 * valide pendant tout l'envoi). Elle doit donc être
	 * non bloquante (pas d'attente active non plus) et
	 * renvoyer 0.
	 *
	 * Si une chaîne est déjà en cours d'envoi, cette
	 * fonction doit renvoyer 1 (et ignorer la nouvelle
	 * chaîne).
	 *
	 * Si s est NULL, le code de retour permet de savoir
	 * si une chaîne est encore en cours d'envoi ou si
	 * une nouvelle chaîne peut être envoyée.
	 */
	/* À compléter */
	
	return 0;
}

int main() {
  
	printf("\e[2J\e[1;1H\r\n");
	printf("\e[01;32m*** Welcome to Nucleo F446 ! ***\e[00m\r\n");

	printf("\e[01;31m\t%08lx-%08lx-%08lx\e[00m\r\n",
	       U_ID[0],U_ID[1],U_ID[2]);
	printf("SYSCLK = %9lu Hz\r\n",get_SYSCLK());
	printf("AHBCLK = %9lu Hz\r\n",get_AHBCLK());
	printf("APB1CLK= %9lu Hz\r\n",get_APB1CLK());
	printf("APB2CLK= %9lu Hz\r\n",get_APB2CLK());
	printf("\r\n");

	init_USART();
	init_PB();
	init_LD2();
	systick_init(1);

	while (1){
		if((GPIOC.IDR&(0x1<<13))==0){
			limite -=100;
			if(limite<=100) limite= 500;
		}
		tempo_500ms();
	}


	return 0;
}

