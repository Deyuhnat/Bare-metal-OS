// -----------------------------------kernel.c -------------------------------------
#include "uart1.h"
#include "printf.h"
#include "mbox.h"
#include "./gcclib/stdint.h"
#include "framebf.h"
#include "image.h"
#include "video.h"

#define MAX_CMD_SIZE 100
#define MAX_HISTORY 10
#define NULL ((void *)0)

// manual string functions declaration
int strlen(const char *str);
char *strcpy(char *dest, const char *src);
int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, int n);
char *strtok(char *str, const char *delim);
int strspn(const char *str, const char *set);
int strcspn(const char *str, const char *set);
char *strchr(const char *str, int c);

char *welcome_message =
    "####### ####### ####### #######  #####  #        #####    ###   \n"
    "#       #       #          #    #     # #    #  #     #  #   #  \n"
    "#       #       #          #          # #    #  #     # #     # \n"
    "#####   #####   #####      #     #####  #    #   ###### #     # \n"
    "#       #       #          #    #       #######       # #     # \n"
    "#       #       #          #    #            #  #     #  #   #  \n"
    "####### ####### #######    #    #######      #   #####    ###   \n"
    "\n"
    "\n"
    " #     #                                                           \n"
    " #  #  # ###### #       ####   ####  #    # ######    #####  ####  \n"
    " #  #  # #      #      #    # #    # ##  ## #           #   #    # \n"
    " #  #  # #####  #      #      #    # # ## # #####       #   #    # \n"
    " #  #  # #      #      #      #    # #    # #           #   #    # \n"
    " #  #  # #      #      #    # #    # #    # #           #   #    # \n"
    "  ## ##  ###### ######  ####   ####  #    # ######      #    ####  \n"
    "\n"
    "\n"
    "    #                     #######  #####  \n"
    "   # #   #    # #    #    #     # #     # \n"
    "  #   #  ##   # #    #    #     # #       \n"
    " #     # # #  # ######    #     #  #####  \n"
    " ####### #  # # #    #    #     #       # \n"
    " #     # #   ## #    #    #     # #     # \n"
    " #     # #    # #    #    #######  #####  \n"
    "\n"
    "Developed by Doan Hoang Anh - s3880604\n"
    "\n";

char *commands[] = {
    "help",
    "clear",
    "setcolor",
    "showinfo",
    "printf",
    "expandscreen",
    "getmacaddress",
    "getuartfreq",
    "getarmfreq"};
char *commandsInfo[] = {
    "*Show detail information of each command\nUsage: help [command_name]\n",
    "clear - Clears the screen\n",
    "setcolor - Sets text and/or background color\nUsage: setcolor -t [color] -b [background_color]\n",
    "showinfo - Displays board information\n",
    "printf - Test the printf function\n",
    "expandscreen - Expand the qemu display screen\n",
    "getmacaddress - Display the MAC Adress\n",
    "getuartfreq - Display the Uart Frequency\n",
    "getarmfreq - Display the ARM Frequency\n}"};
char *commandsDetail[] = {
    "help: This command is used to provide a detailed description of available commands. If you want to know more about a specific command, type 'help [command_name]'.\n",
    "clear: Typing 'clear' will remove all the content from your current view, giving you a clean screen to work with.\n",
    "setcolor: Use this command to customize your text and background colors. To change the text color, use the '-t' flag followed by your desired color. For changing the background, use the '-b' flag followed by your choice of color. For instance, 'setcolor -t red -b blue' will give you red text on a blue background.\n",
    "showinfo: Execute this command to view important board details. It will display essential information about the board you're currently working on.\n",
    "printf: This command lets you test the printf function. 'printf' is a fundamental function used in programming to display text or data.\n",
    "expandscreen: If you feel the qemu display screen is too small or need a larger view, use 'expandscreen'.\n",
    "getmacaddress: To know the MAC address of your board or system, simply type 'getmacaddress'. It will fetch and display the MAC address for you.\n",
    "getuartfreq: By entering 'getuartfreq', you can determine the frequency at which the UART is operating.\n",
    "getarmfreq: If you're interested in the operational frequency of the ARM processor, use 'getarmfreq'. It will show the default rate at which the ARM CPU is running.\n"};

