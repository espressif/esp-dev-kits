# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Apache-2.0
"""Host regressions for network lifecycle and HTTP control boundaries.

Run: python3 tests/test_network.py
The fixtures compile production functions, replacing only ESP-IDF/RTOS I/O.
"""
import pathlib
import os
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]


def function(source, name):
    """Extract a C function without depending on platform headers."""
    matches = list(re.finditer(r"^[\w *]+\b" + re.escape(name) + r"\([^;]*?\)\n\{", source, re.M))
    if not matches:
        raise AssertionError(f"Missing function: {name}")
    match = matches[-1]
    start, cursor = match.start(), match.end()
    depth = 1
    token = re.compile(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|[{}]', re.S)
    for item in token.finditer(source, cursor):
        if item.group() == "{":
            depth += 1
        elif item.group() == "}":
            depth -= 1
        if depth == 0:
            return source[start:item.end()] + "\n"
    raise AssertionError(f"Unclosed function: {name}")


class NetworkTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.compiler = shutil.which("cc")
        if not cls.compiler:
            raise unittest.SkipTest("C compiler required")

    def test_sse_shutdown_and_queue_lifetimes(self):
        source = (ROOT / "main/wifi_stream.c").read_text()
        names = ["sse_close_client", "sse_events_handler", "sse_send_text", "sse_send_work",
                 "sse_stop_work", "stop_http_server", "wifi_stream_send_text"]
        with tempfile.TemporaryDirectory() as temporary:
            temp = pathlib.Path(temporary)
            (temp / "network_lifecycle.inc").write_text("\n".join(function(source, name) for name in names))
            binary = temp / "lifecycle"
            subprocess.run([self.compiler, "-std=c11", "-pthread", "-D_POSIX_C_SOURCE=200809L",
                            "-fsanitize=address,undefined", "-g", "-I", str(temp),
                            str(ROOT / "tests/test_network_lifecycle.c"), "-o", str(binary)], check=True)
            subprocess.run([str(binary)], check=True, timeout=15)

    def test_http_control_and_deadlines(self):
        candidates = list((ROOT / "managed_components").glob("**/cJSON.c"))
        if os.environ.get("CJSON_DIR"):
            candidates += list(pathlib.Path(os.environ["CJSON_DIR"]).glob("**/cJSON.c"))
        if os.environ.get("IDF_PATH"):
            candidates += list((pathlib.Path(os.environ["IDF_PATH"]) / "components/json").glob("**/cJSON.c"))
        if not candidates:
            self.skipTest("Resolve managed components or set IDF_PATH/CJSON_DIR for real cJSON")
        cjson = candidates[0]
        source = (ROOT / "main/web_portal.c").read_text()
        names = ["json_number", "json_integer", "control_error", "audio_post_handler",
                 "web_portal_handle_control_text"]
        with tempfile.TemporaryDirectory() as temporary:
            temp = pathlib.Path(temporary)
            (temp / "network_control.inc").write_text("\n".join(function(source, name) for name in names))
            binary = temp / "control"
            subprocess.run([self.compiler, "-std=c11", "-fsanitize=address,undefined", "-g",
                            "-Wno-deprecated-declarations",
                            "-I", str(temp), "-I", str(cjson.parent), "-I", str(ROOT / "main"),
                            str(ROOT / "tests/test_network_control.c"), str(cjson), "-lm",
                            "-o", str(binary)], check=True)
            subprocess.run([str(binary)], check=True, timeout=15)

    def test_wifi_credentials_and_cancelled_work(self):
        source = (ROOT / "main/wifi_stream.c").read_text()
        names = ["wifi_stop_internal", "wifi_connect_watchdog_task", "wifi_auto_connect_task",
                 "wifi_schedule_auto_connect", "wifi_event_handler", "wifi_credentials_valid",
                 "wifi_init_sta", "start_connect_watchdog", "handle_wifi_cfg_command",
                 "handle_serial_command", "wifi_stream_connect"]
        with tempfile.TemporaryDirectory() as temporary:
            temp = pathlib.Path(temporary)
            (temp / "network_wifi.inc").write_text("\n".join(function(source, name) for name in names))
            binary = temp / "wifi"
            subprocess.run([self.compiler, "-std=c11", "-D_POSIX_C_SOURCE=200809L",
                            "-fsanitize=address,undefined", "-g", "-I", str(temp),
                            str(ROOT / "tests/test_network_wifi.c"), "-o", str(binary)], check=True)
            subprocess.run([str(binary)], check=True, timeout=15)


if __name__ == "__main__":
    unittest.main()
