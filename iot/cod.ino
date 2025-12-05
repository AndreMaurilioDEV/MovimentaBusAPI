#include <WiFi.h>
#include "ThingSpeak.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

const char* ssid = "*";
const char* password = "*";

#define SERIAL_BAUD_RATE 115200

// sensores
#define TRIG_PIN_1 4
#define ECHO_PIN_1 21
#define TRIG_PIN_2 5     
#define ECHO_PIN_2 6
#define TRIG_PIN_3 7
#define ECHO_PIN_3 9
#define TRIG_PIN_4 3
#define ECHO_PIN_4 2

// parametros
#define DISTANCIA_LIMITE 7  
#define TEMPO_MAXIMO_PASSAGEM 4000    
#define TEMPO_ENTRE_PASSAGENS 1000    


long duracao;
int distanciaCm;

// Ids 
const int idOnibus = 1920;    
const int idSensor = 1;       

// estado
int totalPessoas = 0;
unsigned long tempoSensor1Ativado = 0;

bool sensor1Ativo = false;
bool sensor2Ativo = false;

unsigned long tempoSensor4Ativado = 0;  
unsigned long tempoSensor3Ativado = 0;
bool sensor3AtivoSaida = false;          
bool sensor4AtivoSaida = false;          

unsigned long ultimaEntrada = 0;
unsigned long ultimaSaida = 0;


int entradasAcumuladas = 0;
int saidasAcumuladas = 0;
#define INTERVALO_ENVIO 5000  

WiFiClientSecure client;

long medirDistancia(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracao = pulseIn(echoPin, HIGH, 100000);
  
  if (duracao == 0) {
    return 999; 
  }
  
  long distancia = duracao * 0.034 / 2;
  
  if (distancia < 2 || distancia > 400) {
    return 999;
  }
  
  return distancia;
}


void verificarEntrada() {
  unsigned long tempoAtual = millis();

  long distancia1 = medirDistancia(TRIG_PIN_1, ECHO_PIN_1);
  long distancia2 = medirDistancia(TRIG_PIN_2, ECHO_PIN_2);

  bool sensor1Detectou = (distancia1 <= DISTANCIA_LIMITE);
  bool sensor2Detectou = (distancia2 <= DISTANCIA_LIMITE);


  if (sensor1Detectou && !sensor1Ativo) {
    sensor1Ativo = true;
    tempoSensor1Ativado = tempoAtual;
    Serial.println("Sensor externo ativado");
  }
  
  if (sensor2Detectou && sensor1Ativo && !sensor2Ativo) {
    sensor2Ativo = true;
    Serial.println("Sensor interno ativado");
    if (tempoAtual - tempoSensor1Ativado <= TEMPO_MAXIMO_PASSAGEM) {
      
      if (tempoAtual - ultimaEntrada > TEMPO_ENTRE_PASSAGENS) {
        entradasAcumuladas++;
        totalPessoas++;
        precisaEnviar = true;
        ultimaAlteracao = millis();
        ultimaEntrada = tempoAtual;
        
        Serial.println("Pessoa detectada.");
        Serial.println(totalPessoas);
        
      }
    }
    
    sensor1Ativo = false;
    sensor2Ativo = false;
  }
  
  if (sensor1Ativo && !sensor2Detectou && 
      (tempoAtual - tempoSensor1Ativado > TEMPO_MAXIMO_PASSAGEM)) {
    Serial.println("Passagem não confirmada");
    sensor1Ativo = false;
    sensor2Ativo = false;
  }
  
  if (!sensor2Detectou && sensor2Ativo) {
    sensor2Ativo = false;
    Serial.println("sensor interno resetado");
  }
  
  if (!sensor1Detectou && sensor1Ativo && !sensor2Ativo) {
    sensor1Ativo = false;
    Serial.println("sensor externo resetado");
  }
}



