#pragma once

#include <M5Unified.h>

enum class CharacterState : uint8_t {
  Spawning,
  Sleeping,
  Waking,
  Idle,
  Searching,
  Working,
  Curious,
  Happy,
  Excited,
  Surprised,
  Suspicious,
  Angry,
  Drowsy,
  Laughing,
  Scared,
  Playful,
  Proud,
  Shy,
  Sad,
  Confused,
  Bored,
  Listening,
  Thinking,
  Progress,
  PoweringDown,
};

class CharacterRenderer {
 public:
  struct VisualConfig {
    uint8_t eyes[6];
    uint8_t eyeCount;
    uint16_t eyeHoldMs;
  };

  void begin(M5GFX* display);
  void setState(CharacterState state, uint32_t now);
  CharacterState state() const { return state_; }

  void setGaze(float x, float y);
  void setDrag(float x, float y);
  void setProgress(float value);
  void triggerSpin(int8_t direction, uint8_t turns, uint32_t now);
  void triggerHop(uint32_t now);
  void triggerShake(uint32_t now);
  void triggerWink(uint8_t eye, uint32_t now);
  void update(uint32_t now, float dt);
  bool render(uint32_t now);

  float framesPerSecond() const { return fps_; }

 private:
  static const VisualConfig& configFor(CharacterState state);
  static float clamp01(float value);
  static float easeOutBack(float value);

  void chooseNextEye(uint32_t now);
  void drawEyes(float seconds);
  void transformPoint(float sourceX, float sourceY, int16_t& x, int16_t& y) const;

  M5GFX* display_ = nullptr;
  CharacterState state_ = CharacterState::Spawning;
  uint32_t stateAt_ = 0;
  uint32_t lastUpdateAt_ = 0;
  uint32_t lastFrameAt_ = 0;
  uint32_t nextEyeAt_ = 0;
  uint32_t nextBlinkAt_ = 0;
  uint32_t blinkAt_ = 0;
  uint32_t spinAt_ = 0;
  uint32_t hopAt_ = 0;
  uint32_t shakeAt_ = 0;
  uint32_t winkAt_ = 0;
  uint32_t fpsWindowAt_ = 0;
  uint16_t framesInWindow_ = 0;
  float fps_ = 0.0f;
  int8_t spinDirection_ = 1;
  uint8_t spinTurns_ = 1;
  uint8_t winkEye_ = 0;

  uint8_t eyeFrom_ = 0;
  uint8_t eyeTo_ = 0;
  uint8_t eyeIndex_ = 0;
  float eyeBlend_ = 1.0f;
  float gazeX_ = 0.0f;
  float gazeY_ = 0.0f;
  float gazeTargetX_ = 0.0f;
  float gazeTargetY_ = 0.0f;
  float dragX_ = 0.0f;
  float dragY_ = 0.0f;
  float dragTargetX_ = 0.0f;
  float dragTargetY_ = 0.0f;
  float progress_ = 0.0f;

  float centerX_ = 0.0f;
  float centerY_ = 0.0f;
  float scale_ = 1.0f;
  float scaleX_ = 1.0f;
  float scaleY_ = 1.0f;
  float rotation_ = 0.0f;
  float cosRotation_ = 1.0f;
  float sinRotation_ = 0.0f;
  float eyeOpen_ = 1.0f;
  float eyeScaleBoost_ = 1.0f;

  uint16_t bodyColor_ = 0;
  uint16_t eyeColor_ = 0;
};
