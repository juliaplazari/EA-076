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
String typed = ""; // concatena teclas que foram digitadas

//Arrays
int C[3]; //array para informação das colunas
int Ca[3]; // array para mudança na coluna
int C_ones[3] = {1,1,1}; //array de uns (colunas em nível alto) para comparação
int L[4]; //array para informação das linhas

// Para o sensor de temperatura LM35
const int LM35_pin = A1; // pino para o sensor
int medidaSensor; // variável para medida
float temperatura; // variável para temperatura (medida em mV convertida para Celsius)
boolean flag_temperature; // indica se a temperatura deve ser medida

// Display de 7 segmentos
#include <Wire.h> // importa biblioteca para comunicação via protocolo I2C
unsigned int I2Cvar = 0b00000000; //variável para comunicação via protocolo I2C
int digit_to_display; //numero que sera mostrado em um display específico
//variável para cada um dos dígitos da temperatura medida
int temp1;
int temp2;
int temp3;
int temp4;
int temp_to_save;
String hex_temp;

// EEPROM
int memory_size = 2048;
const byte device_address = 0x50;
unsigned char read_byte;
int count_medidas = 0;
byte firstByte;
byte secondByte;

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



void tecla_para_comando(String tecla){
  // Identifica a tecla e printa
  // O display inicia com Hello na primeira linha e dando display
  // dos caracteres digitados na segunda linha

  //Display da tecla identificada
  lcd.print(tecla);

  if(tecla=="#"){
    identify_command();
    typed = "";
  }
  else if(tecla=="*"){
    typed = "";
  }

  else{
    typed += tecla;
  }

  Serial.println(typed);
}

void identify_command(){
  if(typed=="1"){
      Serial.println("reset");
      lcd.setCursor(0, 0);   
      lcd.println("                ");
      lcd.setCursor(0, 0);
      lcd.print("RESET   ");
      lcd.setCursor(0, 1);

      erase_eeprom();
  }

  else if(typed=="2"){
      lcd.setCursor(0, 0); 
      lcd.println("                ");
      lcd.setCursor(0, 0);      
      lcd.print("STATUS");
      lcd.setCursor(0, 1);

      lcd.print(count_medidas/2);
      lcd.print(" med  ");

      lcd.print(1024-count_medidas/2);
      lcd.print(" disp");

  }

  else if(typed=="3"){
      lcd.setCursor(0, 0); 
      lcd.println("                ");
      lcd.setCursor(0, 0);      
      lcd.print("BEGIN   ");
      lcd.setCursor(0, 1); 

      flag_temperature = true;
  } 

 else if(typed=="4"){
      lcd.setCursor(0, 0);  
      lcd.println("                ");
      lcd.setCursor(0, 0);      
      lcd.print("END     ");
      lcd.setCursor(0, 1);

      flag_temperature = false;
  }

 else if(typed=="5"){
      lcd.setCursor(0, 0); 
      lcd.println("                ");
      lcd.setCursor(0, 0);      
      lcd.print("TRANSFER");
      lcd.setCursor(0, 1);
  }

  else{
      lcd.setCursor(0, 0); 
      lcd.println("                ");
      lcd.setCursor(0, 0);      
      lcd.print("NOT A COMMAND");
      lcd.setCursor(0, 1);
  }
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

// O endereço pode variar de 0 a 2048 para o AT24C16, e o tipo "int"
// representa um inteiro de 16-bits, logo, é possível acomodar todo o 
// intervalo de valores necessário

// Já para o dado, por ser 8 bits (0 a 255) podemos usar o tipo "char"
void write_eeprom(unsigned int address, unsigned char data){
    // 3 bits menos significativos do device address são os 3 mais significativos
    // do endereço no qual se quer escrever
    byte device_address_mask = device_address | ((address >> 8) & 0x07);
    // ao criar uma variável do tipo byte, estamos selecionando os 8 bits menos significativos
    // conforme desejado
    byte word_address       = address; 
    
    // Operação de escrita usando a biblioteca Wire
    Wire.beginTransmission(device_address_mask);
    Wire.write(word_address);
    Wire.write(data);
    Wire.endTransmission();

}

void erase_eeprom(){
  for (int i = 0; i < memory_size; i++){
    write_eeprom(i, 0);
  }
}

unsigned char read_eeprom(unsigned int address){
    byte rdata;

    // 3 bits menos significativos do device address são os 3 mais significativos
    // do endereço do qual se quer ler
    byte device_address_mask = device_address | ((address >> 8) & 0x07);
    // ao criar uma variável do tipo byte, estamos selecionando os 8 bits menos significativos
    // conforme desejado
    byte word_address        = address;

    // Operação de escrita (para o endereço) usando a biblioteca Wire
    Wire.beginTransmission(device_address_mask);
    Wire.write(word_address);
    Wire.endTransmission();
    // Operação de leitura (do endereço especificado acima) usando a biblioteca Wire
    Wire.requestFrom(int(device_address_mask), 1);
    if (Wire.available()) {
        rdata = Wire.read();
    }
    return rdata;
}

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

void mede_temp(){
    
    if(count_ISR > 1250){ // a cada 2s
    // Leitura de temperatura
    // Vout (sensor) = 10 mv C / T
    // V (entrada ADC) = 5/1024 * Vout (1024 = 2^10 porque temos 10 bits)
    // Temperatura = 5/1024 * Vout / 10mV = Vout/1024*500

    int medidaSensor = analogRead(LM35_pin); // lê valor do sensor
    // converte valor de mV para Celsius
    // multiplica por 100 para visualização no display
    temperatura = (medidaSensor/1024.0) * 500*100; 
    count_ISR = 0; // zera contador da interrupção

    // temperatura a ser salva tem apenas 1 dígito decimal
    temp_to_save = temperatura;
    temp_to_save = temp_to_save/=100;

    // separa em byte menos e mais significativo
    firstByte = temp_to_save % 256; 
    secondByte = temp_to_save / 256;

    // salva nos próximos dois endereços
    write_eeprom(count_medidas,firstByte);
    count_medidas += 1;
    write_eeprom(count_medidas,secondByte);
    
    // incrementa contador das medições
    count_medidas += 1;
  }
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
        
      if(flag_temperature){
        mede_temp();
      }

      display_temperatura(temperatura);
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
      if(flag_temperature){
        mede_temp();
      }
      display_temperatura(temperatura);
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
      if(flag_temperature){
        mede_temp();
      }
      display_temperatura(temperatura);
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
      if(flag_temperature){
        mede_temp();
      }
      display_temperatura(temperatura);
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
      if(flag_temperature){
        mede_temp();
      }
    	display_temperatura(temperatura);
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
      if(flag_temperature){
        mede_temp();
      }
      display_temperatura(temperatura);
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
      if(flag_temperature){
        mede_temp();
      }
      display_temperatura(temperatura);
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
          tecla_para_comando(tecla); // mostra valor da tecla no display
          estado = 0; // retorna para a varredura
          }

        // se tecla não foi solta
        if(compareArrays(C, C_ones,3) == 0){
          estado = 7; // retorna para o estado anterior
          }
      if(flag_temperature){
        mede_temp();
      }
      display_temperatura(temperatura);
    	break;
  }
  
}

