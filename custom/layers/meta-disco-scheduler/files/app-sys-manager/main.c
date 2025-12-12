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
 *  - run `reboot` command (affects Cortex-M7 application as well)
 *
 * - Time synchronization
 *  - set `time_sync_node` to the node address to sync from (0 = disabled)
 *  - set `time_sync_request` to any value to trigger immediate sync
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h> 
#include <stdarg.h>
#include <sys/select.h>
#include <time.h>
#include <string.h>

#include <csp/csp.h>
#include <csp/csp_cmp.h>
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
void dipp_kiss_baudrate_callback();
void dipp_kiss_netmask_callback();
void mng_util_callback(); 

uint16_t _can_addr;
uint16_t _can_netmask;
uint32_t _dipp_kiss_baudrate;
uint16_t _dipp_kiss_netmask;

param_t can_addr;
param_t can_netmask;
param_t dipp_kiss_baudrate;
param_t dipp_kiss_netmask;
csp_iface_t* can_iface;
#define CAN_ADDR_DEFAULT 21
#define CAN_NETMASK_DEFAULT 8
#define PARAMID_CAN_ADDR 21
#define PARAMID_CAN_NETMASK 22
#define PARAMID_DIPP_KISS_BAUDRATE 24
#define PARAMID_DIPP_KISS_NETMASK 25

PARAM_DEFINE_STATIC_VMEM(PARAMID_CAN_ADDR, can_addr, PARAM_TYPE_UINT16, -1, 0, PM_SYSCONF, can_addr_callback, "", config, 0x10, "CAN interface address");
PARAM_DEFINE_STATIC_VMEM(PARAMID_CAN_NETMASK, can_netmask, PARAM_TYPE_UINT16, -1, 0, PM_SYSCONF, can_netmask_callback, "", config, 0x12, "CAN interface netmask");
PARAM_DEFINE_STATIC_VMEM(PARAMID_DIPP_KISS_BAUDRATE, dipp_kiss_baudrate, PARAM_TYPE_UINT32, -1, 0, PM_SYSCONF, dipp_kiss_baudrate_callback, "", config, 0x14, "DIPP KISS interface baudrate");
PARAM_DEFINE_STATIC_VMEM(PARAMID_DIPP_KISS_NETMASK, dipp_kiss_netmask, PARAM_TYPE_UINT16, -1, 0, PM_SYSCONF, dipp_kiss_netmask_callback, "", config, 0x18, "DIPP KISS interface netmask");

// Uploader Configuration (Stored in VMEM for persistence)
uint16_t _mng_util;       // Client Address (Active)
uint16_t _mng_util_server; // Server Address (Passive)

param_t mng_util;
param_t mng_util_server;

#define PARAMID_MNG_UTIL 42
#define PARAMID_MNG_UTIL_SERVER 46

// mng_util triggers the callback (Active)
PARAM_DEFINE_STATIC_VMEM(PARAMID_MNG_UTIL, mng_util, PARAM_TYPE_UINT16, -1, 0, PM_CONF, mng_util_callback, "", config, 0x102, "Uploader Client Address (0=Stop)");
// mng_util_server is passive (no callback)
PARAM_DEFINE_STATIC_VMEM(PARAMID_MNG_UTIL_SERVER, mng_util_server, PARAM_TYPE_UINT16, -1, 0, PM_CONF, NULL, "", config, 0x104, "Uploader Server Address");

// --- RECOVERY UPLOADER CONFIGURATION ---
void mng_util_rec_callback();
uint16_t _mng_util_rec;        // Client Address (Active)
uint16_t _mng_util_rec_server; // Server Address (Passive)
param_t mng_util_rec;
param_t mng_util_rec_server;
#define PARAMID_MNG_UTIL_REC 47
#define PARAMID_MNG_UTIL_REC_SERVER 48
// mng_util_rec triggers the callback (Active)
PARAM_DEFINE_STATIC_VMEM(PARAMID_MNG_UTIL_REC, mng_util_rec, PARAM_TYPE_UINT16, -1, 0, PM_CONF, mng_util_rec_callback, "", config, 0x106, "Recovery Client Address (0=Stop)");
PARAM_DEFINE_STATIC_VMEM(PARAMID_MNG_UTIL_REC_SERVER, mng_util_rec_server, PARAM_TYPE_UINT16, -1, 0, PM_CONF, NULL, "", config, 0x108, "Recovery Server Address");

