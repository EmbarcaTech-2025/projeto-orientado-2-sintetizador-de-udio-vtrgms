#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "pico/time.h"

// Configurações de áudio
#define SAMPLE_RATE        8000    // 8 kHz (taxa de amostragem)
#define GAIN               2.0f    // Amplificação do volume (1.0 = normal)
#define DC_OFFSET          1.65f   // Offset DC para correção (1.65V para 3.3V)
#define BUFFER_DURATION_MS 2000    // Duração do buffer em milissegundos
#define BUFFER_SIZE (SAMPLE_RATE * BUFFER_DURATION_MS / 1000)
#define SAMPLE_INTERVAL_US (1000000 / SAMPLE_RATE)

// Configuração dos pinos
#define MIC_CHANNEL 2
#define MIC_PIN (26 + MIC_CHANNEL)
#define PIN_BUTTON_A 5
#define PIN_BUTTON_B 6
#define BUZZER_PIN 21
#define RED_LED_PIN 13
#define GREEN_LED_PIN 11

// Estados do sistema
enum SystemState {
    IDLE,
    RECORDING,
    PLAYING
};

// Variáveis
uint16_t record_buffer[BUFFER_SIZE];
uint8_t play_buffer[BUFFER_SIZE];
volatile enum SystemState system_state = IDLE;
volatile uint32_t samples_recorded = 0;
uint32_t last_sample_time; // Variáveis de tempo
uint32_t play_index = 0;
uint pwm_slice; // Configuração PWM


// Inicializa o buzzer com PWM
void pwm_init_buzzer() {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    pwm_slice = pwm_gpio_to_slice_num(BUZZER_PIN);
    
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 1.0f);
    pwm_config_set_wrap(&config, 255);
    pwm_init(pwm_slice, &config, true);
    pwm_set_gpio_level(BUZZER_PIN, 0);
}

// Começa a gravação
void start_recording() {
    // Configuração do ADC
    adc_run(false);
    adc_fifo_drain();
    adc_select_input(MIC_CHANNEL);
    
    // Calcula divisor de clock para taxa de amostragem desejada
    float clock_div = 48000000.0f / (SAMPLE_RATE * 96.0f);
    adc_set_clkdiv(clock_div);
    
    system_state = RECORDING;
    samples_recorded = 0;
    last_sample_time = time_us_32();
    adc_run(true);
    
    printf("Iniciando gravacao...\n");
    printf("Taxa: %d Hz | Ganho: %.1f | Buffer: %d amostras\n", 
           SAMPLE_RATE, GAIN, BUFFER_SIZE);
    gpio_put(RED_LED_PIN, 1);
}

// Guarda a gravação
void record_samples() {
    if (time_us_32() - last_sample_time >= SAMPLE_INTERVAL_US) {
        last_sample_time += SAMPLE_INTERVAL_US;
        
        if (samples_recorded < BUFFER_SIZE) {
            record_buffer[samples_recorded] = adc_read();
            samples_recorded++;
        } else {
            system_state = IDLE;
            printf("Gravacao completa! Amostras: %lu\n", samples_recorded);
        }
    }
}

// Para a gravação
void stop_recording() {
    adc_run(false);
    system_state = IDLE;
    printf("Gravacao interrompida. Amostras: %lu\n", samples_recorded);
    gpio_put(RED_LED_PIN, 0);
}

// Começa a reprodução
void start_playback() {
    if (samples_recorded == 0) {
        printf("Nada para reproduzir!\n");
        return;
    }

    // Processamento das amostras
    for (uint32_t i = 0; i < samples_recorded; i++) {
        float sample_voltage = record_buffer[i] * 3.3f / 4095.0f;
        float processed_sample = (sample_voltage - DC_OFFSET) * GAIN; // Remove DC offset e aplica ganho
        
        processed_sample += DC_OFFSET;// Normaliza para 0-3.3V
        
        // Converte para valor PWM (0-255)
        if (processed_sample > 3.3f) processed_sample = 3.3f;
        else if (processed_sample < 0) processed_sample = 0;
        
        play_buffer[i] = (uint8_t)(processed_sample / 3.3f * 255.0f);
    }

    system_state = PLAYING;
    play_index = 0;
    last_sample_time = time_us_32();
    printf("Iniciando reproducao...\n");
    gpio_put(GREEN_LED_PIN, 1);
}

// Reproduz a gravação
void play_samples() {
    if (time_us_32() - last_sample_time >= SAMPLE_INTERVAL_US) {
        last_sample_time += SAMPLE_INTERVAL_US;
        
        if (play_index < samples_recorded) {
            pwm_set_gpio_level(BUZZER_PIN, play_buffer[play_index]);
            play_index++;
        } else {
            pwm_set_gpio_level(BUZZER_PIN, 0);
            system_state = IDLE;
            printf("Reproducao concluida\n");
            gpio_put(GREEN_LED_PIN, 0);
        }
    }
}

int main() {
    stdio_init_all();
    
    // Inicialização de GPIO
    gpio_init(PIN_BUTTON_A);
    gpio_set_dir(PIN_BUTTON_A, GPIO_IN);
    gpio_pull_up(PIN_BUTTON_A);

    gpio_init(PIN_BUTTON_B);
    gpio_set_dir(PIN_BUTTON_B, GPIO_IN);
    gpio_pull_up(PIN_BUTTON_B);

    gpio_init(RED_LED_PIN);
    gpio_init(GREEN_LED_PIN);
    gpio_set_dir(RED_LED_PIN, GPIO_OUT);
    gpio_set_dir(GREEN_LED_PIN, GPIO_OUT);

    // Inicialmente, desligar o LED RGB
    gpio_put(RED_LED_PIN, 0);
    gpio_put(GREEN_LED_PIN, 0);

    // Inicialização do ADC
    adc_gpio_init(MIC_PIN);
    adc_init();
    adc_select_input(MIC_CHANNEL);
    adc_fifo_setup(true, false, 1, false, false);

    // Inicialização do PWM
    pwm_init_buzzer();

    
    sleep_ms(5000);// Delay para o usuário abrir o monitor serial...
    
    printf("\n===== Sintetizador de Áudio =====\n");
    printf("Configuracao atual:\n");
    printf("- Taxa de amostragem: %d Hz\n", SAMPLE_RATE);
    printf("- Ganho de audio: %.1fx\n", GAIN);
    printf("- Offset DC: %.2fV\n", DC_OFFSET);
    printf("- Duracao do buffer: %dms (%d amostras)\n\n", BUFFER_DURATION_MS, BUFFER_SIZE);
    printf("Instrucoes:\n");
    printf("1. Pressione BOTAO A para gravar\n");
    printf("2. Pressione BOTAO B para reproduzir\n\n");

    while (true) {
        // Controle de estados
        switch(system_state) {
            case RECORDING:
                record_samples();
                
                // Verifica se o botão foi solto
                if (gpio_get(PIN_BUTTON_A)) {
                    stop_recording();
                }
                break;
                
            case PLAYING:
                play_samples();
                break;
                
            case IDLE:
            default:
                // Iniciar gravação quando botão A pressionado
                if (!gpio_get(PIN_BUTTON_A)) {
                    start_recording();
                }
                // Iniciar reprodução quando botão B pressionado
                else if (!gpio_get(PIN_BUTTON_B)) {
                    start_playback();
                }
                break;
        }

        sleep_us(10);
    }
}
