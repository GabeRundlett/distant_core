#include <distant/injection.hpp>

#include <cstdio>
#include <format>
#include <fstream>

namespace distant {
    Injectable::Injectable(const std::filesystem::path & filepath)
        : native{.valid = false} {
        if (!GetFileAttributes(filepath.string().c_str())) {
            std::printf(std::format("Failed to get file attributes of file "
                                    "'{}', does it exist?\n",
                                    filepath.string())
                            .c_str());
            return;
        }
        std::ifstream dll_file_istream(filepath,
                                       std::ios::binary | std::ios::ate);
        if (dll_file_istream.fail()) {
            std::printf(
                std::format("Failed to open file '{}'\n", filepath.string())
                    .c_str());
            return;
        }
        auto file_size = dll_file_istream.tellg();
        if (file_size < 0x1000) {
            std::printf(std::format("File '{}' is too small to be a valid DLL "
                                    "({:x} < 0x1000) bytes\n",
                                    filepath.string(), (std::size_t)file_size)
                            .c_str());
            return;
        }
        binary.resize(file_size);
        dll_file_istream.seekg(0, std::ios::beg);
        dll_file_istream.read(std::bit_cast<char *>(binary.data()), file_size);
        dll_file_istream.close();
        native.dos_header_ptr = &binary[0];
        auto & dos_header =
            *static_cast<IMAGE_DOS_HEADER *>(native.dos_header_ptr);
        if (dos_header.e_magic != 0x5a4d) {
            std::printf(
                std::format(
                    "File '{}' does not have a valid DOS header magic number\n",
                    filepath.string())
                    .c_str());
            return;
        }
        native.nt_headers_ptr =
            std::bit_cast<IMAGE_NT_HEADERS *>(&binary[dos_header.e_lfanew]);
        native.valid = true;
    }

    Injectable::Injectable(Injectable && other) noexcept {
        native       = std::move(other.native);
        binary       = std::move(other.binary);
        other.native = {};
    }

    Injectable & Injectable::operator=(Injectable && other) noexcept {
        native       = std::move(other.native);
        binary       = std::move(other.binary);
        other.native = {};
        return *this;
    }

    Injectable::operator bool() const { return native.valid; }

    InjectionContext::InjectionContext(const Process &    process,
                                       const Injectable & injectable)
        : process(process),
          image_buffer(process.virtual_alloc_unique(
              static_cast<IMAGE_NT_HEADERS *>(injectable.native.nt_headers_ptr)
                  ->OptionalHeader.SizeOfImage)) {
        image_buffer.write(
            0, injectable.binary.data(),
            static_cast<IMAGE_NT_HEADERS *>(injectable.native.nt_headers_ptr)
                ->OptionalHeader.SizeOfHeaders);
        IMAGE_SECTION_HEADER * section_header_ptr =
            (PIMAGE_SECTION_HEADER)(static_cast<IMAGE_NT_HEADERS *>(
                                        injectable.native.nt_headers_ptr) +
                                    1);
        for (int i = 0; i < static_cast<IMAGE_NT_HEADERS *>(
                                injectable.native.nt_headers_ptr)
                                ->FileHeader.NumberOfSections;
             i++) {
            image_buffer.write(
                section_header_ptr[i].VirtualAddress,
                &injectable.binary[section_header_ptr[i].PointerToRawData],
                section_header_ptr[i].SizeOfRawData);
        }
    }

    InjectionContext::InjectionContext(InjectionContext && other) noexcept
        : process(other.process), image_buffer(std::move(other.image_buffer)) {}

    InjectionContext &
    InjectionContext::operator=(InjectionContext && other) noexcept {
        if (&process != &other.process) {
            std::printf("InjectionContext attempted to move assign, but the "
                        "processes that owned the two did not match\n");
            return *this;
        }

        image_buffer = std::move(other.image_buffer);

        return *this;
    }

    InjectionContext::operator bool() const noexcept {
        return static_cast<bool>(image_buffer);
    }

    constexpr auto SHELLCODE_ALIGNMENT = 32;
    struct alignas(SHELLCODE_ALIGNMENT) ShellcodeArgs {
        using FuncLoadLibrary    = decltype(LoadLibrary);
        using FuncGetProcAddress = decltype(GetProcAddress);

        void *               image_buffer_address = nullptr;
        FuncLoadLibrary *    LoadLibrary_ptr      = nullptr;
        FuncGetProcAddress * GetProcAddress_ptr   = nullptr;
    };

