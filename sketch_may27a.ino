#define BLYNK_TEMPLATE_ID "TMPL2nT-l67rq"
#define BLYNK_TEMPLATE_NAME "Caixa1"
#define BLYNK_AUTH_TOKEN "AV48b7y2FuYJrkLUPTZXmiHQieB__41b"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "NNetCom_LABORATORIO";
char pass[] = "patronato70";

#define TRIG 5
#define ECHO 18

// ========================================
// CALIBRAÇÃO REAL DO SENSOR
// ========================================

// Distância com recipiente VAZIO
float distanciaVazio = 21.17;

// Distância com recipiente CHEIO
float distanciaCheio = 6.94;

// ========================================

bool alertaBaixo = false;
bool alertaMetade = false;
bool alertaTopo = false;
bool alertaVazamento = false;

int nivelAnterior = 0;
unsigned long tempoAnterior = 0;

BlynkTimer timer;

// ========================================
// CONECTA WIFI
// ========================================

void conectarWiFi() {

  Serial.println("Conectando ao WiFi...");

  WiFi.begin(ssid, pass);

  int tentativas = 0;

  while (WiFi.status() != WL_CONNECTED &&
         tentativas < 20) {

    delay(500);

    Serial.print(".");

    tentativas++;
  }

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("");
    Serial.println("WiFi conectado!");

    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

  } else {

    Serial.println("");
    Serial.println("Falha ao conectar no WiFi");
  }
}

// ========================================
// CONECTA BLYNK
// ========================================

void conectarBlynk() {

  Serial.println("Conectando ao Blynk...");

  Blynk.config(BLYNK_AUTH_TOKEN);

  if (Blynk.connect(10000)) {

    Serial.println("Blynk conectado!");

  } else {

    Serial.println("Falha ao conectar no Blynk");
  }
}

// ========================================
// LEITURA DO SENSOR
// ========================================

void enviarNivel() {

  // Pulso ultrassônico
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG, LOW);

  // Tempo de resposta
  long duracao =
    pulseIn(ECHO, HIGH, 50000);

  // Verifica erro
  if (duracao == 0) {

    Serial.println(
      "Erro na leitura do sensor"
    );

    return;
  }

  // Distância em cm
  float distancia =
    duracao * 0.034 / 2;

  // ========================================
  // CONVERSÃO PARA PORCENTAGEM
  // ========================================

  int porcentagem = map(
    distancia,
    distanciaVazio,
    distanciaCheio,
    0,
    100
  );

  // Limita entre 0 e 100
  porcentagem = constrain(
    porcentagem,
    0,
    100
  );

  // ========================================
  // SERIAL MONITOR
  // ========================================

  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");

  Serial.print("Nivel: ");
  Serial.print(porcentagem);
  Serial.println("%");

  // ========================================
  // VERIFICA CONEXÃO
  // ========================================

  if (WiFi.status() == WL_CONNECTED &&
      Blynk.connected()) {

    // Envia nível
    Blynk.virtualWrite(
      V0,
      porcentagem
    );

    // ========================================
    // STATUS
    // ========================================

    if (porcentagem >= 80) {

      Blynk.virtualWrite(
        V1,
        "🔵 Caixa quase cheia"
      );

    }
    else if (porcentagem >= 50) {

      Blynk.virtualWrite(
        V1,
        "🟢 Nivel Seguro"
      );

    }
    else if (porcentagem >= 20) {

      Blynk.virtualWrite(
        V1,
        "🟡 Nivel Moderado"
      );

    }
    else {

      Blynk.virtualWrite(
        V1,
        "🔴 Nivel Critico"
      );
    }

    // ALERTA NÍVEL BAIXO

    if (porcentagem < 20 &&
        !alertaBaixo) {

      Serial.println(
        "ALERTA NIVEL BAIXO"
      );

      Blynk.logEvent(
        "nivel_baixo",
        "A caixa d'agua esta abaixo de 20%!"
      );

      alertaBaixo = true;
    }

    // Rearma
    if (porcentagem > 25) {

      alertaBaixo = false;
    }

    // ========================================
    // ALERTA METADE
    // ========================================

    if (porcentagem >= 45 &&
        porcentagem <= 55 &&
        !alertaMetade) {

      Serial.println(
        "CAIXA NA METADE"
      );

      Blynk.logEvent(
        "metade",
        "A caixa d'agua chegou em 50%."
      );

      alertaMetade = true;
    }

    // Rearma
    if (porcentagem < 40 ||
        porcentagem > 60) {

      alertaMetade = false;
    }

    // ========================================
    // ALERTA TOPO
    // ========================================

    if (porcentagem >= 90 &&
        !alertaTopo) {

      Serial.println(
        "CAIXA QUASE CHEIA"
      );

      Blynk.logEvent(
        "transbordando",
        "A caixa d'agua esta quase cheia!"
      );

      alertaTopo = true;
    }

    // Rearma
    if (porcentagem < 85) {

      alertaTopo = false;
    }

    // ========================================
    // DETECÇÃO DE VAZAMENTO
    // ========================================

    unsigned long tempoAtual =
      millis();

    int diferenca =
      nivelAnterior - porcentagem;

    if (diferenca > 15 &&
        (tempoAtual - tempoAnterior)
        < 60000 &&
        !alertaVazamento) {

      Serial.println(
        "POSSIVEL VAZAMENTO"
      );

      Blynk.logEvent(
        "vazamento",
        "Possivel vazamento detectado!"
      );

      alertaVazamento = true;
    }

    // Rearma
    if (diferenca < 5) {

      alertaVazamento = false;
    }

    nivelAnterior = porcentagem;

    tempoAnterior = tempoAtual;

  } else {

    Serial.println(
      "Sem conexao com WiFi/Blynk"
    );
  }

  // Pequeno intervalo para estabilidade
  delay(100);
}

// ========================================
// SETUP
// ========================================

void setup() {

  Serial.begin(115200);

  delay(2000);

  Serial.println(
    "ESP32 INICIADO"
  );

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  conectarWiFi();

  conectarBlynk();

  timer.setInterval(
    2000L,
    enviarNivel
  );
}

// ========================================
// LOOP
// ========================================

void loop() {

  // Reconecta WiFi
  if (WiFi.status() != WL_CONNECTED) {

    Serial.println(
      "WiFi desconectado!"
    );

    conectarWiFi();
  }

  // Reconecta Blynk
  if (!Blynk.connected()) {

    Serial.println(
      "Reconectando Blynk..."
    );

    conectarBlynk();
  }

  Blynk.run();

  timer.run();
}