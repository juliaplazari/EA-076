//--------------------------------DEFINIÇÕES INICIAIS DE VARIÁVEIS-----------------------------------
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
// Declarando pinos
#define SINAL 13
const int pwm = 11; //pino 11 como pwm (EN)
const int in_motor_1 = 9; //pino 9 como input 1 (1A)
const int in_motor_2 = 10; //pino 10 como input 2 (2A)
const int encoder_pin = 2; //pino 2 usado como input pelo encoder (interrupções)

// Declarando variáveis

// variáveis necessárias para contar bordas de subida e estimar velocidade
int count_edges = 0; // contador para o número de bordas de subida (interrupções associadas ao encoder)
int count_ISR =0; // contador para número de interrupções do temporizador
int N = 2; // number of pulses per lap for each encoder
volatile int frequency; // para cálculo da frequência de rotação do motor 

// variáveis necessárias para identificar comando digitado
char incomingByte; // para entrada de data serial
String readString; // string lida
String strToPrint; // string a ser printada
boolean done = false; // indica se string completa já foi lida

// Avalia se string é composta apenas de dígitos
int i;
int count_digits;
boolean isValidNumber(String str){
count_digits =0; // contador do número de dígitos
// itera por todos caracteres da string
for(byte i=0;i<str.length();i++)
{
  if(isDigit(str.charAt(i))){//caso caracter seja um dígito
    count_digits += 1; // incrementa contador
  }
}
// quando iterou pela string toda
if(count_digits == str.length()){ // se número de dígitos = tamanho da str
    return true; // retorna verdadeiro
  }
  else{ // caso contrário
    return false; // retorna falso
  }
}

// variáveis para executar comando desejado
float chose_duty_cycle = 0; // inicializa velocidade de rotação como 0
String chose_direction = "stop"; // inicializa sentido de rotação como parado

// Display 16x2
const int rs = 8, en = 7, d4 = 6, d5 = 5, d6 = 4, d7 = 3; //configura os pinos que serão usados no display
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
// variáveis que controlam o sentido e velocidade a serem mostrados no display
String message_velocity = "0%"; // inicializa velocidade de rotação como 0
String message_direction = "PARADO"; // inicializa sentido de rotação como parado

// Display de 7 segmentos
#include <Wire.h> // importa biblioteca para comunicação via protocolo I2C
unsigned int I2Cvar = 0b00000000; //variável para comunicação via protocolo I2C
int digit_to_display; //numero que sera mostrado em um display específico
//variável para cada um dos dígitos da frequência estimada
int freq1;
int freq2;
int freq3;
int freq4;

SoftwareSerial bluetooth(0, 1);

