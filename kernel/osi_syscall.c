// SPDX-License-Identifier: GPL-2.0
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/smp.h>

#include <uapi/linux/osi_cpuinfo.h>

#ifdef CONFIG_X86
#include <asm/processor.h>   // cpu_data()
#endif

SYSCALL_DEFINE2(osi_hello, struct osi_cpuinfo __user *, uinfo, size_t, usize)
{
    struct osi_cpuinfo info;

    if (!uinfo)
        return -EINVAL;

    /* чтобы можно было расширять struct без поломки старых программ */
    if (usize < sizeof(struct osi_cpuinfo))
        return -EINVAL;

    memset(&info, 0, sizeof(info));

    info.version = OSI_CPUINFO_VERSION;
    info.nr_cpus_online   = num_online_cpus();
    info.nr_cpus_possible = num_possible_cpus();

#ifdef CONFIG_X86
    /* Берём информацию по CPU0 (для QEMU обычно одинаково для всех) */
    info.cpu_family   = cpu_data(0).x86;
    info.cpu_model    = cpu_data(0).x86_model;
    info.cpu_stepping = cpu_data(0).x86_stepping;

    strscpy(info.vendor_id,  cpu_data(0).x86_vendor_id, sizeof(info.vendor_id));
    strscpy(info.model_name, cpu_data(0).x86_model_id,  sizeof(info.model_name));

    /*
     * best-effort topology:
     * siblings = threads per socket, booted_cores = cores per socket
     */
    if (cpu_data(0).booted_cores)
        info.cores_per_socket = cpu_data(0).booted_cores;
    else
        info.cores_per_socket = 1;

    if (cpu_data(0).x86_max_cores)
        info.cores_per_socket = cpu_data(0).x86_max_cores;

    if (info.cores_per_socket) {
        unsigned int sib = cpu_data(0).x86_max_cores ? (cpu_data(0).x86_max_cores) : info.cores_per_socket;
        /* Реально siblings есть не во всех полях одинаково, поэтому делаем простой расчёт */
        if (cpu_data(0).booted_cores && cpu_data(0).x86_max_cores)
            sib = cpu_data(0).x86_max_cores; /* fallback */

        /* На практике для QEMU проще: threads_per_core = online / cores_per_socket (если >0) */
        if (info.cores_per_socket && info.nr_cpus_online)
            info.threads_per_core = max(1U, info.nr_cpus_online / info.cores_per_socket);
        else
            info.threads_per_core = 1;
    } else {
        info.threads_per_core = 1;
    }
#else
    /* если не x86 — просто что-то разумное */
    info.cores_per_socket  = 1;
    info.threads_per_core  = 1;
#endif

    pr_info("osi_hello: cpuinfo requested (online=%u)\n", info.nr_cpus_online);

    if (copy_to_user(uinfo, &info, sizeof(info)))
        return -EFAULT;

    return 0;
}
