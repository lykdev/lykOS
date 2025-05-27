#include "ioapic.h"

#include <arch/x86_64/devices/pic.h>
#include <common/hhdm.h>
#include <common/log.h>
#include <common/panic.h>
#include <dev/acpi/acpi.h>
#include <lib/def.h>

typedef struct
{
    acpi_sdt_t sdt;
    u32 lapic_addr;
    u32 flags;
}
__attribute__((packed))
madt_t;

typedef struct
{
    u8 type;
    u8 length;
    u8 id;
    u8 _rsv;
    u32 ioapic_addr;
    u32 gsi_base;
}
__attribute__((packed))
madt_ioapic_t;

typedef struct
{
    u8 type;
    u8 length;
    u8 bus;
    u8 source;
    u32 gsi;
    u16 flags;
}
__attribute__((packed))
madt_int_source_override_t;

#define IOREGSEL  0x00
#define IOWIN     0x10

#define IOAPICID  0x00
#define IOAPICVER 0x01
#define IOREDTBL(N) (0x10 + 2 * (N))

#define SO_FLAG_POLARITY_LOW  0b0011
#define SO_FLAG_POLARITY_HIGH 0b0001
#define SO_TRIGGER_EDGE       0b0100
#define SO_TRIGGER_LEVEL      0b1100

#define MADT_TYPE_IOAPIC 1
#define MADT_TYPE_SOURCE_OVERRIDE 2

typedef struct
{
    u64 vector          :  8;
    u64 delivery_mode   :  3;
    u64 dest_mode       :  1;
    u64 delivery_status :  1;
    u64 pin_polarity    :  1;
    u64 remote_irr      :  1;
    u64 trigger_mode    :  1;
    u64 mask            :  1;
    u64 _rsv            : 39;
    u64 destination     :  8;
}
__attribute__((packed))
ioapic_redirect_t;

static uptr g_ioapic_base = 0;

static struct
{
    u16 dest;
    u16 flags;
}
g_irq_redirection_table[16] = {
    {  0, 0 },
    {  1, 0 },
    {  2, 0 },
    {  3, 0 },
    {  4, 0 },
    {  5, 0 },
    {  6, 0 },
    {  7, 0 },
    {  8, 0 },
    {  9, 0 },
    { 10, 0 },
    { 11, 0 },
    { 12, 0 },
    { 13, 0 },
    { 14, 0 },
    { 15, 0 }
};

static void ioapic_write(u32 reg, u32 data)
{
    *(volatile u32*)(g_ioapic_base + IOREGSEL) = reg; // IOREGSEL
    *(volatile u32*)(g_ioapic_base + IOWIN) = data;   // IOWIN
}

static u32 ioapic_read(u32 reg)
{
    *(volatile u32*)(g_ioapic_base + IOREGSEL) = reg; // IOREGSEL
    return *(volatile u32*)(g_ioapic_base + IOWIN);   // IOWIN
}

void x86_64_ioapic_init()
{
    madt_t *madt = (madt_t*)acpi_lookup("APIC");
    if (!madt)
        panic("MADT not found!");

    u8 *ptr = (u8*)madt + sizeof(madt_t);
    u8 *end = (u8*)madt + madt->sdt.length;
    while (ptr < end)
    {
        u8 type = ptr[0];
        u8 length = ptr[1];

        switch (type)
        {
            case MADT_TYPE_IOAPIC:
                g_ioapic_base = ((madt_ioapic_t*)ptr)->ioapic_addr + HHDM;
            break;
            case MADT_TYPE_SOURCE_OVERRIDE:
                madt_int_source_override_t *so = (madt_int_source_override_t*)ptr;
                g_irq_redirection_table[so->source].dest  = so->gsi;
                g_irq_redirection_table[so->source].flags = so->flags;
            break;
        }

        ptr += length;
    }

    if (!g_ioapic_base)
        panic("I/O APIC interrupt controller structure not found!");
}

void x86_64_ioapic_map_gsi(u8 gsi, u8 lapic_id, bool low_polarity, bool trigger_mode, u8 vector)
{
    ioapic_redirect_t entry = {
        .vector          = vector,
        .delivery_mode   = 0,    // Fixed
        .dest_mode       = 0,    // Physical
        .delivery_status = 0,    // Read-only
        .pin_polarity    = low_polarity ? 1 : 0,
        .remote_irr      = 0,    // Read-only
        .trigger_mode    = trigger_mode ? 1 : 0,
        .mask            = 0,    // Unmasked
        ._rsv            = 0,
        .destination     = lapic_id
    };

    u32 low  = *(u32 *)&entry;
    u32 high = *((u32 *)&entry + 1);

    ioapic_write(IOREDTBL(gsi), low);
    ioapic_write(IOREDTBL(gsi) + 1, high);
}

void x86_64_ioapic_map_legacy_irq(u8 irq, u8 lapic_id, bool fallback_low_polarity, bool fallback_trigger_mode, u8 vector)
{
    if(irq < 16)
    {
        // Polarity.
        switch(g_irq_redirection_table[irq].flags & 0b0011)
        {
            case SO_FLAG_POLARITY_LOW:
                fallback_low_polarity = true;
                break;
            case SO_FLAG_POLARITY_HIGH:
                fallback_low_polarity = false;
                break;
        }
        // Trigger mode.
        switch(g_irq_redirection_table[irq].flags & 0b1100)
        {
            case SO_TRIGGER_EDGE:
                fallback_trigger_mode = false;
                break;
            case SO_TRIGGER_LEVEL:
                fallback_trigger_mode = true;
                break;
        }

        irq = g_irq_redirection_table[irq].dest;
    }

    x86_64_ioapic_map_gsi(irq, lapic_id, fallback_low_polarity, fallback_trigger_mode, vector);
}
