#include <r4os/r4os.h>

R4OS_TEXT(msg_name, "CSTARTD\r\n");
R4OS_TEXT(msg_context, "C R4XStart context: OK\r\n");
R4OS_TEXT(msg_abi, "C R4XStart ABI: 1.1 size=128 OK\r\n");
R4OS_TEXT(msg_reserved_runtime, "C R4XStart reserved_runtime: OK\r\n");
R4OS_TEXT(msg_reserved_shell_run, "C R4SYS reserved_shell_run: OK\r\n");
R4OS_TEXT(msg_forward_minor, "C R4XStart newer minor: OK\r\n");
R4OS_TEXT(msg_abi_failed, "C R4XStart ABI: FAILED\r\n");
R4OS_TEXT(msg_args, "C R4XStart args: ");
R4OS_TEXT(msg_none, "<none>\r\n");
R4OS_TEXT(msg_import, "R4SYS/R4L import from C: OK\r\n");
R4OS_TEXT(msg_vm_alloc, "C R4SYS VM alloc: OK\r\n");
R4OS_TEXT(msg_vm_failed, "C R4SYS VM alloc: FAILED\r\n");
R4OS_TEXT(msg_result, "CSTARTD result: OK\r\n");
R4OS_TEXT(msg_crlf, "\r\n");
R4OS_TEXT(msg_stack_ok, "CSTARTD large frame: OK\r\n");
R4OS_TEXT(msg_stack_failed, "CSTARTD large frame: FAILED\r\n");

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

_Static_assert(R4XSTART_ABI_MAJOR == 1u, "R4XStart ABI major mismatch");
_Static_assert(R4XSTART_ABI_MINOR == 1u, "R4XStart ABI minor mismatch");
_Static_assert(R4XSTART_CONTEXT_SIZE == 128u, "R4XStart context size mismatch");

int32_t r4_main(const R4XStartContext *ctx, R4Sys *sys) {
    if (ctx == 0 ||
        ctx->abi_major != R4XSTART_ABI_MAJOR ||
        ctx->abi_minor != R4XSTART_ABI_MINOR ||
        ctx->size != R4XSTART_CONTEXT_SIZE ||
        ctx->reserved_runtime != 0u) {
        r4sys_write_cstr(sys, msg_abi_failed);
        return 1;
    }
    const uint8_t *args = r4xstart_args(ctx);
    static const char stack_arg[] = "/STACKPROBE";
    int stack_probe = args != 0 && ctx->args_len == sizeof(stack_arg) - 1u;
    for (uint32_t i = 0; stack_probe && i < sizeof(stack_arg) - 1u; ++i) {
        if (args[i] != (uint8_t)stack_arg[i]) stack_probe = 0;
    }
    if (stack_probe) {
        const int ok = large_stack_frame();
        r4sys_write_cstr(sys, ok ? msg_stack_ok : msg_stack_failed);
        return ok ? 0 : 1;
    }
    const int quiet = args != 0 && ctx->args_len == 6u &&
        args[0] == '/' && (args[1] & ~32u) == 'Q' &&
        (args[2] & ~32u) == 'U' && (args[3] & ~32u) == 'I' &&
        (args[4] & ~32u) == 'E' && (args[5] & ~32u) == 'T';
    if (!quiet) {
        r4sys_write_cstr(sys, msg_name);
        r4sys_write_cstr(sys, msg_context);
        r4sys_write_cstr(sys, msg_abi);
        r4sys_write_cstr(sys, msg_reserved_runtime);
    }
    if (sys->table == 0 || sys->table->reserved_shell_run != 0u) {
        r4sys_write_cstr(sys, msg_abi_failed);
        return 1;
    }
    if (!quiet) r4sys_write_cstr(sys, msg_reserved_shell_run);
    R4XStartContext future = *ctx;
    future.abi_minor = (uint16_t)(R4XSTART_ABI_MINOR + 1u);
    if (!r4xstart_context_valid(&future)) {
        r4sys_write_cstr(sys, msg_abi_failed);
        return 1;
    }
    if (!quiet) {
        r4sys_write_cstr(sys, msg_forward_minor);
        r4sys_write_cstr(sys, msg_args);
        if (args == 0 || ctx->args_len == 0) {
            r4sys_write_cstr(sys, msg_none);
        } else {
            r4sys_write(sys, args, (uint32_t)ctx->args_len);
            r4sys_write_cstr(sys, msg_crlf);
        }
        r4sys_write_cstr(sys, msg_import);
    }
    uint8_t *mem = (uint8_t *)r4sys_vm_alloc(sys, 8192, 4096);
    if (mem == 0) {
        r4sys_write_cstr(sys, msg_vm_failed);
        return 1;
    }
    mem[0] = 0x43u;
    mem[4096] = 0x53u;
    mem[8191] = 0x21u;
    if (mem[0] != 0x43u || mem[4096] != 0x53u || mem[8191] != 0x21u || r4sys_vm_free(sys, mem) != R4OS_VM_OK) {
        r4sys_write_cstr(sys, msg_vm_failed);
        return 1;
    }
    if (!quiet) {
        r4sys_write_cstr(sys, msg_vm_alloc);
        r4sys_write_cstr(sys, msg_result);
    }
    return 0;
}
