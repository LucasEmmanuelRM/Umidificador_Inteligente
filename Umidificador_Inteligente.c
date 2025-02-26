#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/timer.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "ws2812.pio.h"

#define NUM_PIXELS 25  // Número de pixels da matriz de LEDs 5x5

// Definição dos pinos
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define BUTTON_A 5
#define BUTTON_B 6
#define WS2812 7
#define LED_G 11
#define LED_B 12
#define LED_R 13
#define I2C_SDA 14
#define I2C_SCL 15
#define JOYSTICK_BUTTON 22
#define JOYSTICK_X 26
#define JOYSTICK_Y 27

// UART
#define UART_ID uart0
#define BAUD_RATE 115200

// I2C
#define I2C_ID i2c1
#define I2C_ADDR 0x3C   // Endereço do dispositivo i2c
#define I2C_FREQ 100000 // 100kHz

// Variáveis para PWM
const uint16_t wrap_period = 100;  // Valor máximo do contador - WRAP (Pode ir de 1 a 65535)
const float pwm_div = 255.0;        // Divisor do clock para o PWM (Pode ir de 1,0 a 255,9)
uint16_t duty_cycle = 50;            // Nível inicial do pwm
bool bool_pwm = true;               // Estado dos pwm

// Variáveis globais. Não devem ser alteradas manualmente!
static volatile uint32_t last_time = 0; // Armazena o tempo em microssegundos
ssd1306_t ssd;                          // Inicializa a estrutura do display ssd1306
int nivel_agua = 50;                    // Nivel de Água inicial
int umidade    = 50;                    // Umidade inicial

// Protótipos das funções
static void gpio_irq_handler(uint gpio, uint32_t events);
bool repeating_timer_callback(struct repeating_timer *t);
void WS2812_LEDs(uint sm);
uint32_t matrix_rgb(double r, double b, double g);
void inicializar_perifericos();


int main()
{
    stdio_init_all();
    inicializar_perifericos();

    // Configuração da matriz de LEDs
    uint offset = pio_add_program(pio0, &ws2812_program);
    uint sm = pio_claim_unused_sm(pio0, true);
    ws2812_program_init(pio0, sm, offset, WS2812);

    // Configuração da interrupção com callback
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Checagem de nível da água
    struct repeating_timer timer;
    add_repeating_timer_ms(1000, repeating_timer_callback, NULL, &timer);

    // Configuração do Joystick
    uint16_t x, y;                // Para armazenar o valor atual dos eixos do Joystick (0 ~ 4095)
    uint16_t x_offset, y_offset;  // Para correção de leitura dos eixos do Joystick, nem sempre centralizado = 2048

    adc_select_input(0);
    y_offset = adc_read();

    // Configuração inicial do display
    ssd1306_draw_string(&ssd, "Umidade: ", 0, 16);
    ssd1306_draw_string(&ssd, "%", 36, 24);
    ssd1306_draw_string(&ssd, "Nivel de agua: ", 0, 36);
    ssd1306_draw_string(&ssd, "%", 26, 44);
    ssd1306_send_data(&ssd);

    while (true) {
        char u1 = umidade + '0';
        if (nivel_agua > 0)
            nivel_agua--;

        adc_select_input(0);
        y = adc_read();
        umidade = (y*100)/4095;

        ssd1306_draw_number(&ssd, umidade, 0, 24);
        ssd1306_draw_number(&ssd, nivel_agua, 0, 44);
        ssd1306_send_data(&ssd);

        WS2812_LEDs(sm);

        if (umidade < 30)
            printf("Alerta! Umidade abaixo do ideal: %d%%\n", umidade);
        
        sleep_ms(1000);
    }
}


void gpio_irq_handler(uint gpio, uint32_t events){

    uint32_t current_time = to_us_since_boot(get_absolute_time());

    // Verifica se passou tempo suficiente desde o último evento
    if(current_time - last_time > 300000){ // 300 ms de debouncing
        if(gpio == BUTTON_B){
            nivel_agua += 10;
            if (nivel_agua > 100)
                nivel_agua = 100;
        }
        last_time = current_time; // Atualiza o tempo do último evento
    }

}


