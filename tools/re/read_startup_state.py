"""User-approved, bounded read-only FFIII resource-startup state observation.

Requires the earlier approved process snapshot. No scanning, game API calls,
injection, input or target writes. Native layout evidence is in the local dump;
memory observations are stored only in the external snapshot directory.
"""
from pathlib import Path
import ctypes as c
from ctypes import wintypes as w
import json, struct, sys, datetime

directory=Path(sys.argv[1]).resolve()
root=Path(__file__).resolve().parents[2]
assert directory.is_relative_to(root.parent/'tools/startup_hang_20261025')
receipt=json.loads((directory/'capture.json').read_text(encoding='utf-8-sig'))
assert receipt['explicit_user_approval']
mods=json.loads((directory/'modules.json').read_text(encoding='utf-8-sig'))
base=next(m['base'] for m in mods if m['ModuleName']=='GameAssembly.dll')
metadata=json.loads((root.parent/'ff3/script.json').read_text(encoding='utf-8-sig'))
rva=next(m['Address'] for m in metadata['ScriptMetadata'] if m['Name']=='System.SingletonMonoBehaviour<ResourceManager>_TypeInfo')
k=c.WinDLL('kernel32',use_last_error=True)
k.OpenProcess.argtypes=[w.DWORD,w.BOOL,w.DWORD];k.OpenProcess.restype=w.HANDLE
k.CloseHandle.argtypes=[w.HANDLE]
k.ReadProcessMemory.argtypes=[w.HANDLE,c.c_void_p,c.c_void_p,c.c_size_t,c.POINTER(c.c_size_t)];k.ReadProcessMemory.restype=w.BOOL
k.QueryFullProcessImageNameW.argtypes=[w.HANDLE,w.DWORD,w.LPWSTR,c.POINTER(w.DWORD)];k.QueryFullProcessImageNameW.restype=w.BOOL
h=k.OpenProcess(0x0410,False,receipt['process_id'])
if not h:raise c.WinError(c.get_last_error())
observations=[]
def read(address,size):
    assert 0x10000<=address<0x0000800000000000 and 0<size<=256
    buf=c.create_string_buffer(size);got=c.c_size_t()
    if not k.ReadProcessMemory(h,address,buf,size,c.byref(got)) or got.value!=size:raise c.WinError(c.get_last_error())
    observations.append({'address':hex(address),'bytes':size})
    return buf.raw
def ptr(address):return struct.unpack('<Q',read(address,8))[0]
try:
    name=c.create_unicode_buffer(1024);n=w.DWORD(len(name))
    if not k.QueryFullProcessImageNameW(h,0,name,c.byref(n)):raise c.WinError(c.get_last_error())
    assert name.value.lower()==receipt['process_path'].lower()
    if '--scene-owner' in sys.argv:
        where=next(m['Address'] for m in metadata['ScriptMetadataMethod'] if m['Name']=='Method$Last.Management.ResourceManager.AddressableInitializeCompleted()')
        method=ptr(base+where)
        direct=parent=resource=0
        if method>0x100000000 and method%8==0:
            direct=ptr(method+0x18);parent=ptr(direct+0x58);sf=ptr(parent+0xb8);resource=ptr(sf)
        saved=json.loads((directory/'boot_owner_state.json').read_text())
        steam=int(next(x['instance'] for x in saved['states'] if x['owner']=='SteamSDKManager'),16)
        where=next(m['Address'] for m in metadata['ScriptMetadata'] if m['Name']=='Last.Management.SceneManager_TypeInfo')
        cl=ptr(base+where);scene=ptr(ptr(cl+0xb8))
        result={'process_id':receipt['process_id'],'resource_direct_class':hex(direct),'resource_actual_parent':hex(parent),'resource_actual_instance':hex(resource),'resource_actual_ready':bool(read(resource+0x38,1)[0]) if resource else None,'steam_sdk_initialized':bool(read(steam+0x1c,1)[0]),'scene_manager':hex(scene),'scene_load_wait_task':hex(ptr(scene+0x38)) if scene else None,'loading_subscene_manager':hex(ptr(scene+0x40)) if scene else None,'read_operations':observations,'read_bytes':sum(x['bytes'] for x in observations),'target_memory_written':False}
        destination=directory/'scene_owner_state.json';assert not destination.exists();destination.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result));sys.exit(0)
    if '--boot-owners' in sys.argv:
        states=[]
        for target in ('ResourceManager','SteamSDKManager','InputSystemManager','AudioManager'):
            key='System.SingletonMonoBehaviour<'+target+'>_TypeInfo'
            where=next(m['Address'] for m in metadata['ScriptMetadata'] if m['Name']==key)
            cl=ptr(base+where);sf=ptr(cl+0xb8)
            states.append({'owner':target,'class':hex(cl),'cctor_finished':struct.unpack('<I',read(cl+0xe0,4))[0],'instance':hex(ptr(sf)),'available':bool(read(sf+8,1)[0])})
        where=next(m['Address'] for m in metadata['ScriptMetadata'] if m['Name']=='Last.Management.LogManager_TypeInfo')
        cl=ptr(base+where);sf=ptr(cl+0xb8)
        states.append({'owner':'LogManager','class':hex(cl),'cctor_finished':struct.unpack('<I',read(cl+0xe0,4))[0],'instance':hex(ptr(sf))})
        destination=directory/'boot_owner_state.json';assert not destination.exists()
        result={'process_id':receipt['process_id'],'captured_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'states':states,'read_operations':observations,'read_bytes':sum(x['bytes'] for x in observations),'game_input_sent':False,'target_memory_written':False}
        destination.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result));sys.exit(0)
    klass=ptr(base+rva)
    static_fields=ptr(klass+0xb8)  # generated all-pointer Il2CppClass_1, verified x64 header
    resource=ptr(static_fields)  # native SingletonMonoBehaviour<ResourceManager>.instance
    class_name=read(ptr(klass+0x10),64).split(b'\0',1)[0].decode('utf-8',errors='replace')
    available=bool(read(static_fields+8,1)[0])
    # Verified native SceneBoot.Load polling flag; no hidden gameplay information.
    ready=bool(read(resource+0x38,1)[0]) if resource else None
    result={'process_id':receipt['process_id'],'captured_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'game_base':hex(base),'resource_singleton_typeinfo_rva':hex(rva),'singleton_class':hex(klass),'class_name':class_name,'singleton_available':available,'static_fields':hex(static_fields),'resource_manager':hex(resource),'is_init_addressable':ready,'read_operations':observations,'read_bytes':sum(x['bytes'] for x in observations),'game_input_sent':False,'target_memory_written':False}
    destination=directory/'resource_startup_state.json';assert not destination.exists()
    destination.write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(result))
finally:k.CloseHandle(h)
