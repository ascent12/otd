# Libotd - Own the Display

This is a "library" for taking control of the Linux DRM resources, and giving the user
an OpenGL ES buffer for each of the monitors. It would be helpful for Wayland compositors,
but nothing Wayland specific is going on here, and could be used by anything which just
wants to draw shit to the monitors.

It's not actually a library I expect people to use, and was just written so I can learn
all of the parts of the DRM system, and possibly have this be cannibalized for other projects.

The API is extremely inconsistent, and no user-facing header has actually been written.
Error-handling and logging extremely crappy. There are probably several bugs.

## What it does
- Configures each of the displays.
- Monitors devices for changes.
- Session management.

## What it doesn't do
- Input. That's up to you (i.e. use libinput).
- Layout. You just get a buffer: it's up to you how you draw in it.
- Transformations. As above. If you want to rotate something, apply a transformation in your buffer.
- Custom modes (Ones the montior doesn't provide). It seems like a pretty obscure thing to want to do.
- VT switching (This is planned). It doesn't handle DBus signals yet.

## Dependencies
- libdrm
- libgbm
- EGL
- OpenGL ES
- libudev
- libsystemd

## Example
**DO NOT OPEN WITHIN X OR A WAYLAND COMPOSITOR**  
It messes with your ability to change virtual terminals until you reboot your computer.
This needs to be fixed eventually.

```
make
./main
```
The example doesn't support input, so you cannot exit it (via Ctrl+C or Alt+F2 etc.).
It will close itself after 10 seconds.
