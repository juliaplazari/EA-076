//--------------------------------DEFINIÇÕES INICIAIS DE VARIÁVEIS-----------------------------------
// Display 16x2
#include <LiquidCrystal.h>
const int rs = 6, en = 5, d4 = 4, d5 = 3, d6 = 2, d7 = A0; //configura os pinos que serão usados no display
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//Timer 0 => interrupções periódicas
int volatile count_ISR =0;

// Pinos associados ao teclado matricial
const int row_1 = 7;
const int row_2 = 8;
const int row_3 = 9;
const int row_4 = 10;
const int col_1 = 11;
const int col_2 = 12;
const int col_3 = 13;

// Máquina de estados => varredura para identificar tecla apertada
int state; // estado para a máquina de estados
int volatile time; // variável auxiliar para debounce
boolean flag_key; // flag para informar se identificou tecla
String key; // variável para qual tecla foi identificada
String typed = ""; // concatena teclas que foram digitadas para identificar comando
boolean flag_command = 1; // flag que indica que queremos identificar o comando
String transferN = ""; // concatena teclas que foram digitadas para identificar número de pedidas para serem transmitidas
boolean flag_n_transfer = 0; // flag que indica que queremos identificar quantas medidas devem ser transmitidas

//Arrays para identificar tecla apertada
int C[3]; //array para informação das colunas
int Ca[3]; // array para mudança na coluna
int C_ones[3] = {1,1,1}; //array de uns (colunas em nível alto) para comparação
int L[4]; //array para informação das linhas

// Para o sensor de temperatura LM35
const int LM35_pin = A1; // pino para o sensor
int sensorTemp; // variável para medida
float temperature; // variável para temperatura (medida em mV convertida para Celsius e multiplicada por 100 para o display)
boolean flag_temperature; // indica se a temperatura deve ser medida
int temp_to_save; // temperatura que deve ser salva (precisão de décimo)
byte firstByteTemp; // variável auxiliar para primeiro byte da temperatura
byte secondByteTemp; // variável auxiliar para o segundo byte da temperatura

// Display de 7 segmentos
#include <Wire.h> // importa biblioteca para comunicação via protocolo I2C
unsigned int I2Cvar = 0b00000000; //variável para comunicação via protocolo I2C
int digit_to_display; //numero que sera mostrado em um display específico
//variável para cada um dos dígitos da temperatura medida
int temp1;
int temp2;
int temp3;
int temp4;

// EEPROM
const byte device_address = 0x50; // endereço da eeprom
unsigned char read_byte1; // variável auxiliar para ler primeiro byte
unsigned char read_byte2; // variável auxiliar para ler segundo byte
unsigned int count_measures = 0; // contador de número de medidas
unsigned int next_end; // variável auxiliar para próximo endereço a ser gravado
byte firstByteCount; // variável auxiliar para salvar primeiro byte do contador
byte secondByteCount; // variável auxiliar para salvar segundo byte do contador


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
  Serial.println("Oi!");
  
  // Comunicação via I2C
  Wire.begin();
}

//--------------------------------INTERRUPÇÕES TIMER 0-------------------------------
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


//------------------------------------DISPLAY---------------------------------------

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
    Wire.beginTransmission(0x27); // endereço do PCF (pinos A0 a A2 estão conectados ao Vcc)
    Wire.write(I2Cvar);             
    Wire.endTransmission();      
}

void display_temperature(int temperature){
  // display da temperatura medida
  // chama função que seleciona o número a ser transmitido e transmite via I2C
  // um dos displays é acionado de cada vez, mas dado a velocidade parece que todos estão ativos
  // ao mesmo tempo

  //primeiro dígito
  temp1 = (temperature / 1U) % 10; ;
  seleciona_display("D4"); //seleciona D4
  numero_para_display(temp1); //mascara para mostrar numero

  //segundo dígito
  temp2 = (temperature / 10U) % 10; ;
  seleciona_display("D3"); //seleciona D3
  numero_para_display(temp2); //mascara para mostrar numero

  //terceiro dígito
  temp3 = (temperature / 100U) % 10; ;
  seleciona_display("D2"); //seleciona D2
  numero_para_display(temp3); //mascara para mostrar numero

  //quarto dígito
  temp4 = (temperature / 1000U) % 10; ;
  seleciona_display("D1"); //seleciona D1
  numero_para_display(temp4); //mascara para mostrar numero
}