bool repeating_timer_callback(struct repeating_timer *t) {
    if (nivel_agua > 50){
        if (umidade >= 50){
            pwm_set_gpio_level(LED_G, 10);
            pwm_set_gpio_level(LED_B, 50);
            pwm_set_gpio_level(LED_R, 10);
        }
        else if (umidade < 50 && umidade >= 30){
            pwm_set_gpio_level(LED_G, 75);
            pwm_set_gpio_level(LED_B, 25);
            pwm_set_gpio_level(LED_R, 25);
        }
        else{
            pwm_set_gpio_level(LED_G, 50);
            pwm_set_gpio_level(LED_B, 50);
            pwm_set_gpio_level(LED_R, 100);
        }     
    }
    else if (nivel_agua <= 50 && nivel_agua > 0){
        pwm_set_gpio_level(LED_G, 25);
        pwm_set_gpio_level(LED_B, 25);
        pwm_set_gpio_level(LED_R, 25);
        printf("Nivel de agua abaixo da metade: %d%%\n", nivel_agua);
    }
    else{
        pwm_set_gpio_level(LED_G, 0);
        pwm_set_gpio_level(LED_B, 0);
        pwm_set_gpio_level(LED_R, 0);
        printf("Reservatorio vazio, funcionamento suspendido!\n");
        printf("Coloque agua para o umidificador voltar a trabalhar!\n");
    }
    return true;
}


void WS2812_LEDs(uint sm){
    
    uint32_t led;

    int n;
    if (nivel_agua >= 95) n = 5;
    else if (nivel_agua >= 75) n = 4;
    else if (nivel_agua >= 50) n = 3;
    else if (nivel_agua >= 25) n = 2;
    else if (nivel_agua > 0) n = 1;
    else n = 0;

    double numero[6][NUM_PIXELS] = {

    // Nível de Água = 0%
    {0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.0, 0.0},

    // Nível de Água = 10%
    {0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.0, 0.0,
     0.1, 0.1, 0.1, 0.1, 0.1},

    // Nível de Água = 25%
    {0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.0, 0.0,
     0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1},

    // Nível de Água = 50%
    {0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.0, 0.0,
     0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1},

    // Nível de Água = 75%
    {0.0, 0.0, 0.0, 0.0, 0.0,
     0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1},

    // Nível de Água = 100%
    {0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1}};


    for (int16_t i = 0; i < NUM_PIXELS; i++){   // Iteração de pixels

        led = matrix_rgb(0, numero[n][24 - i], 0);
        pio_sm_put_blocking(pio0, sm, led);
         
    }
}


uint32_t matrix_rgb(double r, double b, double g){
    unsigned char R, G, B;
    R = r * 255;
    G = g * 255;
    B = b * 255;
    return (G << 24) | (R << 16) | (B << 8);
}


void inicializar_perifericos(){

    // Inicializa os LEDs Verde, Azul e Vermelho como PWM
    gpio_set_function(LED_G, GPIO_FUNC_PWM);     
    uint slice = pwm_gpio_to_slice_num(LED_G); 
    pwm_set_clkdiv(slice, pwm_div);  
    pwm_set_wrap(slice, wrap_period); 
    pwm_set_gpio_level(LED_G, duty_cycle); 
    pwm_set_enabled(slice, true);

    gpio_set_function(LED_B, GPIO_FUNC_PWM);      // Habilitar o pino GPIO como PWM
    slice = pwm_gpio_to_slice_num(LED_B);         // Obter o canal PWM da GPIO
    pwm_set_clkdiv(slice, pwm_div);               // Define o divisor de clock do PWM
    pwm_set_wrap(slice, wrap_period);             // Definir o valor de wrap
    pwm_set_gpio_level(LED_B, duty_cycle);        // Definir o ciclo de trabalho (duty cycle) do pwm
    pwm_set_enabled(slice, true);                 // Habilita o pwm no slice correspondente

    gpio_set_function(LED_R, GPIO_FUNC_PWM);     
    slice = pwm_gpio_to_slice_num(LED_R); 
    pwm_set_clkdiv(slice, pwm_div);  
    pwm_set_wrap(slice, wrap_period); 
    pwm_set_gpio_level(LED_R, duty_cycle); 
    pwm_set_enabled(slice, true);

    // Inicializa o botão A, B e do Joystick
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);          // Habilita o pull-up interno

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);          // Habilita o pull-up interno

    gpio_init(JOYSTICK_BUTTON);
    gpio_set_dir(JOYSTICK_BUTTON, GPIO_IN);
    gpio_pull_up(JOYSTICK_BUTTON); 

    // Inicializa o ADC no Joystick
    adc_init();
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);  

    // Inicializa o I2C
    i2c_init(I2C_ID, I2C_FREQ);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    // Inicializa o SSD1306
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, I2C_ADDR, I2C_ID); // Inicializa o display
    ssd1306_config(&ssd);                                       // Configura o display
    ssd1306_send_data(&ssd);                                    // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Inicializa e configura UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_fifo_enabled(UART_ID, true); // Evita sobrecarga no buffer
    uart_set_hw_flow(UART_ID, false, false);  // Desativa controle de fluxo de hardware
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);  // 8 bits, 1 stop bit, sem paridade

}