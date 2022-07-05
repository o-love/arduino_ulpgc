#define Serial3 Serial

void setup()
{
    Serial3.begin(9600);

    Serial3.write(0xFE);
    Serial3.write(0x01);
    delay(100);

    Serial3.write(0xFE);
    Serial3.write(0x00);
    delay(100);

    Serial3.write("Hello");
    delay(100);

    Serial3.write(0xFE);
    Serial3.write(0xC0);
    delay(100);

    Serial3.write("Hey");
}

void loop() {}