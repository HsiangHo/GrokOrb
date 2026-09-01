#pragma once

#include <M5Unified.h>

#include "CharacterRenderer.h"

class AppController {
 public:
  void begin();
  void update();

 private:
  void enterState(CharacterState state, uint32_t durationMs = 0);
  void updateInputs(uint32_t now);
  void updateState(uint32_t now);
  void triggerStateMotion(CharacterState state, uint32_t now);
  void pulseHaptic(uint8_t level, uint16_t durationMs, uint32_t now);
  void printDiagnostics();

  CharacterRenderer renderer_;
  CharacterState state_ = CharacterState::Spawning;
  uint32_t stateUntil_ = 0;
  uint32_t progressAt_ = 0;
  uint32_t lastLoopAt_ = 0;
  uint32_t lastLogAt_ = 0;
  uint32_t nextShowcaseAt_ = 0;
  uint32_t vibrationOffAt_ = 0;
  uint8_t showcaseIndex_ = 0;
  uint8_t lifecycleIndex_ = 0;
  bool touchActive_ = false;
  bool showcaseMode_ = false;
};
