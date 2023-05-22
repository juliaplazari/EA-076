//---------------------DEFINIÇÕES INICIAIS DE VARIÁVEIS  E BIBLIOTECAS NECESSÁRIAS-----------------------
#include <Wire.h>

const byte device_address = 0x50;
unsigned char read_byte;

void setup() {
  // configura biblioteca wire
  Wire.begin();

  // configura interface serial
  Serial.begin(9600);
  Serial.println("Hello");
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

void loop() {
  // Código para testar as funções
  // escrever 10 no endereço 100;
  write_eeprom(100, 10);
  _delay_ms(5);  // delay para garantir que operação de escrita foi concluída
  // ler endereço 100
  read_byte = read_eeprom(100);
  Serial.println(read_byte);

  // escrever 0x88 no endereço 0x523;
  write_eeprom(0x523, 0x88);
  _delay_ms(5);  // delay para garantir que operação de escrita foi concluída
  // ler endereço 0x523
  read_byte = read_eeprom(0x523);
  Serial.println(read_byte);
}
