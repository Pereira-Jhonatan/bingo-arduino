// ===================== CONFIGURAÇÃO DE PINOS =====================
const int numDigits = 4;
const int segmentPins[7] = {7, 8, 9, 10, 11, 12, 13};         // Segmentos A-G (comuns a todos os dígitos)
const int digitPins[numDigits] = {6, 5, 4, 3};                // Pinos para seleção dos 4 dígitos
const int buttonPin = A0;                                     // Botão de sorteio / reinício
const int buzzerPin = 2;                                      // Pino para o buzzer

// ===================== VARIÁVEIS DE ESTADO =====================
bool drawnNumbers[76] = {false};   // Controla quais números de 1 a 75 já foram sorteados
int totalDrawn = 0;                // Total de números sorteados
int lastNumber = 0;                // Último número sorteado
bool finished = false;             // Indica se todos os 75 números foram sorteados
bool buttonPressed = false;        // Estado do botão
bool showLastBeforeFim = false;    // Controla exibição final do último número
bool fimMostrado = false;          // Controla se "FIM" já foi exibido

unsigned long buttonPressTime = 0;
const unsigned long longPressDuration = 2000; // Tempo para pressionar e reiniciar

// ===================== MAPA DE NÚMEROS E CARACTERES =====================
// Dígitos de 0 a 9 para display de 7 segmentos (A-G)
const byte digits[10][7] = {
  {1,1,1,1,1,1,0}, // 0
  {0,1,1,0,0,0,0}, // 1
  {1,1,0,1,1,0,1}, // 2
  {1,1,1,1,0,0,1}, // 3
  {0,1,1,0,0,1,1}, // 4
  {1,0,1,1,0,1,1}, // 5
  {1,0,1,1,1,1,1}, // 6
  {1,1,1,0,0,0,0}, // 7
  {1,1,1,1,1,1,1}, // 8
  {1,1,1,1,0,1,1}  // 9
};

// Caracteres personalizados para letras e hífen
const byte letraF[7]      = {1,0,0,1,0,0,1}; // Letra F
const byte letraI[7]      = {1,0,0,1,0,0,1}; // Letra I
const byte letraM[7]      = {1,0,0,1,0,0,1}; // Letra M
const byte letraESPACO[7] = {0,0,0,0,0,0,0}; // Espaço (apagado)
const byte charHifen[7]   = {0,0,0,0,0,0,1}; // Traço (-)

// ===================== CONFIGURAÇÃO INICIAL =====================
void setup() {
  for (int i = 0; i < 7; i++) pinMode(segmentPins[i], OUTPUT);     // Configura segmentos
  for (int i = 0; i < numDigits; i++) pinMode(digitPins[i], OUTPUT); // Configura dígitos
  pinMode(buttonPin, INPUT_PULLUP);  // Botão com pull-up interno
  pinMode(buzzerPin, OUTPUT);        // Buzzer
  randomSeed(analogRead(A5));        // Semente de aleatoriedade
}

// ===================== LOOP PRINCIPAL =====================
void loop() {
  handleButton();  // Lida com o botão de sorteio/reinício

  if (finished) {
    // Exibe o último número por 5 segundos com multiplexação
    if (showLastBeforeFim) {
      unsigned long start = millis();
      while (millis() - start < 5000) {
        displayFormattedNumber(lastNumber);
      }
      showLastBeforeFim = false;
    }
    // Pisca "FIM" com som, 5 vezes
    else if (!fimMostrado) {
      for (int i = 0; i < 5; i++) {
        displayFIM();                  // Mostra "FIM"
        tone(buzzerPin, 2000, 200);    // Emite bip
        delay(500);                    // FIM visível
        clearAllDigits();              // Apaga display
        delay(500);                    // FIM apagado
      }
      fimMostrado = true;
      clearAllDigits();                // Apaga após final
    }
  } else {
    displayFormattedNumber(lastNumber); // Exibe número normalmente
  }
}

