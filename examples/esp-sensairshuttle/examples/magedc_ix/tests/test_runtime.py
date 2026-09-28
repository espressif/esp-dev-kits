# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Apache-2.0
"""Host regressions for the firmware's actual runtime functions (requires c++)."""

import re
import subprocess
import tempfile
import unittest
from pathlib import Path

MAIN = Path(__file__).resolve().parents[1] / "main"


def function(filename, name):
    source = (MAIN / filename).read_text()
    match = re.search(r"^(?:static )?(?:bool|void) " + name + r"\([^;]*?\)\n\{", source, re.M)
    if not match:
        raise AssertionError(f"Missing function: {name}")
    start = source.index("{", match.start())
    depth = 1
    end = start + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[match.start():end]


def run_cpp(source):
    with tempfile.TemporaryDirectory(prefix="magedc_runtime_") as directory:
        path = Path(directory) / "test.cpp"
        path.write_text(source)
        binary = path.with_suffix("")
        subprocess.run(["c++", "-std=c++17", "-Wall", "-Wextra", "-Werror", str(path), "-o", str(binary)], check=True)
        subprocess.run([str(binary)], check=True)


class RuntimeTests(unittest.TestCase):
    def test_idle_uses_elapsed_running_time(self):
        run_cpp("""
#include <cassert>
#include <cstdint>
#define IDLE_SLEEP_MS 60000U
""" + function("main.cpp", "idle_sleep_due") + """
int main() {
    uint32_t last = 0;
    assert(!idle_sleep_due(false, false, 120000, &last));
    assert(!idle_sleep_due(true, false, 120001, &last));
    assert(!idle_sleep_due(true, false, 179999, &last));
    assert(idle_sleep_due(true, false, 180000, &last));
    assert(!idle_sleep_due(true, true, 180001, &last));
    assert(!idle_sleep_due(true, false, 240000, &last));
    last = UINT32_MAX - 1000;
    assert(!idle_sleep_due(true, false, 1000, &last));
    assert(idle_sleep_due(true, false, 59000, &last));
}
""")

    def test_wakeup_requires_every_sensor_operation(self):
        run_cpp("""
#include <cassert>
#include <cstddef>
#include <cstdint>
#define BMI2_OK 0
#define BMI2_ACCEL 0
#define BMI2_GYRO 1
#define BMI2_DISABLE 0
#define BMI2_ENABLE 1
#define BMI2_ACC_ODR_200HZ 8
#define BMI2_ACC_RANGE_16G 3
#define BMI2_ACC_NORMAL_AVG4 2
#define BMI2_PERF_OPT_MODE 1
#define BMI2_GYR_ODR_200HZ 8
#define BMI2_GYR_RANGE_2000 0
#define BMI2_GYR_NORMAL_MODE 2
#define BMI2_INT1 1
#define BMI2_INT_INPUT_DISABLE 0
#define BMI2_INT_ACTIVE_HIGH 1
#define BMI2_INT_PUSH_PULL 0
#define BMI2_INT_OUTPUT_ENABLE 1
#define BMI2_INT_NON_LATCH 0
#define BMI270_TOY_INT_ANY_MOT_MASK 64
#define BMI2_INT1_MAP_FEAT_ADDR 86
struct sensor_config { int odr, range, bwp, filter_perf, noise_perf; };
struct bmi2_sens_config { int type; struct { sensor_config acc, gyr; } cfg; };
struct bmi2_int_pin_config { int pin_type; struct { int input_en, lvl, od, output_en; } pin_cfg[2]; int int_latch; };
static void *s_bmi_handle;
static int fail_at, calls;
template<typename... Args> int8_t operation(Args...) { return ++calls == fail_at ? -1 : BMI2_OK; }
#define bmi2_set_adv_power_save operation
#define bmi2_get_sensor_config operation
#define bmi2_set_sensor_config operation
#define bmi2_set_int_pin_config operation
#define bmi2_set_regs operation
#define bmi2_sensor_enable operation
#define bmi270_enable_toy_any_motion operation
""" + function("main.cpp", "bmi270_configure_any_motion_for_wakeup") + """
int main() {
    s_bmi_handle = nullptr;
    assert(!bmi270_configure_any_motion_for_wakeup());
    assert(calls == 0);
    s_bmi_handle = &calls;
    for (fail_at = 1; fail_at <= 7; ++fail_at) {
        calls = 0;
        assert(!bmi270_configure_any_motion_for_wakeup());
        assert(calls == fail_at);
    }
    fail_at = 0; calls = 0;
    assert(bmi270_configure_any_motion_for_wakeup());
    assert(calls == 7);
}
""")

    def test_stale_audio_is_cancelled(self):
        source = """
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cmath>
#define ESP_LOGD(...) ((void)0)
#define POSITION_INVALID (-1)
#define CENTER_ZONE_ID 5
#define ZONE_TONE_SILENT_ID 5
#define DEBOUNCE_PROFILE_STILL 0
#define DEBOUNCE_PROFILE_SLIDING 1
#define xTaskNotifyGive(t) ((void)0)
enum audio_gate_state_t { AUDIO_GATE_IDLE, AUDIO_GATE_SLIDING_SUPPRESSED, AUDIO_GATE_PENDING_SETTLE, AUDIO_GATE_PLAYED };
static audio_gate_state_t s_audio_gate_state = AUDIO_GATE_IDLE;
static int s_audio_pending_zone = -1, s_audio_last_played_zone = -1, s_motion_profile = 0;
static uint32_t s_audio_pending_since_ms = 0, s_audio_last_played_ms = 0;
static float s_audio_pending_peak_conf = 0;
static struct { uint32_t settle_hold_ms; float min_conf; uint32_t cooldown_ms; } s_audio_gate_cfg = {170, .22f, 20};
static bool s_ready = true, s_enabled = true;
static void *s_tone_task = &s_ready;
static int s_pending_zone = 0;
static struct { const void *wav; } s_zone_wav[10];
"""
        # Select the enabled implementation rather than the disabled-build stub.
        enabled = (MAIN / "zone_tone.c").read_text().split("#else", 1)[1]
        start = enabled.index("void zone_tone_set_zone(int zone_id)")
        end = enabled.index("\nvoid zone_tone_update", start)
        source += enabled[start:end]
        for name in ("audio_zone_is_playable", "audio_gate_reset", "update_audio_zone_gate"):
            source += "\n" + function("main.cpp", name)
        source += """
int main() {
    for (int zone = 1; zone <= 9; ++zone) s_zone_wav[zone].wav = &s_ready;
    update_audio_zone_gate(1, 1, true, 1);
    update_audio_zone_gate(1, 1, false, 172);
    assert(s_pending_zone == 1);
    s_pending_zone = 0; // Tone task starts the 720 ms note.
    update_audio_zone_gate(2, 1, true, 200);
    update_audio_zone_gate(2, 1, false, 371);
    assert(s_pending_zone == 2);
    update_audio_zone_gate(5, 1, true, 400);
    assert(s_pending_zone == 0);
    s_pending_zone = 2;
    s_motion_profile = DEBOUNCE_PROFILE_SLIDING;
    update_audio_zone_gate(3, 1, true, 420);
    assert(s_pending_zone == 0);
    s_motion_profile = DEBOUNCE_PROFILE_STILL;
    s_pending_zone = 2;
    update_audio_zone_gate(-1, 0, true, 500);
    assert(s_pending_zone == 0);
    s_pending_zone = 2;
    update_audio_zone_gate(3, .01f, true, 520);
    assert(s_pending_zone == 0);
}
"""
        run_cpp(source)

    def test_audio_drains_before_pa_off(self):
        run_cpp("""
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <deque>
#define ESP_LOGE(...) ((void)0)
#define ESP_LOGI(...) ((void)0)
#define TONE_SAMPLE_RATE_HZ 24000
#define TONE_DMA_DESC_NUM 6
#define TONE_DMA_FRAME_NUM 240
#define ESP_OK 0
#define pdMS_TO_TICKS(x) (x)
using esp_err_t = int;
static bool s_enabled = true, pa_on;
static void *s_tx_handle;
static int writes, fail_at, played;
static std::deque<bool> dma;
struct zone_tone_config_t { uint32_t max_play_ms, pa_settle_ms, pa_hold_ms; int volume_pct; };
static zone_tone_config_t s_tone_cfg = {80, 0, 0, 1};
struct zone_wav_t { const uint8_t *wav; };
static uint8_t data;
static zone_wav_t s_zone_wav[10];
static size_t zone_wav_len(const zone_wav_t *) { return 100; }
static bool wav_parse_pcm(const uint8_t *, size_t, const int16_t **pcm, size_t *n, uint32_t *rate) {
    static int16_t samples[2000]; *pcm = samples; *n = 2000; *rate = 24000; return true;
}
static void pa_set(bool on) {
    if (!on && fail_at == 0) assert(played == TONE_DMA_DESC_NUM);
    pa_on = on;
}
static void vTaskDelay(uint32_t) {}
static bool i2s_write_pcm(const int16_t *, size_t, const zone_tone_config_t *) {
    dma.assign(TONE_DMA_DESC_NUM, true); return true;
}
static int i2s_channel_write(void *, const int16_t *silence, size_t size, size_t *written, int) {
    assert(pa_on);
    for (size_t i = 0; i < size / sizeof(int16_t); ++i) assert(silence[i] == 0);
    if (++writes == fail_at) return -1;
    if (dma.front()) ++played;
    dma.pop_front(); dma.push_back(false);
    *written = size; return ESP_OK;
}
""" + function("zone_tone.c", "i2s_drain_pcm") + "\n" + function("zone_tone.c", "play_zone_wav") + """
int main() {
    s_zone_wav[1].wav = &data;
    play_zone_wav(1);
    assert(!pa_on && played == TONE_DMA_DESC_NUM);
    played = 0; writes = 0; fail_at = 1;
    play_zone_wav(1);
    assert(!pa_on && writes == 1);
}
""")


    def test_gesture_save_failure_keeps_previous_sequence(self):
        source = """
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstring>
#define ESP_LOGW(...) ((void)0)
#define ESP_LOGI(...) ((void)0)
#define SPEAKER_GESTURE_MAX_LEN 8
#define ESP_OK 0
#define NVS_READWRITE 1
using nvs_handle_t = int;
using esp_err_t = int;
static const char *NVS_NS = "spk_gesture", *NVS_KEY_SEQ = "seq";
static int s_seq[8] = {1, 9};
static size_t s_seq_len = 2;
static int fail_at, calls, reset_count;
static int operation() { return ++calls == fail_at ? -1 : ESP_OK; }
static int nvs_open(const char *, int, int *handle) { *handle = 1; return operation(); }
static int nvs_set_blob(int, const char *, const void *, size_t) { return operation(); }
static int nvs_commit(int) { return operation(); }
static void nvs_close(int) {}
static void speaker_gesture_reset_matcher() { ++reset_count; }
"""
        for name in ("speaker_gesture_validate", "speaker_gesture_store_to_nvs", "speaker_gesture_set_sequence"):
            source += "\n" + function("speaker_gesture.c", name)
        source += """
int main() {
    const int replacement[] = {2, 4, 8};
    for (fail_at = 1; fail_at <= 3; ++fail_at) {
        calls = 0;
        assert(!speaker_gesture_set_sequence(replacement, 3));
        assert(s_seq_len == 2 && s_seq[0] == 1 && s_seq[1] == 9);
        assert(reset_count == 0);
    }
    fail_at = 0; calls = 0;
    assert(speaker_gesture_set_sequence(replacement, 3));
    assert(s_seq_len == 3 && s_seq[0] == 2 && s_seq[2] == 8);
    assert(reset_count == 1);
    calls = 0;
    assert(!speaker_gesture_set_sequence(replacement, 9));
    assert(calls == 0 && s_seq_len == 3);
}
"""
        run_cpp(source)

if __name__ == "__main__":
    unittest.main()
