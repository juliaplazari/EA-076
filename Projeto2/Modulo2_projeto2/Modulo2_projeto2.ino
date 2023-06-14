//--------------------------------DEFINIÇÕES INICIAIS DE VARIÁVEIS-----------------------------------

#include <LiquidCrystal.h>
// Display 16x2
const int rs = 6, en = 5, d4 = 4, d5 = 3, d6 = 2, d7 = A0; //configura os pinos que serão usados no display
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Pinos associados ao teclado matricial
const int row_1 = 7;
const int row_2 = 8;
const int row_3 = 9;
const int row_4 = 10;
const int col_1 = 11;
const int col_2 = 12;
const int col_3 = 13;

//Timer 0
int volatile count_ISR =0;

// Declaração das variáveis
int estado; // estado para a máquina de estados
int volatile tempo; // variável auxiliar para debounce
boolean flag_tecla; // flag para informar se identificou tecla
String tecla; // variável para qual tecla foi identificada
int displayed = 0; // número de caracteres que foi mostrado no display
//Arrays
int C[3]; //array para informação das colunas
int Ca[3]; // array para mudança na coluna
int C_ones[3] = {1,1,1}; //array de uns (colunas em nível alto) para comparação
int L[4]; //array para informação das linhas

//--------------------------------------SETUP----------------------------------------
void setup() {
  // configura interrupções periódicas associadas ao timer 0
  configuracao_Timer0();
  sei();// permite interruções

  //Configura pinos do teclado matricial
  // linhas são saídas e colunas são entradas
  pinMode(row_1,OUTPUT);
  pinMode(row_2,OUTPUT);
  pinMode(row_3,OUTPUT);
  pinMode(row_4,OUTPUT);
  pinMode(col_1,INPUT);
  pinMode(col_2,INPUT);
  pinMode(col_3,INPUT);

  // Display 16x2
  lcd.begin (16,2); //inicializa
  lcd.print("Hello"); // printa mensagem inicial
  lcd.setCursor(0, 1); // cursor na segunda linha
  
  // Interface Serial
  Serial.begin(9600);
  Serial.println("oi");
  
}

void configuracao_Timer0(){
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Configuracao Temporizador 0 (8 bits) para gerar interrupcoes periodicas a cada 1.6ms no modo Clear Timer on Compare Match (CTC)
  // Relogio = 16e6 Hz
  // Prescaler = 1024
  // Faixa = 24 (contagem de 0 a OCR0A = 24)
  // Intervalo entre interrupcoes: (Prescaler/Relogio)*Faixa = (1024/16e6)*(24+1) = 1.6ms
  
  // TCCR0A – Timer/Counter Control Register A
  // COM0A1 COM0A0 COM0B1 COM0B0 – – WGM01 WGM00
  // 0      0      0      0          1     0
  TCCR0A = 0x02;

  // OCR0A – Output Compare Register A
  OCR0A = 24;

  // TIMSK0 – Timer/Counter Interrupt Mask Register
  // – – – – – OCIE0B OCIE0A TOIE0
  // – – – – – 0      1      0
  TIMSK0 = 0x02;
  
  // TCCR0B – Timer/Counter Control Register B
  // FOC0A FOC0B – – WGM02 CS02 CS01 CS0
  // 0     0         0     1    0    1
  TCCR0B = 0x05;
  ////////////////////////////////////////////////////////////////////////////////////////////
}

ISR(TIMER0_COMPA_vect){// a cada interrupção periódica do timer 0 (a cada 1.6ms)
  count_ISR += 1; // conta número de interrupções do temporizador (número de vezes que entrou nessa função)
  } 


void tecla_para_display(String tecla){
  // Identifica a tecla e printa
  // O display inicia com Hello na primeira linha e dando display
  // dos caracteres digitados na segunda linha

  // Quando completa segunda linha, apaga a primeira e coloca o cursor nela
  if(displayed==16){
    lcd.setCursor(0, 0); 
    lcd.println("                ");
    lcd.setCursor(0, 0); 
  }

  // Quando completa primeira linha, apaga a segunda e coloca o cursor nela
  if(displayed==32){
    lcd.setCursor(0, 1); 
    lcd.println("                ");
    displayed = 0;
    lcd.setCursor(0, 1); 
  }

  //Display da tecla identificada
  lcd.print(tecla);
  displayed ++; // increnta variável que indica quantos caracteres já foram no display
}