// Persistent VMEM Paths (Stored in config VMEM)
param_t mng_dipp_vmem_path;
param_t mng_camera_vmem_path;
param_t a53_vmem_path;

#define PARAMID_MNG_DIPP_VMEM_PATH 37
#define PARAMID_MNG_CAMERA_VMEM_PATH 39
#define PARAMID_A53_VMEM_PATH 45

PARAM_DEFINE_STATIC_VMEM(PARAMID_MNG_DIPP_VMEM_PATH, mng_dipp_vmem_path, PARAM_TYPE_STRING, 128, 1, PM_SYSCONF | PM_READONLY, NULL, "", config, 0x200, "DIPP vmem directory path");
PARAM_DEFINE_STATIC_VMEM(PARAMID_MNG_CAMERA_VMEM_PATH, mng_camera_vmem_path, PARAM_TYPE_STRING, 128, 1, PM_SYSCONF | PM_READONLY, NULL, "", config, 0x300, "Camera vmem directory path");
PARAM_DEFINE_STATIC_VMEM(PARAMID_A53_VMEM_PATH, a53_vmem_path, PARAM_TYPE_STRING, 128, 1, PM_SYSCONF | PM_READONLY, NULL, "", config, 0x400, "A53 vmem directory path");


// --- RAM PARAMETERS ---

void suspend_a53_callback();
void vimba_install_callback();
void mng_camera_control_callback();
void mng_dipp_callback();
void switch_m7_bin_callback();

uint8_t _suspend_a53;
uint8_t _vimba_install;
uint8_t _mng_camera_control;
uint8_t _mng_dipp;
uint8_t _switch_m7_bin;
uint8_t _mng_dipp_interface;
uint8_t _mng_camera_interface;
uint8_t _mng_util_interface; // 0=CAN, 1=KISS

#define PARAMID_SUSPEND_A53 31
#define PARAMID_VIMBA_INSTALL 32
#define PARAMID_MNG_CAMERA_CONTROL 33
#define PARAMID_MNG_DIPP 34
#define PARAMID_SWITCH_M7_BIN 35
#define PARAMID_MNG_DIPP_INTERFACE 36
#define PARAMID_MNG_CAMERA_INTERFACE 38
#define PARAMID_MNG_UTIL_INTERFACE 44

PARAM_DEFINE_STATIC_RAM(PARAMID_SUSPEND_A53, suspend_a53, PARAM_TYPE_UINT8, -1, 0, PM_CONF, suspend_a53_callback, "", &_suspend_a53, "STOP A53 cores (suspend linux)");
PARAM_DEFINE_STATIC_RAM(PARAMID_VIMBA_INSTALL, vimba_install, PARAM_TYPE_UINT8, -1, 0, PM_CONF, vimba_install_callback, "", &_vimba_install, "Install Vimba drivers");
PARAM_DEFINE_STATIC_RAM(PARAMID_MNG_CAMERA_CONTROL, mng_camera_control, PARAM_TYPE_UINT16, -1, 0, PM_CONF, mng_camera_control_callback, "", &_mng_camera_control, "Start camera control as this node number (0 kills it)");
PARAM_DEFINE_STATIC_RAM(PARAMID_MNG_DIPP, mng_dipp, PARAM_TYPE_UINT16, -1, 0, PM_CONF, mng_dipp_callback, "", &_mng_dipp, "Start DIPP as this node number (0 kills it)");
PARAM_DEFINE_STATIC_RAM(PARAMID_SWITCH_M7_BIN, switch_m7_bin, PARAM_TYPE_UINT8, -1, 0, PM_READONLY, NULL, "", &_switch_m7_bin, "Switch Cortex-M7 binaries"); // Disabled
PARAM_DEFINE_STATIC_RAM(PARAMID_MNG_DIPP_INTERFACE, mng_dipp_interface, PARAM_TYPE_UINT8, -1, 0, PM_CONF, NULL, "", &_mng_dipp_interface, "DIPP interface type (0=CAN, 1=KISS)");
PARAM_DEFINE_STATIC_RAM(PARAMID_MNG_CAMERA_INTERFACE, mng_camera_interface, PARAM_TYPE_UINT8, -1, 0, PM_CONF, NULL, "", &_mng_camera_interface, "Camera interface type (0=CAN, 1=KISS)");
PARAM_DEFINE_STATIC_RAM(PARAMID_MNG_UTIL_INTERFACE, mng_util_interface, PARAM_TYPE_UINT8, -1, 0, PM_CONF, NULL, "", &_mng_util_interface, "Uploader interface (0=CAN, 1=KISS)");

