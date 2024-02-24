do_install:prepend() {
	if [ "${SERIAL_CONSOLES}" == "FUS_LOGIN_CONSOLE" ] ; then
		install -d ${D}${systemd_unitdir}/system/
		install -m 0644 ${WORKDIR}/serial-getty@.service ${D}${systemd_unitdir}/system/fsserial-getty@.service 
		return 0
	fi
}
