#define BLYNK_TEMPLATE_ID "TMPL2nT-l67rq"
#define BLYNK_TEMPLATE_NAME "Caixa1"
#define BLYNK_AUTH_TOKEN "AV48b7y2FuYJrkLUPTZXmiHQieB__41b"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "NNetCom_LABORATORIO";
char pass[] = "patronato70";

#define TRIG 5
#define ECHO 18

float alturaCaixa = 8;
bool alertaEnviado = false;

int nivelAnterior = 0;
unsigned long tempoAnterior = 0;
bool alertaVazamento = false;

BlynkTimer timer;

void enviarNivel() {

  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duracao = pulseIn(ECHO, HIGH);

  float distancia = duracao * 0.034 / 2;

  float nivel = alturaCaixa - distancia;

  int porcentagem = (nivel / alturaCaixa) * 100;

  porcentagem = constrain(porcentagem, 0, 100);

  Blynk.virtualWrite(V0, porcentagem);

  if (porcentagem >= 50) {

    Blynk.virtualWrite(V1,
    "🟢 Nivel Seguro");
  }

 else if (porcentagem >= 20) {

    Blynk.virtualWrite(V1,
    "🟡 Nivel Moderado");
  }

  else {

    Blynk.virtualWrite(V1,
    "🔴 Nivel Crítico");
  }

  if (porcentagem < 20 && !alertaEnviado) {

  Serial.println("ALERTA ENVIADO");

  Blynk.logEvent("nivel_baixo",
  "A caixa d'agua esta abaixo de 20%!");

  alertaEnviado = true;
  }

  if (porcentagem > 25) {
  alertaEnviado = false;
  }

  unsigned long tempoAtual = millis();

  int diferenca = nivelAnterior - porcentagem;

  if (diferenca > 15 &&
    (tempoAtual - tempoAnterior) < 60000 &&
    !alertaVazamento) {

    Blynk.logEvent("vazamento",
    "Possivel vazamento detectado!");

    alertaVazamento = true;
  }

  if (diferenca < 5) {
    alertaVazamento = false;
  }

  nivelAnterior = porcentagem;
  tempoAnterior = tempoAtual;

  Serial.print("Nivel: ");
  Serial.print(porcentagem);
  Serial.println("%");
}

void setup() {

  Serial.begin(115200);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(2000L, enviarNivel);
}

void loop() {
  Blynk.run();
  timer.run();
}
