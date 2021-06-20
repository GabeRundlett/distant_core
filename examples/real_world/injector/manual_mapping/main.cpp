#define DISTANT_EXPOSE_NATIVE
#include <distant/core.hpp>

#include <iostream>
#include <filesystem>

#include <Windows.h>
#include <TlHelp32.h>

typedef HMODULE(__stdcall * pLoadLibraryA)(LPCSTR);
typedef FARPROC(__stdcall * pGetProcAddress)(HMODULE, LPCSTR);
typedef INT(__stdcall * dllmain)(HMODULE, DWORD, LPVOID);

struct loaderdata {
    LPVOID          ImageBase;
    pLoadLibraryA   fnLoadLibraryA;
    pGetProcAddress fnGetProcAddress;
};

DWORD __stdcall LibraryLoader(loaderdata & LoaderParams) {
    auto   binary     = reinterpret_cast<std::byte *>(LoaderParams.ImageBase);
    auto & dos_header = *reinterpret_cast<IMAGE_DOS_HEADER *>(&binary[0]);
    auto & nt_headers =
        *reinterpret_cast<IMAGE_NT_HEADERS *>(&binary[dos_header.e_lfanew]);
    auto & optional_header = nt_headers.OptionalHeader;
    auto & base_relocation = *reinterpret_cast<IMAGE_BASE_RELOCATION *>(
        &binary[optional_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC]
                    .VirtualAddress]);
    auto & import_directory = *reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR *>(
        &binary[optional_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]
                    .VirtualAddress]);

    auto *         relocation_ptr = &base_relocation;
    std::ptrdiff_t relocation_delta =
        reinterpret_cast<std::ptrdiff_t>(binary - optional_header.ImageBase);

    if (relocation_delta != 0) {
        while (relocation_ptr->VirtualAddress) {
            if (relocation_ptr->SizeOfBlock >= sizeof(IMAGE_BASE_RELOCATION)) {
                int count = (relocation_ptr->SizeOfBlock -
                             sizeof(IMAGE_BASE_RELOCATION)) /
                            sizeof(WORD);
                std::ptrdiff_t * list =
                    reinterpret_cast<std::ptrdiff_t *>(relocation_ptr + 1);
                for (int i = 0; i < count; i++) {
                    if (list[i]) {
                        std::ptrdiff_t * ptr =
                            reinterpret_cast<std::ptrdiff_t *>(
                                &binary[relocation_ptr->VirtualAddress +
                                        (list[i] & 0xFFF)]);
                        *ptr += relocation_delta;
                    }
                }
            }
            relocation_ptr = reinterpret_cast<IMAGE_BASE_RELOCATION *>(
                reinterpret_cast<std::byte *>(relocation_ptr) +
                relocation_ptr->SizeOfBlock);
        }
    }

    auto * import_directory_ptr = &import_directory;

    // Resolve DLL imports
    while (import_directory_ptr->Characteristics) {
        auto OrigFirstThunk = reinterpret_cast<IMAGE_THUNK_DATA *>(
            &binary[import_directory_ptr->OriginalFirstThunk]);
        auto FirstThunk = reinterpret_cast<IMAGE_THUNK_DATA *>(
            &binary[import_directory_ptr->FirstThunk]);
        HMODULE hModule =
            LoaderParams.fnLoadLibraryA(reinterpret_cast<const char *>(
                &binary[import_directory_ptr->Name]));
        if (!hModule) return FALSE;
        while (OrigFirstThunk->u1.AddressOfData) {
            if (OrigFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) {
                // Import by ordinal
                ULONGLONG Function = (ULONGLONG)LoaderParams.fnGetProcAddress(
                    hModule, (LPCSTR)(OrigFirstThunk->u1.Ordinal & 0xFFFF));
                if (!Function) return FALSE;

                FirstThunk->u1.Function = Function;
            } else {
                // Import by name
                PIMAGE_IMPORT_BY_NAME pIBN =
                    reinterpret_cast<IMAGE_IMPORT_BY_NAME *>(
                        &binary[OrigFirstThunk->u1.AddressOfData]);
                ULONGLONG Function = (ULONGLONG)LoaderParams.fnGetProcAddress(
                    hModule, (LPCSTR)pIBN->Name);
                if (!Function) return FALSE;
                FirstThunk->u1.Function = Function;
            }
            OrigFirstThunk++;
            FirstThunk++;
        }
        import_directory_ptr++;
    }

    if (optional_header.AddressOfEntryPoint) {
        using FuncDllMain = BOOL APIENTRY(HMODULE, DWORD, LPVOID);
        auto &                   EntryPoint = *reinterpret_cast<FuncDllMain *>(
            &binary[optional_header.AddressOfEntryPoint]);
        return EntryPoint(reinterpret_cast<HMODULE>(binary), DLL_PROCESS_ATTACH,
                          NULL); // Call the entry point
    }
    return TRUE;
}

