#ifndef __linux__
    #error Error: Only platform running Linux is supported
#endif

#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <pci/pci.h>
#include <getopt.h>

#include "nxfetch.h"
#include "util.h"

static struct NxFetch nxf;

static void get_exec_user(void)
{
    char *user;

    user = getenv("USER");

    nxf.exec_username = user == NULL ? "unknown" : user;
}

static void get_os_info(void)
{
    FILE *fp;
    char buf[MAX_BUFF] = {0}, *tbuf;

    fp = fopen(OS_RELEASE_FILE_0, "r");

    if (fp == NULL) {
        /* Note: By default it should be in /etc and if not
        we'll check another directory */

        fp = fopen(OS_RELEASE_FILE_1, "r");

        /* And now we'll throw an error */
        if (fp == NULL) {
            nxf.os_info_str = "Unknown";
            goto end;
        }
    }

    fread(buf, 1, sizeof(buf), fp);
    fclose(fp);

    tbuf = read_value(buf, "PRETTY_NAME");

    if (tbuf == NULL) {
        nxf.os_info_str = "Unknown";
        goto end;
    }

    strrep(tbuf, "\"", "", 2);

    nxf.os_info_str = tbuf;
end:
    ;
    /* Do nothing */
}

static void get_cpu_info(void)
{
    FILE *fp;
    size_t i, k, len;
    char buf[MAX_LI_BUFF * 2] = {0};
    static char tbuf[MAX_LI_BUFF] = {0};

    fp = fopen(LINUX_CPUINFO, "r");

    if (fp == NULL)
        pxerr("fopen()");

    len = fread(buf, 1, sizeof(buf), fp);
    fclose(fp);

    for (i = 0; i < len; i++) {
        if (buf[i] == 'm' && buf[i + 1] == 'o' &&
            buf[i + 2] == 'd' && buf[i + 3] == 'e' &&
            buf[i + 6] == 'n') {
            for (k = 0; k < 13; k++)
                buf[i + k] = '\0';

            do {
                strncat(tbuf, &buf[i], 1);
                i++;
            } while (buf[i] != '\n');

            break;
        }
    }

    nxf.cpu_info_str = tbuf;
}

static void get_krnl_or_node(int krnl)
{
    struct utsname utn;
    /* We know it can't be larger than 65 bytes */
    static char buf[65] = {0};

    if (uname(&utn) == -1)
        pxerr("uname()");

    sprintf(buf, "%s", krnl ? utn.release : utn.nodename);

    nxf.krnl_or_node_str = buf;
}

static void get_sys_uptime(int show_secs, int is_short)
{
    struct sysinfo si;
    long days, hours, mins, secs;
    static char buf[MAX_LI_BUFF] = {0};

    if (sysinfo(&si) == -1)
        pxerr("sysinfo()");

    days = si.uptime / (24 * 60 * 60);
    hours = (si.uptime / (60 * 60)) - (days * 24);
    mins = (si.uptime / 60) - (days * (24 * 60)) - (hours * 60);
    secs = si.uptime - (days * (24 * 60 * 60)) - (hours * 60 * 60) - (mins * 60);

    ASSERT(days >= 0 && hours >= 0 && mins >= 0 && secs >= 0);

    if (show_secs && !is_short) {
        if (days == 0 && hours == 0 && mins == 0) {
            sprintf(buf,
                "%ld secs", secs
            );
        } else if (days == 0 && hours == 0) {
            sprintf(buf,
                "%ld mins, %ld secs"
                , mins, secs
            );
        } else if (days == 0) {
            sprintf(buf,
                "%ld hours, %ld mins, %ld secs"
                , hours, mins, secs
            );
        } else {
            sprintf(buf,
                "%ld days, %ld hours, %ld mins, %ld secs"
                , days, hours, mins, secs
            );
        }
    }

    if (show_secs && is_short) {
        if (days == 0 && hours == 0 && mins == 0) {
            sprintf(buf,
                "%lds", secs
            );
        } else if (days == 0 && hours == 0) {
            sprintf(buf,
                "%ldm %lds"
                , mins, secs
            );
        } else if (days == 0) {
            sprintf(buf,
                "%ldh %ldm %lds"
                , hours, mins, secs
            );
        } else {
            sprintf(buf,
                "%ldd %ldh %ldm %lds"
                , days, hours, mins, secs
            );
        }
    }

    if (!show_secs && is_short) {
        if (days == 0 && hours == 0) {
            sprintf(buf,
                "%ldm", mins 
            );
        } else if (days == 0) {
            sprintf(buf,
                "%ldh %ldm"
                , hours, mins
            );
        } else {
            sprintf(buf,
                "%ldd %ldh %ldm"
                , days, hours, mins
            );
        }
    }

    if (!show_secs && !is_short) {
        if (days == 0 && hours == 0) {
            sprintf(buf,
                "%ld mins"
                , mins
            );
        } else if (days == 0) {
            sprintf(buf,
                "%ld hours, %ld mins"
                , hours, mins
                );
        } else {
            sprintf(buf,
                "%ld days, %ld hours, %ld mins"
                , days, hours, mins
            );
        }
    }
    
    nxf.sys_info_uptime = buf;
}
    