//------------------------------------EEPROM---------------------------------------


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

//---------------------------------TECLA APERTADA-------------------------------------

// Função auxiliar que compara arrays e retorna true se todos elementos forem iguais
bool compare_arrays(int array1[], int array2[], int size) {
  for (int i = 0; i < size; i++) {
    if (array1[i] != array2[i]) { // caso algum elemento seja diferente
      return false; // retorna false
    }
  }
  return true; // se nenhum elemento foi diferente, retorna true
}

// Função para descobrir qual tecla foi apertada
String identify_key(int Ca[3],int L[4]){
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

  // Compara arrays de linha e coluna com os auxiliares para identificar a keyacionada
  if(compare_arrays(L, arrL1,4) && compare_arrays(Ca,arrCa1,3)){
    key = "1";
  }

  else if(compare_arrays(L, arrL1,4) && compare_arrays(Ca,arrCa2,3)){
    key = "2";
  }

  else if(compare_arrays(L, arrL1,4) && compare_arrays(Ca,arrCa3,3)){
    key = "3";
  }

  else if(compare_arrays(L, arrL2,4) && compare_arrays(Ca,arrCa1,3)){
    key = "4";
  }
  
  else if(compare_arrays(L, arrL2,4) && compare_arrays(Ca,arrCa2,3)){
    key = "5";
  }

  else if(compare_arrays(L, arrL2,4) && compare_arrays(Ca,arrCa3,3)){
    key = "6";
  }

  else if(compare_arrays(L, arrL3,4) && compare_arrays(Ca,arrCa1,3)){
    key = "7";
  }

  else if(compare_arrays(L, arrL3,4) && compare_arrays(Ca,arrCa2,3)){
    key = "8";
  }

  else if(compare_arrays(L, arrL3,4) && compare_arrays(Ca,arrCa3,3)){
    key = "9";
  }

  else if(compare_arrays(L, arrL4,4) && compare_arrays(Ca,arrCa1,3)){
    key = "*";
  }

  else if(compare_arrays(L, arrL4,4) && compare_arrays(Ca,arrCa2,3)){
    key = "0";
  }

  else if(compare_arrays(L, arrL4,4) && compare_arrays(Ca,arrCa3,3)){
    key = "#";
  }

  return key;
}

//---------------------------------COMANDOS-------------------------------------
// função para concatenar teclas apertadas e identificar comando
void key_to_command(String key){
  // Concatena teclas para identificar comando desejado 
  // ou número de medidas desejadas
  // dado qual flag está como true

  if(key=="#"){ // # indica conclusão de comando
    if (flag_command){
      execute_command(); // executa comando
      typed = "";
    }
    else if(flag_n_transfer){
      transfer_measures();
      transferN = "";
    } 
  }
  else if(key=="*"){ // indica que o comando deve ser cancelado
    if (flag_command){
      typed = "";
    }
    else if(flag_n_transfer){
      transferN = "";
    }
  }

  else{ // concantena próximo valor digitado
    if (flag_command){
      typed += key;
    }
    else if(flag_n_transfer){
      transferN += key;
    }
  }
}

