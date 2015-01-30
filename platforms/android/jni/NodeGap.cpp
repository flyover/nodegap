/**
 * Copyright (c) Flyover Games, LLC
 */

#include <jni.h>
#include <android/log.h>

#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "printf", __VA_ARGS__);

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env = 0;
	if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
	{
		return -1;
	}

	// Get jclass with env->FindClass.
	// Register methods with env->RegisterNatives.

	return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT void JNICALL Java_com_flyovergames_nodegap_CordovaApp_nativeInit(JNIEnv* env, jobject obj)
{
	printf("NodeGap.nativeInit()\n");
}