// Função auxiliar que compara arrays e retorna true se todos elementos forem iguais
bool compareArrays(int array1[], int array2[], int size) {
  for (int i = 0; i < size; i++) {
    if (array1[i] != array2[i]) { // caso algum elemento seja diferente
      return false; // retorna false
    }
  }
  return true; // se nenhum elemento foi diferente, retorna true
}

// Função para descobrir qual tecla foi apertada
String descobre_tecla(int Ca[3],int L[4]){
  // Variáveis de entrada são
  // Array L com o estado da linha
  // Array Ca com o estado das colunas no momento da mudança

  // Arrays auxiliares para indicar linhas e colunas específicas
  int arrL1[] = {0,1,1,1};
  int arrL2[] = {1,0,1,1};
  int arrL3[] = {1,1,0,1};
  int arrL4[] = {1,1,1,0};

  int arrCa1[] = {0,1,1};
  int arrCa2[] = {1,0,1}; 
  int arrCa3[] = {1,1,0};

  // Compara arrays de linha e coluna com os auxiliares para identificar a tecla acionada
  if(compareArrays(L, arrL1,4) && compareArrays(Ca,arrCa1,3)){
    tecla = "1";
  }

  else if(compareArrays(L, arrL1,4) && compareArrays(Ca,arrCa2,3)){
    tecla = "2";
  }

  else if(compareArrays(L, arrL1,4) && compareArrays(Ca,arrCa3,3)){
    tecla = "3";
  }

  else if(compareArrays(L, arrL2,4) && compareArrays(Ca,arrCa1,3)){
    tecla = "4";
  }
  
  else if(compareArrays(L, arrL2,4) && compareArrays(Ca,arrCa2,3)){
    tecla = "5";
  }

  else if(compareArrays(L, arrL2,4) && compareArrays(Ca,arrCa3,3)){
    tecla = "6";
  }

  else if(compareArrays(L, arrL3,4) && compareArrays(Ca,arrCa1,3)){
    tecla = "7";
  }

  else if(compareArrays(L, arrL3,4) && compareArrays(Ca,arrCa2,3)){
    tecla = "8";
  }

  else if(compareArrays(L, arrL3,4) && compareArrays(Ca,arrCa3,3)){
    tecla = "9";
  }

  else if(compareArrays(L, arrL4,4) && compareArrays(Ca,arrCa1,3)){
    tecla = "*";
  }

  else if(compareArrays(L, arrL4,4) && compareArrays(Ca,arrCa2,3)){
    tecla = "0";
  }

  else if(compareArrays(L, arrL4,4) && compareArrays(Ca,arrCa3,3)){
    tecla = "#";
  }

  return tecla;
}

