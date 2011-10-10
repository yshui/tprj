#include <dbus/dbus.h>
#include <fcitx/addon.h>
#include <fcitx/ime.h>
#include <fcitx/candidate.h>
#include <fcitx/instance.h>
#include <fcitx/module.h>
#include <fcitx/module/dbus/dbusstuff.h>
#include <fcitx-config/hotkey.h>
#include <fcitx-utils/log.h>
#include <libgen.h>
typedef enum im_status {ACTIVE, SLEEPING} input_method_status;
struct _fcitx_dbus_input_method {
	const char *name;
	FcitxInstance *owner;
	input_method_status status;
};
struct _fcitx_input_bus {
	DBusConnection *conn;
	FcitxInstance *owner;
	struct _fcitx_dbus_input_method **input_method_table;
	int input_method_count;
};
void *input_bus_create(FcitxInstance *);
DBusHandlerResult input_bus_message_dispatcher(DBusConnection *, DBusMessage *, void *);
FCITX_EXPORT_API
FcitxModule module = {
    input_bus_create,
    NULL,
    NULL,
    NULL,
    NULL
};

void *input_bus_create(FcitxInstance *instance){
	struct _fcitx_input_bus	*input_bus_module = (struct _fcitx_input_bus *)fcitx_malloc0(sizeof(struct _fcitx_input_bus));
	if(!input_bus_module)
		return NULL;

	DBusError err;
	dbus_error_init(&err);

	if(!GetAddonByName(FcitxInstanceGetAddons(instance), FCITX_DBUS_NAME))
		return NULL;

	//So the dbus module has been successfully initialized, right? Thus no error handling...
	input_bus_module->conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
	input_bus_module->owner = instance;
	input_bus_module->input_method_count = 0;
	input_bus_module->input_method_table = NULL;
	int ret = dbus_bus_request_name(input_bus_module->conn, "org.fcitx.Fcitx.InputBus",
			DBUS_NAME_FLAG_REPLACE_EXISTING | DBUS_NAME_FLAG_DO_NOT_QUEUE,
			&err);

	if (dbus_error_is_set(&err)) {
		FcitxLog(WARNING, __FILE__, __LINE__, _("Name Error (%s)"), err.message);
		dbus_error_free(&err);
		free(input_bus_module);
		return NULL;
	}
	if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
		FcitxLog(WARNING, __FILE__, __LINE__, _("Name Error"));
		dbus_error_free(&err);
		free(input_bus_module);
		return NULL;
	}

	dbus_connection_add_filter(input_bus_module->conn, input_bus_message_dispatcher, input_bus_module, NULL);
	//No need to export any functions
	dbus_error_free(&err);
	return input_bus_module;
}
boolean input_bus_method_init(void *user_data){

}
void input_bus_method_reset(void *user_data){

}
INPUT_RETURN_VALUE input_bus_method_doinput(void *user_data, FcitxKeySym sym, unsigned int state){

}
void input_bus_method_save(void *user_data){

}
void input_bus_method_reload(void *user_data){

}
DBusHandlerResult input_bus_new_method(
		DBusConnection *conn,
		DBusMessage *msg,
		struct _fcitx_input_bus *input_bus_module){
	const char* unique_name, *name, *icon_name, *lang_code;
	int priority;
	DBusError err;
	dbus_error_init(&err);
	dbus_bool_t ret = dbus_message_get_args(msg, &err,
			DBUS_TYPE_STRING, &unique_name,
			DBUS_TYPE_STRING, &name,
			DBUS_TYPE_STRING, &icon_name,
			DBUS_TYPE_INT32, &priority,
			DBUS_TYPE_STRING, &lang_code,
			DBUS_TYPE_INVALID);
	if(!ret){
		DBusMessage *retmsg = dbus_message_new_error(msg, DBUS_ERROR_INVALID_ARGS, ":P");
		dbus_connection_send(conn, retmsg, NULL);
		dbus_message_unref(retmsg);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	dbus_error_free(&err);

	//Build an instance for this input method, use unique_name to identify it
	struct _fcitx_dbus_input_method *
		instance = (struct _fcitx_dbus_input_method *)fcitx_malloc0(sizeof(struct _fcitx_dbus_input_method));
	instance->owner = input_bus_module->owner;
	instance->name = strdup(unique_name);
	instance->status = SLEEPING;
	//Register this input method to Fcitx
	FcitxRegisterIMv2(input_bus_module->owner,
			instance,
			unique_name,
			name,
			icon_name,
			input_bus_method_init,
			input_bus_method_reset,
			input_bus_method_doinput,
			NULL,
			NULL,
			input_bus_method_save,
			input_bus_method_reload,
			NULL,
			priority,
			lang_code);

	//Use realloc, quite inefficient
	input_bus_module->input_method_table = (struct _fcitx_dbus_input_method **)realloc(input_bus_module->input_method_table,
			sizeof(void *)*(input_bus_module->input_method_count+1));
	input_bus_module->input_method_table[input_bus_module->input_method_count++] = instance;

	//Reply the caller with object path
	DBusMessage *retmsg = dbus_message_new_method_return(msg);
	// /org/fcitx/Fcitx/InputBus/${unique_name}
	char *obj_path = (char *)fcitx_malloc0(sizeof(char)*(27+strlen(unique_name)));
	sprintf(obj_path, "/org/fcitx/Fcitx/InputBus/%s", unique_name);
	dbus_message_append_args(msg,
			DBUS_TYPE_OBJECT_PATH, &obj_path,
			DBUS_TYPE_INVALID);
	dbus_connection_send(conn, retmsg, NULL);
	dbus_message_unref(retmsg);
	return DBUS_HANDLER_RESULT_HANDLED;
}
INPUT_RETURN_VALUE input_bus_commit_callback(void *user_data, CandidateWord *cand){

}
DBusHandlerResult input_bus_update_candidate(
		DBusConnection *conn,
		DBusMessage *msg,
		struct _fcitx_input_bus *input_bus_module){
	char *obj_path = strdup(dbus_message_get_path(msg));
	char *unique_name = basename(obj_path);

	//Get the corresponding input method instance
	int i;
	struct _fcitx_dbus_input_method **table = input_bus_module->input_method_table;
	for(i=0; i<input_bus_module->input_method_count; i++)
		if(strcmp(table[i]->name, unique_name) == 0)break;
	
	if(i >= input_bus_module->input_method_count){
		//Not found, reply error
		DBusMessage *retmsg = dbus_message_new_error(msg, DBUS_ERROR_UNKNOWN_OBJECT, ":P");
		dbus_connection_send(conn, msg, NULL);
		dbus_message_unref(retmsg);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	if(table[i]->status != ACTIVE){
		//Input method not active, reply error
		DBusMessage *retmsg = dbus_message_new_error(msg, DBUS_ERROR_AUTH_FAILED, "Input method not active.");
		dbus_connection_send(conn, msg, NULL);
		dbus_message_unref(retmsg);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	int index = i;

	char **cands;
	int ncands;
	DBusError err;
	dbus_error_init(&err);
	dbus_message_get_args(msg, &err,
			DBUS_TYPE_ARRAY, DBUS_TYPE_STRING, &cands, &ncands,
			DBUS_TYPE_INVALID);
	CleanInputWindowDown(input_bus_module->owner);
	dbus_error_free(&err);

	FcitxInputState *input = FcitxInstanceGetInputState(input_bus_module->owner);
	for(i=0; i<ncands; i++){
		CandidateWord cand;
		cand.callback = input_bus_commit_callback;
		cand.owner = table[index];
		cand.priv = NULL;
		cand.strExtra = NULL;
		cand.strWord = strdup(cands[i]);
		CandidateWordAppend(
				FcitxInputStateGetCandidateList(input),
				&cand);
		if (i == 0)
			AddMessageAtLast(FcitxInputStateGetClientPreedit(input), MSG_INPUT, "%s", cand.strWord);
	}
	return DBUS_HANDLER_RESULT_HANDLED;
}
DBusHandlerResult input_bus_message_dispatcher(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusHandlerResult ret;
	//Interface needed: NewInputMethod, UpdateCandidate, UpdateUI, Commit
	if(dbus_message_is_method_call(msg, "org.fcitx.Fcitx.InputBus", "NewInputMethod"))
		ret = input_bus_new_method(conn, msg, (struct _fcitx_input_bus *)user_data);
	else if(dbus_message_is_method_call(msg, "org.fcitx.Fcitx.InputBus", "UpdateCandidate"))
		ret = input_bus_updata_candidate(conn, msg, (struct _fcitx_input_bus *)user_data);
	else{
		DBusMessage *retmsg = dbus_message_new_error(msg, DBUS_ERROR_UNKNOWN_METHOD, "The method you call doesn't exists");
		dbus_connection_send(conn, retmsg, NULL);
		dbus_message_unref(retmsg);
		ret = DBUS_HANDLER_RESULT_HANDLED;
	}
	dbus_connection_flush(conn);
	dbus_message_unref(msg);
	return ret;
}

