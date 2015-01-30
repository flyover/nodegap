/**
 * Copyright (c) Flyover Games, LLC
 */

package com.flyovergames.nodegap;

import org.apache.cordova.CallbackContext;
import org.apache.cordova.CordovaPlugin;
import org.apache.cordova.PluginResult;
import org.json.JSONArray;
import org.json.JSONException;

public class NodePlugin extends CordovaPlugin
{
    public static final String ACTION_LOOP = "loop";
    public static final String ACTION_DONE = "done";
    public static final String ACTION_EXEC = "exec";

    public native int nativeNodeLoop();
    public native int nativeNodeDone();
    public native int nativeNodeExec(String script);

    private CallbackContext g_callback_context = null;

    private void stdoutWrite(String message)
    {
	if (g_callback_context != null)
	{
	    PluginResult plugin_result = new PluginResult(PluginResult.Status.OK, message);
	    plugin_result.setKeepCallback(true);
	    g_callback_context.sendPluginResult(plugin_result);
	}
    }

    private void stderrWrite(String message)
    {
	if (g_callback_context != null)
	{
	    PluginResult plugin_result = new PluginResult(PluginResult.Status.OK, message);
	    plugin_result.setKeepCallback(true);
	    g_callback_context.sendPluginResult(plugin_result);
	}
    }

    @Override
    public boolean execute(String action, JSONArray args, CallbackContext callbackContext) throws JSONException
    {
	try
	{
	    if (ACTION_LOOP.equals(action)) { return this.loop(args, callbackContext); }
	    if (ACTION_DONE.equals(action)) { return this.done(args, callbackContext); }
	    if (ACTION_EXEC.equals(action)) { return this.exec(args, callbackContext); }
	    callbackContext.error("Invalid action: " + action);
	    return false;
	}
	catch (Exception e)
	{
	    System.err.println("Exception: " + e.getMessage());
	    callbackContext.error(e.getMessage());
	    return false;
	}
    }

    private boolean loop(JSONArray args, final CallbackContext callbackContext) throws JSONException
    {
	cordova.getThreadPool().execute(new Runnable() { public void run() {
	    g_callback_context = callbackContext;
	    int err = nativeNodeLoop();
	    callbackContext.success(err);
	}});
	return true;
    }

    private boolean done(JSONArray args, final CallbackContext callbackContext) throws JSONException
    {
	cordova.getThreadPool().execute(new Runnable() { public void run() {
	    int err = nativeNodeDone();
	    g_callback_context = null;
	    callbackContext.success(err);
	}});
	return true;
    }

    private boolean exec(JSONArray args, final CallbackContext callbackContext) throws JSONException
    {
	final String script = args.getString(0);
	cordova.getThreadPool().execute(new Runnable() { public void run()
	{
	    if ((script != null) && (script.length() > 0))
	    {
		int err = nativeNodeExec(script);

		callbackContext.success(err);
	    }
	    else
	    {
		callbackContext.error("");
	    }
	}});
	return true;
    }
}