// Time sync parameters
void time_sync_node_callback();
void time_sync_request_callback();
uint8_t _time_sync_node = 0;
uint8_t _time_sync_request = 0;
uint32_t _time_sync_count = 0;
uint32_t _time_sync_last_error = 0;
#define PARAMID_TIME_SYNC_NODE 51
#define PARAMID_TIME_SYNC_REQUEST 52
#define PARAMID_TIME_SYNC_COUNT 53
#define PARAMID_TIME_SYNC_LAST_ERROR 54
PARAM_DEFINE_STATIC_VMEM(PARAMID_TIME_SYNC_NODE, time_sync_node, PARAM_TYPE_UINT8, -1, 0, PM_CONF, time_sync_node_callback, "", config, 0x200, "Time sync source node (0=disabled)");
PARAM_DEFINE_STATIC_RAM(PARAMID_TIME_SYNC_REQUEST, time_sync_request, PARAM_TYPE_UINT8, -1, 0, PM_CONF, time_sync_request_callback, "", &_time_sync_request, "Trigger time sync (set to any value)");
PARAM_DEFINE_STATIC_RAM(PARAMID_TIME_SYNC_COUNT, time_sync_count, PARAM_TYPE_UINT32, -1, 0, PM_READONLY, NULL, "", &_time_sync_count, "Number of successful syncs");
PARAM_DEFINE_STATIC_RAM(PARAMID_TIME_SYNC_LAST_ERROR, time_sync_last_error, PARAM_TYPE_UINT32, -1, 0, PM_READONLY, NULL, "", &_time_sync_last_error, "Last sync error code");

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

/**
 * Perform time sync with configured node using CSP CMP clock protocol
 * Returns 0 on success, error code on failure
 */
int do_time_sync(uint8_t node) {
    if (node == 0) {
        csp_print("Time sync disabled (node = 0)\n");
        _time_sync_last_error = 1;
        return -1;
    }

    csp_print("Requesting time from node %d...\n", node);

    // Use CSP CMP (CSP Management Protocol) to get clock time
    csp_timestamp_t time;
    int result = csp_cmp_clock(&time, 1000, node);
    
    if (result != CSP_ERR_NONE) {
        csp_print("Time sync failed: CSP error %d\n", result);
        _time_sync_last_error = result;
        return -1;
    }

    // Set system time from received timestamp
    struct timespec ts;
    ts.tv_sec = time.tv_sec;
    ts.tv_nsec = time.tv_nsec;
    
    if (clock_settime(CLOCK_REALTIME, &ts) != 0) {
        csp_print("Failed to set system time:\n");
        _time_sync_last_error = 255;
        return -1;
    }

    _time_sync_count++;
    _time_sync_last_error = 0;
    
    time_t now = ts.tv_sec;
    struct tm *timeinfo = gmtime(&now);
    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    csp_print("Time synchronized to %s UTC\n", timebuf);
    
    return 0;
}

void time_sync_node_callback() {
    _time_sync_node = param_get_uint8(&time_sync_node);
    if (_time_sync_node == 0) {
        csp_print("Time sync disabled\n");
    } else {
        csp_print("Time sync node set to %d\n", _time_sync_node);
    }
}

void time_sync_request_callback() {
    do_time_sync(_time_sync_node);
}

void can_addr_callback() {
    _can_addr = param_get_uint16(&can_addr);
}

void can_netmask_callback() {
    _can_netmask = param_get_uint16(&can_netmask);
}

void dipp_kiss_baudrate_callback() {
    _dipp_kiss_baudrate = param_get_uint32(&dipp_kiss_baudrate);
}

