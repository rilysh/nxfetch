#ifndef NXFETCH_H
#define NXFETCH_H   1

#include <stdarg.h>

#undef PROGRAM
#define PROGRAM     "nxfetch"

#ifndef TRUE
    #define TRUE    1
#endif
#ifndef FALSE
    #define FALSE   0
#endif
#define MAX_BUFF        1024
#define MAX_LI_BUFF     100
#define MAX_PKG_COUNT   10
#define MAX_BI_BUFF     20
#define MAX_ITER        (MAX_LI_BUFF)
#define OS_RELEASE_FILE_0   "/etc/os-release"
#define OS_RELEASE_FILE_1   "/usr/lib/os-release"
#define LINUX_CPUINFO       "/proc/cpuinfo"

#define ASSERT(x)                                           \
    do {                                                    \
        if (!(x)) {                                         \
            fprintf(stderr,                                 \
                "Error: Assertation failure at line %d, "   \
                "reason %s\n"                               \
                , __LINE__, #x                              \
            );                                              \
            exit(EXIT_FAILURE);                             \
        }                                                   \
    } while (FALSE);

#define ARRAY_SIZE(x)       (sizeof(x) / sizeof(x[0]))
#define SHELL_PATH          "/bin/sh"

#ifndef PCI_CLASS_DISPLAY_VGA
    #define PCI_CLASS_DISPLAY_VGA       0x0300
#endif

#ifndef PCI_VENDOR_ID_INTEL
    #define PCI_VENDOR_ID_INTEL		    0x8086
#endif
#ifndef PCI_VENDOR_ID_COMPAQ
    #define PCI_VENDOR_ID_COMPAQ		0x0E11
#endif
#ifndef PCI_VENDOR_ID_AMD
    #define PCI_VENDOR_ID_AMD           0x1022
#endif
#ifndef PCI_VENDOR_ID_ACER
    #define PCI_VENDOR_ID_ACER          0x18E4
#endif
#ifndef PCI_VENDOR_ID_NVIDIA
    #define PCI_VENDOR_ID_NVIDIA        0x10DE
#endif
#ifndef PCI_VENDOR_ID_MICROSOFT
    #define PCI_VENDOR_ID_MICROSOFT     0x1414
#endif
#ifndef PCI_VENDOR_ID_GOOGLE
    #define PCI_VENDOR_ID_GOOGLE        0x1AE0
#endif
#ifndef PCI_VENDOR_ID_VMWARE
    #define PCI_VENDOR_ID_VMWARE        0x15AD
#endif
#ifndef PCI_VENDOR_ID_VIRTUALBOX
    #define PCI_VENDOR_ID_VIRTUALBOX    0x80EE
#endif

#define PCI_DEVICE_ID_VBOX_ADAPTER      0xBEEF

#define BLACK      "\033[1;90m"
#define RED        "\033[1;91m"
#define GREEN      "\033[1;92m"
#define YELLOW     "\033[1;93m"
#define BLUE       "\033[1;94m"
#define PURPLE     "\033[1;95m"
#define CYAN       "\033[1;96m"
#define WHITE      "\033[1;97m"
#define END         "\033[0m"

struct NxFetch {
    char *exec_username;
    char *os_info_str;
    char *cpu_info_str;
    char *krnl_or_node_str;
    char *sys_info_uptime;
    char *pkgs_info_count;
    char *default_info_shell;
    char *default_dwm_win;
    char *gpu_vendor_name;
    char *mem_or_swap_info;
    char *os_name_str;
};

struct Pkgs {
    char *pkg_mgr;
    char *arg;
};

struct PCIId {
    unsigned int id;
    char *corp;
};

struct Pkgs pkgs[] = {
    { "/usr/bin/alps", "alps showinstalled | wc -l" },
    { "/usr/bin/butch", "butch list | wc -l" },
    { "/usr/bin/swupd", "swupd bundle-list --quiet | wc -l" },
    { "/usr/bin/pisi", "pisi li | wc -l" },
    { "/usr/bin/pacstall", "pacstall -L | wc -l" },
    { "/usr/bin/emerge", "ls /var/db/pkg/*/* | wc -l" },
    { "/usr/bin/eopkg", "ls /var/lib/eopkg/package/* | wc -l" },
    { "/usr/bin/pkgtool", "ls /var/log/packages/* | wc -l" },
    { "/usr/bin/nix-user", "nix-user-pkgs | wc -l" },
    { "/usr/bin/pacman", "pacman -Qq --color never | wc -l" },
    { "/usr/bin/xbps-query", "xbps-query -l | wc -l" },
    { "/usr/bin/apk", "apk info | wc -l" },
    { "/usr/bin/opkg", "opkg list-installed | wc -l" },
    { "/usr/bin/lvu", "lvu installed | wc -l" },
    { "/usr/bin/tce-status", "tce-status -i | wc -l" },
    { "/bin/sorcery", "gaze installed | wc -l" },
    { "/usr/bin/cpt-list", "cpt-list | wc -l" },
    { "/usr/bin/rpm", "rpm -qa | wc -l" },
    { "/usr/bin/dpkg-query", "dpkg-query -f '.\\n' -W | wc -l" },
};

struct PCIId pciid[] = {
    { PCI_VENDOR_ID_INTEL, "Intel Corporation" },
    { PCI_VENDOR_ID_AMD, "Advanced Micro Devices, Inc." },
    { PCI_VENDOR_ID_NVIDIA, "NVIDIA Corporation" },
    { PCI_VENDOR_ID_GOOGLE, "Google, Inc." },
    { PCI_VENDOR_ID_MICROSOFT, "Microsoft Corporation" },
    { PCI_VENDOR_ID_VMWARE, "VMware, Inc." },
    { PCI_VENDOR_ID_VIRTUALBOX, "InnoTek Systemberatung GmbH" },
    { PCI_VENDOR_ID_COMPAQ, "Compaq Computer Corporation" }
};

#endif /* NXFETCH_H */
