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
use stm32f1::stm32f103::{interrupt, Interrupt, NVIC};

#[global_allocator]
static ALLOCATOR: CortexMHeap = CortexMHeap::empty();

const HEAP_SIZE: usize = 1024;

#[entry]
fn main() -> ! {
    unsafe { ALLOCATOR.init(cortex_m_rt::heap_start() as usize, HEAP_SIZE) }

    let mut xs = vec![0, 1, 2];

    let mut t = true;

    loop {
        if t {
            xs.push(1);
        }
        else {
            xs.pop();
        }

        t = !t;
    }
}

// define what happens in an Out Of Memory (OOM) condition
#[alloc_error_handler]
fn alloc_error(_layout: Layout) -> ! {
    asm::bkpt();

    loop {}
}
