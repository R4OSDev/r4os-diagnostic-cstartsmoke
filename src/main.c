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

_Static_assert(R4XSTART_ABI_MAJOR == 1u, "R4XStart ABI major mismatch");
_Static_assert(R4XSTART_ABI_MINOR == 1u, "R4XStart ABI minor mismatch");
_Static_assert(R4XSTART_CONTEXT_SIZE == 128u, "R4XStart context size mismatch");

int32_t r4_main(const R4XStartContext *ctx, R4Sys *sys) {
    r4sys_write_cstr(sys, msg_name);
    r4sys_write_cstr(sys, msg_context);
    if (ctx == 0 ||
        ctx->abi_major != R4XSTART_ABI_MAJOR ||
        ctx->abi_minor != R4XSTART_ABI_MINOR ||
        ctx->size != R4XSTART_CONTEXT_SIZE ||
        ctx->reserved_runtime != 0u) {
        r4sys_write_cstr(sys, msg_abi_failed);
        return 1;
    }
    r4sys_write_cstr(sys, msg_abi);
    r4sys_write_cstr(sys, msg_reserved_runtime);
    if (sys->table == 0 || sys->table->reserved_shell_run != 0u) {
        r4sys_write_cstr(sys, msg_abi_failed);
        return 1;
    }
    r4sys_write_cstr(sys, msg_reserved_shell_run);
    R4XStartContext future = *ctx;
    future.abi_minor = (uint16_t)(R4XSTART_ABI_MINOR + 1u);
    if (!r4xstart_context_valid(&future)) {
        r4sys_write_cstr(sys, msg_abi_failed);
        return 1;
    }
    r4sys_write_cstr(sys, msg_forward_minor);
    r4sys_write_cstr(sys, msg_args);
    const uint8_t *args = r4xstart_args(ctx);
    if (args == 0 || ctx->args_len == 0) {
        r4sys_write_cstr(sys, msg_none);
    } else {
        r4sys_write(sys, args, (uint32_t)ctx->args_len);
        r4sys_write_cstr(sys, msg_crlf);
    }
    r4sys_write_cstr(sys, msg_import);
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
    r4sys_write_cstr(sys, msg_vm_alloc);
    r4sys_write_cstr(sys, msg_result);
    return 0;
}
