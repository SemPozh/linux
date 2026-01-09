/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _UAPI_LINUX_OSI_CPUINFO_H
#define _UAPI_LINUX_OSI_CPUINFO_H

#include <linux/types.h>

#define OSI_CPUINFO_VERSION 1

struct osi_cpuinfo {
    __u32 version;          /* OSI_CPUINFO_VERSION */
    __u32 nr_cpus_online;   /* num_online_cpus() */
    __u32 nr_cpus_possible; /* num_possible_cpus() */

    __u32 cpu_family;
    __u32 cpu_model;
    __u32 cpu_stepping;

    __u32 cores_per_socket;   /* cpu_data(0).booted_cores or fallback */
    __u32 threads_per_core;   /* siblings / cores_per_socket (best-effort) */

    char vendor_id[16];     /* "GenuineIntel", "AuthenticAMD" ... */
    char model_name[64];    /* CPU brand string (best-effort) */
};

#endif