    DWORD __stdcall injection_shellcode(ShellcodeArgs & args) {
        auto * binary =
            reinterpret_cast<std::byte *>(args.image_buffer_address);

        auto & dos_header = *reinterpret_cast<IMAGE_DOS_HEADER *>(&binary[0]);
        auto & nt_headers =
            *reinterpret_cast<IMAGE_NT_HEADERS *>(&binary[dos_header.e_lfanew]);
        auto & optional_header = nt_headers.OptionalHeader;
        auto & base_relocation = *reinterpret_cast<IMAGE_BASE_RELOCATION *>(
            &binary[optional_header
                        .DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC]
                        .VirtualAddress]);
        auto & import_directory = *reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR *>(
            &binary[optional_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]
                        .VirtualAddress]);

        PIMAGE_BASE_RELOCATION pIBR = &base_relocation;
        std::ptrdiff_t         delta =
            (std::ptrdiff_t)(binary - optional_header.ImageBase);

        while (pIBR->VirtualAddress) {
            if (pIBR->SizeOfBlock >= sizeof(IMAGE_BASE_RELOCATION)) {
                int count =
                    (pIBR->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) /
                    sizeof(WORD);
                std::ptrdiff_t * list = (std::ptrdiff_t *)(pIBR + 1);
                for (int i = 0; i < count; i++) {
                    if (list[i]) {
                        std::ptrdiff_t * ptr =
                            (std::ptrdiff_t *)(binary + (pIBR->VirtualAddress +
                                                         (list[i] & 0xFFF)));
                        *ptr += delta;
                    }
                }
            }
            pIBR = (PIMAGE_BASE_RELOCATION)((LPBYTE)pIBR + pIBR->SizeOfBlock);
        }

        PIMAGE_IMPORT_DESCRIPTOR pIID = &import_directory;

        // Resolve DLL imports
        while (pIID->Characteristics) {
            PIMAGE_THUNK_DATA OrigFirstThunk =
                (PIMAGE_THUNK_DATA)(binary + pIID->OriginalFirstThunk);
            PIMAGE_THUNK_DATA FirstThunk =
                (PIMAGE_THUNK_DATA)(binary + pIID->FirstThunk);
            HMODULE hModule = args.LoadLibrary_ptr((LPCSTR)binary + pIID->Name);
            if (!hModule) return FALSE;
            while (OrigFirstThunk->u1.AddressOfData) {
                if (OrigFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) {
                    // Import by ordinal
                    ULONGLONG Function = (ULONGLONG)args.GetProcAddress_ptr(
                        hModule, (LPCSTR)(OrigFirstThunk->u1.Ordinal & 0xFFFF));
                    if (!Function) return FALSE;

                    FirstThunk->u1.Function = Function;
                } else {
                    // Import by name
                    PIMAGE_IMPORT_BY_NAME pIBN =
                        (PIMAGE_IMPORT_BY_NAME)(binary + OrigFirstThunk->u1
                                                             .AddressOfData);
                    ULONGLONG Function = (ULONGLONG)args.GetProcAddress_ptr(
                        hModule, (LPCSTR)pIBN->Name);
                    if (!Function) return FALSE;
                    FirstThunk->u1.Function = Function;
                }
                OrigFirstThunk++;
                FirstThunk++;
            }
            pIID++;
        }

        if (optional_header.AddressOfEntryPoint) {
            using FuncDllMain = BOOL APIENTRY(HMODULE, DWORD, LPVOID);

            auto & EntryPoint =
                *(FuncDllMain *)(binary + optional_header.AddressOfEntryPoint);
            return EntryPoint((HMODULE)binary, DLL_PROCESS_ATTACH,
                              NULL); // Call the entry point
        }
        return TRUE;
    }
    DWORD __stdcall injection_shellstub() { return 0; }

    void InjectionContext::launch_thread() {
        const auto shellcode_size = (std::intptr_t)injection_shellstub -
                                    (std::intptr_t)injection_shellcode;
        const auto loader_size   = shellcode_size + sizeof(ShellcodeArgs);
        auto       loader_buffer = process.virtual_alloc_unique(loader_size);

        auto loader_shellcode_ptr =
            (LPTHREAD_START_ROUTINE)(std::bit_cast<std::intptr_t>(
                                         loader_buffer.get()) +
                                     sizeof(ShellcodeArgs));
        auto loader_shellargs_ptr = std::bit_cast<void *>(loader_buffer.get());
        loader_buffer.write_value(
            0, ShellcodeArgs{
                   .image_buffer_address =
                       std::bit_cast<void *>(image_buffer.get()),
                   .LoadLibrary_ptr    = LoadLibraryA,
                   .GetProcAddress_ptr = GetProcAddress,
               });
        loader_buffer.write(sizeof(ShellcodeArgs), loader_shellcode_ptr,
                            static_cast<DWORD>(shellcode_size));
        HANDLE thread_handle = CreateRemoteThread(
            process.native.handle, NULL, 0, loader_shellcode_ptr,
            loader_shellargs_ptr, 0, NULL);

        // std::printf("Address of Loader: %x\n", loader_buffer.address);
        // std::printf("Address of Image: %x\n", image_buffer.address);
    }
} // namespace distant
