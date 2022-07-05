// Programa de demostración
// Programa para hacer declaración básica
// y enviar un mensaje a la consola

#define LEE_SCL 40 // puerto de entrada para leer el estado de la línea SCL
#define LEE_SDA 41 // puerto de entrada para leer el estado de la línea SDA
#define ESC_SCL 4  // puerto de salida para escribir el valor de la línea SCL-out
#define ESC_SDA 39 // puerto de salida para escribir el valor de la línea SDA-out

// int waitTime = 0; // Utilizdo para poder visualizar en el osciloscopio.

void start()
{
    // Serial.println("Start\n");
    digitalWrite(ESC_SCL, HIGH);
    digitalWrite(ESC_SDA, HIGH); // Soltamos los datos y el reloj, como nadie mas lo esta controlando pasa a estar altos en el bus tambien

    // delay(waitTime);

    digitalWrite(ESC_SDA, LOW); // Bajamos la linea de datos mientras que el reloj sigue en modo lectura

    // delay(waitTime);

    digitalWrite(ESC_SCL, LOW); // Bajamos el reloj para prepararnos para el siguente instruccion.
}

void stop()
{
    // Serial.println("Stop\n");
    digitalWrite(ESC_SCL, LOW);
    digitalWrite(ESC_SDA, LOW); // Bajamos la lina de datos y el reloj tomando control del BUS para preparar el señal stop

    // delay(waitTime);

    digitalWrite(ESC_SCL, HIGH); // Subimos el reloj a modo lectura para prepararnos para el señal stop

    // delay(waitTime / 2);

    digitalWrite(ESC_SDA, HIGH); // Realizamos un flanco de subida mientras en modo lectura indicando el stop

    // delay(waitTime);
}

void E_bit1()
{
    // Serial.println("Ebit1\n");
    digitalWrite(ESC_SCL, LOW); // Bajamos el reloj tomando asi control del BUS para escribir en el

    // delay(waitTime / 2);

    digitalWrite(ESC_SDA, HIGH); // Escribimos nuestro dato en el bus.

    // delay(waitTime / 2);

    digitalWrite(ESC_SCL, HIGH); // Ponemos el bus en modo de lectura

    // delay(waitTime);

    digitalWrite(ESC_SCL, LOW);
    digitalWrite(ESC_SDA, LOW); // Preparamos el bus para el siguiente instruccion.
}

void E_bit0()
{
    // Serial.println("Ebit0\n");
    digitalWrite(ESC_SCL, LOW); // Bajamos el reloj popniendo el BUS en modo escritura.

    // delay(waitTime / 2);

    digitalWrite(ESC_SDA, LOW); // Escribimos el dato en el BUS.

    // delay(waitTime / 2);

    digitalWrite(ESC_SCL, HIGH); // Metemos el BUS en modo lectura

    // delay(waitTime);

    digitalWrite(ESC_SCL, LOW); // Preparamos el bus para la siguiente insturccion.
    digitalWrite(ESC_SDA, LOW);
}

byte R_bit()
{
    // Serial.println("Rbit\n");
    digitalWrite(ESC_SCL, LOW); // Nos aseguramos de que se estemos en modo escritura

    // delay(waitTime / 2);

    digitalWrite(ESC_SDA, HIGH); // Soltamos el control de la linea de datos

    // delay(waitTime / 2);

    digitalWrite(ESC_SCL, HIGH);    // Nos ponemos en modo lectura
    int val = digitalRead(LEE_SDA); // Leemos el valor de la linea de datos

    // delay(waitTime);

    digitalWrite(ESC_SCL, LOW);
    digitalWrite(ESC_SDA, LOW); // Preparamos el bus para la siguiente instruccion
    return (byte)val;
}

void i2c_write_byte(byte toWrite)
{
    // Serial.println("i2cWriteByte\n");
    // Serial.println(toWrite);
    for (int i = 7; i >= 0; i--)
    { // Iteramos cada bit del byte
        if ((toWrite >> i) & 1)
        { // Hacemos un shift para cada iteracion que hemos hecho y utilizando el operador bitwise sacamos el bit menos significativo del entero, la cual pasamos para ser escrito en el BUS.
            E_bit1();
        }
        else
        {
            E_bit0();
        }
    }
}

byte i2c_read_byte()
{
    // Serial.println("i2cReadByte\n");
    byte toReturn = 0;
    for (int i = 0; i < 8; i++)
    {                             // Itera sobre los 8 bits del byte
        toReturn = toReturn << 1; // Hace un shift al entero donde se va a guardar el byte
        toReturn |= R_bit();      // Se añade el nuevo bit en el huco que acabamos de crear.
    }
    return toReturn;
}

