# Feature: satdeploy-agent libparam integration

## Goal
Add a libparam parameter to a53-app-sys-manager to start/stop satdeploy-agent.
This allows remote control of the deployment agent via CSP param protocol.

## Background
- satdeploy-agent runs on the A53 (Linux side) of PicoCoreMX8MP
- Current command: `/opt/satdeploy/bin/satdeploy-agent -i CAN -p can0 -a 5424`
- a53-app-sys-manager already manages: camera control, DIPP, uploader

## Design
Following the pattern of existing managed processes (mng_dipp, mng_camera_control):

### New Parameters
| ID | Name | Type | Description |
|----|------|------|-------------|
| 55 | mng_satdeploy | uint16 | Node address for satdeploy-agent (0=stop) |
| 56 | mng_satdeploy_interface | uint8 | Interface type (0=CAN, 1=KISS) |

### Callback Logic
```
mng_satdeploy_callback():
1. Kill existing: pkill -f /opt/satdeploy/bin/satdeploy-agent
2. If value == 0: stop (done)
3. If value > 0: start with that node address
   - Interface from mng_satdeploy_interface (0=CAN/can0, 1=KISS)
   - Command: /opt/satdeploy/bin/satdeploy-agent -i {CAN|KISS} -p {can0|/dev/ttymxc3} -a {node}
```

## Files to Modify
- `files/app-sys-manager/main.c` - Add param definitions and callback

## Testing
- Set mng_satdeploy to 5424 via csh: `param set mng_satdeploy 5424`
- Verify process starts: `ps aux | grep satdeploy`
- Set to 0, verify process stops
- Deploy new version via satdeploy, verify agent restarts with new binary