static void get_pkgs_count(void)
{
    FILE *fp;
    pid_t pid;
    int pfd[2];
    size_t i, sz, nsz;
    static char pcount[MAX_PKG_COUNT] = {0};

    for (i = 0; i < ARRAY_SIZE(pkgs); i++) {
        if (access(pkgs[i].pkg_mgr, F_OK) == 0) {
            if (pipe(pfd) == -1)
                pxerr("pipe()");

            pid = fork();

            switch (pid) {                
            case 0:
                close(pfd[0]);
                dup2(pfd[1], STDOUT_FILENO);
                close(pfd[1]);

                if (execl(SHELL_PATH, "sh", "-c", pkgs[i].arg, NULL) == -1) {
                    nxf.pkgs_info_count = "Unknown";
                    pxerr("execl()");
                }

                break;

            case -1:
                close(pfd[0]);
                close(pfd[1]);
                pxerr("fork()");
            }

            close(pfd[1]);

            fp = fdopen(pfd[0], "r");
            sz = fread(pcount, 1, sizeof(pcount), fp);
            fclose(fp);

            nsz = sizeof(pcount) - sz - 1;
            pcount[nsz] = '\0';
            nxf.pkgs_info_count = pcount;

            break;
        }

        if (i == (ARRAY_SIZE(pkgs) - 1))
            nxf.pkgs_info_count = "Unknown";
    }
}

static void get_default_shell(void)
{
    char *shell;

    shell = getenv("SHELL");

    if (shell == NULL)
        pxerr("getenv()");

    nxf.default_info_shell = strccut(shell, '/');
}

static void get_default_de_or_wm(void)
{
    char *dwm;

    dwm = getenv("XDG_CURRENT_DESKTOP");

    if (dwm == NULL) {
        /* For older version of Cinnamon DE */
        dwm = getenv("X-CINNAMON");

        if (dwm == NULL) {
            dwm = getenv("WINDOW_MANAGER");

            if (dwm == NULL) {
                nxf.default_dwm_win = tofupper("Unknown");
                goto end;
            }
        }
    }

    nxf.default_dwm_win = word_trim(tofupper(dwm), strlen(dwm));

end: ;
    /* Do nothing */
}

