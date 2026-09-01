#include "CharacterRenderer.h"

#include <math.h>

#include "GrokGeometry.generated.h"

namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr uint32_t kFrameIntervalMs = 33;

const CharacterRenderer::VisualConfig kSpawning = {{3, 0, 0, 0, 0}, 2, 380};
const CharacterRenderer::VisualConfig kSleeping = {{13, 22, 4, 0, 0, 0}, 3, 1300};
const CharacterRenderer::VisualConfig kWaking = {{13, 3, 12, 0, 7, 0}, 5, 430};
const CharacterRenderer::VisualConfig kIdle = {{0, 8, 0, 0, 0}, 2, 4200};
const CharacterRenderer::VisualConfig kSearching = {{15, 9, 3, 20, 12, 18}, 6, 430};
const CharacterRenderer::VisualConfig kWorking = {{7, 16, 11, 10, 0, 0}, 4, 520};
const CharacterRenderer::VisualConfig kCurious = {{3, 21, 0, 15, 0}, 4, 620};
const CharacterRenderer::VisualConfig kHappy = {{2, 11, 17, 19, 0}, 4, 650};
const CharacterRenderer::VisualConfig kExcited = {{2, 17, 21, 3, 11}, 5, 380};
const CharacterRenderer::VisualConfig kSurprised = {{3, 21, 0, 0, 0}, 2, 620};
const CharacterRenderer::VisualConfig kSuspicious = {{14, 5, 23, 0, 0}, 3, 760};
const CharacterRenderer::VisualConfig kAngry = {{7, 16, 0, 0, 0}, 2, 520};
const CharacterRenderer::VisualConfig kDrowsy = {{4, 22, 13, 0, 0}, 3, 900};
const CharacterRenderer::VisualConfig kLaughing = {{2, 11, 17, 0, 0}, 3, 330};
const CharacterRenderer::VisualConfig kScared = {{3, 21, 0, 0, 0}, 2, 330};
const CharacterRenderer::VisualConfig kPlayful = {{2, 17, 11, 8, 0}, 4, 450};
const CharacterRenderer::VisualConfig kProud = {{15, 8, 2, 0, 0}, 3, 700};
const CharacterRenderer::VisualConfig kShy = {{0, 24, 13, 0, 0}, 3, 720};
const CharacterRenderer::VisualConfig kSad = {{4, 13, 22, 0, 0}, 3, 780};
const CharacterRenderer::VisualConfig kConfused = {{14, 5, 8, 0, 0}, 3, 620};
const CharacterRenderer::VisualConfig kBored = {{4, 22, 0, 0, 0}, 3, 850};
const CharacterRenderer::VisualConfig kListening = {{10, 1, 19, 0, 0}, 3, 900};
const CharacterRenderer::VisualConfig kThinking = {{8, 16, 14, 17, 5}, 5, 720};
const CharacterRenderer::VisualConfig kProgress = {{0, 8, 0, 0, 0}, 2, 1700};
const CharacterRenderer::VisualConfig kPoweringDown = {{13, 22, 13, 0, 0}, 3, 900};

float lerpFloat(float from, float to, float amount) {
  return from + (to - from) * amount;
}

}  // namespace

void CharacterRenderer::begin(M5GFX* display) {
  display_ = display;
  bodyColor_ = display_->color565(0, 0, 0);
  eyeColor_ = display_->color565(255, 255, 255);
  display_->setAutoDisplay(false);
  display_->fillScreen(bodyColor_);
  display_->display();

  const uint32_t now = millis();
  eyeFrom_ = eyeTo_ = kSpawning.eyes[0];
  stateAt_ = now;
  nextEyeAt_ = now + 450;
  nextBlinkAt_ = now + 2600;
  fpsWindowAt_ = now;
}

