#include <Wire.h>
#include <Adafruit_VL53L0X.h>

// Sensors
Adafruit_VL53L0X sensor1 = Adafruit_VL53L0X();
Adafruit_VL53L0X sensor2 = Adafruit_VL53L0X();
Adafruit_VL53L0X sensor3 = Adafruit_VL53L0X();

// XSHUT pins
#define XSHUT1 2  // D2
#define XSHUT2 4  // D4
#define XSHUT3 5  // D5

// I2C addresses
#define ADDR1 0x30
#define ADDR2 0x31
#define ADDR3 0x32

// Filtering & timing
const int N_READS = 5;           // median of 5 samples
const int SAMPLE_DELAY_MS = 20;  // delay between samples
const uint32_t TIMING_BUDGET_US = 33000; // adjust between 20000(20ms) and 200000(200ms). Higher value = Higher accuracy

// 5-SEGMENT PIECEWISE LUT (slope + offset for each)- Adjust according to your values in calibration
const uint16_t seg_limits[] = {150, 300, 500, 800}; // segment thresholds (raw sensor values in mm)- Adjust according to your required ranges in mm
// s1 calibration lookup
float s1_slope[]   = {0.9417, 1.0312, 1.2049, 1.0981, 0.9982};
float s1_offset[]  = {-14.7, -41.2, -120.3, -71.2, -42.6};

// s2 calibration lookup
float s2_slope[]   = {0.9442, 0.9955, 1.1186, 1.0655, 0.9006};
float s2_offset[]  = {-3.9, -10.8, -73.3, -29.9, 40.2};

// s3 calibration lookup
float s3_slope[]   = {0.9405, 1.0314, 1.4450, 1.0428, 0.9026};
float s3_offset[]  = {-9.7, -27.6, -191.5, -43.5, 69.2};


void setup() {
  Serial.begin(115200);
  Wire.begin();

  pinMode(XSHUT1, OUTPUT); pinMode(XSHUT2, OUTPUT); pinMode(XSHUT3, OUTPUT);

  // Shutdown all sensors initially
  digitalWrite(XSHUT1, LOW);
  digitalWrite(XSHUT2, LOW);
  digitalWrite(XSHUT3, LOW);
  delay(50);

  // Init sensor 1
  digitalWrite(XSHUT1, HIGH); delay(50);
  if (!sensor1.begin()) { Serial.println("Sensor1 failed"); while(1); }
  sensor1.setAddress(ADDR1);
  sensor1.setMeasurementTimingBudgetMicroSeconds(TIMING_BUDGET_US);

  // Init sensor 2
  digitalWrite(XSHUT2, HIGH); delay(50);
  if (!sensor2.begin()) { Serial.println("Sensor2 failed"); while(1); }
  sensor2.setAddress(ADDR2);
  sensor2.setMeasurementTimingBudgetMicroSeconds(TIMING_BUDGET_US);

  // Init sensor 3
  digitalWrite(XSHUT3, HIGH); delay(50);
  if (!sensor3.begin()) { Serial.println("Sensor3 failed"); while(1); }
  sensor3.setAddress(ADDR3);
  sensor3.setMeasurementTimingBudgetMicroSeconds(TIMING_BUDGET_US);

  Serial.println("Sensors initialized. Using 5-segment calibration LUT.");
}

// Median helper
uint16_t medianOfArray(uint16_t arr[], int n) {
  for (int i = 1; i < n; ++i) {
    uint16_t key = arr[i];
    int j = i - 1;
    while (j >= 0 && arr[j] > key) {
      arr[j + 1] = arr[j];
      j--;
    }
    arr[j + 1] = key;
  }
  return arr[n/2];
}

// Read sensor and filter
uint16_t readMedian(Adafruit_VL53L0X &sensor) {
  VL53L0X_RangingMeasurementData_t measure;
  uint16_t samples[N_READS];

  for (int i = 0; i < N_READS; ++i) {
    sensor.rangingTest(&measure, false);
    if (measure.RangeStatus != 4) samples[i] = measure.RangeMilliMeter;
    else samples[i] = 0xFFFF;
    delay(SAMPLE_DELAY_MS);
  }
  uint16_t med = medianOfArray(samples, N_READS);
  return (med == 0xFFFF ? 0 : med);
}

// LUT calibration
// returns calibrated value in mm
float calibrate(uint16_t raw, char sensorID) {
  if (raw == 0) return -1.0; // invalid

  int seg = 0;
  while (seg < 4 && raw > seg_limits[seg]) seg++;

  float slope = 1.0, offset = 0.0;

  if (sensorID == '1') {
    slope = s1_slope[seg];
    offset = s1_offset[seg];
  } else if (sensorID == '2') {
    slope = s2_slope[seg];
    offset = s2_offset[seg];
  } else if (sensorID == '3') {
    slope = s3_slope[seg];
    offset = s3_offset[seg];
  }

  return raw * slope + offset;
}

// --------- Main loop ---------
void loop() {
  uint16_t raw1 = readMedian(sensor1);
  uint16_t raw2 = readMedian(sensor2);
  uint16_t raw3 = readMedian(sensor3);

  float corr1 = calibrate(raw1, '1');
  float corr2 = calibrate(raw2, '2');
  float corr3 = calibrate(raw3, '3');

  Serial.print("S1 raw: "); Serial.print(raw1);
  if (corr1 >= 0) { Serial.print("  corr: "); Serial.print(corr1, 1); Serial.print(" mm"); } else Serial.print("  out");
  Serial.print(" \t | ");

  Serial.print("S2 raw: "); Serial.print(raw2);
  if (corr2 >= 0) { Serial.print("  corr: "); Serial.print(corr2, 1); Serial.print(" mm"); } else Serial.print("  out");
  Serial.print(" \t | ");

  Serial.print("S3 raw: "); Serial.print(raw3);
  if (corr3 >= 0) { Serial.print("  corr: "); Serial.print(corr3, 1); Serial.print(" mm"); } else Serial.print("  out");
  Serial.println();

  Serial.println("-------------------------------");
  delay(50);
}
