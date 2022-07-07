

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

int TOP_;
void fastPwm3_ini(int freq)
{
    // Calculamos el valor mas pequeño de n capaz de trabajar con la frequencia pasado para aumentar la precision.
    int n;
    int prescale;
    // Los valores de frecuencia vienen establecidos por la frecuencia que cada prescalar es capaz de conseguir con fast PWM
    if (freq < 4)
    {
        prescale = 4; // prescalar = 256
        n = 256;
    }
    else if (freq < 31)
    {
        prescale = 3; // prescalar = 64
        n = 64;
    }
    else if (freq < 245)
    {
        prescale = 2; // prescalar = 8
        n = 8;
    }
    else
    {
        prescale = 1; // prescalar = 1
        n = 1;
    }

    cli();
    // Queremos initializar el Timer 3 a modo CTC de tal forma que genere una interrupcion cada 10ms.
    TCCR3A = 0;
    TCCR3B = 0;
    TCNT3 = 0;

    // Como el tope viene definido en modo 14 por ICR lo calculamos y asignamos.
    TOP_ = (16 * 1000000) / (n * freq) - 1; // ICR3 = 16Mhz/(N/F)-1
    ICR3 = TOP_;
    Serial.println(ICR3);

    // Initializamos Fast PWM con tope en ICR (B1110)
    TCCR3A |= (1 << WGM31);
    TCCR3B |= (1 << WGM33) | (1 << WGM32);

    TCCR3B |= prescale; // Añadimos el prescale

    sei();
}

void fastPwm3_gen(int pin, int percent)
{
    // calculamos el valor del registro OCR para el porcentaje pasado
    int value = (int)(((double)TOP_ + 1.) * ((double)percent / 100.));
    Serial.println(value);

    pinMode(pin, OUTPUT); // Habilitar pin de OC3(pin)

    // Como cada uno de los pines tiene su propio registro OCR detectamos cual es y modificamos ese registro
    if (pin == 5)
    {
        TCCR3A |= (1 << COM3A1); // B10: Clear on compare match, set on BOTTOM
        OCR3A = value;           // Metemos el valor OCR para consegurir el duty cycle necesario
    }
    else if (pin == 2)
    {
        TCCR3A |= (1 << COM3B1); // B10: Clear on compare match, set on BOTTOM
        OCR3B = value;           // Metemos el valor OCR para consegurir el duty cycle necesario
    }
    else if (pin == 3)
    {
        TCCR3A |= (1 << COM3C1); // B10: Clear on compare match, set on BOTTOM
        OCR3C = value;           // Metemos el valor OCR para consegurir el duty cycle necesario
    }
}

void setup()
{
    Serial.begin(9600);
}

void loop()
{
    fastPwm3_ini(2400);
    fastPwm3_gen(5, 25); // OC3A, 25% de TOP
    fastPwm3_gen(2, 50); // OC3B, 50% “”
    fastPwm3_gen(3, 90); // OC3C, 90% “”
    delay(1000);

    fastPwm3_ini(24000);
    fastPwm3_gen(5, 30); // OC3A, 25% de TOP
    fastPwm3_gen(2, 60); // OC3B, 50% “”
    fastPwm3_gen(3, 80); // OC3C, 90% “”
    delay(1000);
}
