#![no_std]
#![no_main]

extern crate panic_halt;

use cortex_m::asm;
use cortex_m::peripheral::syst::SystClkSource;
use cortex_m_rt::entry;
use stm32f1::stm32f103::{interrupt, Interrupt, NVIC};

#[entry]
fn main() -> ! {
    asm::nop();
    asm::nop();

    loop {
        // your code goes here
    }
}
