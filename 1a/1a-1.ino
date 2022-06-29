// Programa de demostración
// Programa para hacer parpadear todos los segmentos del dígito de las unidades del display
// y enviar un mensaje a la consola

#define D4 49 // El pin 49 controla el cátodo común del dígito de las unidades (D4)
#define D3 48 // El pin 48 controla el cátodo común del dígito de las decenas (D3)
#define D2 47 // El pin 47 controla el cátodo común del dígito de las centenas (D2)
#define D1 46 // El pin 46 controla el cátodo común del dígito de las unidades de millar (D1)

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(9600); // inicialización del canal serie para comunicarnos con:
                        // PC --> "Herramientas/monitor serie" en la plataforma Arduino
                        // Proteus --> consola en Proteus (VIRTUAL TERMINAL)

    // Programar PORTA de salida. Conexión a los segmentos a-b-c-d-e-f-g-dp del display 7-segmentos)
    DDRA = 0xFF; // También se puede expresar en binario con B11111111 (DDRA = B1111111;)

    // Escritura en el PORTA --> Todos los segmentos a "1": encendidos
    PORTA = 0xFF; // o también B11111111

    // programación de las señales que controlan los cátodos comunes como de salida (OUTPUT o =1)
    pinMode(D4, OUTPUT);
    pinMode(D3, 1);
    pinMode(D2, OUTPUT);
    pinMode(D1, OUTPUT);

    // Desactivar todos los cátodos comunes (poner a "1" o HIGH). Se activan cuando se ponen a "0" (LOW)
    PORTL |= 0x0F; // equivalente a PORTL = PORTL | B00001111  (cátodos D1D2D3D4 desactivados)
                   // se lee el puerto y se hace el "or" con la máscara B00001111 para poner los 4 bits
                   // menos significativos a "1". Los 4 bits superiores quedan como estaban: no se modifican
                   // Otra forma: pinMode(D4, OUTPUT); pinMode() para programar pin a pin (en este caso D4)
}

void loop()
{

    // Acciones que se repiten indefinidamente

    // envío de un mensaje a Virtual Terminal de Proteus o Monitor serie (PC)
    // Si al simular no aparece la consola automáticamente entonces se ha de seleccionar de forma manual (al menos
    // la primera vez que se simule). Para ello, seleccione la pestaña Debug y haga click en "Virtual Terminal" cuando
    // se esté en simulación (running).

    Serial.println("Hola mundo!");

    // encender y apagar el dígito de las unidades
    digitalWrite(D4, LOW);  // activamos unidades
    delay(2000);            // pausa de medio segundo (500 ms)
    digitalWrite(D4, HIGH); // desactivamos unidades
    delay(2000);            // pausa de medio segundo (500 ms)

    // encender y apagar el dígito de las decimas
    digitalWrite(D3, LOW);  // activamos decimas
    delay(2000);            // pausa de medio segundo (500 ms)
    digitalWrite(D3, HIGH); // desactivamos decimas
    delay(2000);            // pausa de medio segundo (500 ms)

    // encender y apagar el dígito de las centecimas
    digitalWrite(D2, LOW);  // activamos centecimas
    delay(2000);            // pausa de medio segundo (500 ms)
    digitalWrite(D2, HIGH); // desactivamos centecimas
    delay(2000);            // pausa de medio segundo (500 ms)

    // encender y apagar el dígito de las milesimas
    digitalWrite(D1, LOW);  // activamos milesimas
    delay(2000);            // pausa de medio segundo (500 ms)
    digitalWrite(D1, HIGH); // desactivamos milesimas
    delay(2000);            // pausa de medio segundo (500 ms)
}
