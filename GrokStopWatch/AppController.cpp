#include "AppController.h"

#include <esp_system.h>

namespace {

constexpr uint32_t kProgressDurationMs = 10000;

const char* stateName(CharacterState state) {
  switch (state) {
    case CharacterState::Spawning: return "spawning";
    case CharacterState::Sleeping: return "sleeping";
    case CharacterState::Waking: return "waking";
    case CharacterState::Idle: return "idle";
    case CharacterState::Searching: return "searching";
    case CharacterState::Working: return "working";
    case CharacterState::Curious: return "curious";
    case CharacterState::Happy: return "happy";
    case CharacterState::Excited: return "excited";
    case CharacterState::Surprised: return "surprised";
    case CharacterState::Suspicious: return "suspicious";
    case CharacterState::Angry: return "angry";
    case CharacterState::Drowsy: return "drowsy";
    case CharacterState::Laughing: return "laughing";
    case CharacterState::Scared: return "scared";
    case CharacterState::Playful: return "playful";
    case CharacterState::Proud: return "proud";
    case CharacterState::Shy: return "shy";
    case CharacterState::Sad: return "sad";
    case CharacterState::Confused: return "confused";
    case CharacterState::Bored: return "bored";
    case CharacterState::Listening: return "listening";
    case CharacterState::Thinking: return "thinking";
    case CharacterState::Progress: return "progress";
    case CharacterState::PoweringDown: return "powering-down";
    default: return "unknown";
  }
}

}  // namespace

void AppController::begin() {
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;
  cfg.clear_display = true;
  cfg.internal_imu = true;
  cfg.internal_rtc = true;
  cfg.internal_mic = true;
  cfg.internal_spk = true;
  M5.begin(cfg);

  M5.Display.setBrightness(82);
  M5.Display.setRotation(0);
  M5.Speaker.setVolume(24);
  M5.Power.setVibration(0);
  M5.Touch.setHoldThresh(620);
  M5.Touch.setFlickThresh(28);
  renderer_.begin(&M5.Display);
  state_ = CharacterState::Spawning;
  renderer_.setState(state_, millis());
  stateUntil_ = millis() + 1350;
  nextShowcaseAt_ = millis() + 4200;
  printDiagnostics();
}

void AppController::update() {
  const uint32_t now = millis();
  const float dt = lastLoopAt_ == 0 ? 0.016f : min((now - lastLoopAt_) * 0.001f, 0.05f);
  lastLoopAt_ = now;

  M5.update();
  if (vibrationOffAt_ && static_cast<int32_t>(now - vibrationOffAt_) >= 0) {
    M5.Power.setVibration(0);
    vibrationOffAt_ = 0;
  }
  updateInputs(now);
  updateState(now);
  renderer_.update(now, dt);
  renderer_.render(now);

  if (now - lastLogAt_ >= 5000) {
    lastLogAt_ = now;
    Serial.printf("[health] state=%s fps=%.1f heap=%lu psram=%lu battery=%ld%%\n",
                  stateName(state_), renderer_.framesPerSecond(),
                  static_cast<unsigned long>(ESP.getFreeHeap()),
                  static_cast<unsigned long>(ESP.getFreePsram()),
                  static_cast<long>(M5.Power.getBatteryLevel()));
  }
  delay(1);
}

void AppController::updateInputs(uint32_t now) {
  const int width = M5.Display.width();
  const int height = M5.Display.height();
  bool hasTouch = false;

  if (M5.Touch.isEnabled()) {
    const auto touch = M5.Touch.getDetail(0);
    hasTouch = touch.isPressed();
    if (hasTouch) {
      const float nx = constrain((touch.x - width * 0.5f) / (width * 0.34f), -1.0f, 1.0f);
      const float ny = constrain((touch.y - height * 0.5f) / (height * 0.34f), -1.0f, 1.0f);
      renderer_.setGaze(nx, ny);
      renderer_.setDrag(nx * 0.62f, ny * 0.52f);
      if (touch.wasPressed()) {
        pulseHaptic(78, 22, now);
        touchActive_ = true;
        enterState(CharacterState::Curious);
      }
      if (touch.wasDragStart()) {
        pulseHaptic(88, 18, now);
        enterState(CharacterState::Playful);
      }
      if (touch.wasHold()) {
        pulseHaptic(96, 30, now);
        enterState(CharacterState::Suspicious);
      }
    }

    if (touch.wasClicked()) {
      touchActive_ = false;
      renderer_.setDrag(0.0f, 0.0f);
      if (touch.getClickCount() >= 2) {
        enterState(CharacterState::Excited, 1900);
      } else {
        enterState(CharacterState::Happy, 1300);
      }
    }

    if (touch.wasFlicked()) {
      touchActive_ = false;
      renderer_.setDrag(0.0f, 0.0f);
      const int dx = touch.distanceX();
      const int dy = touch.distanceY();
      if (abs(dx) > abs(dy)) {
        enterState(CharacterState::Playful, 1900);
        renderer_.triggerSpin(dx < 0 ? -1 : 1, 1, now);
      } else if (dy < 0) {
        enterState(CharacterState::Surprised, 1600);
      } else {
        enterState(CharacterState::Drowsy, 2300);
      }
    }

    if (touch.wasDragged()) {
      touchActive_ = false;
      renderer_.setDrag(0.0f, 0.0f);
      enterState(CharacterState::Playful, 1400);
    }

    if (touch.wasReleased()) {
      const bool heldSuspicious = state_ == CharacterState::Suspicious;
      touchActive_ = false;
      renderer_.setDrag(0.0f, 0.0f);
      if (heldSuspicious) {
        enterState(CharacterState::Angry, 1050);
      } else if (state_ == CharacterState::Curious) {
        enterState(CharacterState::Idle);
      }
    }
  } else if (touchActive_) {
    touchActive_ = false;
    renderer_.setDrag(0.0f, 0.0f);
  }

  if (!hasTouch && M5.Imu.isEnabled() && M5.Imu.update()) {
    const auto data = M5.Imu.getImuData();
    renderer_.setGaze(constrain(-data.accel.y * 0.7f, -0.75f, 0.75f),
                      constrain(data.accel.x * 0.7f, -0.75f, 0.75f));
  }

  if (M5.BtnA.wasClicked()) {
    static constexpr CharacterState kLifecycle[] = {
        CharacterState::Sleeping, CharacterState::Waking, CharacterState::Idle,
        CharacterState::Listening, CharacterState::Thinking, CharacterState::Searching,
        CharacterState::Working};
    pulseHaptic(108, 30, now);
    showcaseMode_ = false;
    enterState(kLifecycle[lifecycleIndex_ % (sizeof(kLifecycle) / sizeof(kLifecycle[0]))]);
    lifecycleIndex_ = (lifecycleIndex_ + 1) % (sizeof(kLifecycle) / sizeof(kLifecycle[0]));
  }

  if (M5.BtnB.wasPressed()) {
    pulseHaptic(120, 45, now);
    showcaseMode_ = true;
    showcaseIndex_ = 0;
    enterState(CharacterState::Idle);
    nextShowcaseAt_ = now + 900;
    Serial.println("[mode] auto-showcase");
  }
}