void dipp_kiss_netmask_callback() {
    _dipp_kiss_netmask = param_get_uint16(&dipp_kiss_netmask);
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
    static uint16_t prev_val = 0;
    uint16_t param_val = param_get_uint16(&mng_camera_control);

    if (param_val != prev_val) {
        // 1. Kill any existing instance to avoid duplicates
        system("pkill -f /usr/bin/Disco2CameraControl > /dev/null 2>&1");

        // 2. Check value
        if (param_val == 0) {
            csp_print("Stopping Camera Control...\n");
        } else {
            char cmdbuf[256];
            uint8_t interface_type = param_get_uint8(&mng_camera_interface);
            char vmem_path[128];
            // Read from VMEM parameter
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

            csp_print("Starting: %s\n", cmdbuf);
            FILE *fp = popen(cmdbuf, "r");
            if (fp == NULL) {
                csp_print("Failed to start Camera Control\n");
            }
        }
        prev_val = param_val;
    }
}

void mng_dipp_callback() {
    static uint16_t prev_val = 0;
    uint16_t param_val = param_get_uint16(&mng_dipp);

    if (param_val != prev_val) {
        // 1. Kill any existing instance
        system("pkill -f /usr/bin/dipp > /dev/null 2>&1");

        // 2. Check value
        if (param_val == 0) {
            csp_print("Stopping DIPP...\n");
        } else {
            char cmdbuf[256];
            uint8_t interface_type = param_get_uint8(&mng_dipp_interface);
            char vmem_path[128];
            // Read from VMEM parameter
            param_get_string(&mng_dipp_vmem_path, vmem_path, sizeof(vmem_path));

            const char* interface_str = (interface_type == 1) ? "KISS" : "CAN";
            const char* device_str = (interface_type == 1) ? "/dev/ttymcs3" : "can0";

            if (interface_type == 1) { // KISS
                if (strlen(vmem_path) > 0) {
                    sprintf(cmdbuf, "/usr/bin/dipp -i %s -p %s -b %u -m %u -a %u -v %s 2>&1",
                            interface_str, device_str, _dipp_kiss_baudrate, _dipp_kiss_netmask, param_val, vmem_path);
                } else {
                    sprintf(cmdbuf, "/usr/bin/dipp -i %s -p %s -b %u -m %u -a %u 2>&1",
                            interface_str, device_str, _dipp_kiss_baudrate, _dipp_kiss_netmask, param_val);
                }
            } else { // CAN
                if (strlen(vmem_path) > 0) {
                    sprintf(cmdbuf, "/usr/bin/dipp -i %s -p %s -a %u -v %s 2>&1",
                            interface_str, device_str, param_val, vmem_path);
                } else {
                    sprintf(cmdbuf, "/usr/bin/dipp -i %s -p %s -a %u 2>&1",
                            interface_str, device_str, param_val);
                }
            }

            csp_print("Starting: %s\n", cmdbuf);
            FILE *fp = popen(cmdbuf, "r");
            if (fp == NULL) {
                csp_print("Failed to start DIPP\n");
            }
        }
        prev_val = param_val;
    }
}

void mng_util_callback() {
    static uint16_t prev_client_addr = 0;

    // Use param_get_uint16 to access the separate parameters
    uint16_t new_client_addr = param_get_uint16(&mng_util);
    uint16_t server_addr = param_get_uint16(&mng_util_server);

    // LOGIC: Only execute changes if the Client Address changes.
    // Changing Server Address is passive and won't trigger restart until client is toggled/changed.
    if (new_client_addr != prev_client_addr) {
        
        csp_print("\nAddr Change: C:%u->%u, S:%u\n", prev_client_addr, new_client_addr, server_addr);
        
        // 1. ALWAYS kill any existing process first
        system("pkill -f /usr/bin/upload_client > /dev/null 2>&1");
        
        // 2. If the new client address is 0, we stop here.
        if (new_client_addr == 0) {
            csp_print("Stopping uploader (Client Address set to 0)...\n");
        } 
        // 3. If new client address is valid, Start the process
        else {
            uint8_t interface_type = param_get_uint8(&mng_util_interface);
            
            // Sanity check server address
            if (server_addr == 0) server_addr = 4100;

            const char* device_str = (interface_type == 1) ? "/dev/ttymxc3" : "can0";

            char cmdbuf[256];
            sprintf(cmdbuf, "/usr/bin/upload_client -c %s -a %u -s %u 2>&1", 
                    device_str, new_client_addr, server_addr);
            
            csp_print("Starting: %s\n", cmdbuf);

            FILE *fp = popen(cmdbuf, "r");
            if (fp == NULL) {
                csp_print("Failed to start uploader\n");
            }
        }

        prev_client_addr = new_client_addr;
    }
}

