#include <config.h>

#include "plugins_t.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32
#include <dlfcn.h>
#include <glob.h>
#endif

#include "adt/xmalloc.h"

plugin_t *plugins = NULL;

static void load_plugin(const char *filename)
{
#ifdef _WIN32
	// TODO...
#else
	//printf("Opening plugin '%s'...\n", filename);

	void *handle = dlopen(filename, RTLD_NOW | RTLD_GLOBAL);
	if (handle == NULL) {
		fprintf(stderr, "Couldn't load plugin '%s': %s\n", filename, dlerror());
		return;
	}

	void *init_func = dlsym(handle, "init_plugin");
	if (init_func == NULL) {
		dlclose(handle);
		fprintf(stderr, "Plugin '%s' has no init_plugin function.\n", filename);
		return;
	}

	plugin_t *plugin = xmalloc(sizeof(plugin[0]));

	plugin->init_function = (init_plugin_function) init_func;
	plugin->dlhandle      = handle;
	plugin->next          = plugins;
	plugins               = plugin;
#endif
}

void search_plugins(void)
{
#ifndef _WIN32
	/* search for plugins */
	glob_t globbuf;
	if (glob("plugins/plugin_*.dylib", 0, NULL, &globbuf) == 0) {
		for (size_t i = 0; i < globbuf.gl_pathc; ++i) {
			const char *filename = globbuf.gl_pathv[i];

			load_plugin(filename);
		}

		globfree(&globbuf);
	}
#endif
}

void initialize_plugins(void)
{
	/* execute plugin initializers */
	plugin_t *plugin = plugins;
	while (plugin != NULL) {
		plugin->init_function();

		plugin = plugin->next;
	}
}

void free_plugins(void)
{
#ifndef _WIN32
	/* close dl handles */
	plugin_t *plugin = plugins;
	while (plugin != NULL) {
		void *handle = plugin->dlhandle;

		if (handle != NULL) {
			dlclose(handle);
			plugin->dlhandle = NULL;
		}

		plugin = plugin->next;
	}
#endif

}
