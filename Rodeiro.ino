#include "max6675.h"  // Biblioteca utilizada para o amplificador de sinal dos termopares
#include "LiquidCrystal_I2C.h"  // Biblioteca da tela LCD
#include "Wire.h"  // Biblioteca para realizar as comunicações com a tela LCD

LiquidCrystal_I2C lcd(0x27, 16, 2);

int inversor_pin = 8;  // Pino digital conectado ao relé de comando do motor/inversor
int acionar_k1_pin = 7;  // Pino digital conectado ao relé de comando de acionamento da central
int liberar_k2_pin = 6;  // Pino digital conectado ao relé de comando de liberação/comutação da válvula da central
int ciclo = 0;  // Contador de ciclos

// Definindo o termopar e suas conexões
int thermoD0 = 3;  // so
int thermoCS = 4;
int thermoCLK = 5;  // sck
float temp;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoD0);


void setup() {
  Serial.begin(9600);  // Iniciar leitura serial, definindo baudrate = 9600

  // Definindo todos os pinos de relé como OUTPUT (eles terão a função de enviar informação ao relé)
  pinMode(inversor_pin, OUTPUT);
  pinMode(acionar_k1_pin, OUTPUT);
  pinMode(liberar_k2_pin, OUTPUT);

  // Inicialização da tela LCD
  lcd.init();
  lcd.backlight();

  lcd.clear();  // Limpa a tela
  lcd.setCursor(0,0);  // Coloca o cursor no lugar necessário para digitar
  lcd.print("INICIANDO...");  // Escrever na tela que está iniciando
}

void loop() {

  

  // Deixar ambos os comandos da central em seu estado padrão:
  digitalWrite(acionar_k1_pin, HIGH);  // Freio não acionado
  digitalWrite(liberar_k2_pin, LOW);  // Válvula comutada permitindo passagem do fluido (para permitir que a pastilha volte mais facilmente diminuindo a resistência)

  int value = analogRead(A0);  // Ler valor da tensão enviada pelo relé interno do inversor (deve estar em 5V até atingir a velocidade desejada, definida em P281 e P002) 
  float voltage = value*(5.0/1023.0);  // Converter o valor codificado em tensão real 

  //Serial.println(voltage);  // Mostrar a tensão no leitor serial

  if(voltage < 4.90){  // Se a tensão estiver abaixo de 4,90 (compensa pela flutuação entre 5 e 4,90), então terá atingido a velocidade desejada e a frenagem deve ser iniciada

    if(ciclo == 0){  // Se estiver no primeiro ciclo, esperar por 5 min (300 s ou 300000 ms) para aquecer os rolamentos

      // Mostrando na tela que está no período de aquecimento
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("AQUECIMENTO");
      lcd.setCursor(0,1);
      lcd.print("Ciclo: ");
      lcd.setCursor(8,1);
      lcd.print(ciclo + 1);

      // Espera por 5 min
      delay(300000);
    }

    digitalWrite(inversor_pin, HIGH);  // Desliga o motor comandando o relé do inversor
    temp = thermocouple.readCelsius();  // Lê a temperatura atual (temperatura inicial de frenagem)

    // Mostra a temperatura e o ciclo atual
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Temp: ");
    lcd.setCursor(7,0);
    lcd.print(temp);
    lcd.setCursor(0,1);
    lcd.print("Ciclo: ");
    lcd.setCursor(8,1);
    lcd.print(ciclo + 1);

    //Serial.print("Temperatura inicial: ");
    Serial.print(temp);
    Serial.print(", ");

    // Aciona o freio comandando o relé da válvula da central para impedir a liberação de pressão e comandando o relé de acionamento da central
    digitalWrite(liberar_k2_pin, HIGH);
    delay(500);  // Esperar um pouco antes de acionar
    digitalWrite(acionar_k1_pin, LOW);
    //Serial.println("Central acionada");

    delay(500);  // Esperar meio segundo antes de tirar o acionamento (não é necessário muito tempo para que a pressão seja atingida)

    // Parar o acionamento (pressão será mantida)
    digitalWrite(acionar_k1_pin, HIGH);
    //Serial.println("Central desacionada");

    delay(8000);  // Esperar 8 segundos para a frenagem ser realizada

    // Liberar a pressão (voltar ao estado padrão) comandando o relé que comanda a válvula da central
    digitalWrite(liberar_k2_pin, LOW);
    //Serial.println("Central liberada");
    temp = thermocouple.readCelsius();  // Lê a temperatura final da frenagem

    // Mostra a temperatura final da frenagem na tela
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Temp: ");
    lcd.setCursor(7,0);
    lcd.print(temp);
    lcd.setCursor(0,1);
    lcd.print("Ciclo: ");
    lcd.setCursor(8,1);
    lcd.print(ciclo + 1);

    //Serial.print("Temperatura final: ");
    Serial.print(temp);
    Serial.print(", ");

    delay(8000);  // Esperar mais 8 segundos para garantir liberação total da pressão e reiniciar o ciclo

    // OPCIONAL: Comutar a válvula novamente para fechar a liberação de pressão
    //digitalWrite(liberar_k2_pin, HIGH);
    //Serial.println("Comandos resetados");

    ciclo = ciclo + 1;  // Acrescentar à contagem de ciclos
    //Serial.print("Ciclo: ");
    Serial.println(ciclo + 1);

  }else{  // Se não tiver atingido a velocidade, manter o motor ligado até atingir
    digitalWrite(inversor_pin, LOW);
    //Serial.println("ATINGINDO VELOCIDADE");
    temp = thermocouple.readCelsius();

    // Mostra a temperatura atual (neste caso, em tempo real) e o ciclo atual
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Temp: ");
    lcd.setCursor(7,0);
    lcd.print(temp);
    lcd.setCursor(0,1);
    lcd.print("Ciclo: ");
    lcd.setCursor(8,1);
    lcd.print(ciclo + 1);
  }

  delay(200);  // Tempo necessário para leitura adequada do termopar

}