const CharacterRenderer::VisualConfig& CharacterRenderer::configFor(CharacterState state) {
  switch (state) {
    case CharacterState::Spawning: return kSpawning;
    case CharacterState::Sleeping: return kSleeping;
    case CharacterState::Waking: return kWaking;
    case CharacterState::Searching: return kSearching;
    case CharacterState::Working: return kWorking;
    case CharacterState::Curious: return kCurious;
    case CharacterState::Happy: return kHappy;
    case CharacterState::Excited: return kExcited;
    case CharacterState::Surprised: return kSurprised;
    case CharacterState::Suspicious: return kSuspicious;
    case CharacterState::Angry: return kAngry;
    case CharacterState::Drowsy: return kDrowsy;
    case CharacterState::Laughing: return kLaughing;
    case CharacterState::Scared: return kScared;
    case CharacterState::Playful: return kPlayful;
    case CharacterState::Proud: return kProud;
    case CharacterState::Shy: return kShy;
    case CharacterState::Sad: return kSad;
    case CharacterState::Confused: return kConfused;
    case CharacterState::Bored: return kBored;
    case CharacterState::Listening: return kListening;
    case CharacterState::Thinking: return kThinking;
    case CharacterState::Progress: return kProgress;
    case CharacterState::PoweringDown: return kPoweringDown;
    case CharacterState::Idle:
    default: return kIdle;
  }
}

void CharacterRenderer::setState(CharacterState state, uint32_t now) {
  if (state_ == state) return;
  state_ = state;
  stateAt_ = now;
  eyeIndex_ = 0;
  const auto& cfg = configFor(state_);
  eyeFrom_ = eyeTo_;
  eyeTo_ = cfg.eyes[0];
  eyeBlend_ = 0.0f;
  nextEyeAt_ = now + cfg.eyeHoldMs;
}

void CharacterRenderer::setGaze(float x, float y) {
  gazeTargetX_ = constrain(x, -1.0f, 1.0f);
  gazeTargetY_ = constrain(y, -1.0f, 1.0f);
}

void CharacterRenderer::setDrag(float x, float y) {
  dragTargetX_ = constrain(x, -1.0f, 1.0f);
  dragTargetY_ = constrain(y, -1.0f, 1.0f);
}

void CharacterRenderer::setProgress(float value) {
  progress_ = clamp01(value);
}

void CharacterRenderer::triggerSpin(int8_t direction, uint8_t turns, uint32_t now) {
  spinDirection_ = direction < 0 ? -1 : 1;
  spinTurns_ = turns == 0 ? 1 : turns;
  spinAt_ = now;
}

void CharacterRenderer::triggerHop(uint32_t now) {
  hopAt_ = now;
}

void CharacterRenderer::triggerShake(uint32_t now) {
  shakeAt_ = now;
}

void CharacterRenderer::triggerWink(uint8_t eye, uint32_t now) {
  winkEye_ = eye & 1;
  winkAt_ = now;
}

void CharacterRenderer::update(uint32_t now, float dt) {
  if (lastUpdateAt_ == 0) lastUpdateAt_ = now;
  lastUpdateAt_ = now;

  const float gazeFollow = 1.0f - expf(-dt * 9.0f);
  const float dragFollow = 1.0f - expf(-dt * 12.0f);
  gazeX_ = lerpFloat(gazeX_, gazeTargetX_, gazeFollow);
  gazeY_ = lerpFloat(gazeY_, gazeTargetY_, gazeFollow);
  dragX_ = lerpFloat(dragX_, dragTargetX_, dragFollow);
  dragY_ = lerpFloat(dragY_, dragTargetY_, dragFollow);
  eyeBlend_ = clamp01(eyeBlend_ + dt * 5.8f);

  if (now >= nextEyeAt_) chooseNextEye(now);
  if (now >= nextBlinkAt_) {
    blinkAt_ = now;
    nextBlinkAt_ = now + 3200 + random(0, 4200);
  }
}