// função para executar comando identificado
void execute_command(){
  if(typed=="1"){ // se comando 1 => reset
      lcd.clear();
      lcd.setCursor(0,0);  
      lcd.print("RESET   ");
      // zera valor do contador
      write_eeprom(2047,0);
      _delay_ms(5);
      write_eeprom(2046,0);
      _delay_ms(5);
  }

  else if(typed=="2"){ // se comando 2 => status
      lcd.clear();
      lcd.setCursor(0,0);   
      lcd.print("STATUS");
      lcd.setCursor(0, 1);
      // lê valor do contador de medidas
      count_measures = read_eeprom(2047) + 256*read_eeprom(2046);
      // número medidas feitas
      lcd.print(count_measures);
      lcd.print(" med ");
      // número medições disponíveis
      lcd.print(1023-count_measures);
      lcd.print(" disp");

  }

  else if(typed=="3"){ // se comando 3 => begin
      lcd.clear();
      lcd.setCursor(0,0);  
      lcd.print("BEGIN");
      // habilita flag para medir temperatura
      flag_temperature = 1;
  } 

 else if(typed=="4"){ // se comando 4 => end
      lcd.clear();
      lcd.setCursor(0, 0);      
      lcd.print("END");
      // desabilita flag para medir temperatura
      flag_temperature = false;
  }

 else if(typed=="5"){ // se comando 5 => transfer
      lcd.clear();
      lcd.setCursor(0, 0);      
      lcd.print("TRANSFER");
      lcd.setCursor(0, 1);
      lcd.print("Quantas medidas?");
      // habilita flag para identificar número de medidas a ser transmitidas
      flag_n_transfer = 1;
       // desabilita flag para identificar comando
      flag_command = 0;

  }

  else{ // caso algo diferente disso seja digitado, informar que comando é inexistente
      lcd.clear();
      lcd.setCursor(0, 0);      
      lcd.print("NOT A COMMAND");
  }
}

// função para transferir medições para monitor serial
void transfer_measures(){
  
  for (unsigned int j = 0; j < 2*transferN.toInt(); j+=2){
        Serial.println(read_eeprom(j)+ 256*read_eeprom(j+1));
  }
  // Após transferência completa
  // desabilita flag para identificar número de medidas a ser transmitidas
  flag_n_transfer = 0;
  // habilita flag para identificar comando
  flag_command = 1;

  lcd.setCursor(0, 1);
  lcd.print("Concluído!!     ");

}

// função para medir e salvar temperatura
void measure_temp(){
    
    if(count_ISR > 1250){ // a cada 2s
    // Leitura de temperatura
    // Vout (sensor) = 10 mv C / T
    // V (entrada ADC) = 5/1024 * Vout (1024 = 2^10 porque temos 10 bits)
    // Temperatura = 5/1024 * Vout / 10mV = Vout/1024*500

    int sensorTemp = analogRead(LM35_pin); // lê valor do sensor
    // converte valor de mV para Celsius
    // multiplica por 100 para visualização no display
    temperature = (sensorTemp/1024.0) * 500*100; 
    count_ISR = 0; // zera contador da interrupção

    // temperatura a ser salva tem apenas 1 dígito decimal
    temp_to_save = temperature;
    temp_to_save = temp_to_save/=10;

    // separa em byte menos e mais significativo
    firstByteTemp = temp_to_save % 256; 
    secondByteTemp = temp_to_save / 256;
    
    // lê contador salvo nas últimas posições da memória
    count_measures = read_eeprom(2047) + 256*read_eeprom(2046);
    
    // salva nos próximos dois endereços (temp é armazenada em 2 bytes)
    write_eeprom(count_measures*2,firstByteTemp);
    _delay_ms(5);

    next_end = count_measures*2+1;
    write_eeprom(next_end,secondByteTemp);
    _delay_ms(5);
    // incrementa contador das medições
    count_measures += 1;

    // salva contador atualizado
    firstByteCount = count_measures % 256; 
    secondByteCount = count_measures / 256;
    write_eeprom(2047,firstByteCount);
    _delay_ms(5);
    write_eeprom(2046,secondByteCount);
    _delay_ms(5);
  }
}

