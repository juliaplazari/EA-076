//--------------------------------DEFINIÇÕES INICIAIS DE VARIÁVEIS-----------------------------------

#include <LiquidCrystal.h>
// Display 16x2
const int rs = 6, en = 5, d4 = 4, d5 = 3, d6 = 2, d7 = 1; //configura os pinos que serão usados no display
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Teclado matricial
const int row_1 = 7;
const int row_2 = 8;
const int row_3 = 9;
const int row_4 = 10;
const int col_1 = 11;
const int col_2 = 12;
const int col_3 = 13;

//Timer 0
int count_ISR =0;
int count;
int count_50;

// Variáveis
int estado;
int tempo;
boolean flag_tecla;
int tecla;
int C[3]; //array para informação das colunas
int Ca[3]; // array para mudança na coluna
int C_ones[3] = {1,1,1}; //array de uns (colunas em nível alto)
int L[4];

void setup() {
  // configura interrupções periódicas associadas ao timer 0
  configuracao_Timer0();
  sei();// permite interruções

  //configura pinos do teclado matricial
  // linhas são saídas e colunas são entradas
  pinMode(row_1,OUTPUT);
  pinMode(row_2,OUTPUT);
  pinMode(row_3,OUTPUT);
  pinMode(row_4,OUTPUT);
  pinMode(col_1,INPUT);
  pinMode(col_2,INPUT);
  pinMode(col_3,INPUT);

  // display 16x2
  lcd.begin (16,2); //inicializa
  lcd.setCursor (0,0);
  lcd.print("Hello"); // printa mensagem inicial
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



void display_16x2(String message){
  // Mensagem no display 16x2
  // display não será apagado antes de mostrar próxima mensagem
  // para visualizar sequência de comandos e garantir que debounce foi
  // implementado corretamente
  //lcd.clear();
  
  lcd.print(message);
}

void estado_para_display(int tecla){
  if(tecla==0){
    display_16x2("0");
  }

  else if(tecla==1){
    display_16x2("1");
  }
  
  else if(tecla==2){
    display_16x2("2");
  }
  
  else if(tecla==3){
    display_16x2("3");
  }
  
  else if(tecla==4){
    display_16x2("4");
  }
  
  else if(tecla==5){
    display_16x2("5");
  }
  
  else if(tecla==6){
    display_16x2("6");
  }
  
  else if(tecla==7){
    display_16x2("7");
  }
  
  else if(tecla==8){
    display_16x2("8");
  }
  
  else if(tecla==9){
    display_16x2("9");
  }
  
  else if(tecla==10){//*
    display_16x2("*");
  }

  else if(tecla==11){//#
    display_16x2("#");
  }
}

bool compareArrays(int array1[], int array2[], int size) {
  for (int i = 0; i < size; i++) {
    if (array1[i] != array2[i]) {
      return false;
    }
  }
  return true;
}

int descobre_tecla(int Ca[3],int L[4]){
  int arrL1[] = {0,1,1,1};
  int arrL2[] = {1,0,1,1};
  int arrL3[] = {1,1,0,1};
  int arrL4[] = {1,1,1,0};

  int arrCa1[] = {0,1,1};
  int arrCa2[] = {1,0,1}; 
  int arrCa3[] = {1,1,0};

  if(compareArrays(L, arrL1,4) && compareArrays(Ca,arrCa1,3)){
    tecla = 1;
  }

  else if(compareArrays(L, arrL1,4) && compareArrays(Ca,arrCa2,3)){
    tecla = 2;
  }

  else if(compareArrays(L, arrL1,4) && compareArrays(Ca,arrCa3,3)){
    tecla = 3;
  }

  else if(compareArrays(L, arrL2,4) && compareArrays(Ca,arrCa1,3)){
    tecla = 4;
  }
  
  else if(compareArrays(L, arrL2,4) && compareArrays(Ca,arrCa2,3)){
    tecla = 5;
  }

  else if(compareArrays(L, arrL2,4) && compareArrays(Ca,arrCa3,3)){
    tecla = 6;
  }

  else if(compareArrays(L, arrL3,4) && compareArrays(Ca,arrCa1,3)){
    tecla = 7;
  }

  else if(compareArrays(L, arrL3,4) && compareArrays(Ca,arrCa2,3)){
    tecla = 8;
  }

  else if(compareArrays(L, arrL3,4) && compareArrays(Ca,arrCa3,3)){
    tecla = 9;
  }

  else if(compareArrays(L, arrL4,4) && compareArrays(Ca,arrCa1,3)){
    tecla = 10;
  }

  else if(compareArrays(L, arrL4,4) && compareArrays(Ca,arrCa2,3)){
    tecla = 0;
  }

  else if(compareArrays(L, arrL4,4) && compareArrays(Ca,arrCa3,3)){
    tecla = 11;
  }

  return tecla;
}

void loop() {

  switch(estado){
      case 0:
        C[0] = col_1;
        C[1] = col_2;
        C[2] = col_3;
        if(compareArrays(C, C_ones,3)){
          estado = 1;
          digitalWrite(row_1,HIGH);
          digitalWrite(row_2,LOW);
          digitalWrite(row_3,HIGH);
          digitalWrite(row_4,HIGH);
        }

        if(compareArrays(C, C_ones,3)==false){
          estado = 4;          
        }

      case 1:
        C[0] = col_1;
        C[1] = col_2;
        C[2] = col_3;
        if(compareArrays(C, C_ones,3)){
          estado = 2;
          digitalWrite(row_1,HIGH);
          digitalWrite(row_2,HIGH);
          digitalWrite(row_3,LOW);
          digitalWrite(row_4,HIGH);
        }

        if(compareArrays(C, C_ones,3) ==  false){
          estado = 4;
        }

      case 2:
        C[0] = col_1;
        C[1] = col_2;
        C[2] = col_3;

        if(compareArrays(C, C_ones,3)){
          estado = 3;
          digitalWrite(row_1,HIGH);
          digitalWrite(row_2,HIGH);
          digitalWrite(row_3,HIGH);
          digitalWrite(row_4,LOW);
        }

        if(compareArrays(C, C_ones,3) ==  false){
          estado = 4;
        }
        
      case 3:
        C[0] = col_1;
        C[1] = col_2;
        C[2] = col_3;

        if(compareArrays(C, C_ones,3)){
          estado = 0;
          digitalWrite(row_1,LOW);
          digitalWrite(row_2,HIGH);
          digitalWrite(row_3,HIGH);
          digitalWrite(row_4,HIGH);
        }

        if(compareArrays(C, C_ones,3) == false){
          Ca[0] = col_1;
          Ca[1] = col_2;
          Ca[2] = col_3;

          estado = 4;
        }

      case 4:
        
        estado = 5;

      case 5:
        // debounce
        tempo = count_ISR;
        while(count_ISR < tempo+30){
          //VER SE PRECISA DE ALGO CASO NAO FUNCIONE
        }
        if(compareArrays(C, Ca,3)){
          estado = 6;
          }
        if(compareArrays(C, Ca,3) == false){
          estado = 0;
          }

      case 6:
        tecla = descobre_tecla(Ca,L);
        flag_tecla = 1;
        estado = 7;
      
      case 7:
        L[0] = row_1;
        L[1] = row_2;
        L[2] = row_3;
        L[3] = row_4;
        
        if(compareArrays(C, C_ones,3)){
          estado = 8;
        }
        if(compareArrays(C, C_ones,3) == false){
          estado = 7;
        }

      case 8:
        tempo = count_ISR;
        while(count_ISR < tempo+30){
          //VER SE PRECISA DE ALGO CASO NAO FUNCIONE
        }
        if(compareArrays(C, C_ones,3)){
          estado_para_display(tecla);
          estado = 0;
          }
        if(compareArrays(C, C_ones,3) == false){
          estado = 7;
          }
  }
  
}

