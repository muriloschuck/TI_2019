#include "stm32f4xx.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#define LATCH GPIO_ODR_ODR_0
#define CLK GPIO_ODR_ODR_1
#define INFOR GPIO_ODR_ODR_2
#define INFOG GPIO_ODR_ODR_8
#define A GPIO_ODR_ODR_3
#define B GPIO_ODR_ODR_4
#define C GPIO_ODR_ODR_5

int cont = 0, uni, i = 0, num, j = 0;
int red[8][8];
int green[8][8];
int rabo[20][2];	//primeira posicao = tamanho do Rabo; segunda posicao = localizacao de cada parte do rabo
int head[2];		//duas posi��es - localizacao da cabeca
int gameOver;
int x, y;
int fruitX, fruitY;
unsigned int seed = 0;
int startGame = 0;
int bp;
int ajuda = 0;
int score = 0;
int nRabo = 1;

//bluetooth
int a = 0;

enum Direction{
	STOP = 0,
	RIGHT,
	DOWN,
	LEFT,
	UP
} dir;


/**========================================================================================================
 * A fun��o sendData serve para enviar um dado pela serial do bluetooth.
 * ========================================================================================================*/

void sendData(char aux){
	while(!(USART1->SR&USART_SR_TXE));						//Enquanto tiver algo no buffer nada mais ocorre
		USART1->DR = aux;									//Envia o valor de aux
}

/**========================================================================================================
 * A fun��o clearmatriz serve para colocar "0" em todas as posi��es da matriz, deixando-a limpa
 * ========================================================================================================*/

void clearmatriz(int a[8][8]){
	for(int aux = 0; aux<8; aux++){
		for(int temp = 0; temp<8; temp++){
			a[aux][temp] = 0;
		}
	}
}

/**========================================================================================================
 * Abaixo temos a interrup��o da USART1, a qual � respons�vel pelo recebimento da informa��o do teclado.
 * Dependendo da tecla que � apertada, � definida qual dire��o a cobra se deslocar�.
 * ========================================================================================================*/

void USART1_IRQHandler (void)
{
	if(USART1->SR&(USART_SR_RXNE)) 						//Se tiver algo na recep��o
	{
		a = USART1->DR;                                 //Vari�vel recebe esse valor
		if(a == 49)										//Se for 1 (ASCII = 49, no app, up)
		{
			if((dir!=DOWN) || (nRabo == 1))				//Se a dire��o anterior for diferente de Down, ou o rabo for igual a 1
			dir = UP;									//Dire��o agora � up
		}

		if(a == 50){									//2 (ASCII = 50, no app, down)
			if((dir!=UP) || (nRabo == 1))				//Dire��o anterior diferente de up ou rabo igual a 1
			dir = DOWN;									//Dire��o � down
		}

		if(a == 51){									//3 (ASCII = 51, no app, left)
			if((dir!=RIGHT) || (nRabo == 1))			//Dire��o anterior diferente de right ou rabo igual a 1		
			dir = LEFT;									//Dire��o � left
		}

		if(a == 52){									//4 (ASCII = 52, no app, right)
			if((dir!=LEFT) || (nRabo == 1))				//Dire��o anterior diferente de right ou rabo igual a 1
			dir = RIGHT;								//Dire��o � right
		}

		if(a == 53){									//5 (ASCII = 53, no app, espa�o)
			startGame = 1;								//Jogo inicia
		}
		if(a == 54)										//6 (ASCII = 54, no app, sair)
		{
			startGame = 0;								//Jogo � terminado
			gameOver = 1;								
			clearmatriz(red);							//Limpa as matrizes
			clearmatriz(green);
		}
	}

		USART1->SR&=~USART_SR_RXNE;						//Zera a flag da interrup��o
}


/**========================================================================================================
 * A fun��o velocidade define a velocidade em que o jogo acontecer�. A velocidade aumenta conforme a pontua��o
 * do jogador.
 * ========================================================================================================*/