void verificarSaida() {
  unsigned long tempoAtual = millis();

  long distancia3 = medirDistancia(TRIG_PIN_3, ECHO_PIN_3); 
  long distancia4 = medirDistancia(TRIG_PIN_4, ECHO_PIN_4); 

  bool sensor3Detectou = (distancia3 <= DISTANCIA_LIMITE); 
  bool sensor4Detectou = (distancia4 <= DISTANCIA_LIMITE);

  if (sensor4Detectou && !sensor4AtivoSaida) {
    sensor4AtivoSaida = true;
    tempoSensor4Ativado = tempoAtual;
    Serial.println(">>> SAÍDA: Sensor interno ativado (início)");
  }

  if (sensor3Detectou && sensor4AtivoSaida && !sensor3AtivoSaida) {
    sensor3AtivoSaida = true;

    Serial.println(">>> SAÍDA: Sensor externo ativado");

    if (tempoAtual - tempoSensor4Ativado <= TEMPO_MAXIMO_PASSAGEM) {

      if (tempoAtual - ultimaSaida > TEMPO_ENTRE_PASSAGENS) {

        if (totalPessoas > 0) {
          saidasAcumuladas++; 
          totalPessoas--;
          precisaEnviar = true;
          ultimaAlteracao = millis();
          ultimaSaida = tempoAtual;

          Serial.println("SAÍDA CONFIRMADA! Pessoa saiu.");
          Serial.print(" Total: ");
          Serial.println(totalPessoas);

        } else {
          Serial.println("Tentativa de saída com ônibus vazio");
        }
      }
    }

    sensor4AtivoSaida = false;
    sensor3AtivoSaida = false;
  }

  if (sensor4AtivoSaida && 
      (tempoAtual - tempoSensor4Ativado > TEMPO_MAXIMO_PASSAGEM)) {

    Serial.println(">>> SAÍDA: Timeout - passagem não confirmada");
    sensor4AtivoSaida = false;
    sensor3AtivoSaida = false;
  }
}

void conectarWiFi() {
  Serial.println();
  Serial.print("📡 Conectando na rede: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(1000);
    Serial.print(".");
    tentativas++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println(" Wi-Fi conectado!");
    Serial.print(" IP: ");
  } else {
    Serial.println();
    Serial.println(" Falha na conexão Wi-Fi!");
    delay(5000);
    ESP.restart();
  }
}

const char* minhaApiURL = "https://movimentabusapi.onrender.com/api/onibus";

void enviarParaMinhaAPI() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    Serial.print("🎯 Enviando para: ");
    Serial.println(minhaApiURL);
    
    http.begin(client, minhaApiURL);
    http.addHeader("Content-Type", "application/json");
    
    String jsonDados = "{";
    jsonDados += "\"idOnibus\": " + String(idOnibus) + ",";
    jsonDados += "\"totalPessoas\": " + String(totalPessoas) + ",";
    jsonDados += "\"entradas\": " + String(entradasAcumuladas) + ",";
    jsonDados += "\"saidas\": " + String(saidasAcumuladas);
    jsonDados += "}";
    
    Serial.print(" JSON: ");
    Serial.println(jsonDados);
    
    int httpCode = http.POST(jsonDados);
    
    if (httpCode == 200) {
      Serial.println(" Dados enviados para API!");
      entradasAcumuladas = 0;
      saidasAcumuladas = 0;
    } else {
      Serial.print(" Erro API: ");
      Serial.println(httpCode);
    }
    
    http.end();
  } else {
    Serial.println(" Wi-Fi desconectado! Reconectando...");
    conectarWiFi(); // Tenta reconectar
  }
}


void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("\n\n[SISTEMA] Contador com 4 Sensores - Verificação + Confirmação");

  pinMode(TRIG_PIN_1, OUTPUT);
  pinMode(ECHO_PIN_1, INPUT);
  pinMode(TRIG_PIN_2, OUTPUT);
  pinMode(ECHO_PIN_2, INPUT);
  pinMode(TRIG_PIN_3, OUTPUT);
  pinMode(ECHO_PIN_3, INPUT);
  pinMode(TRIG_PIN_4, OUTPUT);
  pinMode(ECHO_PIN_4, INPUT);

  conectarWiFi();

  client.setInsecure();  
  HTTPClient https;

  if (https.begin(client, "https://movimentabusapi.onrender.com/")) {
    int httpCode = https.GET();

    if (httpCode > 0) {
      Serial.printf("Código HTTP: %d\n", httpCode);
      String payload = https.getString();
      Serial.println("Resposta:");
      Serial.println(payload);
    } else {
      Serial.printf("Falha na requisição: %s\n", https.errorToString(httpCode).c_str());
    }

    https.end();
  } else {
    Serial.println("Erro ao iniciar HTTPS");
  }
  Serial.println("[SISTEMA] Pronto para detecção com confirmação");
}

void loop() {
  
  verificarEntrada();
  verificarSaida();
    unsigned long tempoAtual = millis();
    if (precisaEnviar && (tempoAtual - ultimaAlteracao >= 800)) {
    enviarParaMinhaAPI();
    ultimoEnvioThingSpeak = tempoAtual;
}
  delay(100);
}