#![feature(alloc_error_handler)]
#![no_main]
#![no_std]

extern crate alloc;
extern crate panic_halt;

use self::alloc::vec;
use alloc_cortex_m::CortexMHeap;
use core::alloc::Layout;
use cortex_m::asm;
use cortex_m_rt::entry;

#[global_allocator]
static ALLOCATOR: CortexMHeap = CortexMHeap::empty();
const HEAP_SIZE: usize = 256;

#[entry]
fn main() -> ! {
    unsafe { ALLOCATOR.init(cortex_m_rt::heap_start() as usize, HEAP_SIZE) }

    let mut alloc_test = vec![0xffffffffu32, 0xffffffffu32];
    alloc_test.push(0xF0F0F0F0u32);

    unsafe {
        *(0x2000_0100 as *mut u32) = test_stack(10);
    };

    loop {}
}

fn test_stack(n: u32) -> u32 {
    match n {
        0 => 0,
        1 | 2 => 1,
        3 => 2,
        _ => test_stack(n - 1) + test_stack(n - 2),
    }
}

#[alloc_error_handler]
fn alloc_error(_layout: Layout) -> ! {
    asm::bkpt();
    loop {}
}
