// Programa de demostración
// Programa para hacer declaración básica
// y enviar un mensaje a la consola

#define LEE_SCL 40 // puerto de entrada para leer el estado de la línea SCL
#define LEE_SDA 41 // puerto de entrada para leer el estado de la línea SDA
#define ESC_SCL 4  // puerto de salida para escribir el valor de la línea SCL-out
#define ESC_SDA 39 // puerto de salida para escribir el valor de la línea SDA-out

#define DEVICE B01010000

void setup()
{

    // Inicialización del canal serie para comunicarse con el usuario
    Serial.begin(9600);

    // Inicialización de los terminales de entrada
    pinMode(LEE_SDA, INPUT);
    pinMode(LEE_SCL, INPUT);
    // Inicialización de los terminales de salida
    pinMode(ESC_SDA, OUTPUT);
    pinMode(ESC_SCL, OUTPUT);
    // Asegurarse de no intervenir el bus poniendo SDA y SCL a "1"....
    digitalWrite(ESC_SDA, HIGH);
    digitalWrite(ESC_SCL, HIGH);
}

void loop()
{
    Serial.println(" ==> loop de la práctica 2! ");
    delay(1000);
}

void BIT1()
{
    // Write data
    digitalWrite(ESC_SCL, LOW);
    digitalWrite(ESC_SDA, HIGH);
    // enter read
    digitalWrite(ESC_SCL, HIGH);
    // end
    digitalWrite(ESC_SCL, LOW);
}

void BIT0()
{
    // Write data
    digitalWrite(ESC_SCL, LOW);
    digitalWrite(ESC_SDA, LOW);
    // enter read
    digitalWrite(ESC_SCL, HIGH);
    // end
    digitalWrite(ESC_SCL, LOW);
}

int READ_BIT()
{
    // Start
    digitalWrite(ESC_SDA, HIGH);
    digitalWrite(ESC_SCL, LOW);
    // Read data
    digitalWrite(ESC_SCL, HIGH);
    int r = digitalRead(LEE_SDA);

    // End
    digitalWrite(ESC_SCL, LOW);
}

void START()
{
    // Producimos la señal start, todas las demas condiciones estan aseguradas.
    digitalWrite(ESC_SDA, LOW);
    digitalWrite(ESC_SCL, LOW);
}

void STOP()
{
    // Aseguramos que la señal de datos esta bajo antes de renunciar control del clock para garantizar la generacion del señal STOP.
    digitalWrite(ESC_SDA, LOW);

    // Producimos la señal STOP
    digitalWrite(ESC_SCL, HIGH);
    digitalWrite(ESC_SDA, HIGH);
}

void byte_write()
{
START_:
    START();

    // Send device direction

    // Confim ack
    if (READ_BIT() == 1)
    {
        goto START_;
    }

    // Send Least significant byte of address

    // Confim ack
    if (READ_BIT() == 1)
    {
        goto START_;
    }

    // Send most significant byte of address

    // Confim ack
    if (READ_BIT() == 1)
    {
        goto START_;
    }

    // Send byte to write

    // Confim ack
    if (READ_BIT() == 1)
    {
        goto START_;
    }

    STOP();
}