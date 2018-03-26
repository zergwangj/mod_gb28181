/* 
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005-2014, Anthony Minessale II <anthm@freeswitch.org>
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 *
 *
 * mod_gb28181.c -- GB28181 module
 *
 */
#include <switch.h>

/* Prototypes */
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_gb28181_shutdown);
SWITCH_MODULE_RUNTIME_FUNCTION(mod_gb28181_runtime);
SWITCH_MODULE_LOAD_FUNCTION(mod_gb28181_load);


SWITCH_MODULE_DEFINITION(mod_gb28181, mod_gb28181_load, mod_gb28181_shutdown, NULL);

typedef struct {
	int test;
} gb28181_t;

static struct {
	char *server_id;
	switch_memory_pool_t *pool;
	switch_mutex_t *mutex;
} globals;

SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_server_id, globals.server_id);

static switch_status_t do_config(switch_bool_t reload)
{
	char *cf = "gb28181.conf";
	switch_xml_t cfg, xml = NULL, param, settings;
	switch_status_t status = SWITCH_STATUS_SUCCESS;

	if (!(xml = switch_xml_open_cfg(cf, &cfg, NULL))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "open of %s failed\n", cf);
		status = SWITCH_STATUS_FALSE;
		goto done;
	}

	if ((settings = switch_xml_child(cfg, "settings"))) {
		for (param = switch_xml_child(settings, "param"); param; param = param->next) {
			char *var = (char *) switch_xml_attr_soft(param, "name");
			char *val = (char *) switch_xml_attr_soft(param, "value");
			if (!strcasecmp(var, "server_id")) {
				set_global_server_id(val);
			}
		}
	}

	done:

	if (zstr(globals.server_id)) {
		set_global_server_id("34010000002000000001");
	}

	reallydone:

	if (xml) {
		switch_xml_free(xml);
	}

	return status;
}

static switch_status_t request_catalog(char *client_id, char *from_host, char *to_host)
{
	switch_status_t status = SWITCH_STATUS_SUCCESS;
	switch_event_t *event;
	char xml_body[4096];
	char *event_desc;

	memset(xml_body, 0, sizeof(xml_body));

	if (switch_event_create_subclass(&event, SWITCH_EVENT_CUSTOM, "SMS::SEND_MESSAGE") == SWITCH_STATUS_SUCCESS) 
	{
		switch_event_add_header(event, SWITCH_STACK_BOTTOM, "from", "%s@%s", globals.server_id, from_host);
		switch_event_add_header(event, SWITCH_STACK_BOTTOM, "to", "%s@%s", client_id, to_host);
		switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "dest_proto", "sip");
		switch_event_add_header(event, SWITCH_STACK_BOTTOM, "max_forwards", "%d", 70);
		switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "type", "Application/MANSCDP+xml");

		switch_snprintf(xml_body, sizeof(xml_body), "<?xml version=\"1.0\"?>\r\n"
													"<Query>\r\n"
													"<CmdType>Catalog</CmdType>\r\n"/*命令类型*/
													"<SN>%s</SN>\r\n"/*命令序列号*/
													"<DeviceID>%s</DeviceID>\r\n"/*目标设备/区域/系统编码*/
													"</Query>\r\n",
													"285",
													client_id);
		switch_event_add_body(event, xml_body);

		switch_event_serialize(event, &event_desc, SWITCH_FALSE);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, event_desc);

		switch_event_fire(&event);
	}

	return status;
}

static switch_status_t do_play(char *client_id, char *from_host, char *to_host)
{
	
}

static switch_status_t stop_play(char *client_id, char *from_host, char *to_host)
{

}

static void register_event_handler(switch_event_t *event)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "register_event_handler\n");

	char *from_user = switch_event_get_header(event, "from-user");
	char *from_host = switch_event_get_header(event, "from-host");
	char *to_host = switch_event_get_header(event, "to-host");

	request_catalog(from_user, to_host, from_host);
}

#define gb28181_API_SYNTAX	\
	"gb28181 play <clientid>\n"	\
	"gb28181 playback <clientid>\n"	\
	"gb28181 stop <handleid>\n"	\
	"gb28181 reload\n"			\
	"gb28181 help\n"

SWITCH_STANDARD_API(gb28181_api_function)
{
	char *data;
	int argc;
	char *argv[3];
	
	data = strdup(cmd);
	//trim(data);
	if (!(argc = switch_separate_string(data, ' ', argv, (sizeof(argv) / sizeof(argv[0]))))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Invalid usage\n");
		goto done;
	}
	
	if (!strcasecmp(argv[0], "play")) {

	} else if (!strcasecmp(argv[0], "playback")) {

	} else if (!strcasecmp(argv[0], "stop")) {

	}
	
done:
	switch_safe_free(data);
	return SWITCH_STATUS_SUCCESS;
}


SWITCH_MODULE_LOAD_FUNCTION(mod_gb28181_load)
{
	switch_status_t status = SWITCH_STATUS_SUCCESS;
	switch_api_interface_t *api_interface;

    memset(&globals, 0, sizeof(globals));
    globals.pool = pool;
	switch_mutex_init(&globals.mutex, SWITCH_MUTEX_NESTED, globals.pool);
	//switch_application_interface_t *app_interface;
	/* connect my internal structure to the blank pointer passed to me */
	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

	if ((status = switch_event_bind(modname, SWITCH_EVENT_CUSTOM, "sofia::register", register_event_handler, NULL)) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Couldn't bind!\n");
		return status;
	}
	
	do_config(SWITCH_FALSE);

	SWITCH_ADD_API(api_interface, "gb28181", "Control gb28181", gb28181_api_function, "");

	return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_gb28181_shutdown)
{
	switch_event_unbind_callback(register_event_handler);
	switch_safe_free(globals.server_id);

	return SWITCH_STATUS_UNLOAD;
}


/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4 noet
 */
