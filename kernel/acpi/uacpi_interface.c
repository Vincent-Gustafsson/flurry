#include <stdint.h>
#include <flurry/common.h>

#include "flurry/log/tty.h"
#include "flurry/memory/kmalloc.h"
#include "uacpi/uacpi.h"



static uintptr_t rsdp_address;
static uintptr_t hhdm_offset;

void acpi_init(uintptr_t rsdp, uintptr_t offset) {
    rsdp_address = rsdp;
    hhdm_offset = offset;

    void* tmp_buf = kmalloc(4096);
    uacpi_setup_early_table_access(tmp_buf, 4096);

    //madt_init();

    //uacpi_initialize(0);
    //kfree(temp_buffer);

    //uacpi_namespace_load();
    //uacpi_namespace_initialize();
}

/* -- START -- setup_early_table_access requirements */

uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr* out_rsdp_address) {
    *out_rsdp_address = rsdp_address;
    return UACPI_STATUS_OK;
}

void* uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len) {
    return (void*) (addr + hhdm_offset);
}

// Not needed, hhdm-mapping of all physical memory.
void uacpi_kernel_unmap(void* addr, uacpi_size len) {}

void uacpi_kernel_log(uacpi_log_level level, const uacpi_char* msg) {
    switch (level) {
        case UACPI_LOG_INFO:
        case UACPI_LOG_TRACE:
        case UACPI_LOG_DEBUG: break;
        case UACPI_LOG_WARN: kprintf("[UACPI][WARN] %s", msg); break;
        case UACPI_LOG_ERROR: kprintf("[UACPI][ERROR] %s", msg); break;
    }
}

/* -- END -- setup_early_table_access requirements */

void uacpi_kernel_free(void* mem) {
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_free");
}

void* uacpi_kernel_alloc(uacpi_size size) {
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_alloc");
}

uacpi_status uacpi_kernel_pci_device_open(uacpi_pci_address address, uacpi_handle* out_handle) {
	return UACPI_STATUS_UNIMPLEMENTED;
}

void uacpi_kernel_pci_device_close(uacpi_handle) {
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_pci_device_close");
}

uacpi_status uacpi_kernel_pci_read(uacpi_handle device, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64* value)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_pci_device_close");
	return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_pci_write(uacpi_handle device, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64 value)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_pci_write");
	return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_pci_read8(uacpi_handle device, uacpi_size offset, uacpi_u8* value)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_pci_read8");
	return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_pci_read16(uacpi_handle device, uacpi_size offset, uacpi_u16* value)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_pci_read16");
	return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_pci_read32(uacpi_handle device, uacpi_size offset, uacpi_u32* value)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_pci_read32");
	return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_pci_write8(uacpi_handle device, uacpi_size offset, uacpi_u8 value)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_pci_write8");
	return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_pci_write16(uacpi_handle device, uacpi_size offset, uacpi_u16 value)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_pci_write16");
	return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_pci_write32(uacpi_handle device, uacpi_size offset, uacpi_u32 value)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_pci_write32");
	return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_io_map(uacpi_io_addr base, uacpi_size len, uacpi_handle* out_handle)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_io_map");
	return UACPI_STATUS_UNIMPLEMENTED;
}

void uacpi_kernel_io_unmap(uacpi_handle handle)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_io_unmap");
}

uacpi_status uacpi_kernel_io_read8(uacpi_handle h, uacpi_size offset, uacpi_u8* out_value)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_io_read8");
	return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_io_read16(uacpi_handle h, uacpi_size offset, uacpi_u16* out_value)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_io_read16");
	return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_io_read32(uacpi_handle h, uacpi_size offset, uacpi_u32* out_value)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_io_read32");
	return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_io_write8(uacpi_handle h, uacpi_size offset, uacpi_u8 in_value)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_io_write8");
	return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_io_write16(uacpi_handle h, uacpi_size offset, uacpi_u16 in_value)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_io_write16");
	return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_io_write32(uacpi_handle h, uacpi_size offset, uacpi_u32 in_value)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_io_write32");
	return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_u64 uacpi_kernel_get_nanoseconds_since_boot(void)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_get_nanoseconds_since_boot");
}

void uacpi_kernel_stall(uacpi_u8 usec)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_stall");
}

void uacpi_kernel_sleep(uacpi_u64 msec)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_sleep");
}

uacpi_handle uacpi_kernel_create_event(void)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_create_event");

}

void uacpi_kernel_free_event(uacpi_handle handle)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_free_event");

}

uacpi_thread_id uacpi_kernel_get_thread_id(void)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_get_thread_id");

}

uacpi_handle uacpi_kernel_create_mutex(void)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_create_mutex");
}

void uacpi_kernel_free_mutex(uacpi_handle mutex)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_free_mutex");
}

uacpi_status uacpi_kernel_acquire_mutex(uacpi_handle, uacpi_u16)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_acquire_mutex");
}

void uacpi_kernel_release_mutex(uacpi_handle)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_release_mutex");
}

uacpi_bool uacpi_kernel_wait_for_event(uacpi_handle, uacpi_u16)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_wait_for_event");
}

void uacpi_kernel_signal_event(uacpi_handle)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_signal_event");

}

void uacpi_kernel_reset_event(uacpi_handle)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_reset_event");

}

uacpi_status uacpi_kernel_handle_firmware_request(uacpi_firmware_request*)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_handle_firmware_request");
	return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_install_interrupt_handler(uacpi_u32 irq, uacpi_interrupt_handler handler, uacpi_handle ctx,
													uacpi_handle* out_irq_handle)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_install_interrupt_handler");
	return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_uninstall_interrupt_handler(uacpi_interrupt_handler handler, uacpi_handle irq_handle)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_uninstall_interrupt_handler");
	return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_handle uacpi_kernel_create_spinlock(void)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_create_spinlock");
}

void uacpi_kernel_free_spinlock(uacpi_handle lock)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_free_spinlock");

}

uacpi_cpu_flags uacpi_kernel_lock_spinlock(uacpi_handle lock)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_lock_spinlock");

}

void uacpi_kernel_unlock_spinlock(uacpi_handle lock, uacpi_cpu_flags)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_unlock_spinlock");

}

uacpi_status uacpi_kernel_schedule_work(uacpi_work_type type, uacpi_work_handler handler, uacpi_handle ctx)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_schedule_work");

	return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_wait_for_work_completion(void)
{
	kassert(false, "UNIMPLEMENTED: uacpi_kernel_wait_for_work_completion");

	return UACPI_STATUS_UNIMPLEMENTED;
}