void loop() {
  // Máquina de estados que realiza varredura para identificar quando uma tecla foi apertada,
  // debounce, e display da tecla no LCD 16x2
  switch(estado){
      case 0: // estado 0
        // L = [0111] => primeira linha como baixo

        // lê valores das colunas
        C[0] = digitalRead(col_1);
        C[1] = digitalRead(col_2);
        C[2] = digitalRead(col_3);
    	
        // caso todas colunas estejam em alto
        if(compareArrays(C, C_ones,3)==1){
          estado = 1; // vai para o próximo estado
          // L = [1011] => segunda linha como baixo
          digitalWrite(row_1,HIGH);
          digitalWrite(row_2,LOW);
          digitalWrite(row_3,HIGH);
          digitalWrite(row_4,HIGH);
        }

        // caso contrário, salva o estado das colunas em Ca
        // e avança para o estado 4
        else{
        Ca[0] = digitalRead(col_1);
        Ca[1] = digitalRead(col_2);
        Ca[2] = digitalRead(col_3);
        estado = 4;          
        }
    
    	break;

      case 1:
    	  // L = [1011] => segunda linha como baixo
        C[0] = digitalRead(col_1);
        C[1] = digitalRead(col_2);
        C[2] = digitalRead(col_3);
    
        // caso todas colunas estejam em alto
        if(compareArrays(C, C_ones,3)==1){
          estado = 2; // vai para o próximo estado
          // L = [1101] => terceira linha como baixo
          digitalWrite(row_1,HIGH);
          digitalWrite(row_2,HIGH);
          digitalWrite(row_3,LOW);
          digitalWrite(row_4,HIGH);
        }

        // caso contrário, salva o estado das colunas em Ca
        // e avança para o estado 4
        else{
        Ca[0] = digitalRead(col_1);
        Ca[1] = digitalRead(col_2);
        Ca[2] = digitalRead(col_3);
        estado = 4;
        }
    
    	break;

      case 2:
        // L = [1101] => terceira linha como baixo
        C[0] = digitalRead(col_1);
        C[1] = digitalRead(col_2);
        C[2] = digitalRead(col_3);
    
        // caso todas colunas estejam em alto
        if(compareArrays(C, C_ones,3)==1){
          estado = 3; // vai para o próximo estado
          // L = [1110] => quarta linha como baixo
          digitalWrite(row_1,HIGH);
          digitalWrite(row_2,HIGH);
          digitalWrite(row_3,HIGH);
          digitalWrite(row_4,LOW);
        }

        // caso contrário, salva o estado das colunas em Ca
        // e avança para o estado 4
        else{
        Ca[0] = digitalRead(col_1);
        Ca[1] = digitalRead(col_2);
        Ca[2] = digitalRead(col_3);
        estado = 4;
        }
    
    	break;
        
      case 3:
    	  // L = [1110] => quarta linha como baixo
        C[0] = digitalRead(col_1);
        C[1] = digitalRead(col_2);
        C[2] = digitalRead(col_3);

        // caso todas colunas estejam em alto
        if(compareArrays(C, C_ones,3)==1){
          estado = 0;
          // L = [0111] => primeira linha como baixo
          digitalWrite(row_1,LOW);
          digitalWrite(row_2,HIGH);
          digitalWrite(row_3,HIGH);
          digitalWrite(row_4,HIGH);
        }

        // caso contrário, salva o estado das colunas em Ca
        // e avança para o estado 4
        else{
        Ca[0] = digitalRead(col_1);
        Ca[1] = digitalRead(col_2);
        Ca[2] = digitalRead(col_3);

        estado = 4;
        }
    
    	break;

      case 4:
        // estado intermediário quando alguma mudança foi identificada
        estado = 5; // vai para o estado 5 realizar o debounce
    	break;

      case 5:
        // estado para o debounce
        tempo = count_ISR; // salva valor do contador das interrupções do timer 0
        while(count_ISR < tempo+30){ // espera esse valor incrementar 30 (~50ms)
        // para checar se a mudança realmente ocorreu
        }

        // se tecla realmente foi apertada
        if(compareArrays(C, Ca,3)==1){
          estado = 6; // vai para o estado 6
          }

        // se tecla não foi apertada
        else if(compareArrays(C, Ca,3) == 0){
          estado = 0; // retorna para a varredura
          }
    	break;

      case 6:
    	// estado quando uma mudança foi identificada
      // e é preciso identificar qual foi a tecla apertada

      // salva estado das linhas
      L[0] = digitalRead(row_1);
      L[1] = digitalRead(row_2);
      L[2] = digitalRead(row_3);
      L[3] = digitalRead(row_4);

      // chama função para identificar qual a tecla
      tecla = descobre_tecla(Ca,L);
      // flag que indica que uma tecla foi identificada
      flag_tecla = 1;
      estado = 7; // avança para o próximo estado
    	break;
      
      case 7:
    	  // lê estado das colunas
        C[0] = digitalRead(col_1);
        C[1] = digitalRead(col_2);
        C[2] = digitalRead(col_3);

        // se todas retornaram para o nível alto vai para o estado 8
        if(compareArrays(C, C_ones,3)==1){
          estado = 8;
        }
        // se tecla ainda está apertada permanece nesse estado
        if(compareArrays(C, C_ones,3) == 0){
          estado = 7;
        }
    	break;

      case 8:
    	  // Debounce análogo ao estado 5
        // com o objetivo de identificar se a tecla foi solta
        tempo = count_ISR; // salva valor do contador das interrupções do timer 0
        while(count_ISR < tempo+30){ // espera esse valor incrementar 30 (~50ms)
        // para checar se a mudança realmente ocorreu
        }

        // se tecla foi solta        
        if(compareArrays(C, C_ones,3)==1){
          tecla_para_display(tecla); // mostra valor da tecla no display
          estado = 0; // retorna para a varredura
          }

        // se tecla não foi solta
        if(compareArrays(C, C_ones,3) == 0){
          estado = 7; // retorna para o estado anterior
          }
    	break;
  }
  
}