void velocidade(){

	if(score>=30 && score<=60){							//Se o score estiver entre 30 e 60
		TIM13->ARR=449;									//Velocidade n�vel 2
		sendData(2);									//Envia 2 para a serial
	}

	if(score>=70 && score<=90){							//Se estiver entre 70 e 90
		TIM13->ARR=299;									//Velocidade n�vel 3
		sendData(3);									//Envia 3 para a serial
	}

	if(score>=100 && score<=140){						//Se estiver entre 100 e 140
		TIM13->ARR=249;									//Velocidade n�vel 4
		sendData(4);									//Envia 4 para a serial
	}

	if(score>=150){										//Se for maior ou igual a 150
		TIM13->ARR=199;									//Velocidade n�vel 5
		sendData(5);									//Envia 5 na serial
	}
}

/**========================================================================================================
 * A fun��o show serve para colocar na matriz red[8][8] os dados da head[2] e do rabo[20][2]:
 * head[2] --> o vetor head tem duas posi��es, sendo que a posi��o 0 possui a linha da cabe�a e a posi��o
 * 1 possui a coluna.
 * rabo[20][2] --> a matriz rabo tem como primeiro argumento o tamanho do rabo da cobra e o segundo as coordenadas
 * (assim como a head)
 * ========================================================================================================*/

void show(){

	clearmatriz(red);									//Limpa a matriz vermelha
	red[head[0]][head[1]] = 1;							//Coloca 1 na posi��o [4][4] que � a cabe�a

	for(int i=0; i<nRabo; i++){							//Coloca 1 nas posi��es j� ocupadas da matriz vermelhas
		red[rabo[i][0]][rabo[i][1]] = 1;				//Rabo
	}

}

/**========================================================================================================
 * A fun��o messmatriz serve para colocar 1 em todas as posi��es de uma matriz.
 * ========================================================================================================*/
 
void messmatriz(int a[8][8]){
	for(int aux = 0; aux<8; aux++){
		for(int temp = 0; temp<8; temp++){
			a[aux][temp] = 1;
		}
	}
}

/**========================================================================================================
 * A fun��o setup � a configura��o inicial de qualquer partida. Ela seta a posi��o da cobra, a posi��o da
 * ma��, reseta a pontua��o e volta a velocidade do jogo ao normal.
 * ========================================================================================================*/

void setup(){
	clearmatriz(red);									//Limpa as  matrizes
	clearmatriz(green);
	gameOver = 0;										//Jogo ainda n�o acabou
	dir = STOP;											//Seta a dire��o inicial 
	head[0] = 4;										//Coloca 
	head[1] = 4;
	fruitX = rand() % 8;								//Pega um valor de X e Y aleat�rios
	fruitY = rand() % 8;
	if(fruitY == 4) fruitY++;							//Se j� estiver ocupado, pula 
	green[fruitY][fruitX] = 1;							//Coloca na posi�ao determinada a fruta
	score = 0;											//Zera o Score
	nRabo = 1;											//Rabo tem tamanho 1
	TIM13->ARR=649;										//Seta a velocidade n�vel 1
}

/**========================================================================================================
 * A fun��o comefruta � a verifica��o da coleta da ma�� pela cobra.
 * ========================================================================================================*/

void comefruta(){
	for(int aux = 0; aux<8; aux++){
		for(int temp = 0; temp<8; temp++){
			if((red[aux][temp]&green[aux][temp]) == 1){			//Se as duas matrizes s�o 1 naquela posi��o significa que comeu
				clearmatriz(green);								//Limpa a matriz verde
				fruitX = rand() % 8;							//Gera nova fruta
				fruitY = rand() % 8;
				if(fruitY == 4) fruitY++;
				green[fruitY][fruitX] = 1;
				score+=10;										//Acresce o score
				nRabo++;										//Aumenta o rabo
				sendData(score);								//Envia a pontua��o 
			}
		}
	}

}

/**========================================================================================================
 * Na fun��o move, podemos encontrar as condi��es do movimento da cobra. Al�m disso, temos a l�gica do rabo
 * da cobra, que recebe sempre a �ltima posi��o do peda�o do rabo anterior, e os limites das bordas.
 * ========================================================================================================*/

