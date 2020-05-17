#![feature(alloc)]
#![feature(alloc_error_handler)]
#![no_main]
#![no_std]

extern crate alloc;
extern crate panic_halt;

use self::alloc::vec;
use core::alloc::Layout;

use alloc_cortex_m::CortexMHeap;
use cortex_m::asm;
use cortex_m_rt::entry;
use cortex_m_semihosting::hprintln;

// this is the allocator the application will use
#[global_allocator]
static ALLOCATOR: CortexMHeap = CortexMHeap::empty();

const HEAP_SIZE: usize = 128; // in bytes

#[entry]
fn main() -> ! {
    // Initialize the allocator BEFORE you use it
    unsafe { ALLOCATOR.init(cortex_m_rt::heap_start() as usize, HEAP_SIZE) }

    // Growable array allocated on the heap
    let xs = vec![0, 1, 2];

    hprintln!("{:?}", xs).unwrap();

    loop {}
}

// define what happens in an Out Of Memory (OOM) condition
#[alloc_error_handler]
fn alloc_error(_layout: Layout) -> ! {
    asm::bkpt();

    loop {}
}
