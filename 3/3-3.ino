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

void inicializeD3232()
{
    // Set alarm to go off every hour minute match 0x1000

    // Set to 24hour mode
}

#define SECONDS_DS3232 0x00
#define MINUTES_DS3232 0x01
#define HOURS_DS3232 0x02
#define DATE_DS3232 0x04
#define MONTH_DS3232 0x05
#define YEAR_DS3232 0x06
#define MINUTES_ALARM_1 0x08
#define HOURS_ALARM_1 0x09
#define MINUTES_ALARM_2 0x0B
#define HOURS_ALARM_2 0x0C

#define SECONDS_MAX 60
#define MINUTES_MAX 60
#define HOURS_MAX 24
#define DATE_MAX 31
#define MONTH_MAX 12
#define YEAR_MAX 99

void write_DS3232(byte toWrite, byte memoryPos, byte maxValue)
{
start:
    control(DS323_ADDRESS, 0); // Inicializamos una operacion de escritura para poder setear el registro de direcciones de la memoria
    if (R_bit() != 0)
        goto start;

    i2c_write_byte(memoryPos); // Escribimos el byte de la direccion del dato

    if (R_bit() != 0)
        goto start;

    if (toWrite > maxValue) // Si el valor a escribir es mayor que el valor que acepta el DS3232 no le mandamos el dato.
    {
        Serial.print(toWrite);
        Serial.print("; Input invalid, over ");
        Serial.println(maxValue);
        return;
    }

    toWrite = ((toWrite / 10) << 4) + (toWrite % 10);
    i2c_write_byte(toWrite);

    if (R_bit() != 0)
        goto start;

    stop();
}

struct alarm_status
{
    bool alarm1Active;
    bool alarm2Active;
};

struct alarm_status retrieveAlarmStatus()
{
start:
    control(DS323_ADDRESS, 0); // Inicializamos una operacion de escritura para poder setear el registro de direcciones de la memoria
    if (R_bit() != 0)
        goto start;

    i2c_write_byte(0x0E); // Escribimos el byte de la direccion del dato

    if (R_bit() != 0)
        goto start;

    control(DS323_ADDRESS, 1); // Sin ninguna instruccion stop inicializamos la operacion de lectura
    if (R_bit() != 0)
        goto start;

    byte data = i2c_read_byte();

    E_bit1();

    stop();

    struct alarm_status toReturn;
    toReturn.alarm1Active = (data & 0x01) != 0;
    toReturn.alarm2Active = (data & 0x02) != 0;

    return toReturn;
}