void move(){

	switch(dir){										//Switch para verificar a vari�vel dir
			case LEFT:{									//Caso left
			head[1]--;									
			break;
		}


		case RIGHT:{									//Caso right
			head[1]++;
			break;
		}


		case UP:{										//Caso up
			head[0]--;
			break;
		}


		case DOWN:{										//Caso down
			head[0]++;
			break;
		}

	}

	if((head[0] == -1) || (head[0] == 8) || (head[1] == -1) || (head[1] == 8))			//Verifica se ultrapassou um limite 
		gameOver = 1; //limites															  O jogador perde

	for(int aux = 0; aux<8; aux++){
			for(int temp = 0; temp<8; temp++){
				if((red[head[0]][head[1]])&(red[aux][temp]) && (dir!=STOP)){			//Se a cabe�a bater em alguma parte do rabo
					gameOver = 1;														//O jogador eprde
				}
			}
		}

	for(int i=0; i<nRabo; i++){
		rabo[nRabo-i][0] = rabo[nRabo-i-1][0];											//Posi��o conforme dire��o recebe o anterior do rabo
		rabo[nRabo-i][1] = rabo[nRabo-i-1][1];	
	}

	rabo[0][0] = head[0];
	rabo[0][1] = head[1];
}

/**========================================================================================================
 * Abaixo temos a interrup��o do TIM10 que serve para a multiplexa��o. O la�o "for" serve para o deslocamento
 * dos bits e, dependendo do conte�do das matrizes, essa infora��o � jogada ao pino "data" e ent�o a LATCH
 * envia a informa��o pelos pinos deslocados.
 * ========================================================================================================*/

void TIM1_UP_TIM10_IRQHandler (void) 					//multiplexacao
{

		TIM10->SR&=~TIM_SR_UIF; 						//zerando a flag

		seed++;

			GPIOC->ODR&=~LATCH; 						//desativando o latch do shift register

				for(int j = 0; j < 8; j++)				//varredura da matriz
				{

					GPIOC->ODR&=~CLK; 					//clk do shift register em 0 para receber a info

					if(red[j][i]) 						//se na matriz tivermos 1
					{
					GPIOC->ODR|=INFOR;					//envia 1 para o pino de info
					}
						if(red[j][i] == 0) 				//se tivermos 0 na matriz
					{
					GPIOC->ODR&=~INFOR; 				//envia 0 para o pino de info
					}

					if(green[j][i])						//Se na matriz tivermos 1
					{
						GPIOC->ODR|=INFOG;				//Envia 1 para o pino de info
					}
					if(green[j][i] == 0)				//Se tivermos 0 na matriz
					{
						GPIOC->ODR&=~INFOG;				//Envia 0 para o pino de info
					}

					GPIOC->ODR|=CLK; 					//bota 1 no clk
				}

				GPIOC->ODR|=LATCH; 						//envia a informa��o para os deslocadores

				GPIOC->ODR|=(A|B|C);					//Atribui 1 para todos os pinos de contagem
				GPIOC->ODR&=~(i<<3);					//Desloca o valor de linha conforme a vari�vel i

				i++;									//Acresce i
				if(i == 8)	i = 0;						//Se for igual a 8 zera

	}

/**------------------------------------------------------------------------------------------------------**/
/**------------------------------------------------------------------------------------------------------**/
/**------------------------------------------------------------------------------------------------------**/


/**------------------------------------------------------------------------------------------------------**/
/**------------------------------------------------------------------------------------------------------**/
/**------------------------------------------------------------------------------------------------------**/