static void get_gpu_vendor(void)
{
    size_t i;
    struct pci_dev *pd;
    struct pci_access *pa;

    pa = pci_alloc();

    if (pa == NULL)
        pxerr("pci_alloc()");

    pci_init(pa);
    pci_scan_bus(pa);

    for (pd = pa->devices; pd; pd = pd->next) {
        pci_fill_info(
            pd, PCI_FILL_IDENT | PCI_FILL_CLASS | PCI_FILL_BASES
        );
        
        if (pd->device_id == PCI_DEVICE_ID_VBOX_ADAPTER &&
            pd->vendor_id == PCI_VENDOR_ID_VIRTUALBOX) {
            nxf.gpu_vendor_name = pciid[6].corp;
            goto end;
        }

        if (pd->device_class == PCI_CLASS_DISPLAY_VGA ||
            pd->device_class == PCI_CLASS_DISPLAY_XGA ||
            pd->device_class == PCI_CLASS_DISPLAY_3D) {

            for (i = 0; i < ARRAY_SIZE(pciid); i++) {
                if (pd->vendor_id == pciid[i].id) {
                    nxf.gpu_vendor_name = pciid[i].corp;
                    goto end;
                }
            }
        }
    }

    nxf.gpu_vendor_name = "Unknown";
end:
    pci_cleanup(pa);
}

static void get_memory_or_swap_info(int memory)
{
    size_t sz;
    struct sysinfo si;
    static char buf[MAX_LI_BUFF] = {0};
    char totalram[MAX_BI_BUFF] = {0}, freeram[MAX_BI_BUFF] = {0},
        totalswap[MAX_BI_BUFF] = {0}, freeswap[MAX_BI_BUFF] = {0};

    if (sysinfo(&si) == -1)
        pxerr("sysinfo()");

    human_bytes(totalram, si.totalram);
    human_bytes(freeram, si.freeram);
    human_bytes(totalswap, si.totalswap);
    human_bytes(freeswap, si.freeswap);

    if (memory)
        sprintf(buf, "%s (total), %s (free)", totalram, freeram);
    else
        sprintf(buf, "%s (total), %s (free)", totalswap, freeswap);

    sz = strlen(buf);
    buf[sz] = '\0';
    nxf.mem_or_swap_info = buf;
}

static void get_os_name(void)
{
    FILE *fp;
    static char *ret;
    char buf[MAX_BUFF] = {0}, *val;

    fp = fopen("/etc/os-release", "r");

    if (fp == NULL) {
        fp = fopen("/usr/lib/os-release", "r");

        if (fp == NULL) {
            if (access("/bedrock/etc/bedrock-release", F_OK) == 0) {
                sprintf(nxf.os_name_str, "bedrock");
                goto end;
            }

            nxf.os_name_str = "./logo/unknown";
            goto end;
        }
    }

    fread(buf, 1, sizeof(buf), fp);
    fclose(fp);

    ret = strrep(buf, "VERSION_ID", "TMP", 1);
    val = read_value(buf, "ID");

    if (val == NULL) {
        nxf.os_name_str = "./logo/unknown";
        goto end;
    }

    sprintf(ret, "./logo/%s", toalllower(val));

    nxf.os_name_str = ret;

end: ;
    /* Do nothing */
}