void control(byte deviceAdd, byte isRead)
{ // Metodo para facilmente inicializar la comunicacion en i2c
    start();
    i2c_write_byte((deviceAdd << 1) + isRead); // CONTROL
}

void byteWriteToMem(byte memAdd, int dataAdd, byte data)
{ // Escribe un solo byte en memoria especificando la direccion.
  // Serial.println("ByteWriteToMem");
  // Serial.println(memAdd);
  // Serial.println((byte)(dataAdd>>8));
  // Serial.println((byte)dataAdd);
  // Serial.println((byte)data);

start1:
    control(memAdd, 0); // CONTROL  i2c_write_byte((byte)(dataAdd>>8));

    if (R_bit() != 0)
        goto start1;

    i2c_write_byte((byte)(dataAdd >> 8)); // Escribimos el byte mas significativo de la direccion de memoria

    if (R_bit() != 0)
        goto start1;

    i2c_write_byte((byte)dataAdd); // Escribimos el byte menos significativo de la direccion de memoria

    if (R_bit() != 0)
        goto start1;

    i2c_write_byte(data); // Escribimos el dato en el bus
    if (R_bit() != 0)
        goto start1;

    stop();
}

void PageWrite(byte memAdd, int dataAdd, byte data)
{
start2:
    control(memAdd, 0); // CONTROL  i2c_write_byte((byte)(dataAdd>>8));

    if (R_bit() != 0)
        goto start2;

    i2c_write_byte((byte)(dataAdd >> 8)); // Escribimos el byte mas significativo de la direccion de memoria

    if (R_bit() != 0)
        goto start2;

    i2c_write_byte((byte)dataAdd); // Escribimos el byte menos significativo de la direccion de memoria

    if (R_bit() != 0)
        goto start2;

    for (int i = 0; i < 32; i++)
    { // Iteramos sobre una pagina(32 bytes)
        i2c_write_byte(data);

        if (R_bit() != 0)
            goto start2;
    }

    stop();
}

void setEntireMemTo(byte memAdd, byte data)
{
    for (int i = 0; i < 8192; i++)
    {
        byteWriteToMem(memAdd, i, data);
    }
}

void setEntireMemToPageWrite(byte memAdd, byte data)
{
    int dataAdd = 0;

    for (int i = 0; i < 256; i++)
    {                                     // Nuestra memoria contiene 256 paginas de 32 bytes. En el caso de tener una memoria con un numero de paginas distintos sera necesario modificar esto.
        PageWrite(memAdd, dataAdd, data); // Escribimos el dato en el siguiente pagina de la memoria
        dataAdd += 32;                    // Con cada iteracion añadimos los 32 bytes que hemos recorrido en la ultima Page Write
    }
}

byte byteReadFromMemCurrentAddress(byte memAdd)
{ // Lee un solo byte de memoria sin especificar una direccion utlizando asi la que este guardado en el registro de la memoria.
start3:
    control(memAdd, 1); // Decimos al control que queremos leer
    if (R_bit() != 0)
        goto start3;

    byte toReturn = i2c_read_byte(); // Leemos y guardamos el byte que nos da la memoria.

    E_bit1(); // ACK de fin de lectura

    stop();
    return toReturn;
}

byte byteReadFromMem(byte memAdd, int dataAdd)
{ // Lee un solo byte de memoria con la direccion especificada

start4:
    control(memAdd, 0); // Iniciamos un proceso de escritura para setear el registro de direcciones de la memoria
    if (R_bit() != 0)
        goto start4;

    i2c_write_byte((byte)(dataAdd >> 8)); // Escribimos el byte mas significativo de la direccion del dato

    if (R_bit() != 0)
        goto start4;
    i2c_write_byte((byte)dataAdd); // Escribimos el byte menos significativo de la direccion del dato

    if (R_bit() != 0)
        goto start4;

    control(memAdd, 1); // Sin ninguna instruccion stop inicializamos la operacion de lectura
    if (R_bit() != 0)
        goto start4;

    byte toReturn = i2c_read_byte(); // Leemos y guardamos el byte que nos da la memoria

    E_bit1(); // ACK de fin de lectura

    stop();
    return toReturn;
}

void printBlockRead(byte memAdd, int dataAdd)
{
    for (int i = 0; i < 32; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            Serial.print(" 0x");
            Serial.print(byteReadFromMem(memAdd, dataAdd + i + j), HEX);
        }
        Serial.println(".");
    }
}

