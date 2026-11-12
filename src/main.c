#include <r4os/platform2.h>

static const char msg_name[] = "CSTARTD\r\n";
static const char msg_context[] = "C R4XStart2 context: OK\r\n";
static const char msg_abi[] = "C R4XStart2 ABI: 2.0 size=120 OK\r\n";
static const char msg_reserved_fields[] = "C R4XStart2 reserved fields: OK\r\n";
static const char msg_reserved_slots[] = "C R4SYS2 reserved slots: OK\r\n";
static const char msg_forward_minor[] = "C R4XStart2 newer minor: OK\r\n";
static const char msg_abi_failed[] = "C R4XStart2 ABI: FAILED\r\n";
static const char msg_args[] = "C R4XStart2 args: ";
static const char msg_none[] = "<none>\r\n";
static const char msg_import[] = "R4SYS/Query2 import from C: OK\r\n";
static const char msg_vm_alloc[] = "C R4SYS VM alloc: OK\r\n";
static const char msg_vm_failed[] = "C R4SYS VM alloc: FAILED\r\n";
static const char msg_result[] = "CSTARTD result: OK\r\n";
static const char msg_crlf[] = "\r\n";
static const char msg_stack_ok[] = "CSTARTD large frame: OK\r\n";
static const char msg_stack_failed[] = "CSTARTD large frame: FAILED\r\n";

static __attribute__((noinline)) int large_stack_frame(void) {
    volatile uint8_t bytes[384u * 1024u] __attribute__((aligned(8192)));
    if (((uintptr_t)bytes & 8191u) != 0u) return 0;
    for (uint32_t page = 0; page < sizeof(bytes) / 4096u; ++page) {
        bytes[page * 4096u] = (uint8_t)(0x5bu ^ page);
    }
    for (uint32_t page = 0; page < sizeof(bytes) / 4096u; ++page) {
        if (bytes[page * 4096u] != (uint8_t)(0x5bu ^ page)) return 0;
    }
    return 1;
}

// Independent literal expectations: Docs/API/Generation2.txt, Start2 fields.
_Static_assert(sizeof(R4XStartContext2) == 120u, "Published Start2 size");
_Static_assert(sizeof(R4XStartImport) == 40u, "Published import size");
_Static_assert(sizeof(R4LQuery2) == 24u, "Published Query2 size");
_Static_assert(offsetof(R4XStartContext2, args) == 40u, "Start2 args offset");
_Static_assert(offsetof(R4XStartContext2, imports) == 56u, "Start2 imports offset");
_Static_assert(offsetof(R4XStartContext2, exit) == 88u, "Start2 exit offset");
_Static_assert(offsetof(R4XStartContext2, should_close) == 96u, "Start2 close offset");
_Static_assert(offsetof(R4XStartContext2, yield) == 104u, "Start2 yield offset");
_Static_assert(offsetof(R4XStartContext2, reserved2) == 112u, "Start2 tail offset");

static void write_text(const R4OS2Context *view, const char *text) {
    uint64_t address = 0;
    if (R4OS2ContextFunction(view, 1, offsetof(R4XStartR4Sys2, write), &address)) return;
    uint32_t length = 0;
    while (text[length]) ++length;
    ((R4SysWriteFn)(uintptr_t)address)((const uint8_t *)text, length);
}

