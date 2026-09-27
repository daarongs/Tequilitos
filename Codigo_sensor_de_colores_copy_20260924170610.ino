// CONFIGURACIÓN DE PINES 

// Sensor de color: TCS3200
const int S0 = 2; 
const int S1 = 3;
const int S2 = 5;
const int S3 = 6;
const int sensorOut = 4; 

// LEDs de colores
const int ledAmarillo = 11; //Amarillo
const int ledRojo     = 10; //Naranja
const int ledVerde    = 9;  //Rosa
const int ledAzul     = 8;  //Cyan

// Variables para lecturas de frecuencia de color
int redFreq = 0;
int greenFreq = 0;
int blueFreq = 0;

// setup  (configuración inicial)
void setup() {
  
// configuracion pines del sensor
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

// configuracion pines leds
  pinMode(ledAmarillo, OUTPUT);
  pinMode(ledRojo, OUTPUT);
  pinMode(ledVerde, OUTPUT);
  pinMode(ledAzul, OUTPUT);
  
// Escala de frecuencia al 20%
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW); 

// Inicialización de la comunicación serie
  Serial.begin(9600);
}


// loop (bucle)
void loop() {
  leerfrecuencias();

  // filtros para ahorrar energía (negro y blanco no serán colores válidos)
  bool esnegro  = (redFreq > 360) && (greenFreq > 360) && (blueFreq > 280);
  bool esblanco = (redFreq < 105) && (greenFreq < 105) && (blueFreq < 85);

  if (esnegro) {
    apagarTodosLosLEDs();
    Serial.println("COLOR: NEGRO");
    delay(300);
    return; // reiniciar calculo (no medir valores de negro, no sirven)
  }

  if (esblanco) {
    apagarTodosLosLEDs();
    Serial.println("COLOR: BLANCO");
    delay(300);
    return; // reiniciar calculo (no medir valores de blanco, no sirven)
  }

  // step 1: porcentaje del color en referencia al total para evitar leer mal los colores cuando la intensidad del color sube o baja

  float sumafreqtotal = redFreq + greenFreq + blueFreq;
  float porcentajeRed = (redFreq/sumafreqtotal) *100.0;
  float porcentajeGreen = (greenFreq/sumafreqtotal) *100.0;
  float porcentajeBlue = (blueFreq/sumafreqtotal) *100.0;

  Serial.print("Porcentajes | R: "); 
  Serial.print(porcentajeRed, 1); //porcentaje rounded a 1 decimal
  Serial.print("%");
  
  Serial.print(" | G: "); 
  Serial.print(porcentajeGreen, 1);
  Serial.print("%");
  
  Serial.print(" | B: ");
  Serial.println(porcentajeBlue, 1);

// step 2: calcular diferencia entre porcentajes (y hacerlos absolutos, para comparar positivos)
  
  float diferenciaRojoAzul = porcentajeRed - porcentajeBlue;
  if (diferenciaRojoAzul < 0) {
    diferenciaRojoAzul = -diferenciaRojoAzul;
  }

  float diferenciaVerdeRojo = porcentajeGreen - porcentajeRed;
  if (diferenciaVerdeRojo < 0) {
    diferenciaVerdeRojo = -diferenciaVerdeRojo;
  }

  // no necesitamos diferencia verdeazul porque los tonos dados tienen muy similares la diferencia absoluta, excepto rosa y naranja pero se descartan por el rojo azul
  
// step 3: más bools para las if conditions, primero evaluar si es cyan, luego rosa, amarillo y naranja
  
  bool azulesmasfuerte = (porcentajeBlue < porcentajeRed) && (porcentajeBlue < porcentajeGreen); //menor numero de frecuencia = mayor intensidad, cyan tiene el azul mas fuerte
  bool verdeesmasdebil = (porcentajeGreen > porcentajeRed) && (porcentajeGreen > porcentajeBlue); //mayor numero de frecuencia = menor intensidad, rosa tiene el verde mas pronunciado
  bool rojoesmasfuerte = (porcentajeRed < porcentajeBlue); //para separar naranja y amarillo, queremos saber que tan rojizo es, luego medimos diferencia verde rojo (tienen azul similares)

  bool rojoyazulsonparecidos  = diferenciaRojoAzul  <= 10.0; //rosa tiene valores de rojo y azul similares
  bool verdeyrojosonparecidos = diferenciaVerdeRojo <= 13.0; //naranja tiene mayor diferencia de verderojo, amarillo tiene verderojo similares

// step 4: decidir color 
  
  apagarTodosLosLEDs();

  if (azulesmasfuerte && redFreq > 140) {
    digitalWrite(ledAzul, HIGH);
    Serial.println("COLOR: CYAN");
  }
  else if (verdeesmasdebil && rojoyazulsonparecidos) {
    digitalWrite(ledVerde, HIGH);
    Serial.println("COLOR: ROSA");
  }
  else if (rojoesmasfuerte && verdeyrojosonparecidos) {
    digitalWrite(ledAmarillo, HIGH);
    Serial.println("COLOR: AMARILLO");
  }
  else if (rojoesmasfuerte && !verdeyrojosonparecidos) { // not!: si no se parecen es naranja
    digitalWrite(ledRojo, HIGH); 
    Serial.println("COLOR: NARANJA");
  }
  else {
    Serial.println("COLOR: ?"); //si ninguna se cumple, hay un error
  }
}
//fin del loop

//funciones auxiliares

//usar los filtros de colores del sensor 
void leerfrecuencias() {
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  redFreq = pulseIn(sensorOut, LOW);
  delay(100); //cambio de filtro, mas precision con delays

  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  greenFreq = pulseIn(sensorOut, LOW);
  delay(100);

  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  blueFreq = pulseIn(sensorOut, LOW);
  delay(100);
}

//apagar los 4 leds
void apagarTodosLosLEDs() {
  digitalWrite(ledAmarillo, LOW);
  digitalWrite(ledRojo, LOW);
  digitalWrite(ledVerde, LOW);
  digitalWrite(ledAzul, LOW);
}