DWORD __stdcall stub() { return 0; }

int main() {
    // Target Dll
    std::filesystem::path dll_filepath;
    // std::cout << "Enter a dll filepath: ";
    // std::cin >> DllFilepath;
    dll_filepath = std::filesystem::current_path() /
                   "build/examples/injectable_dlls/basic/Release/basic.dll";

    std::string exe_filename;
    // std::cout << "Enter a process exe file name: ";
    // std::cin >> ExeFilename;
    exe_filename = "target.exe";

    distant::Injectable dll_file(dll_filepath.string());
    if (!dll_file) {
        std::cout << "Failed to open '" << dll_filepath << "'\n";
        std::cin.get();
        return EXIT_FAILURE;
    }

    // Opening target process.
    distant::Process process(exe_filename);
    if (!process) {
        std::cout << "Failed to find '" << exe_filename
                  << "' among the enumerated process list. "
                  << "You must run the target process!\n";
        std::cin.get();
        return EXIT_FAILURE;
    }
    // Allocating memory for the DLL
    auto * nt_headers_ptr =
        static_cast<IMAGE_NT_HEADERS *>(dll_file.native.nt_headers_ptr);
    auto executable_image_buffer = process.virtual_alloc_unique(
        nt_headers_ptr->OptionalHeader.SizeOfImage);

    // Copy the headers to target process
    executable_image_buffer.write(
        0, dll_file.binary.data(),
        nt_headers_ptr->OptionalHeader.SizeOfHeaders);

    // Target Dll's Section Header
    IMAGE_SECTION_HEADER * section_header_ptr =
        (PIMAGE_SECTION_HEADER)(nt_headers_ptr + 1);
    // Copying sections of the dll to the target process
    for (int i = 0; i < nt_headers_ptr->FileHeader.NumberOfSections; i++) {
        executable_image_buffer.write(
            section_header_ptr[i].VirtualAddress,
            &dll_file.binary[section_header_ptr[i].PointerToRawData],
            section_header_ptr[i].SizeOfRawData);
    }

    // Allocating memory for the loader code.
    const auto loader_func_size =
        (std::intptr_t)stub - (std::intptr_t)LibraryLoader;
    const auto loader_size   = loader_func_size + sizeof(loaderdata);
    auto       loader_buffer = process.virtual_alloc_unique(loader_size);
    auto loader_func_ptr = (LPTHREAD_START_ROUTINE)(std::bit_cast<loaderdata *>(
                                                        loader_buffer.get()) +
                                                    1);
    auto loader_func_args_ptr = std::bit_cast<void *>(loader_buffer.get());

    // Write the loader information to target process
    loader_buffer.write_value(0, loaderdata{
                               .ImageBase = std::bit_cast<void *>(
                                   executable_image_buffer.get()),
                               .fnLoadLibraryA   = LoadLibraryA,
                               .fnGetProcAddress = GetProcAddress,
                           });
    // Write the loader code to target process
    loader_buffer.write(sizeof(loaderdata), &LibraryLoader,
                               static_cast<DWORD>(loader_func_size));
    // Create a remote thread to execute the loader code
    HANDLE thread_handle =
        CreateRemoteThread(process.native.handle, NULL, 0, loader_func_ptr,
                           loader_func_args_ptr, 0, NULL);

    std::cout << "Address of Loader: " << std::hex << loader_buffer.get()
              << std::endl;
    std::cout << "Address of Image: " << std::hex
              << executable_image_buffer.get() << std::endl;

    // Wait for the loader to finish executing
    // WaitForSingleObject(thread_handle, INFINITE);

    return EXIT_SUCCESS;
}
