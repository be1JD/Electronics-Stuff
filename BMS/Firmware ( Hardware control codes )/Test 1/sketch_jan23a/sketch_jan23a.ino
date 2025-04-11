

const int batt_volt_1 = A1;  // volt of battery 1
const int batt_volt_2 = A2;  // volt of battery 2
const int batt_volt_3 = A3;  // volt of battery 3
const int batt_curr = A0;    // output current
const int balance_v1 = 2;
const int balance_v2 = 3;
const int balance_v3 = 4;
const int power_output = 5;
const int charg_in = 6;
const float charging_cuttoff = 4.30;  //Set for which voltage charging will be cut off
const float indi_minvolt = 3.5;       // min indi voltage
const float indi_maxvolt = 4.2;       // max indi voltage
const float minVoltage = 11.1;        // 0% charge
const float maxVoltage = 12.6;        // 100% charge

float voltage1, voltage2, voltage3, voltage, current, power;
int charge;

// Function to calculate the battery percentage
float calculatePercentage(float voltage) {
  if (voltage <= minVoltage) {
    return 0.0;
  } else if (voltage >= maxVoltage) {
    return 100.0;
  } else {
    return ((voltage - minVoltage) / (maxVoltage - minVoltage)) * 100.0;
  }
}

// Function to average analog readings
float Averagevalue(int Inputpin) {
  long sum = 0;
  for (int i = 0; i < 40; i++) {
    int value = analogRead(Inputpin);  // Read the analog value
    sum += value;                      // Add to sum
    delay(5);                          // Small delay to allow for settling
  }
  return sum / 40.0;  // Return the average value
}

void setup() {
  Serial.begin(115200);

  // pinMode(batt_volt_1, INPUT);
  // pinMode(batt_volt_2, INPUT);
  // pinMode(batt_volt_3, INPUT);
  // pinMode(batt_curr, INPUT);
  pinMode(balance_v1, OUTPUT);
  pinMode(balance_v2, OUTPUT);
  pinMode(balance_v3, OUTPUT);
  pinMode(power_output, OUTPUT);

  // Initialize balance pins to LOW
  digitalWrite(balance_v1, LOW);
  digitalWrite(balance_v2, LOW);
  digitalWrite(balance_v3, LOW);
  digitalWrite(power_output, LOW);
}

void loop() {

  //Reading current before voltage to avoid current interferance because of voltage monitoring
  float currentRaw = Averagevalue(batt_curr);

  // To Turn of all voltage interferance system
  digitalWrite(balance_v1, LOW);
  digitalWrite(balance_v2, LOW);
  digitalWrite(balance_v3, LOW);
  // digitalWrite(power_output, LOW); // We have to turn off power to get accurate voltage that is not good for power output

  // Code for getting accurate analog value of voltage
  float voltageRaw1 = Averagevalue(batt_volt_1);
  float voltageRaw2 = Averagevalue(batt_volt_2);
  float voltageRaw3 = Averagevalue(batt_volt_3);


  // Convert raw values to voltage and current
  voltage1 = (voltageRaw1 / 424.30) * 4.24;                             // New formula for arduino
  voltage2 = ((voltageRaw2 / 301.90) * 8.40) - voltage1;                // New formula for arduino
  voltage3 = ((voltageRaw3 / 422.20) * 12.53) - (voltage1 + voltage2);  // New formula for arduino
  voltage = voltage1 + voltage2 + voltage3;
  current = (currentRaw / 103.10) * 0.69;  // New formula for arduino
  power = (voltage1 + voltage2 + voltage3) * current;
  charge = calculatePercentage(voltage1 + voltage2 + voltage3);

  // Print values for debugging
  // Serial.println(currentRaw);
  Serial.println(current);
  // Serial.println(voltageRaw1);
  // Serial.println(voltageRaw2);
  // Serial.println(voltageRaw3);
  Serial.println(voltage1);
  Serial.println(voltage2);
  Serial.println(voltage3);
  Serial.println("Done");

  // Cell Balancing of Cells, It will create 2 seconds delay if executed
  if (voltage1 >= charging_cuttoff || voltage2 >= charging_cuttoff || voltage3 >= charging_cuttoff) {
    if (voltage1 >= charging_cuttoff) {
      Serial.println("The V1 balancing is runing");
      digitalWrite(balance_v1, HIGH);
    }
    if (voltage2 >= charging_cuttoff) {
      Serial.println("The V2 balancing is runing");
      digitalWrite(balance_v2, HIGH);
    }
    if (voltage3 >= charging_cuttoff) {
      Serial.println("The V3 balancing is runing");
      digitalWrite(balance_v3, HIGH);
    }
    // This delay is for battery discharge
    delay(2000);
  }

  // Charging cut-off code
  if (voltage1 >= charging_cuttoff && voltage2 >= charging_cuttoff && voltage3 >= charging_cuttoff) {
    digitalWrite(charg_in, LOW);
  }
  else{
    digitalWrite(charg_in, HIGH);
  }

  //Output control code
  if (voltage1 <= indi_minvolt || voltage2 <= indi_minvolt || voltage3 <= indi_minvolt) {
    digitalWrite(power_output, LOW);
  } else if (voltage1 >= indi_maxvolt || voltage2 >= indi_maxvolt || voltage3 >= indi_maxvolt) {
    digitalWrite(power_output, LOW);
  } else {
    digitalWrite(power_output, HIGH);  // Turn output On if everything is okay
  }
}
