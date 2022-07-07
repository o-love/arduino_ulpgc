

#define D4 49 // El pin 49 controla el cátodo común del dígito de las unidades (D4)
#define D3 48 // El pin 48 controla el cátodo común del dígito de las decenas (D3)
#define D2 47 // El pin 47 controla el cátodo común del dígito de las centenas (D2)
#define D1 46 // El pin 46 controla el cátodo común del dígito de las unidades de millar (D1)

// Definicion de los pines de pulsadores
#define P_UP 34
#define P_DOWN 31
#define P_CENTRE 33
#define P_RIGHT 30
#define P_LEFT 32

int mostrarNum[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67, /*A*/ 0x77, /*B*/ 0x7C, 0x39, 0x5E, 0x79, 0x71};

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
    pinMode(D3, OUTPUT);
    pinMode(D2, OUTPUT);
    pinMode(D1, OUTPUT);

    // Desactivar todos los cátodos comunes (poner a "1" o HIGH). Se activan cuando se ponen a "0" (LOW)
    PORTL |= 0x0F; // equivalente a PORTL = PORTL | B00001111  (cátodos D1D2D3D4 desactivados)
                   // se lee el puerto y se hace el "or" con la máscara B00001111 para poner los 4 bits
                   // menos significativos a "1". Los 4 bits superiores quedan como estaban: no se modifican
                   // Otra forma: pinMode(D4, OUTPUT); pinMode() para programar pin a pin (en este caso D4)

    // Initializamos los pines de los pulsadores
    DDRC = 0x01;
    PORTC = 0xF8;

    // Initializar timer

    cli();
    // Queremos initializar el Timer 3 a modo CTC de tal forma que genere una interrupcion cada 10ms.
    TCCR3A = 0;
    TCCR3B = 0;
    TCNT3 = 0;

    OCR3A = 9999; // 16MHz/(2*8*1/(10ms)) - 1
    TCCR3A = B00000000;
    TCCR3B = B00001010;

    // Habilitamos interrupciones para el timer con OCIE3A
    TIMSK3 = B00000010;

    sei();

    // Imprimimos menu
    Serial.println("1.- Mostrar el contador en los dos dígitos inferiores del display (decenas-unidades)");
    Serial.println("2.- Mostrar el contador en los dos dígitos superiores del display (millares-centenas)");
    Serial.println("Entrada: ");
}