bool CharacterRenderer::render(uint32_t now) {
  if (!display_ || now - lastFrameAt_ < kFrameIntervalMs) return false;
  lastFrameAt_ = now;

  const float seconds = now * 0.001f;
  const float stateSeconds = (now - stateAt_) * 0.001f;
  const int width = display_->width();
  const int height = display_->height();
  const float shortest = min(width, height);

  centerX_ = width * 0.5f + dragX_ * 38.0f;
  centerY_ = height * 0.5f + dragY_ * 32.0f;
  scale_ = shortest * 0.00425f;
  scaleX_ = 1.0f;
  scaleY_ = 1.0f;
  rotation_ = 0.0f;
  eyeOpen_ = 1.0f;
  eyeScaleBoost_ = 1.0f;

  const float breath = sinf(seconds * 1.7f) * 0.014f;
  scaleX_ += breath;
  scaleY_ -= breath * 0.6f;

  switch (state_) {
    case CharacterState::Spawning: {
      const float amount = easeOutBack(clamp01(stateSeconds / 1.05f));
      scaleX_ *= amount;
      scaleY_ *= amount;
      rotation_ += (1.0f - clamp01(stateSeconds / 1.05f)) * 0.7f;
      break;
    }
    case CharacterState::Sleeping:
      rotation_ += 0.07f + sinf(seconds * 0.25f) * 0.035f;
      centerX_ -= 3.0f;
      centerY_ += 14.0f + sinf(seconds * 0.55f) * 5.0f;
      scaleY_ *= 1.0f + sinf(seconds * 0.55f) * 0.018f;
      eyeOpen_ = 0.11f;
      break;
    case CharacterState::Waking:
      if (stateSeconds < 0.5f) {
        centerY_ += 12.0f;
        eyeOpen_ = 0.08f;
      } else if (stateSeconds < 1.2f) {
        const float wake = (stateSeconds - 0.5f) / 0.7f;
        centerY_ -= sinf(wake * kPi) * 10.0f;
        scaleX_ *= 1.0f + sinf(wake * kPi) * 0.06f;
        scaleY_ *= 1.0f - sinf(wake * kPi) * 0.04f;
        eyeScaleBoost_ = 1.14f;
      } else {
        const float settle = clamp01((stateSeconds - 1.2f) / 1.2f);
        rotation_ += sinf(settle * kPi * 5.0f) * 0.1f * (1.0f - settle);
        eyeScaleBoost_ = 1.0f + (1.0f - settle) * 0.08f;
      }
      break;
    case CharacterState::Searching: {
      const float scan = sinf(seconds * 1.3f);
      rotation_ += scan * 0.23f;
      centerX_ += scan * 11.0f;
      centerY_ += sinf(seconds * 1.7f) * 5.0f;
      break;
    }
    case CharacterState::Working: {
      const float beat = sinf(seconds * kPi * 3.2f);
      rotation_ += 0.07f + beat * 0.045f;
      centerX_ += 4.0f;
      centerY_ += 3.0f + max(0.0f, beat) * 6.0f;
      scaleY_ *= 1.0f - max(0.0f, beat) * 0.025f;
      break;
    }
    case CharacterState::Curious: {
      const float nod = fmodf(stateSeconds, 2.3f);
      rotation_ += 0.17f + sinf(seconds * 0.7f) * 0.10f + gazeX_ * 0.06f;
      centerX_ += sinf(seconds * 0.6f) * 7.0f;
      centerY_ -= 3.0f;
      if (nod < 0.44f) rotation_ += sinf(nod / 0.44f * kPi) * 0.08f;
      eyeScaleBoost_ = 1.08f;
      break;
    }
    case CharacterState::Happy:
      rotation_ += sinf(seconds * 1.2f) * 0.05f;
      centerX_ += sinf(seconds * 1.1f) * 4.0f;
      centerY_ -= fabsf(sinf(seconds * 2.4f)) * 5.0f;
      scaleX_ *= 1.0f + sinf(seconds * 2.4f) * 0.025f;
      scaleY_ *= 1.0f - sinf(seconds * 2.4f) * 0.02f;
      eyeScaleBoost_ = 1.05f;
      break;
    case CharacterState::Excited: {
      const float phase = fmodf(seconds * 2.2f, 1.0f);
      const float jump = sinf(phase * kPi);
      centerY_ -= jump * 17.0f;
      centerX_ += sinf(seconds * 1.1f) * 6.0f;
      rotation_ += sinf(seconds * kPi * 2.2f) * 0.12f;
      scaleX_ *= phase < 0.12f ? 0.94f : phase < 0.32f ? 1.06f : 1.0f;
      scaleY_ *= phase < 0.12f ? 1.06f : phase < 0.32f ? 0.95f : 1.0f;
      eyeScaleBoost_ = 1.08f;
      break;
    }
    case CharacterState::Surprised: {
      const float settle = 1.0f - clamp01(stateSeconds / 1.2f);
      centerX_ -= settle * 7.0f;
      centerY_ -= settle * 14.0f;
      rotation_ += sinf(seconds * 11.0f) * 0.025f * settle;
      scaleY_ *= 1.0f + settle * 0.08f;
      eyeScaleBoost_ = 1.16f - settle * 0.05f;
      break;
    }
    case CharacterState::Suspicious:
      rotation_ -= 0.10f + sinf(seconds * 0.3f) * 0.05f;
      centerX_ -= sinf(seconds * 0.25f) * 6.0f;
      centerY_ += 2.0f;
      eyeOpen_ = 0.82f;
      break;
    case CharacterState::Angry:
      centerY_ += 6.0f;
      scaleY_ *= 0.96f;
      eyeOpen_ = 0.82f;
      break;
    case CharacterState::Drowsy:
      rotation_ += sinf(seconds * 0.32f) * 0.045f;
      centerX_ += sinf(seconds * 0.2f) * 3.0f;
      centerY_ += 12.0f + sinf(seconds * 0.36f) * 4.0f;
      scaleY_ *= 1.0f + sinf(seconds * 0.36f) * 0.025f;
      eyeOpen_ = 0.34f + sinf(seconds * 0.8f) * 0.07f;
      break;
    case CharacterState::Laughing: {
      const float laugh = sinf(seconds * kPi * 6.4f);
      rotation_ += laugh * 0.07f;
      centerX_ += sinf(seconds * 2.0f) * 3.0f;
      centerY_ -= fabsf(laugh) * 8.0f;
      scaleX_ *= 1.0f + laugh * 0.035f;
      scaleY_ *= 1.0f - laugh * 0.03f;
      eyeOpen_ = 0.7f;
      break;
    }
    case CharacterState::Scared:
      rotation_ += sinf(now * 0.04f) * 0.035f;
      centerX_ -= 4.0f + sinf(now * 0.05f) * 3.0f;
      centerY_ += 3.0f + sinf(seconds * 1.5f) * 2.0f;
      scaleY_ *= 0.97f;
      eyeScaleBoost_ = 1.14f;
      break;
    case CharacterState::Playful:
      rotation_ += sinf(seconds * 1.4f) * 0.14f;
      centerX_ += sinf(seconds * 1.1f) * 7.0f;
      centerY_ -= fabsf(sinf(seconds * 2.2f)) * 5.0f;
      scaleX_ *= 1.0f + sinf(seconds * 2.2f) * 0.02f;
      scaleY_ *= 1.0f - sinf(seconds * 2.2f) * 0.018f;
      eyeScaleBoost_ = 1.07f;
      break;
    case CharacterState::Proud:
      rotation_ += sinf(seconds * 0.4f) * 0.045f;
      centerY_ -= 7.0f;
      scaleY_ *= 1.03f;
      eyeOpen_ = 0.9f;
      break;
    case CharacterState::Shy:
      rotation_ -= 0.14f + sinf(seconds * 0.5f) * 0.05f;
      centerX_ -= 6.0f;
      centerY_ += 5.0f;
      scaleY_ *= 0.98f;
      eyeOpen_ = 0.85f;
      eyeScaleBoost_ = 0.96f;
      break;
    case CharacterState::Sad:
      rotation_ += 0.05f + sinf(seconds * 0.3f) * 0.035f;
      centerY_ += 12.0f;
      scaleY_ *= 0.97f;
      eyeOpen_ = 0.7f;
      break;
    case CharacterState::Confused:
      rotation_ += sinf(seconds * 0.8f) * 0.21f;
      centerX_ += sinf(seconds * 0.8f) * 5.0f;
      centerY_ += sinf(seconds * 0.5f) * 3.0f;
      eyeOpen_ = 0.9f;
      break;
    case CharacterState::Bored:
      rotation_ -= 0.05f - sinf(seconds * 0.25f) * 0.07f;
      centerX_ += sinf(seconds * 0.2f) * 6.0f;
      centerY_ += 9.0f + sinf(seconds * 0.35f) * 2.0f;
      eyeOpen_ = 0.58f;
      eyeScaleBoost_ = 0.98f;
      break;
    case CharacterState::Listening: {
      const float pulse = 1.0f + sinf(seconds * kPi * 3.0f) * 0.03f;
      const float nod = fmodf(stateSeconds, 2.35f);
      rotation_ += 0.14f + sinf(seconds * 0.5f) * 0.025f;
      centerX_ += 3.0f;
      centerY_ -= 3.0f;
      if (nod < 0.38f) centerY_ += sinf(nod / 0.38f * kPi) * 7.0f;
      scaleX_ *= pulse;
      scaleY_ *= pulse;
      break;
    }
    case CharacterState::Thinking:
      rotation_ -= 0.16f - sinf(seconds * 0.35f) * 0.09f;
      centerX_ += sinf(seconds * 0.3f) * 8.0f;
      centerY_ += sinf(seconds * 0.6f) * 4.0f;
      break;
    case CharacterState::Progress:
      rotation_ += sinf(seconds * 1.2f) * 0.04f;
      gazeTargetX_ = sinf(seconds * 1.8f) * progress_;
      break;
    case CharacterState::PoweringDown: {
      const float amount = clamp01(stateSeconds / 1.0f);
      scaleY_ *= 1.0f - amount * 0.78f;
      centerY_ += amount * 52.0f;
      break;
    }
    case CharacterState::Idle:
    default:
      rotation_ += sinf(seconds * 0.5f) * 0.026f + sinf(seconds * 0.17f) * 0.01f;
      centerX_ += sinf(seconds * 0.27f) * 2.0f;
      centerY_ += sinf(seconds * 0.85f) * 2.0f;
      break;
  }

  if (spinAt_) {
    const uint32_t elapsed = now - spinAt_;
    const uint32_t spinMs = 620 + spinTurns_ * 170;
    if (elapsed < spinMs) {
      const float t = elapsed / static_cast<float>(spinMs);
      const float smooth = t * t * (3.0f - 2.0f * t);
      rotation_ += spinDirection_ * spinTurns_ * kPi * 2.0f * smooth;
    } else if (elapsed < spinMs + 650) {
      const float t = (elapsed - spinMs) / 650.0f;
      rotation_ += sinf(t * kPi * 5.0f) * 0.24f * (1.0f - t) * spinDirection_;
      centerX_ += cosf(t * kPi * 5.0f) * 8.0f * (1.0f - t);
      eyeOpen_ *= 0.68f + sinf(t * kPi * 6.0f) * 0.08f;
    } else {
      spinAt_ = 0;
    }
  }

  if (hopAt_) {
    static constexpr float kHopHeights[] = {34.0f, 20.0f, 10.0f, 4.0f};
    static constexpr float kHopDurations[] = {0.42f, 0.32f, 0.23f, 0.15f};
    float elapsed = (now - hopAt_) * 0.001f;
    bool active = false;
    for (uint8_t i = 0; i < 4; ++i) {
      if (elapsed < kHopDurations[i]) {
        const float t = elapsed / kHopDurations[i];
        centerY_ -= 4.0f * kHopHeights[i] * t * (1.0f - t);
        active = true;
        break;
      }
      elapsed -= kHopDurations[i];
    }
    if (!active) hopAt_ = 0;
  }

  if (shakeAt_) {
    const uint32_t elapsed = now - shakeAt_;
    if (elapsed < 460) {
      const float decay = 1.0f - elapsed / 460.0f;
      centerX_ += sinf(elapsed * 0.095f) * 10.0f * decay;
      rotation_ += sinf(elapsed * 0.075f) * 0.07f * decay;
    } else {
      shakeAt_ = 0;
    }
  }

  cosRotation_ = cosf(rotation_);
  sinRotation_ = sinf(rotation_);

  display_->startWrite();
  display_->fillScreen(bodyColor_);
  drawEyes(seconds);
  display_->endWrite();
  display_->display();

  ++framesInWindow_;
  if (now - fpsWindowAt_ >= 1000) {
    fps_ = framesInWindow_ * 1000.0f / (now - fpsWindowAt_);
    framesInWindow_ = 0;
    fpsWindowAt_ = now;
  }
  return true;
}

