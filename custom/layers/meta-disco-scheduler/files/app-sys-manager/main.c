/**
 * This is simple application/system manager to manage the linux-side of the PicoCoreMX8MP.
 *
 * This can be used for various tasks such as:
 * - Suspending linux (STOP A53 cores) - this also brings this node down, so it must be woken up from the Cortex-M7
 *  - set `suspend_a53` to any value to suspend the A53 cores
 *
 * - Installing Vimba drivers
 *  - set `vimba_install` to any value to install Vimba drivers
 *
 * - Start/stop camera control process
 *  - set `mng_camera_control n` start camera control as node number `n` (n=0 kills any running camera control process)
 *
 * - Start/stop image processing (DIPP) process
 *  - set `mng_dipp n` start DIPP as node number `n` (n=0 kills any running DIPP process)
 *
 * - Switching the Cortex-M7 binary between the main (/home/root/disco_scheduler.bin) and stage files (/home/root/_stage_disco_scheduler.bin)
 *  - set `switch_m7_bin` to any value to switch the Cortex-M7 binaries (this is disabled for now)
 *
 * - Rebooting PicoCoreMX8MP
 *  - run `reboot` command (affects Cortex-M7 application as well)
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h> 
#include <stdarg.h>
#include <sys/select.h>

#include <csp/csp.h>
#include <csp/interfaces/csp_if_can.h>

#include <param/param.h>
#include <param/param_server.h>
#include <vmem/vmem.h>
#include <vmem/vmem_server.h>
#include <vmem/vmem_file.h>

extern vmem_t vmem_config;
VMEM_DEFINE_FILE(config, "config", "sys-man-config.vmem", 1000);

void can_addr_callback();
void can_netmask_callback();
uint16_t _can_addr;
uint16_t _can_netmask;
param_t can_addr;
csp_iface_t* can_iface;
#define CAN_ADDR_DEFAULT 21
#define CAN_NETMASK_DEFAULT 8
#define PARAMID_CAN_ADDR 21
#define PARAMID_CAN_NETMASK 22
PARAM_DEFINE_STATIC_VMEM(PARAMID_CAN_ADDR, can_addr, PARAM_TYPE_UINT16, -1, 0, PM_SYSCONF, can_addr_callback, "", config, 0x10, "CAN interface address");
PARAM_DEFINE_STATIC_VMEM(PARAMID_CAN_NETMASK, can_netmask, PARAM_TYPE_UINT16, -1, 0, PM_SYSCONF, can_netmask_callback, "", config, 0x102, "CAN interface netmask");

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
#define PARAMID_SUSPEND_A53 31
#define PARAMID_VIMBA_INSTALL 32
#define PARAMID_MNG_CAMERA_CONTROL 33
#define PARAMID_MNG_DIPP 34
#define PARAMID_SWITCH_M7_BIN 35
PARAM_DEFINE_STATIC_RAM(PARAMID_SUSPEND_A53, suspend_a53, PARAM_TYPE_UINT8, -1, 0, PM_CONF, suspend_a53_callback, "", &_suspend_a53, "STOP A53 cores (suspend linux)");
PARAM_DEFINE_STATIC_RAM(PARAMID_VIMBA_INSTALL, vimba_install, PARAM_TYPE_UINT8, -1, 0, PM_CONF, vimba_install_callback, "", &_vimba_install, "Install Vimba drivers");
PARAM_DEFINE_STATIC_RAM(PARAMID_MNG_CAMERA_CONTROL, mng_camera_control, PARAM_TYPE_UINT8, -1, 0, PM_CONF, mng_camera_control_callback, "", &_mng_camera_control, "Start camera control as this node number (0 kills it)");
PARAM_DEFINE_STATIC_RAM(PARAMID_MNG_DIPP, mng_dipp, PARAM_TYPE_UINT8, -1, 0, PM_CONF, mng_dipp_callback, "", &_mng_dipp, "Start DIPP as this node number (0 kills it)");
PARAM_DEFINE_STATIC_RAM(PARAMID_SWITCH_M7_BIN, switch_m7_bin, PARAM_TYPE_UINT8, -1, 0, PM_READONLY, NULL, "", &_switch_m7_bin, "Switch Cortex-M7 binaries"); // Disabled for now

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

// Add onehz hook that checks connection is up?

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
    //can_iface->addr = _can_addr;
}

void can_netmask_callback() {
    _can_netmask = param_get_uint16(&can_netmask);
    //can_iface->netmask = _can_netmask;
}

void suspend_a53_callback() {
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
    uint8_t param_val = param_get_uint8(&mng_camera_control);

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
        default:  // Start camera control as node number `mng_camera_control`
            char cmdbuf[128];
            sprintf(cmdbuf, "/usr/bin/Disco2CameraControl -i can -d can0 -n %u 2>&1", param_val);
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

    uint8_t param_val = param_get_uint8(&mng_dipp);
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
        default:  // Start dipp as node number `mng_dipp`
            char cmdbuf[128];
            sprintf(cmdbuf, "/usr/bin/dipp -i CAN -p can0 -a %u 2>&1", param_val);
            fp = popen(cmdbuf, "r");
            if (fp == NULL) {
                csp_print("Failed %s\n", cmdbuf);
                return;
            }
            break;
    }
}

void switch_m7_bin_callback() {
    // char buffer[128];
    // FILE *fp;
//
    // fp = popen("mv /home/root/disco_scheduler.bin /home/root/_reserve_disco_scheduler.bin 2>&1", "r");
    // if (fp == NULL) {
    //     csp_print("Failed to run command\n");
    // }
    // while (fgets(buffer, sizeof(buffer)-1, fp) != NULL) {
    //     csp_print("%s", buffer);
    // }
    // pclose(fp);
//
    // fp = popen("mv /home/root/_stage_disco_scheduler.bin /home/root/disco_scheduler.bin 2>&1", "r");
    // if (fp == NULL) {
    //     csp_print("Failed to run command\n");
    // }
    // while (fgets(buffer, sizeof(buffer)-1, fp) != NULL) {
    //     csp_print("%s", buffer);
    // }
    // pclose(fp);
//
    // fp = popen("mv /home/root/_reserve_disco_scheduler.bin /home/root/_stage_disco_scheduler.bin 2>&1", "r");
    // if (fp == NULL) {
    //     csp_print("Failed to run command\n");
    // }
    // while (fgets(buffer, sizeof(buffer)-1, fp) != NULL) {
    //     csp_print("%s", buffer);
    // }
    // pclose(fp);
//
    // if (access("/home/root/disco_scheduler.bin", F_OK) != -1) {
    //     csp_print("Attempted to switch Cortex-M7 binaries\n");
    // }
}

void kill_process_callback() {
    char buffer[128];
    FILE *fp;

    fp = popen("pkill -f /path/to/executable", "r");
    if (fp == NULL) {
        csp_print("Failed to run command\n");
        return;
    }
    while (fgets(buffer, sizeof(buffer)-1, fp) != NULL) {
        csp_print("%s", buffer);
    }
    pclose(fp);
}

int main(void) {
    csp_conf.hostname = "app-sys-manager";
    csp_conf.model = "PicoCoreMX8MP-Cortex-A53";
	csp_conf.version = 2;
	csp_conf.dedup = CSP_DEDUP_OFF;
    uint32_t serial = serial_get();
    char *serial_str = malloc(19 * sizeof(char));
    if (serial_str == NULL) {
        csp_print("Failed to allocate memory\n");
        csp_conf.revision = "?";
    } else {
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
        _can_addr = CAN_ADDR_DEFAULT;
        param_set_uint16(&can_addr, _can_addr);
    }
    if (_can_netmask == 0) {
        _can_netmask = CAN_NETMASK_DEFAULT;
        param_set_uint16(&can_netmask, _can_netmask);
    }

    csp_iface_t* can_iface;
    int error = csp_can_socketcan_open_and_add_interface("can0", "CAN", _can_addr, 1000000, 0, &can_iface);
    if (error != 0) {
        csp_print("Failed to open CAN interface\n");
        return error;
    }

    can_iface->is_default = true;
    can_iface->addr = _can_addr;
    can_iface->netmask = _can_netmask;
    can_iface->name = "CAN";

    while (1) {
        sleep(1);
    }

    return 0;
}
