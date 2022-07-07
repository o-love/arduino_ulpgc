

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
void motorPos_ini(int freq)
{
    // Calculamos el valor mas pequeño de n capaz de trabajar con la frequencia pasado para aumentar la precision.
    int n;
    int prescale;
    // Los valores de frecuencia vienen establecidos por la frecuencia que cada prescalar es capaz de conseguir con PWM
    if (freq < 2)
    {
        prescale = 4; // 256
        n = 256;
    }
    else if (freq < 16)
    {
        prescale = 3; // 64
        n = 64;
    }
    else if (freq < 123)
    {
        prescale = 2; // 8
        n = 8;
    }
    else
    {
        prescale = 1; // 1
        n = 1;
    }

    cli();
    TCCR3A = 0;
    TCCR3B = 0;
    TCNT3 = 0;

    TOP_ = (16 * 1000000) / (2 * n * freq); // ICR3 = 16Mhz/(2*N*F)
    ICR3 = TOP_;

    // Initializamos PWM modo 8 con tope en ICR
    TCCR3B |= (1 << WGM33);

    TCCR3B |= prescale; // Añadimos el prescale

    sei();
}

void motorPos_gen(int pin, double degree)
{
    double factor = ((degree / 90.) + 0.5) / 20.;
    // Convertimos grado en porcentaje de valor.
    int value = (int)(((double)TOP_ + 1.) * factor); // Calculamos el valor de OCR3*. Duty cycle% = ((2 * OCR3) / N_cycles) * 100 (formula de no invertido)

    pinMode(pin, OUTPUT); // Habilitar pin de OC3(pin)

    // Para COM nos ponemos en Modo b10 para generar una señal PWM no invertido. clear up set down
    if (pin == 5)
    {
        TCCR3A |= (1 << COM3A1);
        OCR3A = value;
    }
    else if (pin == 2)
    {
        TCCR3A |= (1 << COM3B1);
        OCR3B = value;
    }
    else if (pin == 3)
    {
        TCCR3A |= (1 << COM3C1);
        OCR3C = value;
    }
}

void setup()
{
    Serial.begin(9600);

    motorPos_ini(50);     // configura timer para una frecuencia de 50 Hz
    motorPos_gen(5, 0);   // servo a 0º (ancho de pulso 0.5 ms)
    motorPos_gen(2, 90);  // servo a 90º (ancho de pulso 1.5 ms)
    motorPos_gen(3, 180); // servo a 180º (ancho de pulso 2.5 ms)
    delay(1000);
}

void loop()
{
}