int num_commands = sizeof(commands) / sizeof(commands[0]);
char *colors[] = {
    "BLACK",
    "RED",
    "GREEN",
    "YELLOW",
    "BLUE",
    "PURPLE",
    "CYAN",
    "WHITE"};

char *ansiColors[] = {
    "\033[30m", // BLACK
    "\033[31m", // RED
    "\033[32m", // GREEN
    "\033[33m", // YELLOW
    "\033[34m", // BLUE
    "\033[35m", // PURPLE
    "\033[36m", // CYAN
    "\033[37m", // WHITE
};

char *ansiBackgroundColors[] = {
    "\033[40m", // BLACK
    "\033[41m", // RED
    "\033[42m", // GREEN
    "\033[43m", // YELLOW
    "\033[44m", // BLUE
    "\033[45m", // PURPLE
    "\033[46m", // CYAN
    "\033[47m", // WHITE
};

char cmd_history[MAX_HISTORY][MAX_CMD_SIZE];
int history_index = 0;
int current_index = 0;

void uppercaseLetter(char *str)
{
    while (*str)
    {
        if (*str >= 'a' && *str <= 'z')
        {
            *str = *str - ('a' - 'A');
        }
        str++;
    }
}

void setcolor(const char *textColor, const char *backgroundColor)
{
    char upperTextColor[MAX_CMD_SIZE] = {0};
    char upperBackgroundColor[MAX_CMD_SIZE] = {0};

    if (textColor)
    {
        strcpy(upperTextColor, textColor);
        uppercaseLetter(upperTextColor);
    }
    if (backgroundColor)
    {
        strcpy(upperBackgroundColor, backgroundColor);
        uppercaseLetter(upperBackgroundColor);
    }

    if (textColor)
    {
        for (int i = 0; i < 8; i++)
        {
            if (strcmp(upperTextColor, colors[i]) == 0)
            {
                uart_puts(ansiColors[i]);
            }
        }
    }
    if (backgroundColor)
    {
        for (int i = 0; i < 8; i++)
        {
            if (strcmp(upperBackgroundColor, colors[i]) == 0)
            {
                uart_puts(ansiBackgroundColors[i]);
            }
        }
    }
}
void showBoardInfo()
{
    unsigned int *responseData = 0;
    mbox_buffer_setup(ADDR(mbox), 0x00010002, &responseData, 4, 0, NULL); // tag 0x00010002 for boardrivision
    if (mbox_call(ADDR(mbox), MBOX_CH_PROP))
    {
        uart_puts("Board Revision: ");
        uart_dec(*responseData);
        uart_puts("\n");
        uart_puts("Board Revision(in Hexa): ");
        uart_hex(*responseData);
        uart_puts("\n");
        if (*responseData == 0x00a02082)
        {
            uart_puts("Board model: rpi-3B BCM2837 1GiB Sony UK");
        }
        else if (*responseData == 0x00900092)
        {
            uart_puts("Board model: rpi-Zero BCM2835 512MB Sony UK");
        }
        else if (*responseData == 0x00000010)
        {
            uart_puts("Board model: rpi-1B+ BCM2835");
        }
        else if (*responseData == 0x00a01041)
        {
            uart_puts("Board model: rpi-2B BCM2836 1GiB Sony UK");
        }
        else if (*responseData == 0x00b03111)
        {
            uart_puts("Board model: rpi-4B BCM2711 2GiB Sony UK");
        }
        uart_puts("\n\n");
    }
    else
    {
        uart_puts("Failed to get board revision.\n");
    }
}
void expandScreen()
{
    unsigned int request_vals[2] = {1024, 768};
    unsigned int *physize = 0;
    mbox_buffer_setup(ADDR(mbox), MBOX_TAG_SETPHYWH, &physize, 8, 8, request_vals);
    mbox_call(ADDR(mbox), MBOX_CH_PROP);
    uart_puts("\nGot Actual Physical Width: ");
    uart_dec(physize[0]);
    uart_puts("\nGot Actual Physical Height: ");
    uart_dec(physize[1]);
    uart_puts("\n");
}
void getMacAddress()
{
    unsigned int *responseData = 0;
    mbox_buffer_setup(ADDR(mbox), 0x00010003, &responseData, 8, 0, NULL);
    if (mbox_call(ADDR(mbox), MBOX_CH_PROP))
    {
        unsigned char *macBytes = (unsigned char *)responseData;

        uart_puts("MAC Address: 0x");
        int order1[] = {3, 2, 1, 0}; // Byte order for format
        for (int i = 0; i < 4; i++)
        {
            uart_hex_byte(macBytes[order1[i]]);
        }
        uart_puts("\n");

        uart_puts("MAC Address: 0x0000"); // first 4 is 0x0000 based on the format
        int order2[] = {5, 4};            // Byte order for expected format
        for (int i = 0; i < 2; i++)
        {
            uart_hex_byte(macBytes[order2[i]]);
        }
        uart_puts("\n");

        // Print colon-separated format
        uart_puts("MAC Address: ");
        for (int i = 5; i >= 0; i--)
        {
            uart_hex_byte(macBytes[i]);
            if (i != 0)
                uart_sendc(':');
        }
        uart_puts("\n\n");
    }
    else
    {
        uart_puts("Failed to get MAC address.\n");
    }
}