bool isDisplayUpperNotLower = false;
int contador = 0;
int countBy = 1;
void loop()
{
    // Lectura y control de entrada
    if (Serial.available())
    {
        switch (Serial.read())
        {
        case '1':
            Serial.println("1");
            isDisplayUpperNotLower = false;
            break;

        case '2':
            Serial.println("2");
            isDisplayUpperNotLower = true;
            break;

        default:
            Serial.println("Entrada invalido, solo se permite '1' y '2'.");
            break;
        }
    }

    // Control de pulsadores

    // Incremento, up
    {
        static bool lastWasHigh = true; // Guardar si la ultima iteracion la señal era activa para asi detectar los cambios de signo
        if (digitalRead(P_UP) == LOW)
        {
            static unsigned long debounceCounter = 0; // Guardar el tiempo desde que la señal estaba bajo para evitar debounce

            if (millis() - debounceCounter < 50 || !lastWasHigh) // Si no se produce cambio de signo o el cambio de signo se produce durante el tiempo debounce actualizamos el tiempo debounce
            {
                debounceCounter = millis();
            }
            else
            {
                contador += countBy; // Aumento de contador por 1 o 2 segun se proceda
                if (contador > 99)   // Detecar si nos sobrepasamos del limite superior
                {
                    contador = contador - 100; // Al pasar el limite superior volvemos al limite inferior con la cantidad de digitos con la que sobrepasamos (100 pasa a 0, 101 pasa a 1, etc)
                }

                tone(37, 3000, 500); // Ruido indicando pulsacion de boton
            }
            lastWasHigh = false; // Guardar que en esta iteracion la tension era baja
        }
        else
        {
            lastWasHigh = true; // Guardar que en esta iteracion la tension era alta
        }
    }

    // Decremento, down
    {
        static bool lastWasHigh = true; // Guardar si la ultima iteracion la señal era activa para asi detectar los cambios de signo
        if (digitalRead(P_DOWN) == LOW)
        {
            static unsigned long debounceCounter = 0; // Guardar el tiempo desde que la señal estaba bajo para evitar debounce

            if (millis() - debounceCounter < 50 || !lastWasHigh) // Si no se produce cambio de signo o el cambio de signo se produce durante el tiempo debounce actualizamos el tiempo debounce
            {
                debounceCounter = millis();
            }
            else
            {
                contador -= countBy; // Dieminuimos el contador por 1 o 2 segun proceda
                if (contador < 0)    // Detecatamos cuando nos pasamos por debajo del limite inferior
                {
                    contador = 100 + contador; // Restamos al limite superior por el numero de valores que sobrepasamos el limite inferior (-1 -> 99, -2 -> 98, etc)
                }

                tone(37, 3000, 500); // Ruido indicando pulsacion de boton
            }
            lastWasHigh = false; // Guardar que en esta iteracion la tension era baja
        }
        else
        {
            lastWasHigh = true; // Guardar que en esta iteracion la tension era alta
        }
    }

    // Reset
    {
        static bool lastWasHigh = true; // Guardar si la ultima iteracion la señal era activa para asi detectar los cambios de signo
        if (digitalRead(P_CENTRE) == LOW)
        {
            static unsigned long debounceCounter = 0; // Guardar el tiempo desde que la señal estaba bajo para evitar debounce

            if (millis() - debounceCounter < 50 || !lastWasHigh) // Si no se produce cambio de signo o el cambio de signo se produce durante el tiempo debounce actualizamos el tiempo debounce
            {
                debounceCounter = millis();
            }
            else
            {
                contador = 0;        // Ponemos el contador a cero
                tone(37, 3000, 500); // Ruido indicando pulsacion de boton
            }
            lastWasHigh = false; // Guardar que en esta iteracion la tension era baja
        }
        else
        {
            lastWasHigh = true; // Guardar que en esta iteracion la tension era alta
        }
    }

    // Right, count by 2
    {
        static bool lastWasHigh = true; // Guardar si la ultima iteracion la señal era activa para asi detectar los cambios de signo
        if (digitalRead(P_RIGHT) == LOW)
        {
            static unsigned long debounceCounter = 0; // Guardar el tiempo desde que la señal estaba bajo para evitar debounce

            if (millis() - debounceCounter < 50 || !lastWasHigh) // Si no se produce cambio de signo o el cambio de signo se produce durante el tiempo debounce actualizamos el tiempo debounce
            {
                debounceCounter = millis();
            }
            else
            {
                countBy = 2;         // Fijamos el paso con la que se cuenta a 2
                tone(37, 3000, 500); // Ruido indicando pulsacion de boton
            }
            lastWasHigh = false; // Guardar que en esta iteracion la tension era baja
        }
        else
        {
            lastWasHigh = true; // Guardar que en esta iteracion la tension era alta
        }
    }

    // Left, count by 1
    {
        static bool lastWasHigh = true; // Guardar si la ultima iteracion la señal era activa para asi detectar los cambios de signo
        if (digitalRead(P_LEFT) == LOW)
        {
            static unsigned long debounceCounter = 0; // Guardar el tiempo desde que la señal estaba bajo para evitar debounce

            if (millis() - debounceCounter < 50 || !lastWasHigh) // Si no se produce cambio de signo o el cambio de signo se produce durante el tiempo debounce actualizamos el tiempo debounce
            {
                debounceCounter = millis();
            }
            else
            {
                countBy = 1;         // Fijamos el paso con la que se cuenta a 1
                tone(37, 3000, 500); // Ruido indicando pulsacion de boton
            }
            lastWasHigh = false; // Guardar que en esta iteracion la tension era baja
        }
        else
        {
            lastWasHigh = true; // Guardar que en esta iteracion la tension era alta
        }
    }
}

bool estado = false; // Estado true si unidades, false si decimas.
ISR(TIMER3_COMPA_vect)
{
    PORTL |= 0x0F; // Desactivar todo

    if (estado)
    {
        PORTA = mostrarNum[contador % 10]; // Mostramos digito menos significativo
        if (isDisplayUpperNotLower)
            PORTL ^= 0x04; // Activar centecimas
        else
            PORTL ^= 0x01; // Activar unidades
    }
    else
    {
        PORTA = mostrarNum[contador / 10]; // Mostramos digito mas significativo
        if (isDisplayUpperNotLower)
            PORTL ^= 0x08; // Activar milesimos
        else
            PORTL ^= 0x02; // Activar decimos
    }

    estado = !estado; // Alternamos el estado con cada iteracion
}