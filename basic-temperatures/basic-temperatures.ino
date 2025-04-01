#include <esp_adc_cal.h>

#define SERIES_RESISTOR 10000  // 10kΩ pull-up resistor
#define NOMINAL_RESISTANCE 10000  // 10kΩ at 25°C
#define NOMINAL_TEMPERATURE 25.0  // Temperature at which nominal resistance is measured
#define B_COEFFICIENT 3950  // Beta coefficient of the thermistor
#define ADC_MAX 8191.0
#define REF_VOLTAGE 3.3

#define NUM_THERMISTORS 3
int thermistorPins[NUM_THERMISTORS] = {9, 11, 12};
uint32_t rawValues[NUM_THERMISTORS] = {};
float voltages[NUM_THERMISTORS] = {};
float resistances[NUM_THERMISTORS] = {};
float temperatures[NUM_THERMISTORS] = {};
float averageTemperature = 0;

esp_adc_cal_characteristics_t adc_chars;

void setup() {
    Serial.begin(115200);
    // analogReadResolution(13);
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_13, 1100, &adc_chars);
}

void loop() {
    for (int i = 0; i < NUM_THERMISTORS; i++) {
        rawValues[i] = analogRead(thermistorPins[i]);
        // rawValues[i] = 2047;
        // voltages[i] = (rawValues[i] / ADC_MAX) * REF_VOLTAGE;
        voltages[i] = esp_adc_cal_raw_to_voltage(rawValues[i], &adc_chars) / 1000.0;
        // resistances[i] = SERIES_RESISTOR * (REF_VOLTAGE / voltages[i]); // Voltage divider formula
        // resistances[i] = SERIES_RESISTOR * (voltages[i] / REF_VOLTAGE); // Voltage divider formula
        resistances[i] = (voltages[i]*SERIES_RESISTOR) / (REF_VOLTAGE - voltages[i]);

        // Steinhart-Hart equation
        temperatures[i] = 1.0 / (1.0 / (NOMINAL_TEMPERATURE + 273.15) +
                     (1.0 / B_COEFFICIENT) * log(resistances[i] / NOMINAL_RESISTANCE));
        temperatures[i] -= 273.15; // Convert from Kelvin to Celsius

        // get average temp
        averageTemperature = 0;
        for (int j = 0; j < NUM_THERMISTORS; j++) {
            averageTemperature += temperatures[j];
        }
        averageTemperature = averageTemperature / NUM_THERMISTORS;

        Serial.print("sensor: ");
        Serial.print(i);

        Serial.print(", rawValue: ");
        Serial.print(rawValues[i]);

        Serial.print(", Voltage: ");
        Serial.print(voltages[i]);

        Serial.print(", Resistance: ");
        Serial.print(resistances[i]);

        Serial.print(". Temperature: ");
        Serial.print(temperatures[i]);
        Serial.println(" °C");
    }

    delay(1000);
}