void setAlarmStatus(struct alarm_status alarmSatus)
{

    // Activate inturrupts in the control register

start2:
    control(DS323_ADDRESS, 0); // Inicializamos una operacion de escritura para poder setear el registro de direcciones de la memoria
    if (R_bit() != 0)
        goto start2;

    i2c_write_byte(0x0E); // Escribimos el byte de la direccion del dato

    if (R_bit() != 0)
        goto start2;

    // We first retrive the registeer in order to modify the desired bits
    control(DS323_ADDRESS, 1); // Sin ninguna instruccion stop inicializamos la operacion de lectura
    if (R_bit() != 0)
        goto start2;

    byte data = i2c_read_byte();

    E_bit1();

    stop();

    // Modify the registers

    data = data & 0xFC;
    data |= 0x4;

    if (alarmSatus.alarm1Active)
    {
        data |= 0x1;
    }
    if (alarmSatus.alarm2Active)
    {
        data |= 0x2;
    }
    // Store the registers
start3:
    control(DS323_ADDRESS, 0); // Inicializamos una operacion de escritura para poder setear el registro de direcciones de la memoria
    if (R_bit() != 0)
        goto start3;

    i2c_write_byte(0x0E); // Escribimos el byte de la direccion del dato

    if (R_bit() != 0)
        goto start3;

    i2c_write_byte(data);

    if (R_bit() != 0)
        goto start3;

    stop();
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

void PrintMainMenu()
{
    Serial.println("");
    Serial.println("Elejir entre las siguientes opciones. Introduce el numero asociado y pulsa enter");
    Serial.println("1: Configurar hora");
    Serial.println("2: Configurar fecha");
    Serial.println("3: Configurar alarma");
    Serial.print("Introduce dato: ");
}

void AlarmSetup()
// Set alarm mask
{
    struct
    {
        byte alarm_1_seconds;
        byte alarm_1_minutes;
        byte alarm_1_hours;
        byte alarm_1_day;
        byte alarm_2_minutes;
        byte alarm_2_hours;
        byte alarm_2_day;
    } alarmRegisters;

    // Retrive alarm registers
start:
    control(DS323_ADDRESS, 0); // Inicializamos una operacion de escritura para poder setear el registro de direcciones de la memoria
    if (R_bit() != 0)
        goto start;

    i2c_write_byte(0x07); // Escribimos el byte de la direccion del dato

    if (R_bit() != 0)
        goto start;

    control(DS323_ADDRESS, 1); // Sin ninguna instruccion stop inicializamos la operacion de lectura
    if (R_bit() != 0)
        goto start;

    alarmRegisters.alarm_1_seconds = i2c_read_byte();
    E_bit0();

    alarmRegisters.alarm_1_minutes = i2c_read_byte();
    E_bit0();

    alarmRegisters.alarm_1_hours = i2c_read_byte();
    E_bit0();

    alarmRegisters.alarm_1_day = i2c_read_byte();
    E_bit0();

    alarmRegisters.alarm_2_minutes = i2c_read_byte();
    E_bit0();

    alarmRegisters.alarm_2_hours = i2c_read_byte();
    E_bit0();

    alarmRegisters.alarm_2_day = i2c_read_byte();
    E_bit1();

    stop();

    // modify alarm registers activating and deactivating the apropriate mask bits

    // Alarm 1
    alarmRegisters.alarm_1_seconds &= 0x7F;
    alarmRegisters.alarm_1_minutes &= 0x7F;
    alarmRegisters.alarm_1_hours &= 0x7F;
    alarmRegisters.alarm_1_day |= 0x80;

    // Alarm 2
    alarmRegisters.alarm_2_minutes &= 0x7F;
    alarmRegisters.alarm_2_hours &= 0x7F;
    alarmRegisters.alarm_2_day |= 0x80;

    // store alarm registers
start1:
    control(DS323_ADDRESS, 0); // Inicializamos una operacion de escritura para poder setear el registro de direcciones de la memoria
    if (R_bit() != 0)
        goto start1;

    i2c_write_byte(0x07); // Escribimos el byte de la direccion del dato

    if (R_bit() != 0)
        goto start1;

    i2c_write_byte(alarmRegisters.alarm_1_seconds);
    if (R_bit() != 0)
        goto start1;

    i2c_write_byte(alarmRegisters.alarm_1_minutes);
    if (R_bit() != 0)
        goto start1;

    i2c_write_byte(alarmRegisters.alarm_1_hours);
    if (R_bit() != 0)
        goto start1;

    i2c_write_byte(alarmRegisters.alarm_1_day);
    if (R_bit() != 0)
        goto start1;

    i2c_write_byte(alarmRegisters.alarm_2_minutes);
    if (R_bit() != 0)
        goto start1;

    i2c_write_byte(alarmRegisters.alarm_2_hours);
    if (R_bit() != 0)
        goto start1;

    i2c_write_byte(alarmRegisters.alarm_2_day);
    if (R_bit() != 0)
        goto start1;

    stop();
}

void setup()
{

    // Inicialización del canal serie para comunicarse con el usuario
    Serial.begin(9600);
    Serial3.begin(9600);

    // Activar entrada de INT0
    pinMode(21, INPUT);

    // Inicialización de los terminales de entrada
    pinMode(LEE_SDA, INPUT);
    pinMode(LEE_SCL, INPUT);
    // Inicialización de los terminales de salida
    pinMode(ESC_SDA, OUTPUT);
    pinMode(ESC_SCL, OUTPUT);
    // Asegurarse de no intervenir el bus poniendo SDA y SCL a "1"....
    digitalWrite(ESC_SDA, HIGH);
    digitalWrite(ESC_SCL, HIGH);

    // Initialize 7 bit display
    DDRA = 0xFF;
    PORTA = 0xFF;
    DDRL |= 0x0F;
    PORTL |= 0x0F;

    cli();
    // Queremos initializar el Timer 3 a modo CTC de tal forma que genere una interrupcion cada segundo.
    TCCR3A = 0;
    TCCR3B = 0;
    TCNT3 = 0;

    OCR3A = 62499; // 16MHz/(256*1/(1s)) - 1
    TCCR3A = B00000000;
    TCCR3B = B00001100;

    // Habilitamos interrupciones para el timer con OCIE3A
    TIMSK3 = B00000010;

    // Initializar Timer 1 para interrumpir cada 10 ms

    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    OCR1A = 9999; // 16MHz/(2*8*1/(10ms)) - 1
    TCCR1A = B00000000;
    TCCR1B = B00001010;

    // Habilitamos interrupciones para el timer con OCIE1A
    TIMSK1 = B00000010;

    // Habilitar interrupciones externa
    EICRA |= (1 << ISC01) | (0 << ISC00);
    EIMSK |= (1 << INT0);

    // inicializar LCD
    Serial3.write(0xFE);
    Serial3.write(0);
    delay(100);

    // inicializar la alarma para que toque cuando concida la hora, minutos y segundos con la fecha/hora actual.
    AlarmSetup();

    sei();

    // Imprimir menu principal en el terminal virtual
    PrintMainMenu();
}

int cSelected = 0;
bool isReadingWord = true;
int operationPos = 0;
int wordReading = 0;
int firstOption = 0;
int secondOption = 0;

void buildWord(char c)
{
    wordReading = wordReading * 10;
    int toAdd = c - '0';
    Serial.print(toAdd);
    wordReading = wordReading + toAdd;
}

void menu(char incommingByte)
{
    if ((int)incommingByte == 13) // if enter
    {
        isReadingWord = false;
    }
    else if (!(incommingByte >= '0' && incommingByte <= '9')) // Check if input is valid
    {
        Serial.println(incommingByte);
        Serial.println("Introduce un numero, empieza de nuevo.");
        wordReading = 0;
        return;
    }

    if (isReadingWord)
    {
        buildWord(incommingByte);
    }
    else
    {
        if (operationPos == 0)
        {
            isReadingWord = true;
            cSelected = wordReading;
            Serial.println("");
            switch (cSelected)
            {
            case 1: // Cambiar tiempo
                Serial.println("Elige de las siguientes opciones: ");
                Serial.println("1: Cambiar segundos");
                Serial.println("2: Cambiar minutos");
                Serial.println("3: Cambiar horas");
                Serial.print("Intoduce datos: ");

                operationPos++;
                break;

            case 2: // Cambiar fecha
                Serial.println("Elige entre las siguiente opciones: ");
                Serial.println("1: Cambiar dia");
                Serial.println("2: Cambiar mes");
                Serial.println("3: Cambiar año");
                Serial.print("Introduce datos: ");

                operationPos++;
                break;

            case 3:
                Serial.println("Elige entre las sigueitnes opciones: ");
                Serial.println("1: Opciones Alarma1");
                Serial.println("2: Opciones Alarma2");
                Serial.print("Introduce datos: ");

                operationPos++;
                break;

            default:
                Serial.println("INVLAIDO. Reinicializando menu");
                PrintMainMenu();
                break;
            }
        }
        // Second level
        else if (operationPos == 1)
        {
            firstOption = wordReading;
            Serial.println("");
            switch (cSelected)
            {
            case 1: // Opciones de cambia de hora
                switch (firstOption)
                {
                case 1: // Cambio de segundo
                    Serial.print("Introduce segundos: ");

                    operationPos++;
                    break;

                case 2: // cambio de minutos
                    Serial.print("Introduce minutos: ");

                    operationPos++;
                    break;

                case 3: // cambio de fecha
                    Serial.print("Introduce horas: ");

                    operationPos++;
                    break;

                default:
                    operationPos = 0;
                    Serial.println("INVLAIDO. Reinicializando menu");
                    PrintMainMenu();
                    break;
                }
                break;

            case 2:
                switch (firstOption)
                {
                case 1: // Cambio de dia
                    Serial.print("Introduce dia: ");

                    operationPos++;
                    break;

                case 2: // cambio de minutos
                    Serial.print("Introduce mes: ");

                    operationPos++;
                    break;

                case 3: // cambio de año
                    Serial.print("Introduce año: ");

                    operationPos++;
                    break;

                default:
                    operationPos = 0;
                    Serial.println("INVLAIDO. Reinicializando menu");
                    PrintMainMenu();
                    break;
                }
                break;

            case 3:

                Serial.println("Elige entre las siguiente opciones: ");
                Serial.println("1: Encender/Apagar alarma");
                Serial.println("2: Cambiar minutos");
                Serial.println("3: Cambiar horas");
                Serial.print("Introduce datos: ");

                operationPos++;
                break;

            default:
                operationPos = 0;
                Serial.println("INVLAIDO. Reinicializando menu");
                PrintMainMenu();
                break;
            }
        }
        // Third level
        else if (operationPos == 2)
        {
            secondOption = wordReading;
            Serial.println("");

            switch (cSelected)
            {
            case 1: // Opciones de cambia de hora
                switch (firstOption)
                {
                case 1: // Cambio de segundo
                    write_DS3232(secondOption, SECONDS_DS3232, SECONDS_MAX);
                    Serial.println("Segundos cambiado");

                    // Fin de elecion de menu
                    operationPos = 0;
                    PrintMainMenu();
                    break;

                case 2: // cambio de minutos
                    write_DS3232(secondOption, MINUTES_DS3232, MINUTES_MAX);
                    Serial.println("Minutos cambiado");

                    // Fin de elecion de menu
                    operationPos = 0;
                    PrintMainMenu();
                    break;

                case 3: // cambio de horas
                    write_DS3232(secondOption, HOURS_DS3232, HOURS_MAX);
                    Serial.println("Horas cambiado");

                    // Fin de elecion de menu
                    operationPos = 0;
                    PrintMainMenu();
                    break;

                default:
                    operationPos = 0;
                    Serial.println("INVLAIDO. Reinicializando menu");
                    PrintMainMenu();
                    break;
                }
                break;

            case 2:
                switch (firstOption)
                {
                case 1: // Cambio de dia
                    write_DS3232(secondOption, DATE_DS3232, DATE_MAX);
                    Serial.println("dia cambiado");

                    // Fin de elecion de menu
                    operationPos = 0;
                    PrintMainMenu();
                    break;

                case 2: // cambio de mes
                    write_DS3232(secondOption, MONTH_DS3232, MONTH_MAX);
                    Serial.println("Mes cambiado");

                    // Fin de elecion de menu
                    operationPos = 0;
                    PrintMainMenu();
                    break;

                case 3: // cambio de año
                    write_DS3232(secondOption, YEAR_DS3232, YEAR_MAX);
                    Serial.println("Horas cambiado");

                    // Fin de elecion de menu
                    operationPos = 0;
                    PrintMainMenu();
                    break;

                default:
                    operationPos = 0;
                    Serial.println("INVLAIDO. Reinicializando menu");
                    PrintMainMenu();
                    break;
                }
                break;

            case 3:
                switch (secondOption) // Switch between type of alarm opration
                {
                case 1:
                {
                    struct alarm_status AlarmStatus = retrieveAlarmStatus();

                    switch (firstOption) // Switch between alarm 1 and alarm 2
                    {
                    case 1:
                        AlarmStatus.alarm1Active = !AlarmStatus.alarm1Active;
                        break;

                    case 2:
                        AlarmStatus.alarm2Active = !AlarmStatus.alarm2Active;
                        break;
                    default:
                        operationPos = 0;
                        Serial.println("INVLAIDO. Reinicializando menu");
                        PrintMainMenu();
                        break;
                    }

                    setAlarmStatus(AlarmStatus);
                }
                    // Fin de elecion de menu
                    operationPos = 0;
                    PrintMainMenu();
                    break;

                case 2:
                    Serial.print("Introduce minutos: ");
                    operationPos++;
                    break;

                case 3:
                    Serial.print("Introduce hora: ");
                    operationPos++;
                    break;

                default:
                    operationPos = 0;
                    Serial.println("INVLAIDO. Reinicializando menu");
                    PrintMainMenu();
                    break;
                }
                break;

            default:
                operationPos = 0;
                Serial.println("INVLAIDO. Reinicializando menu");
                PrintMainMenu();
                break;
            }
        }
        // Forth level
        else if (operationPos == 3)
        {
            Serial.println("");
            switch (cSelected)
            {
            case 3:

                switch (secondOption) // Switch between type of alarm opration
                {
                case 2: // Change minutes

                    switch (firstOption) // Switch entre distinto alarmas
                    {
                    case 1:
                        write_DS3232(wordReading, MINUTES_ALARM_1, MINUTES_MAX);
                        Serial.println("Actualizado minutos de alarma 1");
                        break;

                    case 2:
                        write_DS3232(wordReading, MINUTES_ALARM_2, MINUTES_MAX);
                        Serial.println("Actualizado minutos de alarma 2");
                        break;

                    default:
                        break;
                    }

                    // Fin
                    operationPos = 0;
                    PrintMainMenu();
                    break;

                case 3: // Change hour

                    switch (firstOption) // Switch entre distinto alarmas
                    {
                    case 1:
                        write_DS3232(wordReading, HOURS_ALARM_1, HOURS_MAX);
                        Serial.println("Actualizado horas de alarma 1");
                        break;

                    case 2:
                        write_DS3232(wordReading, HOURS_ALARM_2, HOURS_MAX);
                        Serial.println("Actualizado horas de alarma 2");
                        break;

                    default:
                        break;
                    }

                    // Fin
                    operationPos = 0;
                    PrintMainMenu();
                    break;

                default:
                    operationPos = 0;
                    Serial.println("INVLAIDO. Reinicializando menu");
                    PrintMainMenu();
                    break;
                }
                break;

            default:
                operationPos = 0;
                Serial.println("INVLAIDO. Reinicializando menu");
                PrintMainMenu();
                break;
            }
        }

        wordReading = 0;
        isReadingWord = true;
    }
}

int mostrarNum[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67, /*A*/ 0x77, /*B*/ 0x7C, 0x39, 0x5E, 0x79, 0x71};

ISR(TIMER1_COMPA_vect)
{
    if (Serial.available())
    {
        menu(Serial.read());
    }

    static int interleave = 0;

    switch (interleave)
    {
    case 0:
        PORTL = ~0x01;
        PORTA = mostrarNum[(wordReading) % 10];
        interleave++;
        break;
    case 1:
        PORTL = ~0x02;
        PORTA = mostrarNum[(wordReading / 10) % 10];
        interleave++;
        break;
    case 2:
        PORTL = ~0x04;
        PORTA = mostrarNum[(wordReading / 100) % 10];
        interleave++;
        break;
    case 3:
        PORTL = ~0x08;
        PORTA = mostrarNum[(wordReading / 1000) % 10];
        interleave = 0;
        break;

    default:
        interleave = 0;
        break;
    }
}

void loop()
{
}

const char *months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

ISR(TIMER3_COMPA_vect) // Update LCD
{
    // Retrive data
    int currentTemp = (int)retrieveTemp();
    struct date_time Date = retriveDateTime();
    struct alarm_status alarmStatus = retrieveAlarmStatus();

    // Print data

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

    if (alarmStatus.alarm1Active)
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

    Serial3.write("DDMMMYY");

    // Print Alarm2 time
    Serial3.write(0xFE);
    Serial3.write(212 + 0);

    sprintf(buffer, "%02d:%02d", Date.alarm2Hour, Date.alarm2Minute);
    Serial3.write(buffer);

    // Print alarm active or not
    Serial3.write(0xFE);
    Serial3.write(212 + 5);

    if (alarmStatus.alarm2Active)
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

    sprintf(buffer, "%02d%s%02d", Date.date, months[Date.month - 1], Date.year);
    Serial3.write(buffer);
}

ISR(INT0_vect)
{
    // Check which alarm produced the inturrupt and deactivate flag.
start:
    control(DS323_ADDRESS, 0); // Inicializamos una operacion de escritura para poder setear el registro de direcciones de la memoria
    if (R_bit() != 0)
        goto start;

    i2c_write_byte(0x0F); // Escribimos el byte de la direccion del dato

    if (R_bit() != 0)
        goto start;

    control(DS323_ADDRESS, 1); // Sin ninguna instruccion stop inicializamos la operacion de lectura
    if (R_bit() != 0)
        goto start;

    byte data = i2c_read_byte();

    E_bit1();

    stop();

start1:
    control(DS323_ADDRESS, 0); // Inicializamos una operacion de escritura para poder setear el registro de direcciones de la memoria
    if (R_bit() != 0)
        goto start1;

    i2c_write_byte(0x0F); // Escribimos el byte de la direccion del dato

    if (R_bit() != 0)
        goto start1;

    i2c_write_byte(data & 0xFC);

    if (R_bit() != 0)
        goto start1;

    stop();

    // Make noise based on alarm and print which alarm went off

    if ((data & 0x1) != 0)
    {
        Serial.println("Ha sonado la alarma 1");
        tone(37, 3000, 5000);
    }

    if ((data & 0x2) != 0)
    {
        Serial.println("Ha sonado la alarma 2");
        tone(37, 2000, 5000);
    }
}