void mng_util_rec_callback() {
    static uint16_t prev_client_addr = 0;

    uint16_t new_client_addr = param_get_uint16(&mng_util_rec);
    uint16_t server_addr = param_get_uint16(&mng_util_rec_server);

    // LOGIC: Restart only if Client Address (Active switch) changes
    if (new_client_addr != prev_client_addr) {
        
        csp_print("\nREC Addr Change: C:%u->%u, S:%u\n", prev_client_addr, new_client_addr, server_addr);
        
        system("pkill -f /usr/bin/upload_client_rec > /dev/null 2>&1");
        
        if (new_client_addr == 0) {
            csp_print("Stopping recovery uploader...\n");
        } else {
            uint8_t interface_type = param_get_uint8(&mng_util_interface);
            if (server_addr == 0) server_addr = 4100;
            const char* device_str = (interface_type == 1) ? "/dev/ttymcs3" : "can0";

            char cmdbuf[256];
            // Uses upload_client_rec binary
            sprintf(cmdbuf, "/usr/bin/upload_client_rec -c %s -a %u -s %u 2>&1", 
                    device_str, new_client_addr, server_addr);
            
            csp_print("Starting REC: %s\n", cmdbuf);
            FILE *fp = popen(cmdbuf, "r");
            if (fp == NULL) {
                csp_print("Failed to start recovery uploader\n");
            }
        }
        prev_client_addr = new_client_addr;
    }
}

int main(int argc, char *argv[]) {
    // Default values
    uint16_t default_can_addr = 5421;
    uint16_t default_can_netmask = 8;
    uint16_t default_interface_type = 0;
    uint32_t cli_baudrate = 0;
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
        } else if (i == 4 && argv[i][0] != '-') {
            cli_baudrate = (uint32_t)atoi(argv[i]);
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

    // Initialize variables from VMEM
    _dipp_kiss_baudrate = param_get_uint32(&dipp_kiss_baudrate);
    _dipp_kiss_netmask = param_get_uint16(&dipp_kiss_netmask);
    
    // Default Netmask to 8 if not set
    if (_dipp_kiss_netmask == 0) {
        _dipp_kiss_netmask = 11;
        param_set_uint16(&dipp_kiss_netmask, _dipp_kiss_netmask);
    }

    // Override baudrate from CLI if present
    if (cli_baudrate > 0) {
        _dipp_kiss_baudrate = cli_baudrate;
        param_set_uint32(&dipp_kiss_baudrate, _dipp_kiss_baudrate);
    }

    if (_dipp_kiss_baudrate == 0) {
        _dipp_kiss_baudrate = 115200; // Default
        param_set_uint32(&dipp_kiss_baudrate, _dipp_kiss_baudrate);
    }

    char path_buf[128];
    
    // DIPP Path
    param_get_string(&mng_dipp_vmem_path, path_buf, sizeof(path_buf));
    if (strlen(path_buf) == 0) {
        param_set_string(&mng_dipp_vmem_path, "/home/root/dippvmem", 128);
    }

    // Camera Path
    param_get_string(&mng_camera_vmem_path, path_buf, sizeof(path_buf));
    if (strlen(path_buf) == 0) {
        param_set_string(&mng_camera_vmem_path, "/home/root/camctlvmem", 128);
    }

    // A53 Path
    param_get_string(&a53_vmem_path, path_buf, sizeof(path_buf));
    if (strlen(path_buf) == 0) {
        param_set_string(&a53_vmem_path, "/home/root/a53vmem", 128);
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
            "/dev/ttymcs3", _dipp_kiss_baudrate, 8, 1, 0, 0
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
    can_iface->name = "CAN";

    // Load time sync configuration
    _time_sync_node = param_get_uint8(&time_sync_node);
    
    // Perform time sync on startup if configured
    if (_time_sync_node != 0) {
        sleep(2);  // Give network time to stabilize
        csp_print("Performing startup time sync...\n");
        do_time_sync(_time_sync_node);
    }

    while (1) {
        sleep(1);
    }

    return 0;
}
