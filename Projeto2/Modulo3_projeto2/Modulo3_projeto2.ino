//--------------------------------DEFINIÇÕES INICIAIS DE VARIÁVEIS-----------------------------------

//Timer 0
int volatile count_ISR =0;

// Para o sensor de temperatura LM35
const int LM35_pin = A1; // pino para o sensor
int medidaSensor; // variável para medida
float temperatura; // variável para temperatura (medida em mV convertida para Celsius)

// Display de 7 segmentos
#include <Wire.h> // importa biblioteca para comunicação via protocolo I2C
unsigned int I2Cvar = 0b00000000; //variável para comunicação via protocolo I2C
int digit_to_display; //numero que sera mostrado em um display específico
//variável para cada um dos dígitos da temperatura medida
int temp1;
int temp2;
int temp3;
int temp4;

//--------------------------------------SETUP----------------------------------------
void setup() {
  // configura interrupções periódicas associadas ao timer 0
  configuracao_Timer0();
  sei();// permite interruções
  
  // Interface Serial
  Serial.begin(9600);
  Serial.println("oi");
  
  // Comunicação via I2C
  Wire.begin();
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

//-------------------------------------FUNÇÕES----------------------------------------
void seleciona_display(String display){
  //atualiza valor de I2Cvar de modo a selecionar o display desejado (obs: display ativo em baixo)
  //P3 a P1 em alto pra mascara do número funcionar
  
  if(display=="D1"){
  I2Cvar = 0b11101111; 
  }

  else if(display=="D2"){
  I2Cvar = 0b11011111; 
  }

  else if(display=="D3"){
  I2Cvar = 0b10111111;
  }

  else if(display=="D4"){
  I2Cvar = 0b01111111; 
  }
}

void seleciona_numero(int numero){
  // máscara para atualizar variável e mostrar o número desejado no display
  // P4 a P7 em alto para não influenciar no display escolhido
  if(numero==0){
  I2Cvar= I2Cvar & 0b11110000; 
  }

  else if(numero==1){
  I2Cvar= I2Cvar & 0b11110001; 
  }

  else if(numero==2){
  I2Cvar= I2Cvar & 0b11110010; 
  }

  else if(numero==3){
  I2Cvar= I2Cvar & 0b11110011; 
  }

  else if(numero==4){
  I2Cvar= I2Cvar & 0b11110100; 
  }

  else if(numero==5){
  I2Cvar= I2Cvar & 0b11110101; 
  }

  else if(numero==6){
  I2Cvar= I2Cvar & 0b11110110; 
  }

  else if(numero==7){
  I2Cvar= I2Cvar & 0b11110111; 
  }

  else if(numero==8){
  I2Cvar= I2Cvar & 0b11111000; 
  }

  else if(numero==9){
  I2Cvar= I2Cvar & 0b11111001; 
  }

}

void numero_para_display(int digito){
    // seleciona o digito display e realiza a comunicação via protocolo I2C
    seleciona_numero(digito);
    Wire.beginTransmission(39); // endereço do PCF (pinos A0 a A2 estão conectados ao Vcc)
    Wire.write(I2Cvar);             
    Wire.endTransmission();      
}

void display_temperatura(int temperatura){
  // display da temperatura medida
  // chama função que seleciona o número a ser transmitido e transmite via I2C
  // um dos displays é acionado de cada vez, mas dado a velocidade parece que todos estão ativos
  // ao mesmo tempo

  //primeiro dígito
  temp1 = (temperatura / 1U) % 10; ;
  seleciona_display("D4"); //seleciona D4
  numero_para_display(temp1); //mascara para mostrar numero

  //segundo dígito
  temp2 = (temperatura / 10U) % 10; ;
  seleciona_display("D3"); //seleciona D3
  numero_para_display(temp2); //mascara para mostrar numero

  //terceiro dígito
  temp3 = (temperatura / 100U) % 10; ;
  seleciona_display("D2"); //seleciona D2
  numero_para_display(temp3); //mascara para mostrar numero

  //quarto dígito
  temp4 = (temperatura / 1000U) % 10; ;
  seleciona_display("D1"); //seleciona D1
  numero_para_display(temp4); //mascara para mostrar numero
}

void loop() {
  if(count_ISR > 625){ // a cada 1s
    // Leitura de temperatura
    // Vout (sensor) = 10 mv C / T
    // V (entrada ADC) = 5/1024 * Vout (1024 = 2^10 porque temos 10 bits)
    // Temperatura = 5/1024 * Vout / 10mV = Vout/1024*500

    int medidaSensor = analogRead(LM35_pin); // lê valor do sensor
    // converte valor de mV para Celsius
    // multiplica por 100 para visualização no display
    temperatura = (medidaSensor/1024.0) * 500*100; 

    count_ISR = 0; // zera contador da interrupção
  }
  // para o display de 7 segmentos
  display_temperatura(temperatura);
}

