// Programa de demostración
// Programa para hacer declaración básica
// y enviar un mensaje a la consola

#define LEE_SCL 40 // puerto de entrada para leer el estado de la línea SCL
#define LEE_SDA 41 // puerto de entrada para leer el estado de la línea SDA
#define ESC_SCL 4  // puerto de salida para escribir el valor de la línea SCL-out
#define ESC_SDA 39 // puerto de salida para escribir el valor de la línea SDA-out

int waitTime = 0; // Utilizdo para poder visualizar en el osciloscopio.

void start()
{
    // Serial.println("Start\n");
    digitalWrite(ESC_SCL, HIGH);
    digitalWrite(ESC_SDA, HIGH); // Soltamos los datos y el reloj, como nadie mas lo esta controlando pasa a estar altos en el bus tambien

    delay(waitTime);

    digitalWrite(ESC_SDA, LOW); // Bajamos la linea de datos mientras que el reloj sigue en modo lectura

    delay(waitTime);

    digitalWrite(ESC_SCL, LOW); // Bajamos el reloj para prepararnos para el siguente instruccion.
}

void stop()
{
    // Serial.println("Stop\n");
    digitalWrite(ESC_SCL, LOW);
    digitalWrite(ESC_SDA, LOW); // Bajamos la lina de datos y el reloj tomando control del BUS para preparar el señal stop

    delay(waitTime);

    digitalWrite(ESC_SCL, HIGH); // Subimos el reloj a modo lectura para prepararnos para el señal stop

    delay(waitTime / 2);

    digitalWrite(ESC_SDA, HIGH); // Realizamos un flanco de subida mientras en modo lectura indicando el stop

    delay(waitTime);
}

void E_bit1()
{
    // Serial.println("Ebit1\n");
    digitalWrite(ESC_SCL, LOW); // Bajamos el reloj tomando asi control del BUS para escribir en el

    delay(waitTime / 2);

    digitalWrite(ESC_SDA, HIGH); // Escribimos nuestro dato en el bus.

    delay(waitTime / 2);

    digitalWrite(ESC_SCL, HIGH); // Ponemos el bus en modo de lectura

    delay(waitTime);

    digitalWrite(ESC_SCL, LOW);
    digitalWrite(ESC_SDA, LOW); // Preparamos el bus para el siguiente instruccion.
}

void E_bit0()
{
    // Serial.println("Ebit0\n");
    digitalWrite(ESC_SCL, LOW); // Bajamos el reloj popniendo el BUS en modo escritura.

    delay(waitTime / 2);

    digitalWrite(ESC_SDA, LOW); // Escribimos el dato en el BUS.

    delay(waitTime / 2);

    digitalWrite(ESC_SCL, HIGH); // Metemos el BUS en modo lectura

    delay(waitTime);

    digitalWrite(ESC_SCL, LOW); // Preparamos el bus para la siguiente insturccion.
    digitalWrite(ESC_SDA, LOW);
}

byte R_bit()
{
    // Serial.println("Rbit\n");
    digitalWrite(ESC_SCL, LOW); // Nos aseguramos de que se estemos en modo escritura

    delay(waitTime / 2);

    digitalWrite(ESC_SDA, HIGH); // Soltamos el control de la linea de datos

    delay(waitTime / 2);

    digitalWrite(ESC_SCL, HIGH);    // Nos ponemos en modo lectura
    int val = digitalRead(LEE_SDA); // Leemos el valor de la linea de datos

    delay(waitTime);

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

    Serial.println("1:  Guardar un dato (de 0 a 255) en cualquier dirección de memoria del dispositivo 24LC64.");
    Serial.println("2:  Leer una posición (de 0 a 8191) del 24LC64");
    Serial.println("3:  Inicializar toda la memoria del 24LC64 a un valor");
    Serial.println("4:  Mostrar el contenido de un bloque de 256 bytes del 24LC64, comenzando en una dirección especificada");
    Serial.println("5:  Inicializar usando Page Write toda la memoria del 24LC64 a un valor");
    Serial.println("6:  Mostrar el contenido de un bloque de 256 bytes del 24LC64 (Sequential Read), comenzando en una dirección especificada");
    Serial.println("7:  Mostrar la fecha y hora tomado del RTC");
    Serial.println("8:  Mostrarla temperatura leida del RTC");
    Serial.println("Despues de introducir cada valor pulsa enter");
    Serial.println("Input Operation number: ");
}
int cSelected = 0;
bool isReadingWord = true;
int operationPos = 0;
int wordReading = 0;
int firstOption = 0;

