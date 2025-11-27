/**
 * This is simple application/system manager to manage the linux-side of the PicoCoreMX8MP.
 *
 * This can be used for various tasks such as:
 * - Suspending linux (STOP A53 cores)
 * - Installing Vimba drivers
 * - Start/stop camera control process
 * - Start/stop image processing (DIPP) process
 * - Start/stop Sat Uploader process
 * - Rebooting PicoCoreMX8MP
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <sys/select.h>

#include <csp/csp.h>
#include <csp/interfaces/csp_if_can.h>
#include <csp/drivers/usart.h>

#include <param/param.h>
#include <param/param_server.h>
#include <vmem/vmem.h>
#include <vmem/vmem_server.h>
#include <vmem/vmem_file.h>

extern vmem_t vmem_config;
VMEM_DEFINE_FILE(config, "config", "sys-man-config.vmem", 1000);

// --- VMEM PARAMETERS ---

void can_addr_callback();
void can_netmask_callback();
uint16_t _can_addr;
uint16_t _can_netmask;
param_t can_addr;
param_t can_netmask;
csp_iface_t* can_iface;
#define PARAMID_CAN_ADDR 21
#define PARAMID_CAN_NETMASK 22
PARAM_DEFINE_STATIC_VMEM(PARAMID_CAN_ADDR, can_addr, PARAM_TYPE_UINT16, -1, 0, PM_SYSCONF, can_addr_callback, "", config, 0x10, "CAN interface address");
PARAM_DEFINE_STATIC_VMEM(PARAMID_CAN_NETMASK, can_netmask, PARAM_TYPE_UINT16, -1, 0, PM_SYSCONF, can_netmask_callback, "", config, 0x12, "CAN interface netmask");

void suspend_on_boot_callback();
param_t suspend_on_boot;
#define PARAMID_SUSPEND_ON_BOOT 23
PARAM_DEFINE_STATIC_VMEM(PARAMID_SUSPEND_ON_BOOT, suspend_on_boot, PARAM_TYPE_UINT8, -1, 0, PM_SYSCONF, suspend_on_boot_callback, "", config, 0x100, "Whether to suspend A53 cores on boot");

// Uploader Configuration (Stored in VMEM for persistence)
// Index 0: Client Address, Index 1: Server Address
uint16_t _util[2] = {5424, 4100}; // Default values if VMEM is empty
param_t util;
#define PARAMID_UTIL 42
PARAM_DEFINE_STATIC_VMEM(PARAMID_UTIL, util, PARAM_TYPE_UINT16, 2, sizeof(_util), PM_CONF, NULL, "", config, 0x102, "Uploader Config [Client, Server]");

// --- RAM PARAMETERS ---

void suspend_a53_callback();
void vimba_install_callback();
void mng_camera_control_callback();
void mng_dipp_callback();
void switch_m7_bin_callback();
void mng_util_callback();

uint8_t _suspend_a53;
uint8_t _vimba_install;
uint16_t _mng_camera_control;
uint16_t _mng_dipp;
uint8_t _switch_m7_bin;
uint8_t _mng_dipp_interface;
char _mng_dipp_vmem_path[128];
uint8_t _mng_camera_interface;
char _mng_camera_vmem_path[128];
uint8_t _mng_util; // Switch to start/stop uploader
uint8_t _mng_util_interface; // 0=CAN, 1=KISS

#define PARAMID_SUSPEND_A53 31
#define PARAMID_VIMBA_INSTALL 32
#define PARAMID_MNG_CAMERA_CONTROL 33
#define PARAMID_MNG_DIPP 34
#define PARAMID_SWITCH_M7_BIN 35
#define PARAMID_MNG_DIPP_INTERFACE 36
#define PARAMID_MNG_DIPP_VMEM_PATH 37
#define PARAMID_MNG_CAMERA_INTERFACE 38
#define PARAMID_MNG_CAMERA_VMEM_PATH 39
#define PARAMID_MNG_UTIL 43
#define PARAMID_MNG_UTIL_INTERFACE 44

PARAM_DEFINE_STATIC_RAM(PARAMID_SUSPEND_A53, suspend_a53, PARAM_TYPE_UINT8, -1, 0, PM_CONF, suspend_a53_callback, "", &_suspend_a53, "STOP A53 cores (suspend linux)");
PARAM_DEFINE_STATIC_RAM(PARAMID_VIMBA_INSTALL, vimba_install, PARAM_TYPE_UINT8, -1, 0, PM_CONF, vimba_install_callback, "", &_vimba_install, "Install Vimba drivers");
PARAM_DEFINE_STATIC_RAM(PARAMID_MNG_CAMERA_CONTROL, mng_camera_control, PARAM_TYPE_UINT16, -1, 0, PM_CONF, mng_camera_control_callback, "", &_mng_camera_control, "Start camera control as this node number (0 kills it)");
PARAM_DEFINE_STATIC_RAM(PARAMID_MNG_DIPP, mng_dipp, PARAM_TYPE_UINT16, -1, 0, PM_CONF, mng_dipp_callback, "", &_mng_dipp, "Start DIPP as this node number (0 kills it)");
PARAM_DEFINE_STATIC_RAM(PARAMID_SWITCH_M7_BIN, switch_m7_bin, PARAM_TYPE_UINT8, -1, 0, PM_READONLY, NULL, "", &_switch_m7_bin, "Switch Cortex-M7 binaries"); // Disabled
PARAM_DEFINE_STATIC_RAM(PARAMID_MNG_DIPP_INTERFACE, mng_dipp_interface, PARAM_TYPE_UINT8, -1, 0, PM_CONF, NULL, "", &_mng_dipp_interface, "DIPP interface type (0=CAN, 1=KISS)");
PARAM_DEFINE_STATIC_RAM(PARAMID_MNG_DIPP_VMEM_PATH, mng_dipp_vmem_path, PARAM_TYPE_STRING, 128, 1, PM_CONF, NULL, "", &_mng_dipp_vmem_path, "DIPP vmem directory path");
PARAM_DEFINE_STATIC_RAM(PARAMID_MNG_CAMERA_INTERFACE, mng_camera_interface, PARAM_TYPE_UINT8, -1, 0, PM_CONF, NULL, "", &_mng_camera_interface, "Camera interface type (0=CAN, 1=KISS)");
PARAM_DEFINE_STATIC_RAM(PARAMID_MNG_CAMERA_VMEM_PATH, mng_camera_vmem_path, PARAM_TYPE_STRING, 128, 1, PM_CONF, NULL, "", &_mng_camera_vmem_path, "Camera vmem directory path");
PARAM_DEFINE_STATIC_RAM(PARAMID_MNG_UTIL, mng_util, PARAM_TYPE_UINT8, -1, 0, PM_CONF, mng_util_callback, "", &_mng_util, "Start Uploader (1=Start, 0=Stop)");
PARAM_DEFINE_STATIC_RAM(PARAMID_MNG_UTIL_INTERFACE, mng_util_interface, PARAM_TYPE_UINT8, -1, 0, PM_CONF, NULL, "", &_mng_util_interface, "Uploader interface (0=CAN, 1=KISS)");


// Circular buffer for standard output
#define STDBUF_SIZE 110
char _stdbuf[STDBUF_SIZE + 1];
char tmpbuf[STDBUF_SIZE];
int stdbuf_pos = 0;
#define PARAMID_STDBUF 41
PARAM_DEFINE_STATIC_RAM(PARAMID_STDBUF, stdout_buf, PARAM_TYPE_STRING, STDBUF_SIZE + 1, 1, PM_READONLY, NULL, "", &_stdbuf, "Standard output buffer");


void csp_print_func(const char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[256];
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    printf("%s", buf);

    for (int i = 0; buf[i] != '\0' && i < sizeof(buf); i++) {
        tmpbuf[stdbuf_pos] = buf[i];
        stdbuf_pos = (stdbuf_pos + 1) % STDBUF_SIZE;
    }
    for (int i = 0; i < STDBUF_SIZE; i++) {
        _stdbuf[i] = tmpbuf[(stdbuf_pos + i) % STDBUF_SIZE];
    }
    _stdbuf[STDBUF_SIZE] = '\0';
    param_set_string(&stdout_buf, _stdbuf, STDBUF_SIZE + 1);
}

void * vmem_server_task(void * param) {
	vmem_server_loop(param);
	return NULL;
}

void* router_task(void* param) {
    while (1) {
        csp_route_work();
    }
    return NULL;
}

uint32_t serial_get(void) {
    uint32_t hash = 0;
    char *date_time = __DATE__ __TIME__;
    for (int i = 0; date_time[i] != '\0'; i++) {
        hash = 31 * hash + date_time[i];
    }
    return hash;
}

void can_addr_callback() {
    _can_addr = param_get_uint16(&can_addr);
}

void can_netmask_callback() {
    _can_netmask = param_get_uint16(&can_netmask);
}

void suspend_a53_callback() {
    system("echo N > /sys/module/printk/parameters/console_suspend");
    system("echo mem > /sys/power/state");
}

void vimba_install_callback() {
    char buffer[128];
    FILE *fp;

    fp = popen("/etc/lib/VimbaX_2023-4-ARM64/cti/VimbaUSBTL_Install.sh 2>&1", "r");
    if (fp == NULL) {
        csp_print("Failed to run command\n");
        return;
    }
    while (fgets(buffer, sizeof(buffer)-1, fp) != NULL) {
        csp_print("%s", buffer);
    }
}

void mng_camera_control_callback() {
    char buffer[128];
    FILE *fp;
    uint16_t param_val = param_get_uint16(&mng_camera_control);

    switch (param_val) {
        case 0:  // Kill camera control
            fp = popen("pkill -f /usr/bin/Disco2CameraControl 2>&1", "r");
            if (fp == NULL) {
                csp_print("Failed to run command\n");
                return;
            }
            while (fgets(buffer, sizeof(buffer)-1, fp) != NULL) {
                csp_print("%s", buffer);
            }
            break;
        default:  // Start camera control
            char cmdbuf[256];
            uint8_t interface_type = param_get_uint8(&mng_camera_interface);
            char vmem_path[128];
            param_get_string(&mng_camera_vmem_path, vmem_path, sizeof(vmem_path));

            const char* interface_str = (interface_type == 1) ? "kiss" : "can";
            const char* device_str = (interface_type == 1) ? "/dev/ttymxc3" : "can0";

            if (strlen(vmem_path) > 0) {
                sprintf(cmdbuf, "/usr/bin/Disco2CameraControl -i %s -d %s -n %u -v %s 2>&1",
                        interface_str, device_str, param_val, vmem_path);
            } else {
                sprintf(cmdbuf, "/usr/bin/Disco2CameraControl -i %s -d %s -n %u 2>&1",
                        interface_str, device_str, param_val);
            }

            fp = popen(cmdbuf, "r");
            if (fp == NULL) {
                csp_print("Failed %s\n", cmdbuf);
                return;
            }
            break;
    }
}

void mng_dipp_callback() {
    char buffer[128];
    FILE *fp;

    uint16_t param_val = param_get_uint16(&mng_dipp);
    switch (param_val) {
        case 0:  // Kill dipp
            fp = popen("pkill -f /usr/bin/dipp 2>&1", "r");
            if (fp == NULL) {
                csp_print("Failed to run command\n");
                return;
            }
            while (fgets(buffer, sizeof(buffer)-1, fp) != NULL) {
                csp_print("%s", buffer);
            }
            break;
        default:  // Start dipp
            char cmdbuf[256];
            uint8_t interface_type = param_get_uint8(&mng_dipp_interface);
            char vmem_path[128];
            param_get_string(&mng_dipp_vmem_path, vmem_path, sizeof(vmem_path));

            const char* interface_str = (interface_type == 1) ? "KISS" : "CAN";
            const char* device_str = (interface_type == 1) ? "/dev/ttymxc3" : "can0";

            if (strlen(vmem_path) > 0) {
                sprintf(cmdbuf, "/usr/bin/dipp -i %s -p %s -a %u -v %s 2>&1",
                        interface_str, device_str, param_val, vmem_path);
            } else {
                sprintf(cmdbuf, "/usr/bin/dipp -i %s -p %s -a %u 2>&1",
                        interface_str, device_str, param_val);
            }

            fp = popen(cmdbuf, "r");
            if (fp == NULL) {
                csp_print("Failed %s\n", cmdbuf);
                return;
            }
            break;
    }
}

void mng_util_callback() {
    char buffer[128];
    FILE *fp;
    uint8_t start = param_get_uint8(&mng_util);

    if (start == 0) {
        // Kill uploader
        fp = popen("pkill -f /usr/bin/upload_sat-client 2>&1", "r");
        if (fp == NULL) {
            csp_print("Failed to run pkill command\n");
            return;
        }
        while (fgets(buffer, sizeof(buffer)-1, fp) != NULL) {
            csp_print("%s", buffer);
        }
    } else {
        // Start uploader
        uint16_t cfg[2];
        param_get_data(&util, cfg, sizeof(cfg));
        uint8_t interface_type = param_get_uint8(&mng_util_interface);
        
        uint16_t client_addr = cfg[0];
        uint16_t server_addr = cfg[1];

        // Sanity check defaults if VMEM was empty/zeroed
        if (client_addr == 0) client_addr = 5424;
        if (server_addr == 0) server_addr = 4100;

        // Determine device string (0 = CAN, 1 = KISS)
        const char* device_str = (interface_type == 1) ? "/dev/ttymxc3" : "can0";

        char cmdbuf[256];
        // ./usr/bin/upload_sat-client -c <DEVICE> -a <CLIENT_ADDRESS> -s <SERVER_ADDRESS>
        sprintf(cmdbuf, "/usr/bin/upload_sat-client -c %s -a %u -s %u 2>&1", 
                device_str, client_addr, server_addr);
        
        csp_print("Starting: %s\n", cmdbuf);

        fp = popen(cmdbuf, "r");
        if (fp == NULL) {
            csp_print("Failed to start uploader\n");
            return;
        }
        // Read immediate output if any (usually startup logs)
        // Since the process likely runs in background or hangs, popen might not return EOF 
        // immediately unless the program forks. If upload_sat-client blocks, this will block.
        // Assuming upload_sat-client behaves like standard daemons or we rely on pkill to stop it.
    }
}

void suspend_on_boot_callback() {
    uint8_t param_val = param_get_uint8(&suspend_on_boot);
    FILE *file = fopen("/home/root/suspend_on_boot", "w");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }
    fprintf(file, "%d", param_val);
    fclose(file);
}

int suspend_on_boot_read() {
    FILE *file = fopen("/home/root/suspend_on_boot", "r");
    if (file == NULL) {
        perror("Failed to open file");
        return -1;
    }
    int param_val;
    fscanf(file, "%d", &param_val);
    fclose(file);
    param_set_uint8(&suspend_on_boot, param_val);
    return param_val;
}

int main(int argc, char *argv[]) {
    // Default values
    uint16_t default_can_addr = 5421;
    uint16_t default_can_netmask = 8;
    uint16_t default_interface_type = 0;
    char* vmem_dir = NULL;
    char vmem_full_path[512];

    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--vmem-path") == 0) {
            if (i + 1 < argc) {
                vmem_dir = argv[i + 1];
                i++; 
            } else {
                fprintf(stderr, "Error: %s requires a directory path argument\n", argv[i]);
                return 1;
            }
        } else if (i == 1 && argv[i][0] != '-') {
            default_can_addr = (uint16_t)atoi(argv[i]);
        } else if (i == 2 && argv[i][0] != '-') {
            default_can_netmask = (uint16_t)atoi(argv[i]);
        } else if (i == 3 && argv[i][0] != '-') {
            default_interface_type = (uint16_t)atoi(argv[i]);
        }
    }

    if (vmem_dir != NULL) {
        snprintf(vmem_full_path, sizeof(vmem_full_path), "%s/sys-man-config.vmem", vmem_dir);
        ((vmem_file_driver_t *)vmem_config.driver)->filename = vmem_full_path;
        printf("Using vmem directory: %s\n", vmem_dir);
    }

    printf("Using CAN address: %u, netmask: %u\n", default_can_addr, default_can_netmask);
    
    csp_conf.hostname = "app-sys-manager";
    csp_conf.model = "PicoCoreMX8MP-Cortex-A53";
	csp_conf.version = 2;
	csp_conf.dedup = CSP_DEDUP_OFF;
    uint32_t serial = serial_get();
    char *serial_str = malloc(19 * sizeof(char));
    if (serial_str) {
        sprintf(serial_str, "Rev1.10-%u", serial);
        csp_conf.revision = serial_str;
    }

    csp_init();

    csp_bind_callback(csp_service_handler, CSP_ANY);
    csp_bind_callback(param_serve, PARAM_PORT_SERVER);

    static pthread_t vmem_server_handle;
	pthread_create(&vmem_server_handle, NULL, &vmem_server_task, NULL);

    static pthread_t router_handle;
    pthread_create(&router_handle, NULL, &router_task, NULL);

    _can_addr = param_get_uint16(&can_addr);
    _can_netmask = param_get_uint16(&can_netmask);
    if (_can_addr == 0) {
        _can_addr = default_can_addr;
        param_set_uint16(&can_addr, _can_addr);
    }
    if (_can_netmask == 0) {
        _can_netmask = default_can_netmask;
        param_set_uint16(&can_netmask, _can_netmask);
    }

    // Check uploader config defaults if VMEM is fresh
    uint16_t current_cfg[2];
    param_get_data(&util, current_cfg, sizeof(current_cfg));
    if (current_cfg[0] == 0) {
        // Set defaults [5424, 4100] to VMEM if empty
        uint16_t defaults[2] = {5424, 4100};
        param_set_data(&util, defaults, sizeof(defaults));
    }

    csp_iface_t* can_iface;

    if (default_interface_type == 0) {
        int error = csp_can_socketcan_open_and_add_interface("can0", "CAN", _can_addr, 1000000, 0, &can_iface);
        if (error != 0) {
            csp_print("Failed to open CAN interface\n");
            return error;
        }
        can_iface->name = "CAN";
    } else if (default_interface_type == 1) {
        csp_usart_conf_t conf = {
            "/dev/ttymxc3", 115200, 8, 1, 0, 0
        };
        int error = csp_usart_open_and_add_kiss_interface(&conf, "KISS", &can_iface);
        if (error != 0) {
            csp_print("Failed to open KISS interface\n");
            return error;
        }
        can_iface->name = "KISS";
    } else {
        csp_print("Invalid interface type: %u\n", default_interface_type);
        return -1;
    }

    can_iface->is_default = true;
    can_iface->addr = _can_addr;
    can_iface->netmask = _can_netmask;

    if (suspend_on_boot_read() >= 1) {
        suspend_a53_callback();
    }

    while (1) {
        sleep(1);
    }

    return 0;
}