bool configuration_mode = false;

int operationPos = 0;
int option_mag10 = 0;
int option_mag1 = 0;
void menu(char c)
{
    if (operationPos == 0) // Entrada en modo configuracion
    {
        static int preOp = 0;
        if (preOp == 0)
        {
            if (c == '*')
                preOp++;
        }
        else
        {
            if (c == '#') // Entering configuration mode
            {
                preOp = 0;
                operationPos++;
                configuration_mode = true;
                // TODO: Display configuration options on virtual
            }
            else
            {
                preOp = 0;
            }
        }
    }
    else if (operationPos == 1) // dentro de modo configuracion. primer nivel
    {
    }
}