void printBlockPageRead(byte memAdd, int dataAdd)
{ // Lee un conjunto de datos de la memoria con la direccion de inicio especificada.
start5:
    control(memAdd, 0); // Inicializamos una operacion de escritura para poder setear el registro de direcciones de la memoria
    if (R_bit() != 0)
        goto start5;

    i2c_write_byte((byte)(dataAdd >> 8)); // Escribimos el byte mas significativo de la direccion del dato

    if (R_bit() != 0)
        goto start5;

    i2c_write_byte((byte)dataAdd); // Escribimos el byte menos significativo de la direccion del dato

    if (R_bit() != 0)
        goto start5;

    control(memAdd, 1); // Sin ninguna instruccion stop inicializamos la operacion de lectura
    if (R_bit() != 0)
        goto start5;

    for (int j = 0; j < 32; j++)
    {
        for (int i = 0; i < 8; i++)
        {
            Serial.print("0x");
            Serial.print(i2c_read_byte(), HEX); // Leemos y imprimimos el dato que nos da la memoria
            Serial.print(" ");
            if (i + j < 255)
            {             // Si estamos en el ultimo dato mandamos un ACK de fin de lectura. Sino mandamos una de continuacion de lectura.
                E_bit0(); // ACK de continuacion de lectura
            }
            else
            {
                E_bit1(); // ACK de fin de lectura
            }
        }
        Serial.println(":");
    }
    stop();
}

struct date_time
{
    int second;
    int minute;
    int hour;
    int day;
    int date;
    int month;
    int year;
    int alarm1Hour;
    int alarm1Minute;
    int alarm2Hour;
    int alarm2Minute;
};

#define DS323_ADDRESS B1101000
struct date_time retriveDateTime()
{
    // TODO: setup time to get hour in 24 hour format
    struct date_time date;
start6:
    control(DS323_ADDRESS, 0); // Inicializamos una operacion de escritura para poder setear el registro de direcciones de la memoria
    if (R_bit() != 0)
        goto start6;

    i2c_write_byte(0); // Escribimos el byte menos significativo de la direccion del dato

    if (R_bit() != 0)
        goto start6;

    control(DS323_ADDRESS, 1); // Sin ninguna instruccion stop inicializamos la operacion de lectura
    if (R_bit() != 0)
        goto start6;

    // Empezamos lectura
    int temp;

    // second
    temp = i2c_read_byte();
    date.second = temp & 0xF;
    date.second += ((temp & 0x70) >> 4) * 10;
    E_bit0();

    // minute
    temp = i2c_read_byte();
    date.minute = temp & 0xF;
    date.minute += ((temp & 0x70) >> 4) * 10;
    E_bit0();

    // hour
    temp = i2c_read_byte();
    date.hour = temp & 0xF;
    date.hour += ((temp & 0x30) >> 4) * 10;
    E_bit0();

    // day
    date.day = i2c_read_byte() & 0x7;
    E_bit0();

    // date
    temp = i2c_read_byte();
    date.date = temp & 0xF;
    date.date += ((temp & 0x30) >> 4) * 10;
    E_bit0();

    // month
    temp = i2c_read_byte();
    date.month = temp & 0xF;
    date.month += ((temp & 0x10) >> 4) * 10;
    E_bit0();

    // year
    temp = i2c_read_byte();
    date.year = temp & 0xF;
    date.year += ((temp & 0xF0) >> 4) * 10;
    E_bit0();

    // Alarm1 second
    i2c_read_byte();
    E_bit0();

    // Alarm1 minute
    temp = i2c_read_byte();
    date.alarm1Minute = temp & 0xF;
    date.alarm1Minute += ((temp & 0x70) >> 4) * 10;
    E_bit0();

    // Alarm1 hour
    temp = i2c_read_byte();
    date.alarm1Hour = temp & 0xF;
    date.alarm1Hour += ((temp & 0x30) >> 4) * 10;
    E_bit0();

    // Alarm 1 day
    i2c_read_byte();
    E_bit0();

    // Alarm2 second
    i2c_read_byte();
    E_bit0();

    // Alarm2 minute
    temp = i2c_read_byte();
    date.alarm2Minute = temp & 0xF;
    date.alarm2Minute += ((temp & 0x70) >> 4) * 10;
    E_bit0();

    // Alarm1 hour
    temp = i2c_read_byte();
    date.alarm2Hour = temp & 0xF;
    date.alarm2Hour += ((temp & 0x30) >> 4) * 10;
    E_bit1();

