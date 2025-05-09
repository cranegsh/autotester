/*
 * Utility.c
 *
 *  Created on: Jul. 27, 2022
 *      Author: Crane Shao
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "utility.h"
#include "sysconfig.h"
#include "app_canfd.h"

char *project_name[] = {
	PROJECT_ARGU_ID4,
	PROJECT_ARGU_G3,
	PROJECT_ARGU_G4R,
	PROJECT_ARGU_C3,
	PROJECT_ARGU_NAVY,
	PROJECT_ARGU_VOLVO,
};

struct debugData dData = {
    .mark = 0,
    .buffer = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    .number = 0x3FFFF,
    .display = MAIN_LOOP_DISPLAY_STATUS
};

union time_bcd_t {
    struct {
        uint16_t unit: 4;
        uint16_t ten: 3;
        uint16_t os: 1;
        uint16_t unused: 8;
    } bF;
    uint16_t hword;
};

void abort_program(void)
{
	ndPrintf("Free CANFD handler ...\n");
	msg_canfd_deinit();
	iPrintf("Exiting with failure ...\n");
	exit(EXIT_FAILURE);
}

void clear_stdin(void)
{
    // keep reading 1 more char as long as the end of the stream, indicated by the newline char,
    // has NOT been reached
    while (1)
    {
        int c = getc(stdin);
        if (c == EOF || c == '\n')
        {
            break;
        }
    }
}

char get_a_char(void)
{
	char ch;

	scanf("%c", &ch);

	/* clean the last ENTER after inputting a number. Any API to clear std io? -> write own */
	if('\r' == ch) scanf("%c", &ch);

	return ch;
}

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

void enable_non_blocking_mode() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~ICANON; // Disable canonical mode
    t.c_lflag &= ~ECHO;   // Disable echo
    tcsetattr(STDIN_FILENO, TCSANOW, &t);

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

void restore_terminal_mode() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag |= ICANON; // Enable canonical mode
    t.c_lflag |= ECHO;   // Enable echo
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

/* get a char without blocking the program */
char get_a_char_nb() {
    struct timeval timeout;
    fd_set set;
    char buffer[128], ch;

    // Configure select timeout
    timeout.tv_sec = 0;  	// Wait up to 0 seconds
    timeout.tv_usec = 0;	// Wait up to 0 nanoseconds

    // Initialize the file descriptor set
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set); // Add stdin (file descriptor 0) to the set

    // Check if there's input in stdin
    int result = select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout);

    if (result > 0) {
        // Input is available
        fgets(buffer, sizeof(buffer), stdin); // Read input
        ch = buffer[0];
        ndPrintf("You entered: %s\n", buffer);

        ndPrintf("\nBuffer: ");
        for(uint32_t i=0; i<sizeof(buffer); i++) {
		   ndPrintf("%d/%c | ", buffer[i], buffer[i]);
        }
        //fflush(stdout);
    } else if (result == 0) {
        // Timeout
        ndPrintf("No input received within the timeout.\n");
    } else {
        // Error occurred, don't display it
        //perror("select");
    }

    return ch;
}

char get_a_char_nb_voidHanlder(void (*handler)(void))
{
    char ch;
    enable_non_blocking_mode();

    while (1) {
        fd_set set;
        struct timeval timeout;
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);

        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; // Check every 100ms

        int rv = select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout);
        if (rv > 0) {
            read(STDIN_FILENO, &ch, 1);
            if ((' ' <= ch) && ('~' >= ch)) {
            	break;
            }
            ndPrintf("You pressed: %c\n", ch);
        } else {
            ndPrintf("."); // Indicate no key press
            fflush(stdout);
        }
    }

    restore_terminal_mode();
	return ch;
}

char get_a_char_nb_wHandler(void (*handler)(uint32_t), uint32_t prj_num)
{
    char ch;
    enable_non_blocking_mode();

    while (1) {
        fd_set set;
        struct timeval timeout;
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);

        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; // Check every 100ms

        int rv = select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout);
        if (rv > 0) {
            read(STDIN_FILENO, &ch, 1);
            if ((' ' <= ch) && ('~' >= ch)) {
            	break;
            }
            ndPrintf("You pressed: %c\n", ch);
        } else {
            ndPrintf("."); // Indicate no key press
            handler(prj_num);
            fflush(stdout);
        }
    }

    restore_terminal_mode();
	return ch;
}

/* Function to get an integer allowing negative value
 * return INVALID_INPUT if failed to acquire a number: there is an assumption that it is an invalid value in all cases.
 * TODO: the caller must check the value it gets.
 * */
int32_t get_a_number(const char *msg)
{
    int hitkey = 0;
    unsigned int digit = 0;
    int number = 0;
    unsigned int count = 0;
    int sign = 1;

#if 0
    iPrintf("%s: ", msg);
    sign = scanf("%d", &number);
    if(1 != sign) {
    	number = INVALID_INPUT;
    }
    return number;
#else
    iPrintf("\r\n Please input %s: ", msg);
    hitkey = getc(stdin);
    if('\n' == hitkey)
    {
        return INVALID_INPUT;
    }
    else if ('-' == hitkey)
    {
        sign = -1;
    }
    else if(('0'<=hitkey) && ('9'>=hitkey))
    {
        digit = hitkey - '0';
        number = digit;
    }

    do{
          hitkey = getc(stdin);
          count++;
          if(('0'<=hitkey) && ('9'>=hitkey))
          {
              digit = hitkey - '0';
              number = number*10 + digit;
          }
    } while(('\n' != hitkey) && (1000000 > number));

    return (number * sign);
#endif
}

