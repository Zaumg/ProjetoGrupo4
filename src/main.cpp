#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

#include "WifiManager.h"
#include "MqttManager.h"
#include "DebugManager.h"

// ===== TÓPICOS =====
#define TOPICO_PERGUNTA  "quiz/pergunta"
#define TOPICO_RESPOSTA  "quiz/resposta"

// ===== LED RGB =====
// ⚠️ Se não funcionar, teste pinos: 2, 4 ou 5
#define PIN_RGB 48

Adafruit_NeoPixel led(1, PIN_RGB, NEO_GRB + NEO_KHZ800);

// ===== FUNÇÃO COR =====
void setColor(int r, int g, int b)
{
  led.setPixelColor(0, led.Color(r, g, b));
  led.show();

  debugInfo("LED -> R:" + String(r) + " G:" + String(g) + " B:" + String(b));
}

// ===== MQTT =====
void tratarMensagem(const char *topico, const String &mensagem)
{
  debugInfo("========== MQTT ==========");
  debugInfo("TOPICO: " + String(topico));
  debugInfo("MSG: " + mensagem);

  JsonDocument doc;
  DeserializationError erro = deserializeJson(doc, mensagem);
  static String respostaCorretaDaPergunta = "";

  if (erro)
  {
    debugErro("Erro ao ler JSON");
    return;
  }

  String tipo = doc["tipo"].as<String>();

  // 📩 RECEBE PERGUNTA → responde automático
  if (strcmp(topico, TOPICO_PERGUNTA) == 0 && tipo == "pergunta")
  {
    delay(2000);

    JsonDocument resp;
    resp["tipo"] = "resposta";
    resp["resposta"] = "V"; // pode mudar depois para botão real
    respostaCorretaDaPergunta = resp["resposta"].as<String>();

    String msg;
    serializeJson(resp, msg);

    publicarMensagem(TOPICO_RESPOSTA, msg.c_str());
  }

  // 📊 RECEBE RESULTADO → muda LED
  if (strcmp(topico, TOPICO_RESPOSTA) == 0 && tipo == "resposta")
  {
    String resultado = doc["resposta"].as<String>();

    debugInfo("Resultado recebido: " + resultado);

    if (resultado == respostaCorretaDaPergunta)
    {
      setColor(0, 255, 0); // 🟢 verde
    }
    else if (resultado != respostaCorretaDaPergunta)
    {
      debugInfo("entrou no else if em resposta errada");
      setColor(255, 0, 0); // 🔴 vermelho
    }
    else
    {
      debugErro("Resultado inválido");
    }
  }
}

// ===== SETUP =====
void setup()
{
  configurarDebug();
  conectarWiFi();
  configurarMQTT();

  registrarCallbackMensagem(tratarMensagem);
  conectarMQTT();

  // Inicializa LED
  led.begin();
  led.setBrightness(80);
  led.clear();
  led.show();

  // 🧪 TESTE INICIAL (confirma LED funcionando)
  setColor(255, 0, 0);
  delay(1000);
  setColor(0, 255, 0);
  delay(1000);
  setColor(0, 0, 255);
  delay(1000);
  setColor(0, 0, 0);

  debugInfo("ESP2 pronto");
}

// ===== LOOP =====
void loop()
{
  garantirWiFiConectado();
  garantirMQTTconectado();
  loopMQTT();
}

