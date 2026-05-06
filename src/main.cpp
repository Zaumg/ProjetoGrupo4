#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
 
#include "WifiManager.h"
#include "MqttManager.h"
#include "DebugManager.h"
#include "LED.h"                
 
//*====CONSTANTES====
const int pinoLedRGB = 48;
const int QntLeds = 1;
const char TOPICO_COMANDO[] = "senai134/peach/lampada";
 
//*====INSTANCIAS====
Adafruit_NeoPixel ledRGB(
    QntLeds,
    pinoLedRGB,
    82);
 
Led lampada(2);

void tratarJsonComando(const String &mensagem);
void configurarLedRGB();
void alterarCorLedRGB(int vermelho, int verde, int azul);
void tratarMensagemRecebida(const char *topico, const String &mensagem);
 
void setup()
{
  configurarDebug();
  conectarWiFi();
  configurarMQTT();
  registrarCallbackMensagem(tratarMensagemRecebida);
  conectarMQTT();
  configurarLedRGB();
}
 
void loop()
{
  garantirWiFiConectado();
  garantirMQTTconectado();
  loopMQTT();
 
  lampada.update();             
}
 
void tratarMensagemRecebida(const char *topico, const String &mensagem)
{
  debugInfo("==============================");
  debugInfo("Mensagem recebida na Aplicação");
  debugInfo("==============================");
 
  if (topico == nullptr)
  {
    debugErro("Tópico MQTT inválido.");
  }
  debugInfo("Tópico: " + String(topico));
  debugInfo("Mensagem: " + mensagem);
 
  if (strcmp(topico, TOPICO_COMANDO) == 0)
  {
    tratarJsonComando(mensagem);
    return;
  }
  debugErro("Tópico não tratado: " + String(topico));
}
 
void configurarLedRGB()
{
  ledRGB.begin();
  ledRGB.setBrightness(80);
  ledRGB.clear();
  ledRGB.show();
 
  debugInfo("LED RGB configurado no pino " + String(pinoLedRGB));
}
 
void alterarCorLedRGB(int vermelho, int verde, int azul)
{
  vermelho = constrain(vermelho, 0, 255);
  verde    = constrain(verde,    0, 255);
  azul     = constrain(azul,     0, 255);
 
  ledRGB.setPixelColor(0, ledRGB.Color(vermelho, verde, azul));
  ledRGB.show();
 
  debugInfo("Cor aplicada no LED RGB");
  debugInfo("R: " + String(vermelho));
  debugInfo("G: " + String(verde));
  debugInfo("B: " + String(azul));
}
 
void tratarJsonComando(const String &mensagem)
{
  JsonDocument doc;
  DeserializationError erro = deserializeJson(doc, mensagem);
 
  if (erro)
  {
    debugErro("Erro ao interpretar JSON");
    debugErro(erro.c_str());
    return;
  }
 
  // --- Controle do LED RGB ---
  if (doc["led"].is<JsonObject>())          
  {
    if (!doc["led"]["r"].is<int>() ||
        !doc["led"]["g"].is<int>() ||
        !doc["led"]["b"].is<int>())
    {
      debugErro("JSON inválido. Use led.r, led.g e led.b");
    }
    else
    {
      int vermelho = doc["led"]["r"].as<int>();
      int verde    = doc["led"]["g"].as<int>();
      int azul     = doc["led"]["b"].as<int>();
 
      alterarCorLedRGB(vermelho, verde, azul);
    }
  }
  else
  {
    debugInfo("Não encontrado o comando para LED RGB");  
  }
 
  // --- Controle da Lâmpada (biblioteca Led) ---
  if (doc["lampada"].is<JsonObject>())
  {
    String acao = doc["lampada"]["acao"].as<String>();
 
    if (acao == "ligar")
    {
      lampada.ligar();
      debugInfo("Lâmpada ligada");
    }
    else if (acao == "desligar")
    {
      lampada.desligar();
      debugInfo("Lâmpada desligada");
    }
    else if (acao == "piscar")
    {
      float freq = doc["lampada"]["frequencia"] | 1.0f;
      lampada.piscar(freq);
      debugInfo("Lâmpada piscando a " + String(freq) + "Hz");
    }
    else
    {
      debugErro("Ação desconhecida para lâmpada: " + acao);
    }
  }
}