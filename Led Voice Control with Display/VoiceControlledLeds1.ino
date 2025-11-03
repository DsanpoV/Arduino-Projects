/* MicroSpeech + NeoPixel + LCD I2C
   Fita de 8 LEDs RGB no pino D11
   LCD I2C (16x2) mostra o comando reconhecido e a cor
*/

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// TinyML / TensorFlow Lite Micro
#include <Arduino_TensorFlowLite.h>
#include "MicroSpeech/micro_features_micro_model_settings.h"
#include "MicroSpeech/micro_model_settings.h"
#include "MicroSpeech/recognize_commands.h"
#include "MicroSpeech/audio_provider.h"
#include "MicroSpeech/command_responder.h"

#define LED_PIN 11
#define NUM_LEDS 8

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Ajusta se necessário

// ---------- TinyML globals ----------
namespace {
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* model_input = nullptr;
  FeatureProvider* feature_provider = nullptr;
  RecognizeCommands* recognizer = nullptr;
  int32_t previous_time = 0;

  constexpr int kTensorArenaSize = 20 * 1024;
  alignas(16) uint8_t tensor_arena[kTensorArenaSize];
  int8_t feature_buffer[kFeatureElementCount];
  int8_t* model_input_buffer = nullptr;

  // Controle de LEDs
  String last_command = "";
}

// ---------- Atualizar LEDs ----------
void update_leds_effect() {
  if (last_command == "YES") {  // verde
    for (int i = 0; i < NUM_LEDS; i++)
      strip.setPixelColor(i, strip.Color(0, 255, 0));
    strip.show();
  }
  else if (last_command == "NO") {  // vermelho
    for (int i = 0; i < NUM_LEDS; i++)
      strip.setPixelColor(i, strip.Color(255, 0, 0));
    strip.show();
  }
}

// ---------- Resposta ao comando ----------
void MyRespondToCommand(int32_t current_time, const char* found_command,
                        uint8_t score, bool is_new_command) {
  if (!is_new_command) return;

  String cmd = String(found_command);
  String display_text = "";

  if (cmd == "YES") display_text = "VERDE";
  else if (cmd == "NO") display_text = "VERMELHO";

  if (display_text != "") {
    last_command = cmd;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Comando:");
    lcd.setCursor(0, 1);
    lcd.print(display_text);
  }
}

// ---------- Setup ----------
void setup() {
  tflite::InitializeTarget();

  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(50);
  strip.show();

  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Aguardando...");

  // Carregar modelo original do Arduino MicroSpeech
  model = tflite::GetModel(g_micro_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model version mismatch");
    return;
  }

  static tflite::AllOpsResolver micro_op_resolver;
  static tflite::MicroInterpreter static_interpreter(model, micro_op_resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("AllocateTensors() failed");
    return;
  }

  model_input = interpreter->input(0);
  model_input_buffer = model_input->data.int8;

  static FeatureProvider static_feature_provider(kFeatureElementCount, feature_buffer);
  feature_provider = &static_feature_provider;

  static RecognizeCommands static_recognizer;
  recognizer = &static_recognizer;

  previous_time = 0;

  if (InitAudioRecording() != kTfLiteOk) {
    Serial.println("Unable to initialize audio");
    return;
  }

  Serial.println("Setup complete. Speak 'yes' or 'no'.");
}

// ---------- Loop ----------
void loop() {
  const int32_t current_time = LatestAudioTimestamp();
  int how_many_new_slices = 0;
  if (feature_provider->PopulateFeatureData(previous_time, current_time, &how_many_new_slices) != kTfLiteOk) {
    Serial.println("Feature generation failed");
    return;
  }

  previous_time += how_many_new_slices * kFeatureSliceStrideMs;

  if (how_many_new_slices > 0) {
    for (int i = 0; i < kFeatureElementCount; i++)
      model_input_buffer[i] = feature_buffer[i];

    if (interpreter->Invoke() != kTfLiteOk)
      Serial.println("Invoke failed");

    TfLiteTensor* output = interpreter->output(0);
    const char* found_command = nullptr;
    uint8_t score = 0;
    bool is_new_command = false;

    if (recognizer->ProcessLatestResults(output, current_time, &found_command, &score, &is_new_command) != kTfLiteOk)
      Serial.println("ProcessLatestResults failed");
    else
      MyRespondToCommand(current_time, found_command, score, is_new_command);
  }

  if (last_command != "") update_leds_effect();
}