void CharacterRenderer::chooseNextEye(uint32_t now) {
  const auto& cfg = configFor(state_);
  eyeFrom_ = eyeTo_;
  eyeIndex_ = (eyeIndex_ + 1) % cfg.eyeCount;
  eyeTo_ = cfg.eyes[eyeIndex_];
  eyeBlend_ = 0.0f;
  nextEyeAt_ = now + cfg.eyeHoldMs + random(0, cfg.eyeHoldMs / 2 + 1);
}

void CharacterRenderer::drawEyes(float) {
  const uint32_t now = millis();
  float blink = 1.0f;
  const uint32_t blinkElapsed = now - blinkAt_;
  if (blinkAt_ != 0 && blinkElapsed < 190) {
    blink = 1.0f - sinf(blinkElapsed / 190.0f * kPi) * 0.94f;
  }

  float sourceX[2][grok_geometry::kEyePointCount];
  float sourceY[2][grok_geometry::kEyePointCount];
  float centroidX[2] = {0.0f, 0.0f};
  float centroidY[2] = {0.0f, 0.0f};
  float faceCenterX = 0.0f;
  float faceCenterY = 0.0f;
  for (uint8_t eye = 0; eye < 2; ++eye) {
    for (uint8_t i = 0; i < grok_geometry::kEyePointCount; ++i) {
      const auto from = grok_geometry::kEyes[eyeFrom_][eye][i];
      const auto to = grok_geometry::kEyes[eyeTo_][eye][i];
      sourceX[eye][i] = lerpFloat(from.x, to.x, eyeBlend_) / grok_geometry::kPointScale;
      sourceY[eye][i] = lerpFloat(from.y, to.y, eyeBlend_) / grok_geometry::kPointScale;
      centroidX[eye] += sourceX[eye][i];
      centroidY[eye] += sourceY[eye][i];
      faceCenterX += sourceX[eye][i];
      faceCenterY += sourceY[eye][i];
    }
    centroidX[eye] /= grok_geometry::kEyePointCount;
    centroidY[eye] /= grok_geometry::kEyePointCount;
  }
  faceCenterX /= grok_geometry::kEyePointCount * 2;
  faceCenterY /= grok_geometry::kEyePointCount * 2;

  float wink = 1.0f;
  if (winkAt_) {
    const uint32_t elapsed = now - winkAt_;
    if (elapsed < 260) {
      wink = 1.0f - sinf(elapsed / 260.0f * kPi) * 0.96f;
    } else {
      winkAt_ = 0;
    }
  }

  for (uint8_t eye = 0; eye < 2; ++eye) {
    const float open = max(0.035f, blink * eyeOpen_ * (eye == winkEye_ ? wink : 1.0f));
    int16_t pointsX[grok_geometry::kEyePointCount];
    int16_t pointsY[grok_geometry::kEyePointCount];
    int32_t screenCentroidX = 0;
    int32_t screenCentroidY = 0;
    for (uint8_t i = 0; i < grok_geometry::kEyePointCount; ++i) {
      const float blinkY = centroidY[eye] + (sourceY[eye][i] - centroidY[eye]) * open;
      const float localX = (sourceX[eye][i] - faceCenterX) * scale_ * scaleX_ * eyeScaleBoost_;
      const float localY = (blinkY - faceCenterY) * scale_ * scaleY_ * eyeScaleBoost_;
      pointsX[i] = static_cast<int16_t>(lroundf(
          centerX_ + localX * cosRotation_ - localY * sinRotation_ + gazeX_ * 13.0f));
      pointsY[i] = static_cast<int16_t>(lroundf(
          centerY_ + localX * sinRotation_ + localY * cosRotation_ + gazeY_ * 11.0f));
      screenCentroidX += pointsX[i];
      screenCentroidY += pointsY[i];
    }
    const int16_t cx = screenCentroidX / grok_geometry::kEyePointCount;
    const int16_t cy = screenCentroidY / grok_geometry::kEyePointCount;

    for (uint8_t i = 0; i < grok_geometry::kEyePointCount; ++i) {
      const uint8_t next = (i + 1) % grok_geometry::kEyePointCount;
      display_->fillTriangle(cx, cy, pointsX[i], pointsY[i], pointsX[next], pointsY[next], eyeColor_);
    }
  }
}

void CharacterRenderer::transformPoint(float sourceX, float sourceY, int16_t& x, int16_t& y) const {
  const float localX = (sourceX - grok_geometry::kSourceCenter) * scale_ * scaleX_;
  const float localY = (sourceY - grok_geometry::kSourceCenter) * scale_ * scaleY_;
  x = static_cast<int16_t>(lroundf(centerX_ + localX * cosRotation_ - localY * sinRotation_));
  y = static_cast<int16_t>(lroundf(centerY_ + localX * sinRotation_ + localY * cosRotation_));
}

float CharacterRenderer::clamp01(float value) {
  return constrain(value, 0.0f, 1.0f);
}

float CharacterRenderer::easeOutBack(float value) {
  const float c1 = 1.70158f;
  const float c3 = c1 + 1.0f;
  const float x = value - 1.0f;
  return 1.0f + c3 * x * x * x + c1 * x * x;
}
