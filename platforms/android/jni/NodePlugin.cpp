/**
 * Copyright (c) Flyover Games, LLC
 */

/// node-gap.h

#include <node.h>
#include <nan.h>

namespace node_gap {

#if NODE_VERSION_AT_LEAST(0,11,0)
void init(v8::Handle<v8::Object> exports, v8::Handle<v8::Value> module, v8::Handle<v8::Context> context);
#else
void init(v8::Handle<v8::Object> exports/*, v8::Handle<v8::Value> module*/);
#endif

} // namespace node_gap

int node_gap_loop(void (*stdout_write)(const char* str), void (*stderr_write)(const char* str));
int node_gap_done();
int node_gap_exec(const char* code);

/// node-gap.cc

//#include "node-gap.h"
#include <node.h>
#include <nan.h>

#if defined(__ANDROID__)
#include <android/log.h>
#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "printf", __VA_ARGS__);
#endif

#define countof(_a) (sizeof(_a)/sizeof((_a)[0]))

using namespace v8;

namespace node_gap {

static void (*g_stdout_write_callback)(const char* str) = NULL;
static void (*g_stderr_write_callback)(const char* str) = NULL;
static uv_async_t g_exec_async;
static uv_async_t g_done_async;
static Persistent<Function> g_exec_callback;
static Persistent<Function> g_done_callback;

static void _exec_async_callback(uv_async_t* async, int status)
{
	NanScope();

	char* code = static_cast<char*>(async->data); async->data = NULL;

	if (!g_exec_callback.IsEmpty())
	{
		Handle<Value> argv[] = { NanNew<String>(code) };
		NanMakeCallback(NanGetCurrentContext()->Global(), NanNew<Function>(g_exec_callback), countof(argv), argv);
	}

	free(code); // strdup(code)
}

static void _done_async_callback(uv_async_t* async, int status)
{
	NanScope();

	uv_close(reinterpret_cast<uv_handle_t*>(&g_exec_async), NULL);
	NanDisposePersistent(g_exec_callback);

	if (!g_done_callback.IsEmpty())
	{
		NanMakeCallback(NanGetCurrentContext()->Global(), NanNew<Function>(g_done_callback), 0, NULL);
	}

	uv_close(reinterpret_cast<uv_handle_t*>(async), NULL);
	NanDisposePersistent(g_done_callback);
}

static NAN_METHOD(_method_stdout_write)
{
	NanScope();
	Local<String> str = Local<String>::Cast(args[0]);
	if (g_stdout_write_callback)
	{
		g_stdout_write_callback(*String::Utf8Value(str));
	}
	NanReturnValue(NanNew<v8::Boolean>(g_stdout_write_callback != NULL));
}

static NAN_METHOD(_method_stderr_write)
{
	NanScope();
	Local<String> str = Local<String>::Cast(args[0]);
	if (g_stderr_write_callback)
	{
		g_stderr_write_callback(*String::Utf8Value(str));
	}
	NanReturnValue(NanNew<v8::Boolean>(g_stderr_write_callback != NULL));
}

static NAN_METHOD(_method_loop)
{
	NanScope();

	Local<Function> exec_callback = Local<Function>::Cast(args[0]);
	Local<Function> done_callback = Local<Function>::Cast(args[1]);

	NanAssignPersistent(g_exec_callback, exec_callback);
	NanAssignPersistent(g_done_callback, done_callback);

	uv_async_init(uv_default_loop(), &g_exec_async, _exec_async_callback);
	assert(g_exec_async.data == NULL);
	uv_async_init(uv_default_loop(), &g_done_async, _done_async_callback);

	NanReturnUndefined();
}

#if NODE_VERSION_AT_LEAST(0,11,0)
void init(Handle<Object> exports, Handle<Value> module, Handle<Context> context)
#else
void init(Handle<Object> exports/*, Handle<Value> module*/)
#endif
{
	NODE_SET_METHOD(exports, "stdout_write", _method_stdout_write);
	NODE_SET_METHOD(exports, "stderr_write", _method_stderr_write);
	NODE_SET_METHOD(exports, "loop", _method_loop);
}

} // namespace node_gap

#if NODE_VERSION_AT_LEAST(0,11,0)
NODE_MODULE_CONTEXT_AWARE_BUILTIN(node_gap, node_gap::init)
#else
NODE_MODULE(node_gap, node_gap::init)
#endif

#if NODE_VERSION_AT_LEAST(0,11,0)
#else
static node::node_module_struct* _get_external_module(const char* name)
{
	node::node_module_struct* module = NULL;
	if (strcmp(name, "node_gap") == 0) { module = &node_gap_module; }
	if (module) { printf("get_external_module: %s\n", name); }
	return module;
}
#endif

static char** copy_argv(int argc, const char** argv) {
	size_t strlen_sum;
	char** argv_copy;
	char* argv_data;
	size_t len;
	int i;

	strlen_sum = 0;
	for(i = 0; i < argc; i++) {
		strlen_sum += strlen(argv[i]) + 1;
	}

	argv_copy = (char**) malloc(sizeof(char*) * (argc + 1) + strlen_sum);
	if (!argv_copy) {
		return NULL;
	}

	argv_data = (char*) argv_copy + sizeof(char*) * (argc + 1);

	for(i = 0; i < argc; i++) {
		argv_copy[i] = argv_data;
		len = strlen(argv[i]) + 1;
		memcpy(argv_data, argv[i], len);
		argv_data += len;
	}

	argv_copy[argc] = NULL;

	return argv_copy;
}

