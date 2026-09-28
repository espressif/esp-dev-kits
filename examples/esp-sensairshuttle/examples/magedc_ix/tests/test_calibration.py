# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Apache-2.0
"""Host regressions for calibration sampling and interrupted NVS writes.

Run: python3 tests/test_calibration.py
Uses the actual C modules and a small in-memory NVS with write-failure injection.
"""
import pathlib
import shutil
import subprocess
import tempfile
import unittest

MAIN = pathlib.Path(__file__).resolve().parents[1] / "main"
HEADERS = {
    "esp_log.h": """
#pragma once
#define ESP_LOGI(...) ((void)0)
#define ESP_LOGW(...) ((void)0)
#define ESP_LOGE(...) ((void)0)
""",
    "nvs_flash.h": "#pragma once\n",
    "esp_app_desc.h": """
#pragma once
#include <stdint.h>
typedef struct { char date[16], time[16]; uint8_t app_elf_sha256[32]; } esp_app_desc_t;
const esp_app_desc_t *esp_app_get_description(void);
""",
    "nvs.h": """
#pragma once
#include <stdint.h>
#include <stddef.h>
typedef int nvs_handle_t;
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_ERR_NVS_NOT_FOUND 1
#define NVS_READONLY 0
#define NVS_READWRITE 1
esp_err_t nvs_open(const char *, int, nvs_handle_t *);
void nvs_close(nvs_handle_t);
esp_err_t nvs_commit(nvs_handle_t);
esp_err_t nvs_erase_key(nvs_handle_t, const char *);
esp_err_t nvs_erase_all(nvs_handle_t);
esp_err_t nvs_get_blob(nvs_handle_t, const char *, void *, size_t *);
esp_err_t nvs_set_blob(nvs_handle_t, const char *, const void *, size_t);
esp_err_t nvs_get_str(nvs_handle_t, const char *, char *, size_t *);
esp_err_t nvs_set_str(nvs_handle_t, const char *, const char *);
esp_err_t nvs_get_u8(nvs_handle_t, const char *, uint8_t *);
esp_err_t nvs_set_u8(nvs_handle_t, const char *, uint8_t);
esp_err_t nvs_get_u32(nvs_handle_t, const char *, uint32_t *);
esp_err_t nvs_set_u32(nvs_handle_t, const char *, uint32_t);
esp_err_t nvs_get_i32(nvs_handle_t, const char *, int32_t *);
esp_err_t nvs_set_i32(nvs_handle_t, const char *, int32_t);
""",
}
HARNESS = r"""
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nvs.h"
#include "esp_app_desc.h"
#include "zone_storage.h"
#include "zone_wizard.h"
#include "zone_polar_detect.h"
#include "calib_nvs.h"

typedef struct { int ns; char key[32]; unsigned char data[1024]; size_t size; } entry_t;
static entry_t entries[32];
static const char *fail_op, *fail_key;
static uint32_t now = 1;
static int should_fail(const char *op, const char *key) {
    if (fail_op && strcmp(fail_op, op) == 0 && (!fail_key || strcmp(fail_key, key) == 0)) {
        fail_op = fail_key = NULL;
        return 1;
    }
    return 0;
}
static entry_t *find(nvs_handle_t ns, const char *key) {
    for (int i=0; i<32; ++i) if (entries[i].ns == ns && strcmp(entries[i].key, key) == 0) return &entries[i];
    return NULL;
}
static int get(nvs_handle_t ns, const char *key, void *out, size_t *size) {
    entry_t *e = find(ns, key);
    if (!e) return ESP_ERR_NVS_NOT_FOUND;
    if (*size < e->size) return 2;
    memcpy(out, e->data, e->size);
    *size = e->size;
    return ESP_OK;
}
static int set(nvs_handle_t ns, const char *key, const void *data, size_t size, const char *op) {
    if (should_fail(op, key)) return 2;
    entry_t *e = find(ns, key);
    if (!e) for (int i=0; i<32; ++i) if (entries[i].ns == 0) { e=&entries[i]; break; }
    assert(e && size <= sizeof(e->data));
    e->ns=ns; strcpy(e->key,key); e->size=size; memcpy(e->data,data,size);
    return ESP_OK;
}
int nvs_open(const char *ns, int mode, nvs_handle_t *out) { (void)mode; *out=strcmp(ns,"mag_zones")==0 ? 1 : 2; return ESP_OK; }
void nvs_close(nvs_handle_t h) { (void)h; }
int nvs_commit(nvs_handle_t h) { (void)h; return should_fail("commit", "") ? 2 : ESP_OK; }
int nvs_erase_key(nvs_handle_t h,const char *key) {
    if (should_fail("erase", key)) return 2;
    entry_t *e=find(h,key); if (!e) return ESP_ERR_NVS_NOT_FOUND; e->ns=0; return ESP_OK;
}
int nvs_erase_all(nvs_handle_t h) { for(int i=0;i<32;i++) if(entries[i].ns==h) entries[i].ns=0; return ESP_OK; }
int nvs_get_blob(nvs_handle_t h,const char *k,void *v,size_t *n) { return get(h,k,v,n); }
int nvs_set_blob(nvs_handle_t h,const char *k,const void *v,size_t n) { return set(h,k,v,n,"blob"); }
int nvs_get_str(nvs_handle_t h,const char *k,char *v,size_t *n) { return get(h,k,v,n); }
int nvs_set_str(nvs_handle_t h,const char *k,const char *v) { return set(h,k,v,strlen(v)+1,"str"); }
#define SCALAR(suffix,type) \
int nvs_get_##suffix(nvs_handle_t h,const char *k,type *v) { size_t n=sizeof(*v); return get(h,k,v,&n); } \
int nvs_set_##suffix(nvs_handle_t h,const char *k,type v) { return set(h,k,&v,sizeof(v),#suffix); }
SCALAR(u8,uint8_t)
SCALAR(u32,uint32_t)
SCALAR(i32,int32_t)
const esp_app_desc_t *esp_app_get_description(void) { static const esp_app_desc_t a={"Sep 22 2026","12:00:00",{1}}; return &a; }
void zone_tone_set_zone(int zone) { (void)zone; }

static bool feed(float x,float z,bool stable) { now+=21; return zone_wizard_feed_sample(x,0,z,stable,now); }
static void start(void) {
    zone_wizard_start();
    feed(0,0,true); now+=4000; feed(0,0,true);
    assert(zone_wizard_get_target_point()==5);
}
static void collect(float x,float z) {
    for(int i=0;i<98;i++) feed(x,z,true);
    assert(zone_wizard_get_phase()==ZONE_WIZARD_POINT_OK);
}
static bool advance(void) { now+=600; return feed(0,0,true); }
static void finish_remaining(void) {
    while(zone_wizard_get_phase()==ZONE_WIZARD_COLLECT) {
        int id=zone_wizard_get_target_point();
        collect(id==5 ? 0 : 1000+id*300, id==5 ? 0 : id*123);
        if(advance()) return;
    }
    assert(0);
}
static void save_center(void) {
    nvs_set_i32(2,"center_x",100); nvs_set_i32(2,"center_y",200); nvs_set_i32(2,"center_z",300);
    nvs_set_u8(2,"has_center",1); nvs_commit(2);
}
static void ready(void) { zone_storage_init(); start(); finish_remaining(); }
static void calibrated(void) { ready(); save_center(); assert(zone_wizard_complete()); }

static void test_z_and_restart(void) {
    ready();
    polar_zone_t before[9]; memcpy(before,zone_storage_get(),sizeof(before));
    for(int i=0;i<9;i++) {
        const polar_zone_t *z=&before[i];
        assert(z->z_max-z->z_min>=866);
        float mid=(z->z_min+z->z_max)*0.5f;
        assert(zone_polar_z_gate_score(mid,z)>0.99f);
        assert(zone_polar_z_gate_score(mid+10000,z)==0);
    }
    save_center(); assert(zone_wizard_complete());
    zone_storage_init();
    assert(zone_storage_calibration_complete());
    assert(memcmp(before,zone_storage_get(),sizeof(before))==0);
}
static void test_unstable_sampling(void) {
    start(); collect(0,0); assert(!advance()); assert(zone_wizard_get_target_point()==1);
    for(int i=0;i<50;i++) feed(1000,123,true);
    assert(zone_wizard_get_sample_count()==48);
    feed(2500,123,false);
    assert(zone_wizard_get_sample_count()==0);
    for(int i=0;i<50;i++) feed(4000,123,true);
    assert(zone_wizard_get_phase()==ZONE_WIZARD_COLLECT);
    for(int i=0;i<48;i++) feed(4000,123,true);
    assert(zone_wizard_get_phase()==ZONE_WIZARD_POINT_OK);
    advance(); finish_remaining();
    const polar_zone_t *z=&zone_storage_get()[0];
    assert(z->r_min>2500 && z->r_max<5500);
}
static void test_bad_quality(void) {
    start(); collect(0,0); advance();
    for(int i=0;i<50;i++) feed(1000,123,true);
    for(int i=0;i<48;i++) feed(4000,123,true);
    assert(zone_wizard_get_phase()==ZONE_WIZARD_COLLECT);
    assert(zone_wizard_get_sample_count()==0);
    assert(!zone_storage_wizard_done());
    collect(4000,123);
}
static void test_center_restart(void) {
    start();
    for(int i=0;i<98;i++) feed((float)i*5,0,true);
    assert(zone_wizard_get_phase()==ZONE_WIZARD_COLLECT);
    assert(zone_wizard_get_sample_count()==0);
    for(int i=0;i<98;i++) feed(i%2 ? -1 : 1,i%2 ? 1 : -1,true);
    assert(zone_wizard_get_phase()==ZONE_WIZARD_POINT_OK);
    advance(); finish_remaining();
    const polar_zone_t *center=&zone_storage_get()[4];
    assert(center->quality_score>=50);
}
static void test_completion_order(void) {
    ready();
    assert(!zone_storage_wizard_done());
    assert(!zone_wizard_complete());
    nvs_set_u8(2,"has_center",1); nvs_set_i32(2,"center_x",1); nvs_set_i32(2,"center_y",2);
    assert(!calib_nvs_has_center()); assert(!zone_wizard_complete());
    save_center();
    fail_op="u32"; fail_key="calib_schema";
    assert(!zone_wizard_complete()); assert(!zone_storage_wizard_done());
    fail_op="u8"; fail_key="wizard_done";
    assert(!zone_wizard_complete()); assert(!zone_storage_wizard_done());
    assert(zone_wizard_complete());
    zone_wizard_abort(); zone_storage_init();
    assert(zone_storage_calibration_complete() && calib_nvs_has_center());
}
static void test_failed_replacement(void) {
    calibrated();
    polar_zone_t old[9], changed[9];
    memcpy(old,zone_storage_get(),sizeof(old)); memcpy(changed,old,sizeof(old));
    changed[0].r_min+=100; changed[0].r_max+=100;
    fail_op="erase"; fail_key="wizard_done";
    assert(!zone_storage_save_zones(changed,9));
    zone_storage_init(); assert(zone_storage_calibration_complete());
    assert(memcmp(old,zone_storage_get(),sizeof(old))==0);
    fail_op="commit";
    assert(!zone_storage_save_zones(changed,9));
    assert(memcmp(old,zone_storage_get(),sizeof(old))==0);
    zone_storage_init(); assert(!zone_storage_calibration_complete());
    fail_op="blob"; fail_key="calib_v2";
    assert(!zone_storage_save_zones(changed,9));
    assert(memcmp(old,zone_storage_get(),sizeof(old))==0);
    zone_storage_init(); assert(!zone_storage_calibration_complete());
    assert(zone_storage_save_zones(changed,9)); save_center(); assert(zone_storage_mark_wizard_done());
    zone_storage_init(); assert(zone_storage_calibration_complete());
    assert(memcmp(changed,zone_storage_get(),sizeof(changed))==0);
}
static void test_corrupt_record(void) {
    calibrated();
    entry_t *blob=find(1,"calib_v2"); assert(blob);
    uint32_t wrong_count=8; memcpy(blob->data+sizeof(uint32_t),&wrong_count,sizeof(wrong_count));
    zone_storage_init();
    assert(zone_storage_wizard_done());
    assert(!zone_storage_calibration_complete());
    assert(!zone_storage_can_resume_after_sleep());
}
int main(int argc,char **argv) {
    assert(argc==2);
    if(strcmp(argv[1],"z_restart")==0) test_z_and_restart();
    else if(strcmp(argv[1],"unstable")==0) test_unstable_sampling();
    else if(strcmp(argv[1],"quality")==0) test_bad_quality();
    else if(strcmp(argv[1],"center")==0) test_center_restart();
    else if(strcmp(argv[1],"completion")==0) test_completion_order();
    else if(strcmp(argv[1],"replacement")==0) test_failed_replacement();
    else if(strcmp(argv[1],"corrupt")==0) test_corrupt_record();
    else abort();
    puts("PASS"); return 0;
}
"""


class CalibrationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        compiler = shutil.which("cc")
        if not compiler:
            raise unittest.SkipTest("A C compiler is required")
        cls.temp = tempfile.TemporaryDirectory(prefix="magedc_calibration_")
        cls.addClassCleanup(cls.temp.cleanup)
        root = pathlib.Path(cls.temp.name)
        for name, text in HEADERS.items():
            (root / name).write_text(text)
        source = root / "test_calibration.c"
        source.write_text(HARNESS)
        cls.binary = root / "test_calibration"
        modules = ("zone_wizard.c", "zone_storage.c", "zone_polar_detect.c", "calib_nvs.c")
        subprocess.run(
            [compiler, "-std=gnu11", "-I", str(root), "-I", str(MAIN), str(source),
             *[str(MAIN / module) for module in modules], "-lm", "-o", str(cls.binary)],
            check=True, capture_output=True, text=True,
        )

    def run_case(self, name):
        result = subprocess.run([str(self.binary), name], capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_fitted_z_gate_and_legacy_record_survive_restart(self):
        self.run_case("z_restart")

    def test_motion_restarts_current_point_sampling(self):
        self.run_case("unstable")

    def test_low_quality_fit_is_recollected(self):
        self.run_case("quality")

    def test_center_restarts_and_accepts_small_noise_across_zero(self):
        self.run_case("center")

    def test_completion_waits_for_center_and_successful_marker_write(self):
        self.run_case("completion")

    def test_failed_writes_never_publish_mixed_calibration(self):
        self.run_case("replacement")

    def test_invalid_record_cannot_resume_with_old_completion_marker(self):
        self.run_case("corrupt")


if __name__ == "__main__":
    unittest.main(verbosity=2)
