// Programa de demostración
// Programa para hacer declaración básica
// y enviar un mensaje a la consola

#define LEE_SCL 40 // puerto de entrada para leer el estado de la línea SCL
#define LEE_SDA 41 // puerto de entrada para leer el estado de la línea SDA
#define ESC_SCL 4  // puerto de salida para escribir el valor de la línea SCL-out
#define ESC_SDA 39 // puerto de salida para escribir el valor de la línea SDA-out

#define DEVICE B1010000

#define IS_READ 1
#define IS_WRITE 0

void BIT1()
{
    // Write data
    digitalWrite(ESC_SCL, LOW);
    digitalWrite(ESC_SDA, HIGH);
    // enter read
    digitalWrite(ESC_SCL, HIGH);
    // end
    digitalWrite(ESC_SCL, LOW);
    digitalWrite(ESC_SDA, LOW);
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
    digitalWrite(ESC_SDA, LOW);
}

int READ_BIT()
{
    // Start
    digitalWrite(ESC_SCL, LOW); // Entramos en modo escritura
    digitalWrite(ESC_SDA, HIGH);
    // Read data
    digitalWrite(ESC_SCL, HIGH);
    int r = digitalRead(LEE_SDA);

    // End
    digitalWrite(ESC_SCL, LOW);
    return (byte)r;
}

void START()
{
    // Aseguramos las condiciones necesarias para realizar el start.
    digitalWrite(ESC_SCL, HIGH);
    digitalWrite(ESC_SDA, HIGH);

    // Producimos la señal start, todas las demas condiciones estan aseguradas.
    digitalWrite(ESC_SDA, LOW);
    digitalWrite(ESC_SCL, LOW);
}

void STOP()
{
    // Aseguramos que la señal de datos esta bajo antes de renunciar control del clock para garantizar la generacion del señal STOP.
    digitalWrite(ESC_SCL, LOW);
    digitalWrite(ESC_SDA, LOW);

    // Producimos la señal STOP
    digitalWrite(ESC_SCL, HIGH);
    digitalWrite(ESC_SDA, HIGH);
}

void byte_write(byte toWrite)
{
    for (int i = 7; i >= 0; i--) // Iteramos cada bit del byte
    {
        if ((toWrite >> i) & 1) // Hacemos un shift para cada iteracion que hemos hecho y utilizando el operador bitwise sacamos el bit menos significativo del entero, la cual pasamos para ser escrito en el BUS.
        {
            BIT1();
        }
        else
        {
            BIT0();
        }
    }
}

byte byte_read()
{
    byte toReturn = 0;
    for (int i = 0; i < 8; i++)
    {                             // Itera sobre los 8 bits del byte
        toReturn = toReturn << 1; // Hace un shift al entero donde se va a guardar el byte
        toReturn |= READ_BIT();   // Se añade el nuevo bit en el huco que acabamos de crear.
    }
    return toReturn;
}

void byte_store(int address, byte word)
{
START_:
    START();

    // Send device direction

    byte_write((DEVICE << 1) + IS_WRITE);

    // Confim ack
    if (READ_BIT() != 0)
    {
        goto START_;
    }

    // Send Most significant byte of address

    byte_write(address >> 8);

    // Confim ack
    if (READ_BIT() != 0)
    {
        goto START_;
    }

    // Send Least significant byte of address

    byte_write(address);

    // Confim ack
    if (READ_BIT() != 0)
    {
        goto START_;
    }

    // Send byte to write

    byte_write(word);

    // Confim ack
    if (READ_BIT() == 1)
    {
        goto START_;
    }

    STOP();
}

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

    byte_store(8, 78);
}

void loop()
{
    Serial.println(" ==> loop de la práctica 2! ");
    delay(1000);
}