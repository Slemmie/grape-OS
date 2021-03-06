# OS (64-bit)
Miserably failing at creating an operating system!

# Usage
To run the system, simply (using qemu):

	$ make run

To make the os-image without running directly in qemu (can be used in other emulators/installed onto somewhere):

	$ make

Simply compiling does not require qemu as a dependency.

# Dependencies (see Installation...)
- nasm (assembler)
- gcc (C compiler)
- make (build system)
- ld (linker)
- qemu-system-x86 (machine emulator) (is not required for raw building of the os-image)

#### Installation (will most likely require sudo privileges):

	$ apt install nasm
	$ apt install gcc
	$ apt install make
	$ apt install binutils #(for ld linker tool)
	$ apt install qemu-system-x86

These packages are common, and should also exist in other mainstream package managers (like PacMan, etc.).

# Style
For now, there is not really an established conduct, however;
- No use of C-stl or 3rd-party libraries
- Everything written in assembly or C (aside from linker scripts and makefiles)

# TODO
- kmalloc function (allocate memory smaller than page size inside kernel)
- vmalloc function (allocate page-aligned memory inside kernel)
- timer utility (sleep, wake-up every x jiffies callbacks, convert kernel to infinite loop launching the callbacks)
- multi threading support
- file system
- shell-like something something?
- event dispatch system (callbacks)