/* Function to get an integer allowing negative value
 * Blocking call to must get a number!
 * */
int64_t get_a_number_mt(const char *msgPromot)
{
    int hitkey = 0;
    unsigned int digit = 0;
    int number = 0;
    unsigned int count = 0;
    int sign = 1;

    iPrintf("\n Please input %s: ", msgPromot);

    do {
    	hitkey = getc(stdin);
#if 0
    	if(-1 != hitkey) {
    		ndPrintf("\n0: Get an input %d / %c\n", hitkey, hitkey);
    		if ('-' == hitkey)
    	    {
    	        sign = -1;
    	        break;
    	    }
    	    else if(('0'<=hitkey) && ('9'>=hitkey))
    	    {
    	        digit = hitkey - '0';
    	        number = digit;
    	        break;
    	    }
    	}
    } while(1);
#else
//    } while( -1 == hitkey);
    } while (('-' != hitkey)
			&& (!(('0' <= hitkey) && ('9' >= hitkey))));

    if ('-' == hitkey)
    {
        sign = -1;
    }
    else if(('0'<=hitkey) && ('9'>=hitkey))
    {
        digit = hitkey - '0';
        number = digit;
    }
#endif

    do{
          hitkey = getc(stdin);
          ndPrintf("\n1: Get an input %d / %c\n", hitkey, hitkey);
          count++;
          if(('0'<=hitkey) && ('9'>=hitkey))
          {
              digit = hitkey - '0';
              number = number*10 + digit;
          }
    } while(('\n' != hitkey) && (1000000 > number));

    ndPrintf("Get number %d and sign %d\n", number, sign);
    return (number * sign);
}

/* Function to get number from Stdio inputs and display the result */
int32_t get_a_number_print(const char *msgPromot)
{
    int32_t number;

    number = get_a_number(msgPromot);
    iPrintf("You input %d\n", number);

    return number;
}

void print_array_byte(uint8_t *dat, uint32_t num)
{
    uint32_t i;

    for(i=0; i<num; i++)
    {
        iPrintf(" %02X", dat[i] & 0xFF);
    }

    return;
}

void print_array(uint16_t *dat, uint32_t num)
{
    uint32_t i;

    for(i=0; i<num; i++)
    {
        iPrintf(" %02X", dat[i] & 0xFF);
    }

    return;
}

float round_float(float x)
{
    uint32_t temp;
    float value;

    temp = (uint32_t)x;
    value = x - temp;

    if(0.5 <= value)
        return (temp+1);
    else
        return temp;
}

/* Function to convert 32-bit integer to hex string */
BOOL_INT32 dec2hex(uint32_t n, char *ans)
{
    // ch variable to store character temporarily
    char ch;
    // i variable to count
    uint32_t i = 0;

    if(0 == n)
    {
        ans[0] = '0'; ans[1]='\0';
        return BOOL_TRUE;
    }

    while (n != 0) {
        // remainder variable to store remainder
        uint32_t rem = 0;

        // storing remainder in rem variable.
        rem = n % 16;

        // check if temp < 10
        if (rem < 10) {
            ch = rem + '0';
        }
        else {
            ch = rem - 10 + 'A';
        }

        // updating the ans string with the character variable
        ans[i++] = ch;
        n = n / 16;
    }

    // reversing the ans string to get the final result
    i = 0;
    uint32_t j;
    j = strlen(ans) - 1;
    while(i < j)
    {
      // swap ans[i] and ans[j]
      ch = ans[i];
      ans[i] = ans[j];
      ans[j] = ch;
      i++;
      j--;
    }

    return BOOL_TRUE;
}

/* Function to convert 32-bit integer to hex string */
char* dec2hex_word(uint32_t n, char *ans)
{
    dec2hex(n, ans);
    return ans;
}

/* Function to convert 8-bit integer to hex string */
char* dec2hex_byte(uint16_t n, char *ans)
{
    uint16_t a, a16;
    char cha, cha16;

    n = n&0xFF;

    a = n%16;
    a16 = n/16;

    if (a < 10) {
        cha = a + '0';
    }
    else {
        cha = a - 10 + 'A';
    }
    if (a16 < 10) {
        cha16 = a16 + '0';
    }
    else {
        cha16 = a16 - 10 + 'A';
    }

    ans[0] = cha16;
    ans[1] = cha;
    ans[2] = '\0';

    return ans;
}

uint16_t bcd2bin(uint16_t value)
{
    union time_bcd_t temp;

    temp.hword = value;
    return temp.bF.ten * 10 + temp.bF.unit;
}

uint16_t bin2bcd(uint16_t value)
{
    union time_bcd_t temp;

    temp.bF.ten = value/10;
    temp.bF.unit = value % 10;

    return temp.hword;
}