void getUartClock()
{
    unsigned int request_values[] = {2, 0}; // 2 is the clock id for UART and 0 to clear output buffer
    unsigned int *responseData = 0;
    mbox_buffer_setup(ADDR(mbox), 0x00030002, &responseData, 8, 8, request_values); // tag 0x00030002 clock
    if (mbox_call(ADDR(mbox), MBOX_CH_PROP))
    {
        uart_puts("UART Clock Rate: ");

        uart_dec(responseData[1]);
        uart_puts(" Hz\n\n");
    }
    else
    {
        uart_puts("Failed to get UART clock rate.\n");
    }
}
void getArmFrequency()
{
    unsigned int request_values[] = {3, 0}; // 3 is the clock id for ARM and 0 to clear output buffer
    unsigned int *responseData = 0;
    mbox_buffer_setup(ADDR(mbox), 0x00030002, &responseData, 8, 8, request_values); // tag 0x00030002 for clock

    if (mbox_call(ADDR(mbox), MBOX_CH_PROP))
    {
        uart_puts("ARM Frequency: ");
        uart_dec(responseData[1]);
        uart_puts(" Hz\n\n");
    }
    else
    {
        uart_puts("Failed to get ARM frequency.\n");
    }
    // //mailbox data buffer: Read ARM frequency
    // mbox[0] = 8 * 4;        // Message Buffer Size in bytes (8 elements * 4 bytes (32 bit) each)
    // mbox[1] = MBOX_REQUEST; // Message Request Code (this is a request message)
    // mbox[2] = 0x00030002;   // TAG Identifier: Get clock rate
    // mbox[3] = 8;            // Value buffer size in bytes (max of request and response lengths)
    // mbox[4] = 0;            // REQUEST CODE = 0
    // mbox[5] = 3;            // clock id: ARM system clock
    // mbox[6] = 0;            // clear output buffer (response data are mbox[5] & mbox[6])
    // mbox[7] = MBOX_TAG_LAST;

    // if (mbox_call(ADDR(mbox), MBOX_CH_PROP))
    // {
    //     uart_puts("ARM clock rate: ");
    //     uart_dec(mbox[6]);
    //     uart_puts(" Hz");
    //     uart_puts("\n");
    // }
    // else
    // {
    //     uart_puts("Unable to query!\n");
    // }
}
void drawLargeImageScroll()
{
    uart_puts("Use WASD to scroll. Press Enter to quit scroll mode ");
    int y = 0;
    int x = 0;
    drawImage(image2image2, x, y, 1920, 1080);

    while (1)
    {
        char c = uart_getc();

        if (c == 'w')
        {
            y -= 20;
        }
        else if (c == 's')
        {
            y += 20;
        }
        else if (c == 'd')
        {
            x += 20;
        }
        else if (c == 'a')
        {
            x -= 20;
        }
        else if (c == '\n')
        {
            uart_puts("\n");
            break;
        }
        clearScreen(0);

        drawImage(image2image2, x, y, 1920, 1080);
    }
}
void playVideo()
{
    uart_puts("Playing video \n");
    uart_puts("Press Enter to stop ");
    char c = uart_get_char();
    int i = 0;
    while (c != '\n')
    {
        if (i > 7)
            i = 0;
        // printf("%d\n", i);
        drawImage(video_frames[i], 0, 0, 453, 421);
        wait_ms(60000);
        i++;
        c = uart_get_char();
    }
    uart_puts("\nVideo stopped");
}
void wait_ms(unsigned int n)
{
    register unsigned long f, t, r;

    // Get the current counter frequency
    asm volatile("mrs %0, cntfrq_el0"
                 : "=r"(f));
    // Read the current counter
    asm volatile("mrs %0, cntpct_el0"
                 : "=r"(t));
    // Calculate expire value for counter
    t += ((f / 1000) * n) / 1000;
    do
    {
        asm volatile("mrs %0, cntpct_el0"
                     : "=r"(r));
    } while (r < t);
}
void display_prompt()
{
    uart_puts("AnhDoanOS> ");
}
void execute_command(char *cmd)
{
    if (strcmp(cmd, commands[0]) == 0)
    {
        uart_puts("*Supported commands:\n");
        uart_puts("help, clear, setcolor, showinfo, printf, expandscreen, getmacaddress, getuartfreq, getarmfreq\n\n");
        uart_puts("*General description:\n");
        uart_puts(commandsInfo[1]);
        uart_puts(commandsInfo[2]);
        uart_puts(commandsInfo[3]);
        uart_puts(commandsInfo[4]);
        uart_puts(commandsInfo[5]);
        uart_puts(commandsInfo[6]);
        uart_puts(commandsInfo[7]);
        uart_puts("\n");
        uart_puts(commandsInfo[0]);
        uart_puts("\n");
    }
    else if (strcmp(cmd, "help help") == 0)
    {
        uart_puts(commandsDetail[0]);
    }
    else if (strcmp(cmd, "help clear") == 0)
    {
        uart_puts(commandsDetail[1]);
    }
    else if (strcmp(cmd, "help setcolor") == 0)
    {
        uart_puts(commandsDetail[2]);
    }
    else if (strcmp(cmd, "help showinfo") == 0)
    {
        uart_puts(commandsDetail[3]);
    }
    else if (strcmp(cmd, "help printf") == 0)
    {
        uart_puts(commandsDetail[4]);
    }
    else if (strcmp(cmd, "help expandscreen") == 0)
    {
        uart_puts(commandsDetail[5]);
    }
    else if (strcmp(cmd, "help getmacaddress") == 0)
    {
        uart_puts(commandsDetail[6]);
    }
    else if (strcmp(cmd, "help getuartfreq") == 0)
    {
        uart_puts(commandsDetail[7]);
    }
    else if (strcmp(cmd, "help getarmfreq") == 0)
    {
        uart_puts(commandsDetail[8]);
    }
    else if (strcmp(cmd, "showimage") == 0)
    {
        //framebf_init(1024, 720);
        drawImage(image1image1, 0, 0, 480, 270);
    }
    else if (strcmp(cmd, "showlargeimage") == 0)
    {
        //framebf_init(1024, 720);
        drawLargeImageScroll();
    }
    else if (strcmp(cmd, "showvideo") == 0)
    {
        //framebf_init(1024, 720);
        playVideo();
    }

    else if (strcmp(cmd, commands[1]) == 0)
    {
        // for (int i = 0; i < 50; i++)
        //     uart_puts("\n");
        uart_puts("\033[2J\033[H");
    }
    else if (strncmp(cmd, commands[2], 8) == 0) // setcolor command
    {
        char *token = strtok(cmd, " ");
        char *textColor = NULL;
        char *backgroundColor = NULL;

        while (token != NULL)
        {
            if (strcmp(token, "-t") == 0)
            {
                token = strtok(NULL, " ");
                textColor = token;
            }
            else if (strcmp(token, "-b") == 0)
            {
                token = strtok(NULL, " ");
                backgroundColor = token;
            }
            token = strtok(NULL, " ");
        }

        setcolor(textColor, backgroundColor);
    }
    else if (strcmp(cmd, commands[3]) == 0)
    {
        showBoardInfo();
        getMacAddress();
    }
    else if (strcmp(cmd, commands[4]) == 0)
    {
        printf("String: %s\n", "Hello");
        printf("Character: %c\n", 'Z');
        printf("Percentage sign: %%\n");
        printf("Decimal/integer number: %d\n", 212);
        printf("This is a Float number: %f \n", 0.21);
        printf("Hexadecimal: %x\n", 195);
    }
    else if (strcmp(cmd, commands[5]) == 0)
    {
        expandScreen();
    }
    else if (strcmp(cmd, commands[6]) == 0)
    {
        getMacAddress();
    }
    else if (strcmp(cmd, commands[7]) == 0)
    {
        getUartClock();
    }
    else if (strcmp(cmd, commands[8]) == 0)
    {
        getArmFrequency();
    }
    else
    {
        uart_puts("Unrecognized command!\n");
    }
}