int main(void)
{

	//Ativando os clocks das GPIOS
  	RCC->AHB1ENR|=(RCC_AHB1ENR_GPIOAEN|RCC_AHB1ENR_GPIOBEN|RCC_AHB1ENR_GPIOCEN);

	//Ativando os clocks dos Timers
	RCC->APB2ENR|=(RCC_APB2ENR_TIM10EN|RCC_APB2ENR_TIM11EN);
	RCC->APB1ENR|=RCC_APB1ENR_TIM13EN;

	//clock usart
	RCC->APB2ENR|=(RCC_APB2ENR_USART1EN);
	RCC->APB1ENR|=(RCC_APB1ENR_USART2EN);

	//Setando PA2, PA3, PA9 e PA10 como fun��o alternativa(AFR)
	GPIOA->MODER&=~(GPIO_MODER_MODER2|GPIO_MODER_MODER3|GPIO_MODER_MODER9|GPIO_MODER_MODER10);
	GPIOA->MODER|=(GPIO_MODER_MODER2_1|GPIO_MODER_MODER3_1|GPIO_MODER_MODER9_1|GPIO_MODER_MODER10_1);

	//Setando PA5 como sa�da
	GPIOA->MODER&=~(GPIO_MODER_MODER5);
	GPIOA->MODER|=(GPIO_MODER_MODER5_0);

	//Setando a fun��o alternativa a ser executada pelo PA2, PA3, PA9 e PA10
	GPIOA->AFR[1]|=0x770;
	GPIOA->AFR[0]|=0x7700;

	//config das usarts
	USART1->CR1|=(USART_CR1_UE); //Habilitando a USART
	USART1->CR1&=~(USART_CR1_M); //Tamanho da word
	USART1->CR1&=~(USART_CR1_PCE); //Habilitando paridade
	USART1->CR1|=(USART_CR1_TE); //Habilitando transmissor
	USART1->CR1|=(USART_CR1_RE); //Habilitando receptor
	USART1->CR1|=(USART_CR1_RXNEIE);
	USART1->CR2=0; //Zerando o CR2
	USART1->CR3=0; //Zerando o CR3
	USART1->BRR|=(104<<4); //Setando o Baudrate em: 9600
	NVIC_EnableIRQ(USART1_IRQn); //Habilitando a Interrup��o da USART1
	NVIC_SetPriority(USART1_IRQn, 1); //Setando como prioridade 1

	//Sa�das de controle do demux e do shift register
	GPIOC->MODER&=~(GPIO_MODER_MODER0|GPIO_MODER_MODER1|GPIO_MODER_MODER2|GPIO_MODER_MODER3|GPIO_MODER_MODER4|GPIO_MODER_MODER5|GPIO_MODER_MODER8);
	GPIOC->MODER|=(GPIO_MODER_MODER0_0|GPIO_MODER_MODER1_0|GPIO_MODER_MODER2_0|GPIO_MODER_MODER3_0|GPIO_MODER_MODER4_0|GPIO_MODER_MODER5_0|GPIO_MODER_MODER8_0);

	//Configura��o do tim10
	TIM10->PSC=7999;
	TIM10->ARR=2;
	TIM10->CR1=TIM_CR1_CEN;
	TIM10->DIER|=TIM_DIER_UIE;

	//Configura��o do tim13
	TIM13->PSC=15999;
	TIM13->ARR=649;
	TIM13->CR1=TIM_CR1_CEN;
	TIM13->DIER|=TIM_DIER_UIE;

	//Interrup��o do tim10
	NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
	NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 2);

	//Botando 0 em toda matriz
	clearmatriz(green);
	clearmatriz(red);

while (1)
  {
	/**========================================================================================================
	 * A while(1) apenas utiliza as fun��es feitas a uma frequ�ncia de, inicialmente, aprox. 1,5s e que aumenta
	 * conforme a pontua��o do jogador.
	 * ========================================================================================================*/

	if(startGame == 1){						//Jogo come�a

		srand(seed);						//atualiza��o da jogada aleatoria
		if(seed>65000){seed=0;}

		setup();							//Chama a void setup
		while(gameOver == 0){				//Se o jogo estiver rolando
			if(TIM13->SR&TIM_SR_UIF){

			TIM13->SR&=~TIM_SR_UIF;

			move();							//Executa as voids
			show();
			comefruta();
			velocidade();
			}
		}
		if(gameOver == 1)					//Caso o jogo for encerrado
		{
			sendData(11);					//Envia 11 na serial
		}
		}
	}
}

