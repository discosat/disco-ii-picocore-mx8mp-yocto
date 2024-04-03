project('a53-app-sys-manager', 'c', subproject_dir: 'lib', default_options: [
	'buildtype=release', 
	'c_std=gnu11', 
	'b_lto=false',
	'default_library=static',
	'csp:packet_padding_bytes=42', 
	'csp:buffer_count=1000',
	'csp:buffer_size=2048',
	'csp:conn_max=20',
	'csp:conn_rxqueue_len=1000',
	'csp:qfifo_len=1000',
	'csp:rdp_max_window=1000',
	'csp:port_max_bind=16',
	'csp:use_rtable=true',
	'param:have_fopen=true', 
	'param:collector=false', 
	'param:list_dynamic=true'
])

sources = files(['main.c',])

deps = []
deps += dependency('csp', fallback: ['csp', 'csp_dep'])
deps += dependency('param', fallback: ['param', 'param_dep'])

a53_app_sys_manager = executable('a53-app-sys-manager', sources,
	dependencies : deps,
	install : true,
	link_args : ['-Wl,--export-dynamic', '-ldl'],
)