//-------------------------------------SETUP------------------------------------------
void setup() {
  // configura pinos associados a rotação do motor como output
  pinMode(pwm, OUTPUT);
  pinMode(in_motor_1, OUTPUT);
  pinMode(in_motor_2, OUTPUT);
  
  // configura pino associado ao encoder como input
  pinMode(encoder_pin,INPUT);
  // configura interrupção associada a uma borda de subida do pino associado ao encoder
  sei();
  attachInterrupt(digitalPinToInterrupt(encoder_pin), isr_edges, RISING);

  // Inicializa serial port, com data rate de 9600 bps
  Serial.begin(9600);
  pinMode(SINAL, OUTPUT);
  digitalWrite(SINAL, LOW);
  
  // configuração do timer 0 para interrupções periódicas
  configuracao_Timer0();
  sei();// permite interruções
  
  // display 16x2
  lcd.begin (16,2); //inicializa
  lcd.setCursor (0,0);
  lcd.print("Hello"); // printa mensagem inicial

  //I2 
  Wire.begin();

  bluetooth.begin(9600);
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

//------------------------------CONFIGURAÇÃO DAS INTERRUPÇÕES---------------------------------
void isr_edges(){ // a cada borda de subida do encoder
  count_edges += 1; // incrementa contador do número de bordas de subida
}

ISR(TIMER0_COMPA_vect){// a cada interrupção periódica do timer 0 (a cada 1.6ms)
  count_ISR += 1; // conta número de interrupções do temporizador (número de vezes que entrou nessa função)
  
  // frequência de rotação será calculada a cada 1s (625*1.6ms)
  if (count_ISR == 625){
    //frequência é dada por
    // - f = Np/(NT)
    // - Np é o número de pulsos gerados pelo encoder em T segundos (número de bordas de subida - count_edges)
    // - N é o número de pulsos o encoder produz por volta (no thinkercad é 60, no motor é 1)
	  // - T é o intervalo de tempo (1 s)
    frequency = 60*(count_edges/(1*N)); // calcula frequência
  	Serial.print(frequency); // printa frequência calculada (no momento está sendo usado para debugar)
    Serial.println(" RPM");
    // zera contadores
    count_edges = 0;
    count_ISR = 0;
  } 
}

//------------------------------------ROTAÇÃO DO MOTOR------------------------------------
// Rotaciona motor dado velocidade (duty cycle) e direção
void rotate_motor(String direction, float duty_cycle){
  // Sentido horário => VENT
  if(direction=="clockwise"){
    // Clockwise => 1A=L, 2A=H
	  digitalWrite(in_motor_1,LOW);
	  digitalWrite(in_motor_2,HIGH);
    // utiliza pwm para obter velocidade desejada (255 é o valor máximo)
    analogWrite(pwm,255*duty_cycle);
  }
  // Sentido anti-horário => EXAUST
  else if(direction=="counterclockwise"){
    // Counterclockwise => 1A=H, 2A=L
    digitalWrite(in_motor_1,HIGH);
  	digitalWrite(in_motor_2,LOW);
    // utiliza pwm para obter velocidade desejada (255 é o valor máximo)
    analogWrite(pwm,255*duty_cycle);
  }
  
  //Parado
  else if(direction=="stop"){
    // Motor stop => 1A=H, 2A=H, EN=H
    digitalWrite(in_motor_1,HIGH);
  	digitalWrite(in_motor_2,HIGH);
    analogWrite(pwm,255);
    // Espera a estimativa de frequência ser 0 para sair dessa função
    while(frequency != 0.0)
    // obs: esse while só funciona com variáveis do tipo volatile
    {
      display_frequency(frequency);
    }
  }
  
}

//----------------------------------RECEBE COMANDO------------------------------------
void receive_command() {
// envia dados apenas depois que dados são recebidos
  if (bluetooth.available() > 0 && done==false) {
    digitalWrite(SINAL, HIGH);
    // lê incoming byte
    incomingByte = bluetooth.read();
    digitalWrite(SINAL, LOW);
    
    // se byte é * (sinaliza fim da mesnagem)
	if (incomingByte == '*'){
      // atualiza variável para indicar que string completa foi recebida
      done==true;
      execute_command(); // chama função para executar o comando
      readString=""; // limpa string
     
    }
    
    // se byte é outro caracter
    else{// adiciona caracter lido às string
      readString += incomingByte;
      i++;
    }   

    digitalWrite(SINAL, HIGH);
  }
}

//----------------------------------EXECUTA COMANDO------------------------------------
void execute_command(){
  // Caso primeiros caracteres da string sejam VEL
  if (readString.substring(0, 4) == "VEL "){
    
    // Verifica se parâmetro foi recebido
    if (readString.length() < 7){
    	Serial.println("ERRO: PARAMETRO AUSENTE");
    }
    
    // Verifica se parâmetro é composto apenas por dígitos
    else if (isValidNumber(readString.substring(4,7))==false){
      Serial.println("ERRO: PARAMETRO INCORRETO");
    }
    
    // Verifica se parâmetro é um número válido (menor ou igual a 100)
    else if (readString.substring(3, 7).toInt()>100){
      Serial.println("ERRO: PARAMETRO INCORRETO");
    }
    
    else{
    	strToPrint = "OK VEL";
    	strToPrint += readString.substring(3, 7);
    	strToPrint += "% \n";
    	Serial.println(strToPrint); //printa mensagem associada ao comando
      // atualiza valor da variável que indica velocidade 
      chose_duty_cycle = readString.substring(4, 7).toFloat()/100;
      // atualiza valor do duty cycle a ser mostrado no display
      message_velocity = readString.substring(4, 7) + "%";
    }
  }
  
  // Caso string seja VENT
  else if (readString == "VENT"){
  	Serial.println("OK VENT\n");//printa mensagem associada ao comando
    rotate_motor("stop",1); // para motor antes de trocar o sentido
    chose_direction = "clockwise"; // atualiza valor da velocidade que indica direção
  	message_direction = "VENTILADOR"; // atualiza valor da direção a ser mostrada no display
  }
  
  // Caso string seja EXAUST
  else if (readString == "EXAUST"){
  	Serial.println("OK EXAUST\n"); //printa mensagem associada ao comando
    rotate_motor("stop",1); // para motor antes de trocar o sentido
    chose_direction = "counterclockwise"; // atualiza valor da velocidade que indica direção
  	message_direction = "EXAUSTOR"; // atualiza valor da direção a ser mostrada no display
  }
  
  // caso string seja PARA
  else if (readString == "PARA"){
  	Serial.println("OK PARA\n"); //printa mensagem associada ao comando
    chose_direction = "stop"; // atualiza valor da velocidade que indica direção
  	message_direction = "PARADO"; // atualiza valor da direção a ser mostrada no display
  }
  
  // Caso string seja REVEL
  else if (readString == "RETVEL"){
    Serial.print("VEL: ");
    Serial.print(frequency); // printa frequência calculada nas interrupções do timer0
    Serial.println(" RPM");
  }
  
  // Caso string digitada seja diferente disso
  else{
  	Serial.println("ERRO: COMANDO INEXISTENTE"); //printa mensagem de erro
  }
  
  // Chama função para rotacionar motor no sentido e velocidade desejados
  rotate_motor(chose_direction,chose_duty_cycle);

   display_16x2(message_direction,message_velocity);

}

//----------------------------------DISPLAYS------------------------------------
void display_16x2(String message_direction, String message_velocity){
  // Mensagem no display 16x2
  lcd.clear(); // apaga display antes de mostrar nova mensagem
  lcd.setCursor(0,0); // mensagem de direção na primeira linha
  lcd.print(message_direction);
  
  lcd.setCursor(0,1); // mensagem de velocidade na segunda linha
  lcd.print(message_velocity);

}

void select_display(String display){
  //atualiza valor de I2Cvarde modo a selecionar o display desejado (obs: display ativo em baixo)
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

void select_number(int number){
  //mascara para atualizar variável e mostrar o número desejado no display
  if(number==0){
  I2Cvar= I2Cvar & 0b11110000; 
  }

  else if(number==1){
  I2Cvar= I2Cvar & 0b11110001; 
  }

  else if(number==2){
  I2Cvar= I2Cvar & 0b11110010; 
  }

  else if(number==3){
  I2Cvar= I2Cvar & 0b11110011; 
  }

  else if(number==4){
  I2Cvar= I2Cvar & 0b11110100; 
  }

  else if(number==5){
  I2Cvar= I2Cvar & 0b11110101; 
  }

  else if(number==6){
  I2Cvar= I2Cvar & 0b11110110; 
  }

  else if(number==7){
  I2Cvar= I2Cvar & 0b11110111; 
  }

  else if(number==8){
  I2Cvar= I2Cvar & 0b11111000; 
  }

  else if(number==9){
  I2Cvar= I2Cvar & 0b11111001; 
  }

}

void display_frequency(int frequency){
  //primeiro dígito
  freq1 = (frequency / 1U) % 10; ;
  select_display("D4"); //seleciona D4
  number_to_display(freq1); //mascara para mostrar numero

  //segundo dígito
  freq2 = (frequency / 10U) % 10; ;
  select_display("D3"); //seleciona D3
  number_to_display(freq2); //mascara para mostrar numero

  freq3 = (frequency / 100U) % 10; ;
  select_display("D2"); //seleciona D2
  number_to_display(freq3); //mascara para mostrar numero

  freq4 = (frequency / 1000U) % 10; ;
  select_display("D1"); //seleciona D1
  number_to_display(freq4); //mascara para mostrar numero
}

void number_to_display(int digit_to_display){
    select_number(digit_to_display);
    Wire.beginTransmission(39);  
    Wire.write(I2Cvar);             
    Wire.endTransmission();      
}

void loop() {
  // repetidamente chama função de receber o comando
  receive_command();

  // mostra frequência estimada pelo encoder
  display_frequency(frequency);
  }