void loop() {
  // Máquina de estados que realiza varredura para identificar quando uma tecla foi apertada (com debounce)
  switch(state){
      case 0: // estado 0
        // L = [0111] => primeira linha como baixo

        // lê valores das colunas
        C[0] = digitalRead(col_1);
        C[1] = digitalRead(col_2);
        C[2] = digitalRead(col_3);
    	
        // caso todas colunas estejam em alto
        if(compare_arrays(C, C_ones,3)==1){
          state = 1; // vai para o próximo estado
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
        state = 4;          
        }
        
      if(flag_temperature){
        measure_temp();
      }

      display_temperature(temperature);
    	break;

      case 1:
    	  // L = [1011] => segunda linha como baixo
        C[0] = digitalRead(col_1);
        C[1] = digitalRead(col_2);
        C[2] = digitalRead(col_3);
    
        // caso todas colunas estejam em alto
        if(compare_arrays(C, C_ones,3)==1){
          state = 2; // vai para o próximo estado
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
        state = 4;
        }
      if(flag_temperature){
        measure_temp();
      }
      display_temperature(temperature);
    	break;

      case 2:
        // L = [1101] => terceira linha como baixo
        C[0] = digitalRead(col_1);
        C[1] = digitalRead(col_2);
        C[2] = digitalRead(col_3);
    
        // caso todas colunas estejam em alto
        if(compare_arrays(C, C_ones,3)==1){
          state = 3; // vai para o próximo estado
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
        state = 4;
        }
      if(flag_temperature){
        measure_temp();
      }
      display_temperature(temperature);
    	break;
        
      case 3:
    	  // L = [1110] => quarta linha como baixo
        C[0] = digitalRead(col_1);
        C[1] = digitalRead(col_2);
        C[2] = digitalRead(col_3);

        // caso todas colunas estejam em alto
        if(compare_arrays(C, C_ones,3)==1){
          state = 0;
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

        state = 4;
        }
      if(flag_temperature){
        measure_temp();
      }
      display_temperature(temperature);
    	break;

      case 4:
        // estado intermediário quando alguma mudança foi identificada
        state = 5; // vai para o estado 5 realizar o debounce
    	break;

      case 5:
        // estado para o debounce
        time = count_ISR; // salva valor do contador das interrupções do timer 0
        while(count_ISR < time+30){ // espera esse valor incrementar 30 (~50ms)
        // para checar se a mudança realmente ocorreu
        }

        // se tecla realmente foi apertada
        if(compare_arrays(C, Ca,3)==1){
          state = 6; // vai para o estado 6
          }

        // se tecla não foi apertada
        else if(compare_arrays(C, Ca,3) == 0){
          state = 0; // retorna para a varredura
          }
      if(flag_temperature){
        measure_temp();
      }
    	display_temperature(temperature);
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
      key = identify_key(Ca,L);
      // flag que indica que uma tecla foi identificada
      flag_key = 1;
      state = 7; // avança para o próximo estado
      if(flag_temperature){
        measure_temp();
      }
      display_temperature(temperature);
    	break;
      
      case 7:
    	  // lê estado das colunas
        C[0] = digitalRead(col_1);
        C[1] = digitalRead(col_2);
        C[2] = digitalRead(col_3);

        // se todas retornaram para o nível alto vai para o estado 8
        if(compare_arrays(C, C_ones,3)==1){
          state = 8;
        }
        // se tecla ainda está apertada permanece nesse estado
        if(compare_arrays(C, C_ones,3) == 0){
          state = 7;
        }
      if(flag_temperature){
        measure_temp();
      }
      display_temperature(temperature);
    	break;

      case 8:
    	  // Debounce análogo ao estado 5
        // com o objetivo de identificar se a tecla foi solta
        time = count_ISR; // salva valor do contador das interrupções do timer 0
        while(count_ISR < time+30){ // espera esse valor incrementar 30 (~50ms)
        // para checar se a mudança realmente ocorreu
        }

        // se tecla foi solta        
        if(compare_arrays(C, C_ones,3)==1){
          key_to_command(key); // mostra valor da tecla no display
          state = 0; // retorna para a varredura
          }

        // se tecla não foi solta
        if(compare_arrays(C, C_ones,3) == 0){
          state = 7; // retorna para o estado anterior
          }
      if(flag_temperature){
        measure_temp();
      }
      display_temperature(temperature);
    	break;
  }
  
}

