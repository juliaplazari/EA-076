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
int novo_estado;
int estado;

void setup() {
  // configura interrupções periódicas associadas ao timer 0
  configuracao_Timer0();
  sei();// permite interruções


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
  
  lcd.setCursor(0,0); // será que precisa?
  lcd.print(message);
}

void varredura(){
  if(row_1==1 && col_1==1){
    count = count_ISR;
    count_50 = count + 30; // approx 50ms
    novo_estado = 1;
  }

  else if(row_1==1 && col_1==2){
    count = count_ISR;
    count_50 = count + 30; // approx 50ms
    novo_estado = 2;
  }

  else if(row_1==1 && col_1==3){
    count = count_ISR;
    count_50 = count + 30; // approx 50ms
    novo_estado = 3;
  }

  else if(row_1==2 && col_1==1){
    count = count_ISR;
    count_50 = count + 30; // approx 50ms
    novo_estado = 4;
  }

  else if(row_1==2 && col_1==2){
    count = count_ISR;
    count_50 = count + 30; // approx 50ms
    novo_estado = 5;
  }

  else if(row_1==2 && col_1==3){
    count = count_ISR;
    count_50 = count + 30; // approx 50ms
    novo_estado = 6;
  }

  else if(row_1==3 && col_1==1){
    count = count_ISR;
    count_50 = count + 30; // approx 50ms
    novo_estado = 7;
  }

  else if(row_1==3 && col_1==2){
    count = count_ISR;
    count_50 = count + 30; // approx 50ms
    novo_estado = 8;
  }

  else if(row_1==3 && col_1==3){
    count = count_ISR;
    count_50 = count + 30; // approx 50ms
    novo_estado = 9;
  }

  else if(row_1==4 && col_1==1){
    count = count_ISR;
    count_50 = count + 30; // approx 50ms
    novo_estado = 10;
  }

  else if(row_1==4 && col_1==2){
    count = count_ISR;
    count_50 = count + 30; // approx 50ms
    novo_estado = 0;
  }
  
  else if(row_1==4 && col_1==3){
    count = count_ISR;
    count_50 = count + 30; // approx 50ms
    novo_estado = 11;
  }

  else{
    novo_estado = estado;
  }
}

void varredura_debounce(){
  if(novo_estado == 1){
    if(row_1==1 && col_1==1){
      novo_estado = 1;
    }
  }
  
  else if(novo_estado == 2){
    if(row_1==1 && col_1==2){
      novo_estado = 2;
    }
  }

  else if(novo_estado == 3){
    if(row_1==1 && col_1==3){
      novo_estado = 3;
    }
  }

  else if(novo_estado == 4){
    if(row_1==2 && col_1==1){
      novo_estado = 4;
    }
  }

  else if(novo_estado == 5){
    if(row_1==2 && col_1==2){
      novo_estado = 5;
    }
  }

  else if(novo_estado == 6){
    if(row_1==2 && col_1==3){
      novo_estado = 6;
    }
  }

  else if(novo_estado == 7){
    if(row_1==3 && col_1==1){
      novo_estado = 7;
    }
  }

  else if(novo_estado == 8){
    if(row_1==3 && col_1==2){
      novo_estado = 8;
    }
  }

  else if(novo_estado == 9){
    if(row_1==3 && col_1==3){
      novo_estado = 9;
    }
  }

  else if(novo_estado == 10){
    if(row_1==4 && col_1==1){
      novo_estado = 10;
    }
  }

  else if(novo_estado == 0){
    if(row_1==4 && col_1==2){
      novo_estado = 0;
    }
  }

  else if(novo_estado == 11){
    if(row_1==4 && col_1==3){
      novo_estado = 11;
    }
  }

  else{
    novo_estado = estado;
  }

}

void loop() {
  varredura();

  if(count == count_50){
    varredura_debounce();
  }

  // Máquina de estados, cada estado corresponde a uma tecla
  if(estado==0){
    display_16x2("0");
  }

  else if(estado==1){
    display_16x2("1");
  }
  
  else if(estado==2){
    display_16x2("2");
  }
  
  else if(estado==3){
    display_16x2("3");
  }
  
  else if(estado==4){
    display_16x2("4");
  }
  
  else if(estado==5){
    display_16x2("5");
  }
  
  else if(estado==6){
    display_16x2("6");
  }
  
  else if(estado==7){
    display_16x2("7");
  }
  
  else if(estado==8){
    display_16x2("8");
  }
  
  else if(estado==9){
    display_16x2("9");
  }
  
  else if(estado==10){//*
    display_16x2("*");
  }

  else if(estado==11){//#
    display_16x2("#");
  }
}
