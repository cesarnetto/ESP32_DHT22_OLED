/***************************************************************************
       Comunicação com display OLED, DHT22 e horario via NTP Wifi
****************************************************************************
  -- IDE do Arduino Versão 1.8.15
  -- Biblioteca DHT22: https://github.com/piettetech/PietteTech_DHT
  -- Biblioteca do display OLED: https://github.com/ThingPulse/esp8266-oled-ssd1306
/*****************************************************************************
                                Ligações
                  ESP32      DHT22     OLED display
                  3V3        Vcc            Vcc
                  GND        GND            GND
                  D23(37)    DAT             -
                  D21(33)     -             SDA
                  D22(36)     -             SCL
**************************************************************************** */
// Include
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "SSD1306.h"
#include "PietteTech_DHT.h"
 
// Defines
#define NTP_OFFSET  -3  * 60 * 60       // Em segundos
#define NTP_INTERVAL 60 * 1000          // Em milissegundos
#define NTP_ADDRESS  "0.pool.ntp.org"   // Url do site de serviço NTP
#define SDA_PIN 33                      // GPIO21 -> SDA
#define SCL_PIN 32                      // GPIO22 -> SCL
#define SSD_ADDRESS 0x3c                // Endereço do display no bus i2c
#define DHTTYPE  DHT22                  // Tipo de sensor DHT11/21/22/AM2301/AM2302
#define DHTPIN   27                     // DPino digital para comunicação
 
const char *ssid     = "iPhone de Cesar";    // Nome da rede Wifi
const char *password = "cesar246";   // Senha da rede Wifi
 
// Declaração de variaveis globais
String formattedTime;
float localHum = 0;
float localTemp = 0;
String ipStr;
int count = 0;
 
// Construtores c++
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
SSD1306  display(SSD_ADDRESS, SDA_PIN, SCL_PIN);
 
// Protótipo de função
void dht_wrapper(); //Deve ser declarada antes da inicialização da biblioteca
 
// Instanciação da biblioteca
PietteTech_DHT DHT(DHTPIN, DHTTYPE, dht_wrapper);
 
// Modo de uso da conexão Wifi
WiFiClient client;
 
void setup()
{
  Serial.begin(115200);
  Serial.println("");
  Serial.println("ESP32 DHT Temp & Humidity com Time Stamp - OLED Display");
  Serial.println("");
 
  display.init();
  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
 
  connectWiFi();        // Chama a função para conexão com a rede Wifi
  timeClient.begin();   // Inicia o serviço NTP
}
 
void loop() {
  count ++;
  Serial.print("Coletando informações dos sensores: ");
 
  // Função que lê os dados de status do sensor e aguarda
  int result = DHT.acquireAndWait(0);
 
  // Trabalha em cima do resultado da leitura.
  switch (result) {
    case DHTLIB_OK:
      Serial.println("OK");
      break;
    case DHTLIB_ERROR_CHECKSUM:
      Serial.println("Error\n\r\tChecksum error");
      break;
    case DHTLIB_ERROR_ISR_TIMEOUT:
      Serial.println("Error\n\r\tISR time out error");
      break;
    case DHTLIB_ERROR_RESPONSE_TIMEOUT:
      Serial.println("Error\n\r\tResposta time out error");
      break;
    case DHTLIB_ERROR_DATA_TIMEOUT:
      Serial.println("Error\n\r\tData time out error");
      break;
    case DHTLIB_ERROR_ACQUIRING:
      Serial.println("Error\n\r\tAquisição");
      break;
    case DHTLIB_ERROR_DELTA:
      Serial.println("Error\n\r\tVariação de tempo muito curto");
      break;
    case DHTLIB_ERROR_NOTSTARTED:
      Serial.println("Error\n\r\tNão iniciado");
      break;
    default:
      Serial.println("Erro desconhecido");
      break;
  }
  timeClient.update();    // Atualiza leitura de hora da rede
  displayData();          // indica no display
 
  if (count == 10) {
    getDHT();             // Realiza leitura
    displayData();
    count = 0;
  }
  delay(1000);
}
/***************************************************************************
  Função que acessa classes das funções de leitura do DHT que estão
  armazenadas em outro local - Wrapper
  Chama a função de interrupção ISR
***************************************************************************/
void dht_wrapper() {
  DHT.isrCallback();
}
/***************************************************************************
  Função que faz leitura da temperatura e umidade indicadas pelo sensor DHT22
***************************************************************************/
void getDHT()
{
  localTemp = DHT.getCelsius();
  delay(5);
  localHum = DHT.getHumidity();
 
  // Confere se alguma leitura falhou
  if (isnan(localHum) || isnan(localTemp))
  {
    return;
  }
}
/**************************************************************************
  função que envia os dados para o display
**************************************************************************/
void displayData()
{
  formattedTime = timeClient.getFormattedTime();
  Serial.print(formattedTime);
 
  Serial.print("  Temp: => ");
  Serial.print(localTemp);
  Serial.print("  Hum => ");
  Serial.println(localHum);
  display.clear();                // Apaga o display
  display.setFont(ArialMT_Plain_10);
  display.drawString(3, 0,  "T:");
  display.drawString(14, 0,  String(localTemp));
  display.drawString(44, 0,  "oC");
  display.drawString(75, 0, "H:");
  display.drawString(87, 0,  String(localHum));
  display.drawString(117, 0 ,  "%");
 
  display.setFont(ArialMT_Plain_24);
  display.drawString(17, 18,  String(formattedTime));
 
  display.setFont(ArialMT_Plain_10);
  display.drawString(15, 50 ,  "IP: " + ipStr);
 
  display.display();   // Envia o buffer de dados para o display
  delay(10);
}
/**************************************************************************
  Função de conexão com a rede Wifi
**************************************************************************/
void connectWiFi(void)
{
  int i;
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);
 
  display.drawString(2, 10, "Conectando a: ");
  display.drawString(1, 25, String(ssid));
  display.display();
 
  // Inicia a conexão passando as credenciais como parametros
  WiFi.begin(ssid, password);
 
  // Aguarda a conexão
  while ((WiFi.status() != WL_CONNECTED) && i < 100) {
    i ++;
    delay(500);
    Serial.print(".");
    display.drawString((3 + i * 2), 35, "." );
    display.display();
  }
 
  // Se a conexão foi bem sucedida
  if ( WiFi.status() == WL_CONNECTED) {
 
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.drawString(3, 20,  "WiFi conectado!!");
 
    IPAddress ip = WiFi.localIP(); // Converte o IP
 
    // Monta a string com o numero IP para indicar no display
    ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    Serial.println("");
    Serial.println("WiFi Conectado.");
    Serial.println("Endereço IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Máscara de rede: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());
    display.display();
  } else {  // Se não conectou
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.drawString(3, 16,  "WiFi nao conectado");
    ipStr = "NAO CONECTADO";
    Serial.println("");
    Serial.println("WiFi Não conectado.");
    display.display();
  }
  delay(1000);
}
