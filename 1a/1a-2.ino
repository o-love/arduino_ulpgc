// Programa de demostración
// Programa para hacer parpadear todos los segmentos del dígito de las unidades del display
// y enviar un mensaje a la consola

#define D4 49 // El pin 49 controla el cátodo común del dígito de las unidades (D4)
#define D3 48 // El pin 48 controla el cátodo común del dígito de las decenas (D3)
#define D2 47 // El pin 47 controla el cátodo común del dígito de las centenas (D2)
#define D1 46 // El pin 46 controla el cátodo común del dígito de las unidades de millar (D1)

int mostrarNum[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67, /*A*/ 0x77, /*B*/ 0x7C, 0x39, 0x5E, 0x79, 0x71};

int hexAint(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return -1;
}

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

    Serial.println("1: Parpadeo de las unidades");
    Serial.println("2: Parpadeo de las decenas");
    Serial.println("3: Parpadeo de las centenas");
    Serial.println("4: Parpadeo de las unidades del millar");
    Serial.println("5: Parpadeo secuencial de todos los digitos");
    Serial.println("6: Selecion del caracter hexadecimal(0-F) a visualizar");
    Serial.println("Seleciona: ");
}

void loop()
{
    // Variables para controlar el estado.
    static bool isFlashNotContinue = false; // Indica si parpadeamos un numero o vamos iterando por cada posicion.
    static int toActivate = mostrarNum[0];

    if (Serial.available()) // Si hay input leemos.
    {
        static bool toDisplay = false; // Indicar si estamos esperando por el caracter hexadecimal de la opcion de menu 6

        if (toDisplay) // Si estamos esperando por la segunda entrada del menu 6.
        {
            char read = Serial.read();
            Serial.println(read);
            PORTA = mostrarNum[hexAint(read)];
            toDisplay = false;
        }
        else
        {
            char read = Serial.read();
            Serial.println(read);
            switch (read)
            {
            case '1':
                isFlashNotContinue = true;
                toActivate = 0x01;
                break;
            case '2':
                isFlashNotContinue = true;
                toActivate = 0x02;
                break;
            case '3':
                isFlashNotContinue = true;
                toActivate = 0x04;
                break;
            case '4':
                isFlashNotContinue = true;
                toActivate = 0x08;
                break;
            case '5':
                isFlashNotContinue = false;
                toActivate = 0x01;
                break;
            case '6':
                toDisplay = true;
                Serial.print("Introduce num hexadecimal: ");
                break;
            }
        }
    }

    if (isFlashNotContinue)
    {
        PORTL |= 0x0F;
        PORTL ^= toActivate;
        delay(1000);
        PORTL |= 0x0F;
        delay(1000);
    }
    else
    {
        PORTL |= 0x0F;
        PORTL ^= toActivate;
        delay(1000);
        PORTL |= 0x0F;
        delay(1000);
        toActivate = toActivate << 1;
        if (toActivate > 0x08)
            toActivate = 0x01;
    }
}