void AppController::updateState(uint32_t now) {
  if (state_ == CharacterState::Progress) {
    const uint32_t elapsed = now - progressAt_;
    renderer_.setProgress(min(elapsed / static_cast<float>(kProgressDurationMs), 1.0f));
    if (elapsed >= kProgressDurationMs) {
      enterState(CharacterState::Happy, 1350);
    }
    return;
  }

  if (stateUntil_ && now >= stateUntil_) {
    stateUntil_ = 0;
    if (state_ == CharacterState::Thinking) {
      enterState(CharacterState::Happy, 900);
    } else {
      enterState(CharacterState::Idle);
    }
  }

  if (state_ == CharacterState::Idle && !touchActive_ && showcaseMode_ && now >= nextShowcaseAt_) {
    static constexpr CharacterState kShowcase[] = {
        CharacterState::Sleeping, CharacterState::Waking, CharacterState::Listening,
        CharacterState::Thinking, CharacterState::Searching, CharacterState::Working,
        CharacterState::Curious, CharacterState::Happy, CharacterState::Playful,
        CharacterState::Excited, CharacterState::Proud, CharacterState::Laughing,
        CharacterState::Shy, CharacterState::Suspicious};
    enterState(kShowcase[showcaseIndex_ % (sizeof(kShowcase) / sizeof(kShowcase[0]))], 2400);
    showcaseIndex_ = (showcaseIndex_ + 1) % (sizeof(kShowcase) / sizeof(kShowcase[0]));
  }
}

void AppController::enterState(CharacterState state, uint32_t durationMs) {
  const uint32_t now = millis();
  state_ = state;
  stateUntil_ = durationMs ? now + durationMs : 0;
  renderer_.setState(state, now);
  triggerStateMotion(state, now);
  if (state == CharacterState::Idle) {
    nextShowcaseAt_ = now + random(1800, 3200);
  }
  Serial.printf("[state] %s\n", stateName(state));
}

void AppController::pulseHaptic(uint8_t level, uint16_t durationMs, uint32_t now) {
  M5.Power.setVibration(level);
  vibrationOffAt_ = now + durationMs;
}

void AppController::triggerStateMotion(CharacterState state, uint32_t now) {
  switch (state) {
    case CharacterState::Happy:
    case CharacterState::Laughing:
      renderer_.triggerHop(now);
      break;
    case CharacterState::Excited:
      renderer_.triggerHop(now);
      renderer_.triggerSpin(random(0, 2) ? 1 : -1, 1, now);
      break;
    case CharacterState::Surprised:
      renderer_.triggerHop(now);
      break;
    case CharacterState::Angry:
    case CharacterState::Scared:
    case CharacterState::Confused:
      renderer_.triggerShake(now);
      break;
    case CharacterState::Playful:
    case CharacterState::Proud:
      renderer_.triggerWink(random(0, 2), now);
      break;
    default:
      break;
  }
}

void AppController::printDiagnostics() {
  Serial.println();
  Serial.println("GROK_STOPWATCH_BOOT v0.4.0");
  Serial.printf("board=%d display=%dx%d touch=%d imu=%d rtc=%d mic=%d speaker=%d\n",
                static_cast<int>(M5.getBoard()), static_cast<int>(M5.Display.width()),
                static_cast<int>(M5.Display.height()),
                M5.Touch.isEnabled(), M5.Imu.isEnabled(), M5.Rtc.isEnabled(),
                M5.Mic.isEnabled(), M5.Speaker.isEnabled());
  Serial.printf("flash=%lu psram=%lu free_psram=%lu heap=%lu battery=%ld%%\n",
                static_cast<unsigned long>(ESP.getFlashChipSize()),
                static_cast<unsigned long>(ESP.getPsramSize()),
                static_cast<unsigned long>(ESP.getFreePsram()),
                static_cast<unsigned long>(ESP.getFreeHeap()),
                static_cast<long>(M5.Power.getBatteryLevel()));
}