void cli()
{
    static char cli_buffer[MAX_CMD_SIZE];
    static int index = 0;
    char c;
    while (1)
    {
        c = uart_getc();
        uart_sendc(c); // echo the character
        // Auto-completion
        if (c == '\t')
        {
            for (int i = 0; i < num_commands; i++)
            {
                if (strncmp(cli_buffer, commands[i], strlen(cli_buffer)) == 0)
                {
                    strcpy(cli_buffer, commands[i]);
                    index = strlen(cli_buffer);

                    uart_puts("\rAnhDoanOS>                                                                                            "); // Clear line
                    uart_puts("\rAnhDoanOS> ");
                    uart_puts(cli_buffer); // print the entire buffer, including the auto-completed part

                    break;
                }
            }
        }
        // History UP
        else if (c == '_')
        {
            if (current_index == history_index)
            {
                // If currently at the most recent command, save current input first
                strcpy(cmd_history[history_index % MAX_HISTORY], cli_buffer);
            }
            current_index = (current_index - 1 + MAX_HISTORY) % MAX_HISTORY;
            strcpy(cli_buffer, cmd_history[current_index]);
            index = strlen(cli_buffer);
            uart_puts("\rAnhDoanOS>                                                                                                   ");
            uart_puts("\rAnhDoanOS> ");
            uart_puts(cli_buffer);
        }
        // History DOWN
        else if (c == '+')
        {
            current_index = (current_index + 1) % MAX_HISTORY;
            if (current_index == history_index)
            {
                // If going back to the most recent command, retrieve the saved input
                strcpy(cli_buffer, cmd_history[current_index]);
            }
            else
            {
                strcpy(cli_buffer, cmd_history[current_index]);
            }
            index = strlen(cli_buffer);
            uart_puts("\rAnhDoanOS>                                                                                                   "); // Clear line
            uart_puts("\rAnhDoanOS> ");
            uart_puts(cli_buffer);
        }
        else if (c == 0x08 || c == 0x7F) // ASCII for backspace key and del key
        {
            if (index > 0) // check there are characters to delete
            {
                index = index - 1;
                cli_buffer[index] = '\0';                                                                                                // "delete" the last character in the buffer
                uart_puts("\b \b");                                                                                                      // echo backspace, space, backspace to terminal
                uart_puts("\rAnhDoanOS>                                                                                              "); // Clear the line
                uart_puts("\rAnhDoanOS> ");
                uart_puts(cli_buffer);
            }
            else
            {
                uart_puts("\rAnhDoanOS> "); // prevent delete through display prompt
            }
        }
        else if (c != '\n')
        {
            cli_buffer[index++] = c;
        }
        else
        {
            cli_buffer[index] = '\0';

            // Store in history
            if (index > 0)
            {
                strcpy(cmd_history[history_index % MAX_HISTORY], cli_buffer);
                history_index++;
                if (history_index >= MAX_HISTORY)
                {
                    history_index = 0;
                }
            }
            current_index = history_index;
            uart_puts("\n");
            if (index > 0)
            {
                execute_command(cli_buffer);
            }
            else
            {
                uart_puts("");
            }
            index = 0;
            display_prompt();
        }
    }
}

