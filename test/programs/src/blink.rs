#![feature(alloc_error_handler)]
#![no_main]
#![no_std]

extern crate alloc;
extern crate panic_halt;

use alloc::vec::Vec;
use core::alloc::{GlobalAlloc, Layout};
use core::ptr;

use cortex_m::interrupt;
use cortex_m::asm;
use cortex_m_rt::entry;
use core::cell::UnsafeCell;

#[entry]
fn main() -> ! {
    let mut xs = Vec::new();

    xs.push(42);

    loop {}
}

struct BumpPointerAlloc {
    head: UnsafeCell<usize>,
    end: usize,
}

unsafe impl Sync for BumpPointerAlloc {}

unsafe impl GlobalAlloc for BumpPointerAlloc {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        interrupt::free(|_| {
            let head = self.head.get();

            let align = layout.align();
            let res = *head % align;
            let start = if res == 0 { *head } else { *head + align - res };
            if start + align > self.end {
                ptr::null_mut()
            } else {
                *head = start + align;
                start as *mut u8
            }
        })
    }

    unsafe fn dealloc(&self, _: *mut u8, _: Layout) {}
}

#[global_allocator]
static HEAP: BumpPointerAlloc = BumpPointerAlloc {
    head: UnsafeCell::new(0x2000_0100),
    end: 0x2000_0200,
};

#[alloc_error_handler]
fn on_oom(_: Layout) -> ! {
    asm::bkpt();

    loop {}
}