// ===================== LÓGICA DO BOTÃO =====================
void handleButton() {
  static bool wasHeld = false;

  if (digitalRead(buttonPin) == LOW) {
    // Pressionado pela primeira vez
    if (!buttonPressed) {
      buttonPressed = true;
      buttonPressTime = millis();
    }
    // Pressionado por mais de 2 segundos => reinicia
    else if (!wasHeld && millis() - buttonPressTime >= longPressDuration) {
      wasHeld = true;
      resetGame();
      tone(buzzerPin, 2000, 300);
    }
  } else {
    if (buttonPressed && !wasHeld) {
      if (finished) {
        resetGame(); // Se fim, reinicia ao clicar
      } else {
        animateDraw();               // Animação antes do sorteio
        int number = drawNumber();  // Sorteia
        if (number > 0) {
          lastNumber = number;
          tone(buzzerPin, 4000, 200);
          if (totalDrawn >= 75) {
            showLastBeforeFim = true;
            finished = true;
          }
        }
      }
    }
    buttonPressed = false;
    wasHeld = false;
  }
}

// ===================== SORTEIO DE NÚMERO =====================
int drawNumber() {
  if (totalDrawn >= 75) return -1;

  int n;
  do {
    n = random(1, 76); // Gera de 1 a 75
  } while (drawnNumbers[n]);

  drawnNumbers[n] = true;
  totalDrawn++;
  return n;
}

// ===================== REINICIAR O JOGO =====================
void resetGame() {
  for (int i = 1; i <= 75; i++) drawnNumbers[i] = false;
  totalDrawn = 0;
  lastNumber = 0;
  finished = false;
  showLastBeforeFim = false;
  fimMostrado = false;
}

// ===================== ANIMAÇÃO ANTES DO SORTEIO =====================
void animateDraw() {
  unsigned long start = millis();
  while (millis() - start < 800) {
    int n = random(1, 76);
    displayFormattedNumber(n); // Mostra número aleatório
    delay(50);
  }
}

// ===================== EXIBIR NÚMERO NO FORMATO PREFIXO-XX =====================
void displayFormattedNumber(int number) {
  if (number == 0) {
    // Exibe 0000 no início
    showDigit(0, 0);
    showDigit(1, 0);
    showDigit(2, 0);
    showDigit(3, 0);
    return;
  }

  // Define prefixo numérico baseado na faixa
  int prefix;
  if (number <= 15)      prefix = 1;
  else if (number <= 30) prefix = 2;
  else if (number <= 45) prefix = 3;
  else if (number <= 60) prefix = 4;
  else                   prefix = 5;

  int dezena = (number / 10) % 10;
  int unidade = number % 10;

  // Exibe ex: 2-43 no display
  showDigit(0, prefix);
  showCustomChar(1, charHifen);
  showDigit(2, dezena);
  showDigit(3, unidade);
}

// ===================== EXIBE "FIM" NO DISPLAY =====================
void displayFIM() {
  showCustomChar(0, letraF);
  showCustomChar(1, letraI);
  showCustomChar(2, letraM);
  showCustomChar(3, letraESPACO);
}

// ===================== EXIBIÇÃO DE DÍGITOS E CARACTERES =====================
void showDigit(int pos, int num) {
  for (int i = 0; i < numDigits; i++) digitalWrite(digitPins[i], HIGH);
  for (int i = 0; i < 7; i++) digitalWrite(segmentPins[i], digits[num][i]);
  digitalWrite(digitPins[pos], LOW);
  delay(5); // Multiplexação
}

void clearDigit(int pos) {
  for (int i = 0; i < numDigits; i++) digitalWrite(digitPins[i], HIGH);
  for (int i = 0; i < 7; i++) digitalWrite(segmentPins[i], LOW);
  digitalWrite(digitPins[pos], LOW);
  delay(5);
}

void showCustomChar(int pos, const byte seg[7]) {
  for (int i = 0; i < numDigits; i++) digitalWrite(digitPins[i], HIGH);
  for (int i = 0; i < 7; i++) digitalWrite(segmentPins[i], seg[i]);
  digitalWrite(digitPins[pos], LOW);
  delay(5);
}

void clearAllDigits() {
  for (int i = 0; i < numDigits; i++) clearDigit(i);
}