int node_gap_loop(void (*stdout_write)(const char* str), void (*stderr_write)(const char* str))
{
	node_gap::g_stdout_write_callback = stdout_write;
	node_gap::g_stderr_write_callback = stderr_write;

	const char* node_script =
		"var node_gap = process.binding('node_gap');"

		"var process_stdout_write = process.stdout.write;"
		"process.stdout.write = function ()"
		"{"
		"	var args = [].splice.call(arguments, 0);"
		"	if (!node_gap.stdout_write.apply(this, args))"
		"	{"
		"		process_stdout_write.apply(this, args);"
		"	}"
		"};"

		"var process_stderr_write = process.stderr.write;"
		"process.stderr.write = function ()"
		"{"
		"	var args = [].splice.call(arguments, 0);"
		"	if (!node_gap.stderr_write.apply(this, args))"
		"	{"
		"		process_stderr_write.apply(this, args);"
		"	}"
		"};"

		"var exec = function (code)"
		"{"
		"	var vm = require('vm');"
		"	var sandbox = {};"
		"	sandbox.require = require;"
		"	sandbox.console = console;"
		"	var context = vm.createContext(sandbox);"
		"	var script = vm.createScript(code);"
		"	var result = script.runInContext(context);"
		"	return result || 0;"
		"};"

		"var done = function ()"
		"{"
		"	process.stdout.write = process_stdout_write;"
		"	process.stderr.write = process_stderr_write;"
		"};"

		"node_gap.loop(exec, done);"
		;

	#if NODE_VERSION_AT_LEAST(0,11,0)
	#else
	node::get_external_module = _get_external_module;
	#endif

	const char* node_argv[] = { "node", "-e", node_script };
	int node_argc = sizeof(node_argv) / sizeof(node_argv[0]);
	char** copy_node_argv = copy_argv(node_argc, node_argv);
	int node_err;
	v8::Isolate* isolate = v8::Isolate::New();
	{
		v8::Locker locker(isolate);
		v8::Isolate::Scope isolate_scope(isolate);
		node_err = node::Start(node_argc, copy_node_argv);
	}
	isolate->Dispose(); isolate = NULL;
	free(copy_node_argv); copy_node_argv = NULL;

	#if NODE_VERSION_AT_LEAST(0,11,0)
	#else
	node::get_external_module = NULL;
	#endif

	return node_err;
}

int node_gap_done()
{
	int err = uv_async_send(&node_gap::g_done_async);

	node_gap::g_stdout_write_callback = NULL;
	node_gap::g_stderr_write_callback = NULL;

	return err;
}

int node_gap_exec(const char* code)
{
	assert(node_gap::g_exec_async.data == NULL);
	node_gap::g_exec_async.data = strdup(code);

	int err = uv_async_send(&node_gap::g_exec_async);

	return err;
}

///

#include <jni.h>

static JNIEnv* g_loop_env = NULL;
static jobject g_loop_obj = NULL;
static jclass g_loop_obj_class = NULL;
static jmethodID g_stdout_write_method = NULL;
static jmethodID g_stderr_write_method = NULL;

static void _stdout_write(const char* str)
{
	if (g_loop_env && g_loop_obj && g_stdout_write_method)
	{
		jstring message = g_loop_env->NewStringUTF(str);
		g_loop_env->CallVoidMethod(g_loop_obj, g_stdout_write_method, message);
	}
}

static void _stderr_write(const char* str)
{
	if (g_loop_env && g_loop_obj && g_stderr_write_method)
	{
		jstring message = g_loop_env->NewStringUTF(str);
		g_loop_env->CallVoidMethod(g_loop_obj, g_stderr_write_method, message);
	}
}

extern "C" JNIEXPORT jint JNICALL Java_com_flyovergames_nodegap_NodePlugin_nativeNodeLoop(JNIEnv* env, jobject obj)
{
	g_loop_env = env;
	g_loop_obj = obj;
	g_loop_obj_class = g_loop_env->GetObjectClass(g_loop_obj);
	g_stdout_write_method = g_loop_env->GetMethodID(g_loop_obj_class, "stdoutWrite", "(Ljava/lang/String;)V");
	g_stderr_write_method = g_loop_env->GetMethodID(g_loop_obj_class, "stderrWrite", "(Ljava/lang/String;)V");

	int err = node_gap_loop(_stdout_write, _stderr_write);

	return err;
}

extern "C" JNIEXPORT jint JNICALL Java_com_flyovergames_nodegap_NodePlugin_nativeNodeDone(JNIEnv* env, jobject obj)
{
	int err = node_gap_done();

	g_loop_env = NULL;
	g_loop_obj = NULL;
	g_loop_obj_class = NULL;
	g_stdout_write_method = NULL;
	g_stderr_write_method = NULL;

	return err;
}

extern "C" JNIEXPORT jint JNICALL Java_com_flyovergames_nodegap_NodePlugin_nativeNodeExec(JNIEnv* env, jobject obj, jstring script)
{
	const char* node_script = env->GetStringUTFChars(script, 0);

	int err = node_gap_exec(node_script);

	env->ReleaseStringUTFChars(script, node_script);

	return err;
}