void buildWord(char c)
{
    wordReading = wordReading * 10;
    int toAdd = c - '0';
    Serial.print(toAdd);
    wordReading = wordReading + toAdd;
}

int timei = 0;

void menu()
{
    if (Serial.available() > 0)
    {
        char incommingByte = Serial.read();

        if ((int)incommingByte == 13)
        {
            isReadingWord = false;
        }
        else if (!(incommingByte >= '0' && incommingByte <= '9'))
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
            { // Intial setup for reading
                firstOption = 0;
                isReadingWord = true;
                cSelected = wordReading;
                Serial.println("");
                switch (cSelected)
                {
                case 7:
                    // Retrive all necesary data

                    // Display data
                    break;
                case 8:

                    break;

                default:
                    Serial.println("Enter first data piece according to type of operation:");
                    operationPos++; // Pasamos a la siguiente paso de la operacion selecionado
                    break;
                }
            }
            else if (operationPos == 1)
            {
                Serial.println("first");
                Serial.println(cSelected);
                switch (cSelected)
                {
                case 1:
                    firstOption = wordReading;
                    Serial.println("Type in data now:");
                    operationPos++; // Pasamos al siguiente paso de esta operacion
                    break;
                case 2:
                    Serial.print("0x");
                    Serial.println(byteReadFromMem(B1010000, wordReading), HEX);
                    operationPos = 0; // Terminamos con la operacion
                    Serial.println("Done printing individual word.");
                    break;
                case 3:
                    timei = millis();
                    setEntireMemTo(B1010000, wordReading);
                    Serial.print("Done setting memory using individual writes. Total time ");
                    Serial.print(millis() - timei);
                    Serial.println("ms");
                    operationPos = 0; // Terminamos con la operacion
                    break;
                case 4:
                    timei = millis();
                    printBlockRead(B1010000, wordReading);
                    Serial.print("Done reading block using individual reads. Total time ");
                    Serial.print(millis() - timei);
                    Serial.println("ms");
                    operationPos = 0; // Terminamos con la operacion
                    break;
                case 5:
                    timei = millis();
                    setEntireMemToPageWrite(B1010000, wordReading);
                    Serial.print("Done setting memory using page writes. Total time ");
                    Serial.print(millis() - timei);
                    Serial.println("ms");
                    operationPos = 0; // Terminamos con la operacion
                    break;
                case 6:
                    timei = millis();
                    printBlockPageRead(B1010000, wordReading);
                    Serial.print("Done reading memory using sequencal read. Total time ");
                    Serial.print(millis() - timei);
                    Serial.println("ms");
                    operationPos = 0; // Terminamos con la operacion
                    break;
                default:
                    Serial.println("Unkown command. Please ensure the command you introduced is valid.");
                    operationPos = 0;
                    break;
                }
            }
            else if (operationPos == 2)
            {
                Serial.println("");
                switch (cSelected)
                {
                case 1:
                    byteWriteToMem(B1010000, firstOption, wordReading);
                    Serial.println("Done writing word to memory");
                    operationPos = 0; // Terminamos con la operacion
                    break;
                default:
                    Serial.println("Unkown command. Please ensure the command you introduced is valid.");
                    operationPos = 0;
                    break;
                }
            }
            wordReading = 0;
            isReadingWord = true;
        }
    }
}

void loop()
{
    menu();
}