/*
   Copyright (c) 2016, The CyanogenMod Project
   Copyright (C) 2019 The LineageOS Project.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.
   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <android-base/properties.h>

#include <sys/sysinfo.h>

#include <stdlib.h>
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include "vendor_init.h"
#include "property_service.h"
#include "log/log.h"
#include <sys/stat.h>
#include <sys/types.h>

char const *heaptargetutilization;
char const *heapminfree;
char const *heapmaxfree;

void check_device() {
    struct sysinfo sys;
    sysinfo(&sys);

    if (sys.totalram > 2048ull * 1024 * 1024) {
        // from phone-xhdpi-4096-dalvik-heap.mk
        heaptargetutilization = "0.6";
        heapminfree = "8m";
        heapmaxfree = "16m";
    } else {
        // from phone-xhdpi-2048-dalvik-heap.mk
        heaptargetutilization = "0.75";
        heapminfree = "512k";
        heapmaxfree = "8m";
    }
}

/* From Magisk@jni/magiskhide/hide_utils.c */
static const char *snet_prop_key[] = {
	"ro.boot.vbmeta.device_state",
	"ro.boot.verifiedbootstate",
	"ro.boot.flash.locked",
	"ro.boot.selinux",
	"ro.boot.veritymode",
	"ro.boot.warranty_bit",
	"ro.warranty_bit",
	"ro.debuggable",
	"ro.secure",
	"ro.build.type",
	"ro.build.tags",
	"ro.build.selinux",
	NULL
};

static const char *snet_prop_value[] = {
	"locked",
	"green",
	"1",
	"enforcing",
	"enforcing",
	"0",
	"0",
	"0",
	"1",
	"user",
	"release-keys",
	"1",
	NULL
};

void property_override(char const prop[], char const value[], bool add = true) {
    auto pi = (prop_info *) __system_property_find(prop);

    if (pi != nullptr) {
        __system_property_update(pi, value, strlen(value));
    } else if (add) {
        __system_property_add(prop, strlen(prop), value, strlen(value));
    }
}

void property_override_triple(char const product_prop[], char const system_prop[], char const vendor_prop[],
    char const value[]) {
    property_override(product_prop, value);
    property_override(system_prop, value);
    property_override(vendor_prop, value);
}

void set_avoid_gfxaccel_config() {
    struct sysinfo sys;
    sysinfo(&sys);

    if (sys.totalram <= 3072ull * 1024 * 1024) {
        // Reduce memory footprint
        property_override("ro.config.avoid_gfx_accel", "true");
    }
}

static void workaround_snet_properties() {
	// Hide all sensitive props
	for (int i = 0; snet_prop_key[i]; ++i) {
		property_override(snet_prop_key[i], snet_prop_value[i]);
	}
	chmod("/sys/fs/selinux/enforce", 0640);
	chmod("/sys/fs/selinux/policy", 0440);
}

void vendor_load_properties()
{
    check_device();
    set_avoid_gfxaccel_config();
    // Workaround SafetyNet
    workaround_snet_properties();

    property_override("dalvik.vm.heapstartsize", "8m");
    property_override("dalvik.vm.heapgrowthlimit", "192m");
    property_override("dalvik.vm.heapsize", "512m");
    property_override("dalvik.vm.heaptargetutilization", heaptargetutilization);
    property_override("dalvik.vm.heapminfree", heapminfree);
    property_override("dalvik.vm.heapmaxfree", heapmaxfree);

    property_override("ro.product.model", "Redmi 5");
    property_override("ro.build.product", "rosy");
    property_override("ro.product.device", "rosy");
    property_override("ro.build.description", "redfin-user 11 RD1A.201105.003.C1 6886399 release-keys");
    property_override_triple("ro.build.fingerprint", "ro.system.build.fingerprint", "ro.vendor.build.fingerprint", "google/redfin/redfin:11/RQ1D.210105.003/7005430:user/release-keys");
}
