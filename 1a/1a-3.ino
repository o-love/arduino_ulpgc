

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
}

void loop()
{

    static int contador = 0; // Contador

    // Leer entradas

    // Incremento
    {
        static bool lastWasHigh = true;
        if (digitalRead(P_UP) == LOW)
        {
            static unsigned long debounceCounter = 0;

            if (millis() - debounceCounter < 50 || !lastWasHigh)
            {
                debounceCounter = millis();
            }
            else
            {
                contador++;
                if (contador > 99)
                {
                    contador = 0;
                }

                tone(37, 3000, 500);
            }
            lastWasHigh = false;
        }
        else
        {
            lastWasHigh = true;
        }
    }

    // Decremento
    {
        static bool lastWasHigh = true;
        if (digitalRead(P_DOWN) == LOW)
        {
            static unsigned long debounceCounter = 0;

            if (millis() - debounceCounter < 50 || !lastWasHigh)
            {
                debounceCounter = millis();
            }
            else
            {
                contador--;
                if (contador < 0)
                {
                    contador = 99;
                }

                tone(37, 3000, 500);
            }
            lastWasHigh = false;
        }
        else
        {
            lastWasHigh = true;
        }
    }

    // Reset
    {
        static bool lastWasHigh = true;
        if (digitalRead(P_CENTRE) == LOW)
        {
            static unsigned long debounceCounter = 0;

            if (millis() - debounceCounter < 50 || !lastWasHigh)
            {
                debounceCounter = millis();
            }
            else
            {
                contador = 0;
                tone(37, 3000, 500);
            }
            lastWasHigh = false;
        }
        else
        {
            lastWasHigh = true;
        }
    }

    // Encender y apagar unidades y decimas
    static bool estado = false; // Estado true si unidades, false si decimas.

    PORTL |= 0x0F; // Desactivar todo
    if (estado)
    {
        PORTA = mostrarNum[contador % 10];
        PORTL ^= 0x01; // Activar unidades
    }
    else
    {
        PORTA = mostrarNum[contador / 10];
        PORTL ^= 0x02; // Activar decimos
    }
    delay(10);        // Delay de 10 ms
    estado = !estado; // Alternamos el estado con cada iteracion
}