static void print_all_info(void)
{
    FILE *fp;
    size_t i, sz;
    char *buf, *tbuf;
    const char *colors_str[] = {
        "$BLACK$", "$RED$", "$GREEN$", "$YELLOW$",
        "$BLUE$", "$PURPLE$", "$CYAN$", "$WHITE$",
        "$END$"
    };
    const char *colors_cst[] = {
        BLACK, RED, GREEN, YELLOW, BLUE, PURPLE,
        CYAN, WHITE, END
    };

    get_os_name();

    fp = fopen(nxf.os_name_str, "r");

    if (fp == NULL) {
        fprintf(stderr,
            "Error: No ascii logo found for %s\n", nxf.os_name_str
        );
        exit(EXIT_FAILURE);
    }

    buf = xcalloc(MAX_BUFF);
    fread(buf, 1, MAX_BUFF, fp);
    fclose(fp);

    get_exec_user();
    get_os_info();
    get_krnl_or_node(FALSE);
    get_sys_uptime(TRUE, TRUE);
    get_pkgs_count();
    get_default_de_or_wm();
    get_default_shell();
    get_cpu_info();
    get_gpu_vendor();

    for (i = 0; i < ARRAY_SIZE(colors_cst); i++)
        strrep(buf, colors_str[i], colors_cst[i], MAX_ITER);

    strrep(buf, "$BLOCK$", ":::", MAX_ITER);

    sz = strlen(nxf.exec_username) + strlen(nxf.krnl_or_node_str);
    tbuf = xcalloc(sz + 2);

    strrep(buf, "$USER$", nxf.exec_username, 1);
    strrep(buf, "$HOST$", nxf.krnl_or_node_str, 1);

    for (i = 0; i <= sz; i++)
        strncat(tbuf, "-", 2);

    strrep(buf, "$LINE$", tbuf, 1);
    free(tbuf);

    strrep(buf, "$OS$", nxf.os_info_str, 1);

    get_krnl_or_node(TRUE);
    strrep(buf, "$KERNEL$", nxf.krnl_or_node_str, 1);
    strrep(buf, "$UPTIME$", nxf.sys_info_uptime, 1);
    strrep(buf, "$PACKAGES$", nxf.pkgs_info_count, 1);
    strrep(buf, "$DWM$", nxf.default_dwm_win, 1);
    strrep(buf, "$SHELL$", nxf.default_info_shell, 1);
    strrep(buf, "$CPU$", nxf.cpu_info_str, 1);
    strrep(buf, "$GPU$", nxf.gpu_vendor_name, 1);

    get_memory_or_swap_info(TRUE);
    strrep(buf, "$MEMORY$", nxf.mem_or_swap_info, 1);

    get_memory_or_swap_info(FALSE);
    strrep(buf, "$SWAP$", nxf.mem_or_swap_info, 1);

    fprintf(stdout, "%s", buf);
    free(buf);
}

static void print_text_info(void)
{
    get_exec_user();
    get_os_info();
    get_sys_uptime(TRUE, TRUE);
    get_pkgs_count();
    get_default_de_or_wm();
    get_default_shell();
    get_cpu_info();
    get_gpu_vendor();
    get_memory_or_swap_info(TRUE);

    get_krnl_or_node(FALSE);
    fprintf(stdout, "Host: %s\n", nxf.krnl_or_node_str);

    get_krnl_or_node(TRUE);
    fprintf(stdout,
        "User: %s\n" "OS: %s\n"
        "Kernel: %s\n" "Uptime: %s\n"
        "Packages: %s\n" "DE/WM: %s\n"
        "Shell: %s\n" "CPU: %s\n"
        "G-Vendor: %s\n" "Memory: %s\n"
        , nxf.exec_username, nxf.os_info_str
        , nxf.krnl_or_node_str, nxf.sys_info_uptime
        , nxf.pkgs_info_count, nxf.default_dwm_win
        , nxf.default_info_shell, nxf.cpu_info_str
        , nxf.gpu_vendor_name, nxf.mem_or_swap_info
    );

    get_memory_or_swap_info(FALSE);

    fprintf(stdout,
        "Swap: %s\n"
        , nxf.mem_or_swap_info
    );
}

