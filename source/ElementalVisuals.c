/* Arcane Elemental Visuals: narrowly scoped asset-directory redirect.
 * Directory redirect uses no C++ ABI; the optional ember module uses
 * KenshiLib hooks and verified opaque Ogre APIs. No combat state changes.
 * This build only supports the original DLL identified below.
 */
typedef unsigned long DWORD;
typedef unsigned long long UPTR;
typedef int BOOL;
typedef void* HANDLE;
typedef void* HMODULE;
__declspec(dllimport) HMODULE __stdcall GetModuleHandleA(const char*);
__declspec(dllimport) DWORD __stdcall GetModuleFileNameA(HMODULE,char*,DWORD);
__declspec(dllimport) BOOL __stdcall VirtualProtect(void*,UPTR,DWORD,DWORD*);
__declspec(dllimport) HANDLE __stdcall CreateFileA(const char*,DWORD,DWORD,void*,DWORD,DWORD,HANDLE);
__declspec(dllimport) BOOL __stdcall WriteFile(HANDLE,const void*,DWORD,DWORD*,void*);
__declspec(dllimport) BOOL __stdcall CloseHandle(HANDLE);
void* _ReturnAddress(void);
#pragma intrinsic(_ReturnAddress)
static HMODULE selfModule, arcaneModule;
static char redirectedPath[1024], logPath[1024];
typedef DWORD (__stdcall *FilenameFn)(HMODULE,char*,DWORD);
static FilenameFn originalFilename;
void EmberStart(HMODULE self);
static unsigned int len(const char* s) { unsigned int i=0; while(s[i]) ++i; return i; }
static void copy(char* d,const char* s) { while((*d++=*s++)!=0) {} }
static void logMessage(const char* s) {
    DWORD wrote;
    HANDLE f=CreateFileA(logPath,0x40000000,3,0,4,0x80,0);
    if(f==(HANDLE)(UPTR)-1) return;
    /* One startup status, overwritten for each start. */
    CloseHandle(f);
    f=CreateFileA(logPath,0x40000000,3,0,2,0x80,0);
    if(f!=(HANDLE)(UPTR)-1) { WriteFile(f,s,len(s),&wrote,0); CloseHandle(f); }
}
static DWORD __stdcall redirectFilename(HMODULE module,char* buffer,DWORD capacity) {
    UPTR caller=(UPTR)_ReturnAddress()-(UPTR)arcaneModule;
    /* Return addresses must be inside the exact two verified PE functions. */
    if(module==arcaneModule && ((caller>=0x9510 && caller<0x9a33) ||
                               (caller>=0xbfb0 && caller<0xc4d3))) {
        DWORD n=len(redirectedPath);
        if(n<capacity) { copy(buffer,redirectedPath); return n; }
        return 0;
    }
    return originalFilename(module,buffer,capacity);
}
__declspec(dllexport) void startPlugin(void) {
    unsigned char* base;
    unsigned int peOffset, timestamp, imageSize, i, lastSlash=0;
    DWORD oldProtection, ignored;
    FilenameFn* slot;
    if(originalFilename) return;
    if(!GetModuleFileNameA(selfModule,redirectedPath,sizeof(redirectedPath))) return;
    for(i=0;redirectedPath[i];++i) if(redirectedPath[i]=='\\' || redirectedPath[i]=='/') lastSlash=i;
    if(!lastSlash) return;
    redirectedPath[lastSlash]=0;
    copy(logPath,redirectedPath); copy(logPath+len(logPath),"\\ElementalVisuals.log");
    EmberStart(selfModule);
    /* A fictitious plugin filename under this add-on; base code strips twice. */
    copy(redirectedPath+len(redirectedPath),"\\ElementalVisuals.dll");
    arcaneModule=GetModuleHandleA("ArcaneMagicNative.dll");
    if(!arcaneModule) { logMessage("INACTIVE: ArcaneMagicNative.dll not loaded. Place add-on after ArcaneMagic.\r\n"); return; }
    base=(unsigned char*)arcaneModule;
    if(*(unsigned short*)base!=0x5a4d) return;
    peOffset=*(unsigned int*)(base+0x3c);
    if(*(unsigned int*)(base+peOffset)!=0x4550) return;
    timestamp=*(unsigned int*)(base+peOffset+8);
    imageSize=*(unsigned int*)(base+peOffset+24+56);
    if(timestamp!=0x6ae2bea8 || imageSize!=0x55000) {
        logMessage("INACTIVE: unsupported Arcane Magic DLL build. No redirect installed. Rebuild compatibility mapping after reviewing new binary.\r\n"); return;
    }
    slot=(FilenameFn*)(base+0x37088);
    if(*slot!=GetModuleFileNameA) {
        logMessage("INACTIVE: unexpected GetModuleFileNameA import. No redirect installed.\r\n"); return;
    }
    if(!VirtualProtect(slot,sizeof(*slot),0x04,&oldProtection)) return;
    originalFilename=*slot;
    *slot=redirectFilename;
    VirtualProtect(slot,sizeof(*slot),oldProtection,&ignored);
    logMessage("ACTIVE: EarthSpike and Whirlwind asset folders redirected to add-on. Other calls and gameplay unchanged.\r\n");
}
BOOL __stdcall DllMain(HMODULE module,DWORD reason,void* reserved) {
    (void)reserved;
    if(reason==1) selfModule=module;
    return 1;
}
