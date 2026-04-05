#include <reg51.h> 
#include <string.h> 
 
#define LCD_PORT P0 
sbit RS = P2^0; 
sbit RW = P2^1; 
sbit EN = P2^2; 
sbit BUZZER = P2^3; 
 
// Known Tag UIDs 
code char tag1[11] = "0005803672"; 
code char tag2[11] = "0004911024"; 
 
// Global variables 
unsigned char received_tag[12]; 
unsigned char index = 0; 
float balance = 100.00; 
bit tag_ready = 0; 
bit lcd_busy = 0; 
 
// Function prototypes 
void lcd_cmd(unsigned char cmd); 
void lcd_data(unsigned char dat); 
void lcd_init(void); 
void lcd_string(char *s); 
void delay(unsigned int ms); 
15 
 
void process_tag(void); 
void beep(void); 
bit compare_tag(const char *valid_tag); 
void hardware_init(void); 
 
// Simplified hardware init 
void hardware_init(void) { 
    EA = 0;          // Disable interrupts during setup 
    delay(100);      // Power stabilization delay 
} 
 
// Serial ISR 
void serial_isr(void) interrupt 4 { 
    if(RI) { 
        received_tag[index++] = SBUF; 
        RI = 0; 
        if(index >= 10) { 
            received_tag[10] = '\0'; 
            index = 0; 
            tag_ready = 1; 
        } 
    } 
} 
 
void main(void) { 
    // Hardware initialization 
    hardware_init(); 
     
16 
 
    // UART setup 
    TMOD = 0x20;    // Timer1, Mode 2 
    TH1 = 0xFD;     // 9600 baud 
    SCON = 0x50;    // Serial Mode 1, REN=1 
    TR1 = 1;        // Start Timer1 
    IE = 0x90;      // Enable serial interrupt 
     
    // LCD initialization 
    lcd_init(); 
    lcd_string(" RFID Toll System "); 
    delay(1000); 
    lcd_cmd(0x01);  // Clear display 
    lcd_string(" Scan Tag... "); 
     
    // Main loop 
    while(1) { 
        if(tag_ready) { 
            process_tag(); 
            tag_ready = 0; 
        } 
    } 
} 
 
void process_tag(void) { 
    while(lcd_busy); // Wait if LCD is busy 
    lcd_busy = 1; 
     
    lcd_cmd(0x01);  // Clear display 
17 
 
     
    if(compare_tag(tag1) || compare_tag(tag2)) { 
        lcd_string("VALID TAG:"); 
        lcd_string(received_tag); 
         
        // Deduct toll and update balance 
        balance -= 10.00; 
        lcd_cmd(0xC0); // Move to second line 
        lcd_string("Bal:Rs."); 
        lcd_data(((unsigned char)(balance/10)) + 48); // Tens digit 
        lcd_data('.'); 
        lcd_data(((unsigned char)balance%10) + 48);  // Units digit 
    } else { 
        lcd_string("INVALID TAG!"); 
        lcd_cmd(0xC0); 
        lcd_string(received_tag); 
    } 
     
    beep(); 
    delay(3000); 
    lcd_cmd(0x01); 
    lcd_string(" Scan Tag... "); 
     
    lcd_busy = 0; 
} 
 
bit compare_tag(const char *valid_tag) { 
    unsigned char i; 
18 
 
    for(i=0; i<10; i++) { 
        if(received_tag[i] != valid_tag[i]) 
            return 0; 
    } 
    return 1; 
} 
 
void beep(void) { 
    BUZZER = 1; 
    delay(100); 
    BUZZER = 0; 
} 
 
// LCD Functions 
void lcd_cmd(unsigned char cmd) { 
    LCD_PORT = cmd; 
    RS = 0; 
    RW = 0; 
    EN = 1; 
    delay(5); 
    EN = 0; 
} 
 
void lcd_data(unsigned char dat) { 
    LCD_PORT = dat; 
    RS = 1; 
    RW = 0; 
    EN = 1; 
19 
 
    delay(5); 
    EN = 0; 
} 
 
void lcd_init(void) { 
    delay(50);  // Power-on delay 
    lcd_cmd(0x38);  // 8-bit mode, 2 lines 
    lcd_cmd(0x0C);  // Display on, cursor off 
    lcd_cmd(0x01);  // Clear display 
    lcd_cmd(0x80);  // Set cursor to start 
} 
 
void lcd_string(char *s) { 
    while(*s) { 
        lcd_data(*s++); 
    } 
} 
 
void delay(unsigned int ms) { 
    unsigned int i,j; 
    for(i=0;i<ms;i++) 
        for(j=0;j<1275;j++); 
}