int32_t r4_main2(const R4OS2Context *view) {
    const R4XStartContext2 *ctx = view->raw;
    if (ctx->magic != UINT32_C(0x32583452) || ctx->abi_major != 2u ||
        ctx->abi_minor != 0u || ctx->size != 120u || ctx->reserved0 ||
        ctx->reserved1 || ctx->reserved_memory0 || ctx->reserved_memory1 || ctx->reserved2) {
        write_text(view, msg_abi_failed);
        return 1;
    }
    uint64_t args_length = 0;
    const uint8_t *args = R4OS2ContextArgs(view, &args_length);
    if (args_length != ctx->args_len) return 1;
    static const char stack_arg[] = "/STACKPROBE";
    int stack_probe = args != 0 && ctx->args_len == sizeof(stack_arg) - 1u;
    for (uint32_t i = 0; stack_probe && i < sizeof(stack_arg) - 1u; ++i) {
        if (args[i] != (uint8_t)stack_arg[i]) stack_probe = 0;
    }
    if (stack_probe) {
        const int ok = large_stack_frame();
        write_text(view, ok ? msg_stack_ok : msg_stack_failed);
        return ok ? 0 : 1;
    }
    const int quiet = args != 0 && ctx->args_len == 6u &&
        args[0] == '/' && (args[1] & ~32u) == 'Q' &&
        (args[2] & ~32u) == 'U' && (args[3] & ~32u) == 'I' &&
        (args[4] & ~32u) == 'E' && (args[5] & ~32u) == 'T';
    if (!quiet) {
        write_text(view, msg_name);
        write_text(view, msg_context);
        write_text(view, msg_abi);
        write_text(view, msg_reserved_fields);
    }
    const R4XStartR4Sys2 *sys = (const R4XStartR4Sys2 *)R4OS2ContextGroup(view, 1);
    const R4XStartImport *imports = (const R4XStartImport *)(uintptr_t)ctx->imports;
    if (!sys || sys->reserved0 || sys->reserved1 || ctx->import_count != 1 ||
        imports[0].group_id != 1 || imports[0].min_version != 2 || imports[0].resolved_version != 2 || imports[0].table != (uintptr_t)sys) {
        write_text(view, msg_abi_failed);
        return 1;
    }
    if (!quiet) write_text(view, msg_reserved_slots);
    R4XStartContext2 future = *ctx;
    R4OS2Context checked;
    future.abi_minor = 1u;
    if (R4OS2ContextInit(&future, &checked) != R4OS2_CONTEXT_OK) {
        write_text(view, msg_abi_failed);
        return 1;
    }
    // Fail closed on a short record and invoke both real typed callbacks.
    future.size = 119u;
    if (R4OS2ContextInit(&future, &checked) != R4OS2_CONTEXT_INVALID ||
        R4OS2ContextShouldClose(view) || !R4OS2ContextYield(view) || R4OS2ContextShouldClose(view)) return 1;
    if (!quiet) {
        write_text(view, msg_forward_minor);
        write_text(view, msg_args);
        if (args == 0 || ctx->args_len == 0) {
            write_text(view, msg_none);
        } else {
            uint64_t write = 0;
            if (R4OS2ContextFunction(view, 1, offsetof(R4XStartR4Sys2, write), &write)) return 1;
            ((R4SysWriteFn)(uintptr_t)write)(args, (uint32_t)ctx->args_len);
            write_text(view, msg_crlf);
        }
        write_text(view, msg_import);
    }
    uint64_t reserve_address = 0, commit_address = 0, release_address = 0;
    if (R4OS2ContextFunction(view, 1, offsetof(R4XStartR4Sys2, vm_reserve), &reserve_address) ||
        R4OS2ContextFunction(view, 1, offsetof(R4XStartR4Sys2, vm_commit), &commit_address) ||
        R4OS2ContextFunction(view, 1, offsetof(R4XStartR4Sys2, vm_release), &release_address)) return 1;
    const R4SysVmReserveFn reserve = (R4SysVmReserveFn)(uintptr_t)reserve_address;
    const R4SysVmCommitFn commit = (R4SysVmCommitFn)(uintptr_t)commit_address;
    const R4SysVmReleaseFn release = (R4SysVmReleaseFn)(uintptr_t)release_address;
    R4ProgramVmRegionInfo region = {0};
    if (reserve(8192u, 4096u, R4OS_VM_REGION_FLAGS_DEFAULT, &region) != R4OS_VM_OK) {
        write_text(view, msg_vm_failed);
        return 1;
    }
    if (!region.id || !region.base || region.base % 4096u || region.len < 8192u ||
        commit(region.id, 0, 8192u, 0) != R4OS_VM_OK) {
        (void)release(region.id);
        write_text(view, msg_vm_failed);
        return 1;
    }
    volatile uint8_t *mem = (volatile uint8_t *)(uintptr_t)region.base;
    mem[0] = 0x43u;
    mem[4096] = 0x53u;
    mem[8191] = 0x21u;
    const int data_ok = mem[0] == 0x43u && mem[4096] == 0x53u && mem[8191] == 0x21u;
    const int32_t released = release(region.id);
    if (!data_ok || released != R4OS_VM_OK) {
        write_text(view, msg_vm_failed);
        return 1;
    }
    if (!quiet) {
        write_text(view, msg_vm_alloc);
        write_text(view, msg_result);
    }
    return 0;
}
