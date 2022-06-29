

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

int var_npulsos;
int var_rps;

void setup()
{
    Serial.begin(9600);

    var_npulsos = 0;
    var_rps = 0;

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

    pinMode(3, INPUT);

    // External inturrupt and timer
    cli();

    // Activar interrupcion interna del timer y interrupcioin externa por el pin 3(INT5) en el flanco de bajada.
    EICRB |= (1 << ISC51);
    EIMSK |= (1 << INT5);

    // Activar timer y sus interrupciones

    // Queremos initializar el Timer 3 a modo CTC de tal forma que genere una interrupcion cada segundo.
    TCCR3A = 0;
    TCCR3B = 0;
    TCNT3 = 0;

    // N = 256
    // T = 1s

    OCR3A = 31249; // 16MHz/(2*256*1/(1s)) - 1
    pinMode(5, OUTPUT); // Para poder visualizar y comporbar con el osciloscopio se activa la linea del pin 5
    TCCR3A = B01000000;
    TCCR3B = B00001100;

    // Habilitamos interrupciones para el timer con OCIE3A
    TIMSK3 = B00000010;

    sei();
}

void loop()
{
    // Encender y apagar unidades y decimas
    static bool estado = false; // Estado true si unidades, false si decimas.

    PORTL |= 0x0F; // Desactivar todo
    if (estado)
    {
        PORTA = mostrarNum[var_rps % 10];
        PORTL ^= 0x01; // Activar unidades
    }
    else
    {
        PORTA = mostrarNum[var_rps / 10];
        PORTL ^= 0x02; // Activar decimos
    }
    delay(10);        // Delay de 10 ms
    estado = !estado; // Alternamos el estado con cada iteracion
}

ISR(INT5_vect)
{
    var_npulsos++;
}

ISR(TIMER3_COMPA_vect)
{
    var_rps = var_npulsos;
    var_npulsos = 0;
}