static void usage(void)
{
    fprintf(stdout,
        "%s - A fetch utility for linux systems\n\n"
        "Usage\n"
        "   --all       - fetch information with OS ascii icon, if available\n"
        "   --textall   - fetch information without ascii icon\n"
        "   --user      - current logged in username\n"
        "   --os        - currently running unix-based OS name\n"
        "   --cpu       - currently running CPU name\n"
        "   --kernel    - kernel version\n"
        "   --host      - currently running host name\n"
        "   --uptime    - systemwide uptime\n"
        "   --pkgcnt    - count numbers of packages installed\n"
        "   --shell     - default logged in users' shell\n"
        "   --dwm       - default DE or WM, if available\n"
        "   --gpu       - single GPU vendor name\n"
        "   --memory    - total RAM and free RAM\n"
        "   --swap      - total swap and free swap\n"
        , PROGRAM
    );
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        usage();
        exit(EXIT_SUCCESS);
    }

    if (argv[1][0] != '-') {
        fprintf(stdout, "a: %c\nb: %c\n", argv[1][0], argv[1][1]);
        usage();
        exit(EXIT_FAILURE);
    }

    int opt, optidx;

    optidx = 0;

    struct option long_opts[] = {
        { "all", no_argument, 0, 0 },
        { "textall", no_argument, 0, 1 },
        { "user", no_argument, 0, 2 },
        { "os", no_argument, 0, 3 },
        { "cpu", no_argument, 0, 4 },
        { "kernel", no_argument, 0, 5 },
        { "host", no_argument, 0, 6 },
        { "uptime", required_argument, 0, 7 },
        { "pkgcnt", no_argument, 0, 8 },
        { "shell", no_argument, 0, 9 },
        { "dwm", no_argument, 0, 10 },
        { "gpu", no_argument, 0, 11 },
        { "memory", no_argument, 0, 12 },
        { "swap", no_argument, 0, 13 },
        { "help", no_argument, 0, 14 },
        { 0,      0,           0, 0 }
    };

    while ((opt = getopt_long(argc, argv, "", long_opts, &optidx)) != -1) {
        switch (opt) {
        case 0:
            print_all_info();
            break;

        case 1:
            print_text_info();
            break;

        case 2:
            get_exec_user();
            fprintf(stdout, "%s\n", nxf.exec_username);
            break;

        case 3:
            get_os_info();
            fprintf(stdout, "%s\n", nxf.os_info_str);
            break;

        case 4:
            get_cpu_info();
            fprintf(stdout, "%s\n", nxf.cpu_info_str);
            break;

        case 5:
            get_krnl_or_node(TRUE);
            fprintf(stdout, "%s\n", nxf.krnl_or_node_str);
            break;

        case 6:
            get_krnl_or_node(FALSE);
            fprintf(stdout, "%s\n", nxf.krnl_or_node_str);
            break;

        case 7:
            if (strncmp(optarg, "secs", 5) == 0)
                get_sys_uptime(TRUE, FALSE);
            else if (strncmp(optarg, "short", 6) == 0)
                get_sys_uptime(FALSE, TRUE);
            else if (strncmp(optarg, "secsshort", 10) == 0 ||
                strncmp(optarg, "short secs", 11) == 0)
                get_sys_uptime(TRUE, TRUE);
            else if (strncmp(optarg, "nosecs", 7) == 0)
                get_sys_uptime(FALSE, FALSE);
            else
                get_sys_uptime(FALSE, FALSE);

            fprintf(stdout, "%s\n", nxf.sys_info_uptime);
            break;

        case 8:
            get_pkgs_count();
            fprintf(stdout, "%s\n", nxf.pkgs_info_count);
            break;

        case 9:
            get_default_shell();
            fprintf(stdout, "%s\n", nxf.default_info_shell);
            break;

        case 10:
            get_default_de_or_wm();
            fprintf(stdout, "%s\n", nxf.default_dwm_win);
            break;

        case 11:
            get_gpu_vendor();
            fprintf(stdout, "%s\n", nxf.gpu_vendor_name);
            break;

        case 12:
            get_memory_or_swap_info(TRUE);
            fprintf(stdout, "%s\n", nxf.mem_or_swap_info);
            break;

        case 13:
            get_memory_or_swap_info(FALSE);
            fprintf(stdout, "%s\n", nxf.mem_or_swap_info);
            break;

        case 14:
            usage();
            break;

        case '?':
        default:
            exit(EXIT_FAILURE);
        }
    }
}
