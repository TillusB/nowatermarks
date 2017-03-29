#include <psp2/kernel/modulemgr.h>
#include <psp2/sysmodule.h>
#include <psp2/screenshot.h>
#include <taihen.h>

static tai_hook_ref_t load_hook;
static tai_hook_ref_t unload_hook;
static tai_hook_ref_t ss_overlay_hook;
static SceUID ss_overlay_uid;
//empty dummy function
int dummy(void){
  return 0;
}
// hook to never watermark screenshots
int hook_ss_overlay(void) {
  int ret;
  TAI_CONTINUE(int, ss_overlay_hook); // so others get a chance to hook
  ret = dummy(); // we simply return an empty function
  return ret;
}
// hook load module
int hook_sysmodule_load(uint16_t id) {
  int ret;
  ret = TAI_CONTINUE(int, load_hook, id);
  if (ret >= 0) { // load successful
    switch (id) {
      case SCE_SYSMODULE_SCREEN_SHOT:
        ss_overlay_uid = 
          taiHookFunctionImport(&ss_overlay_hook, // Output a reference
                        "AppName",                // Name of module being hooked
                        0xF26FC97D,               // NID specifying SceScreenShot
                        0x7061665B,               // NID specifying sceScreenShotSetOverlayImage
                        hook_ss_overlay);         // Name of the hook function
        break;
      // you can consider other loaded modules too here ...
      default:
        break;
    }
  }
  return ret;
}
// hook unload module
int hook_sysmodule_unload(uint16_t id) {
  int ret;
  ret = TAI_CONTINUE(int, unload_hook, id);
  if (ret >= 0) { // unload successful
    switch (id) {
      case SCE_SYSMODULE_SCREEN_SHOT:
        if (ss_overlay_uid >= 0) {
          taiHookRelease(ss_overlay_uid, ss_overlay_hook);
          ss_overlay_uid = -1;
        }
        break;
      // you can consider other loaded modules too here ...
      default:
        break;
    }
  }
  return ret;
}
// our own plugin entry
int module_start(SceSize argc, const void *args) {
  ss_overlay_uid = -1;
  taiHookFunctionImport(&load_hook,             // Output a reference
                        "AppName",              // Name of module being hooked
                        0x03FCF19D,             // NID specifying SceSysmodule
                        0x79A0160A,             // NID specifying sceSysmoduleLoadModule
                        hook_sysmodule_load);   // Name of the hook function
  taiHookFunctionImport(&unload_hook,           // Output a reference
                        "AppName",              // Name of module being hooked
                        0x03FCF19D,             // NID specifying SceSysmodule
                        0x31D87805,             // NID specifying sceSysmoduleUnloadModule
                        hook_sysmodule_unload); // Name of the hook function
  return SCE_KERNEL_START_SUCCESS;
}