    // end
    stop();
    return date;
}

float retrieveTemp()
{
start:
    control(DS323_ADDRESS, 0); // Inicializamos una operacion de escritura para poder setear el registro de direcciones de la memoria
    if (R_bit() != 0)
        goto start;

    i2c_write_byte(0x11); // Escribimos el byte menos significativo de la direccion del dato

    if (R_bit() != 0)
        goto start;

    control(DS323_ADDRESS, 1); // Sin ninguna instruccion stop inicializamos la operacion de lectura
    if (R_bit() != 0)
        goto start;

    float toReturn = i2c_read_byte();
    E_bit0();

    toReturn += 0.25 * ((float)(i2c_read_byte() >> 6));
    E_bit1();

    stop();
    return toReturn;
}

void setup()
{

    // Inicialización del canal serie para comunicarse con el usuario
    Serial.begin(9600);
    Serial3.begin(9600);

    // Inicialización de los terminales de entrada
    pinMode(LEE_SDA, INPUT);
    pinMode(LEE_SCL, INPUT);
    // Inicialización de los terminales de salida
    pinMode(ESC_SDA, OUTPUT);
    pinMode(ESC_SCL, OUTPUT);
    // Asegurarse de no intervenir el bus poniendo SDA y SCL a "1"....
    digitalWrite(ESC_SDA, HIGH);
    digitalWrite(ESC_SCL, HIGH);

    // Initializar Timer 3 para interrumpir cada segundo.

    cli();
    // Queremos initializar el Timer 3 a modo CTC de tal forma que genere una interrupcion cada segundo.
    TCCR3A = 0;
    TCCR3B = 0;
    TCNT3 = 0;

    OCR3A = 31249; // 16MHz/(2*256*1/(1s)) - 1
    TCCR3A = B00000000;
    TCCR3B = B00001100;

    // Habilitamos interrupciones para el timer con OCIE3A
    TIMSK3 = B00000010;

    // inicializar LCD
    Serial3.write(0xFE);
    Serial3.write(0);
    delay(100);

    sei();
}

void loop()
{
}

const char *months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

ISR(TIMER3_COMPA_vect) // Update LCD
{
    float currentTemp = retrieveTemp();
    struct date_time Date = retriveDateTime();

    char buffer[40];

    // Print hour
    Serial3.write(0xFE);
    Serial3.write(128 + 5);

    sprintf(buffer, "%02d:%02d:%02d", Date.hour, Date.minute, Date.second);
    Serial3.write(buffer);

    // Print tempeture static
    Serial3.write(0xFE);
    Serial3.write(128 + 15);

    Serial3.write("T=");

    // Print tempeture
    Serial3.write(0xFE);
    Serial3.write(128 + 17);

    sprintf(buffer, "%02d", currentTemp);
    Serial3.write(buffer);

    // Print tempeture static
    Serial3.write(0xFE);
    Serial3.write(128 + 19);

    Serial3.write("C");

    // Print Alarm Static
    Serial3.write(0xFE);
    Serial3.write(192 + 0);

    Serial3.write("ALARM");

    // Print Alarm1 time
    Serial3.write(0xFE);
    Serial3.write(148 + 0);

    sprintf(buffer, "%02d:%02d", Date.alarm1Hour, Date.alarm1Minute);
    Serial3.write(buffer);

    // Print alarm active or not
    Serial3.write(0xFE);
    Serial3.write(148 + 5);
    bool Alarm1active;

    if (Alarm1active)
    {
        Serial3.write("*");
    }
    else
    {
        Serial3.write(" ");
    }

    // Print date static
    Serial3.write(0xFE);
    Serial3.write(148 + 13);

    Serial.write("DDMMMYY");

    // Print Alarm2 time
    Serial3.write(0xFE);
    Serial3.write(212 + 0);

    sprintf(buffer, "%02d:%02d", Date.alarm2Hour, Date.alarm2Minute);
    Serial3.write(buffer);

    // Print alarm active or not
    Serial3.write(0xFE);
    Serial3.write(212 + 5);
    bool Alarm2active;

    if (Alarm2active)
    {
        Serial3.write("*");
    }
    else
    {
        Serial3.write(" ");
    }

    // Print Date
    Serial3.write(0xFE);
    Serial3.write(212 + 13);

    sprintf(buffer, "%02d%s%02d", Date.date, months[Date.month], Date.year);
    Serial3.write(buffer);
}