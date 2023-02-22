pub mod sigslot;

#[cfg(not(any(target_os = "android", target_os = "ios", target_arch = "wasm32")))]
//pub mod system;
#[cfg(all(
    feature = "system_screen_capture",
    not(any(target_os = "android", target_os = "ios", target_arch = "wasm32"))
))]
pub mod screen;

#[cfg(all(feature = "system_gamepad_virtual", target_os = "windows"))]
pub mod gamepad_virtual;
#[cfg(all(feature = "system_gamepad_virtual", target_os = "windows"))]
pub mod gamepad_virtual_bus;
