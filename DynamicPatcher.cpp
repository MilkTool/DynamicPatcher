﻿// created by i-saint
// distributed under Creative Commons Attribution (CC BY) license.
// https://github.com/i-saint/DynamicObjLoader

#include "DynamicPatcher.h"
#include "dpInternal.h"

static dpContext *g_dpDefaultContext = nullptr;
static __declspec(thread) dpContext *g_dpCurrentContext = nullptr;

dpAPI dpContext* dpCreateContext()
{
    return new dpContext();
}

dpAPI void dpDeleteContext(dpContext *ctx)
{
    delete ctx;
};

dpAPI dpContext* dpGetDefaultContext()
{
    return g_dpDefaultContext;
}

dpAPI void dpSetCurrentContext(dpContext *ctx)
{
    g_dpCurrentContext = ctx;
}

dpAPI dpContext* dpGetCurrentContext()
{
    if(!g_dpCurrentContext) { g_dpCurrentContext=g_dpDefaultContext; }
    return g_dpCurrentContext;
}


dpAPI bool dpInitialize(const dpConfig &conf)
{
    if(!g_dpDefaultContext) {
        ::SymInitialize(::GetCurrentProcess(), NULL, TRUE);
        ::SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);

        dpConfig &g_dpConfig = dpGetConfig();
        g_dpConfig = conf;
        g_dpConfig.starttime = dpGetSystemTime();

        dpConfigFile cf;
        bool config_loaded = false;
        if((conf.sys_flags&dpE_SysLoadConfig)!=0 && cf.load()) {
            config_loaded = true;
            if(cf.log_flags!=-1) { g_dpConfig.log_flags=cf.log_flags; }
            if(cf.sys_flags!=-1) { g_dpConfig.sys_flags=cf.sys_flags; }
            if(cf.vc_ver!=-1)    { g_dpConfig.vc_ver=cf.vc_ver; }
        }
        g_dpDefaultContext = new dpContext();

        if(config_loaded) {
            if(!cf.loads.empty()) {
                dpEach(cf.loads, [](const std::string &path){
                    dpLoad(path.c_str());
                });
                dpLink();
            }
            dpEach(cf.loads,            [](const std::string &v){ dpLoad(v.c_str()); });
            dpEach(cf.source_paths,     [](const std::string &v){ dpAddSourcePath(v.c_str()); });
            dpEach(cf.load_paths,       [](const std::string &v){ dpAddLoadPath(v.c_str()); });
            dpEach(cf.msbuild_commands, [](const std::string &v){ dpAddMSBuildCommand(v.c_str()); });
            dpEach(cf.build_commands,   [](const std::string &v){ dpAddBuildCommand(v.c_str()); });
            if(!cf.source_paths.empty() && (!cf.msbuild_commands.empty() || !cf.build_commands.empty())) {
                dpStartAutoBuild();
            }

            if((g_dpConfig.sys_flags & dpE_SysOpenConsole)!=0) {
                ::AllocConsole();
            }
        }

        return true;
    }
    return false;
}

dpAPI bool dpFinalize()
{
    if(g_dpDefaultContext) {
        delete g_dpDefaultContext;
        g_dpDefaultContext = nullptr;
        if((dpGetConfig().sys_flags & dpE_SysOpenConsole)!=0) {
            ::FreeConsole();
        }
        return true;
    }
    return false;
}


dpAPI size_t dpLoad(const char *path)  { return dpGetCurrentContext()->load(path); }
dpAPI bool dpLoadObj(const char *path) { return dpGetCurrentContext()->loadObj(path)!=nullptr; }
dpAPI bool dpLoadLib(const char *path) { return dpGetCurrentContext()->loadLib(path)!=nullptr; }
dpAPI bool dpLoadDll(const char *path) { return dpGetCurrentContext()->loadDll(path)!=nullptr; }
dpAPI bool dpUnload(const char *path)  { return dpGetCurrentContext()->unload(path); };
dpAPI bool dpLink() { return dpGetCurrentContext()->link(); }

dpAPI size_t dpPatchByFile(const char *filename, const char *filter_regex)
{
    return dpGetCurrentContext()->patchByFile(filename, filter_regex);
}

dpAPI size_t dpPatchByFile(const char *filename, const std::function<bool (const dpSymbolS&)> &condition)
{
    return dpGetCurrentContext()->patchByFile(filename, condition);
}

dpAPI bool dpPatchNameToName(const char *target_name, const char *hook_name)
{
    return dpGetCurrentContext()->patchNameToName(target_name, hook_name);
}

dpAPI bool dpPatchAddressToName(const char *target_name, void *hook_addr)
{
    return dpGetCurrentContext()->patchAddressToName(target_name, hook_addr);
}

dpAPI bool dpPatchAddressToAddress(void *target_adr, void *hook_addr)
{
    return dpGetCurrentContext()->patchAddressToAddress(target_adr, hook_addr);
}

dpAPI bool dpPatchByAddress(void *hook_addr)
{
    return dpGetCurrentContext()->patchByAddress(hook_addr);
}

dpAPI bool   dpUnpatchByAddress(void *target_or_hook_addr)
{
    return dpGetCurrentContext()->unpatchByAddress(target_or_hook_addr);
};

dpAPI void* dpGetUnpatched(void *target_or_hook_addr)
{
    return dpGetCurrentContext()->getUnpatched(target_or_hook_addr);
}


dpAPI void dpAddLoadPath(const char *path)
{
    dpGetCurrentContext()->getBuilder()->addLoadPath(path);
}

dpAPI void dpAddSourcePath(const char *path)
{
    dpGetCurrentContext()->getBuilder()->addSourcePath(path);
}
dpAPI void dpAddMSBuildCommand(const char *msbuild_option)
{
    dpGetCurrentContext()->getBuilder()->addMSBuildCommand(msbuild_option);
}

dpAPI void dpAddBuildCommand(const char *any_command)
{
    dpGetCurrentContext()->getBuilder()->addBuildCommand(any_command);
}

dpAPI bool dpStartAutoBuild()
{
    return dpGetCurrentContext()->getBuilder()->startAutoBuild();
}

dpAPI bool dpStopAutoBuild()
{
    return dpGetCurrentContext()->getBuilder()->stopAutoBuild();
}

dpAPI void dpUpdate()
{
    dpGetCurrentContext()->getBuilder()->update();
}

dpAPI const char* dpGetVCVars()
{
    return dpGetCurrentContext()->getBuilder()->getVCVars();
}