void main()
{
    framebf_init(1024, 720);
    // intitialize UART
    uart_init();
    setcolor("red", "black");
    uart_puts(welcome_message);
    display_prompt();

    while (1)
    {
        cli();
    }
}
// some manual string functions (cannot include "string.h")
int strlen(const char *str)
{
    const char *s = str;
    while (*s)
        ++s;
    return s - str;
}

char *strcpy(char *dest, const char *src)
{
    char *save = dest;
    while ((*dest++ = *src++))
        ;
    return save;
}

int strcmp(const char *str1, const char *str2)
{
    while (*str1 && (*str1 == *str2))
    {
        str1++;
        str2++;
    }
    return *(unsigned char *)str1 - *(unsigned char *)str2;
}

int strncmp(const char *str1, const char *str2, int n)
{
    while (n && *str1 && (*str1 == *str2))
    {
        str1++;
        str2++;
        n--;
    }
    if (n == 0)
        return 0;
    return *(unsigned char *)str1 - *(unsigned char *)str2;
}
char *strtok(char *str, const char *delim)
{
    static char *next_token = NULL;
    char *token_start;

    // If the input string is NULL, continue tokenizing the previous string
    if (str == NULL)
    {
        str = next_token;
    }

    // If the string is NULL or an empty string, return NULL
    if (str == NULL || *str == '\0')
    {
        next_token = NULL;
        return NULL;
    }

    // Tokenize and skip any leading delimiters
    token_start = str + strspn(str, delim);
    if (*token_start == '\0')
    {
        next_token = NULL;
        return NULL;
    }

    // Find the end of the token
    next_token = token_start + strcspn(token_start, delim);
    if (*next_token == '\0')
    {
        next_token = NULL;
    }
    else
    {
        // Replace the following delimiter with a null terminator
        *next_token = '\0';
        next_token++;
    }

    return token_start;
}

int strspn(const char *str, const char *set)
{
    int len = 0;
    while (*str && strchr(set, *str))
    {
        str++;
        len++;
    }
    return len;
}

int strcspn(const char *str, const char *set)
{
    int len = 0;
    while (*str && !strchr(set, *str))
    {
        str++;
        len++;
    }
    return len;
}

char *strchr(const char *str, int c)
{
    while (*str)
    {
        if (*str == (char)c)
        {
            return (char *)str;
        }
        str++;
    }
    if (*str == (char)c)
    {
        return (char *)str;
    }
    return NULL;
}
