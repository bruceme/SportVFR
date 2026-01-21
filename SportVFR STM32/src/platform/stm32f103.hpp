// Pins...
#define TACH_PIN PA11
#define ALERT_PIN PA12
#define FUEL_FLOW_PIN PA15
#define LCD_EN_PIN PB3
#define LCD_D4_PIN PB4
#define LCD_D5_PIN PB5    
#define LCD_D6_PIN PB8
#define LCD_D7_PIN PB9
#define LCD_RS_PIN PB10

#define SCL_PIN PB8
#define SDA_PIN PB9 

#define ENC_A_PIN PA8
#define ENC_B_PIN PA9
#define BUTTON_PIN PA10

#define FQ2_PIN PB1
#define FQ1_PIN PB0
#define FP_PIN AUX1_PIN // PA7 - is an amplified line, I want to just read the raw voltage, AUX1
#define OT_PIN PA5 // this is marked OP on schematic, but I need to swap the amplifier output for a raw line
#define OP_PIN PA6 // this is marked OT on schematic, it is a raw-line with input at voltage 0-3.3v on pressure
#define OAT_PIN PA4
#define AUX4_PIN PA3
#define AUX3_PIN PA2
#define AUX2_PIN PA1
#define AUX1_PIN PA0 // used for FP..