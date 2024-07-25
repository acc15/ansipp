# ansipp

Simple, small, fast and portable library to use colors/drawing directly in terminal.

Not `ncurses` killer, but if you don't need all power of `ncurses` this might be better as its simpler, smaller and faster.
It doesn't store terminal screen in memory.

## Cross platform

It builds and works on 

![Linux](https://github.com/acc15/ansipp/actions/workflows/linux.yml/badge.svg)
![MacOS](https://github.com/acc15/ansipp/actions/workflows/macos.yml/badge.svg)
![Windows](https://github.com/acc15/ansipp/actions/workflows/windows.yml/badge.svg)

## Hello world

Build & install library: `./install.sh`

Add to your `CMakeLists.txt`:

```cmake
find_package(ansipp REQUIRED)
target_link_libraries(your_executable PRIVATE ansipp::ansipp)
```

Code:

```c++
#include <iostream>
#include <ansipp.hpp>

int main() {
    using namespace ansipp;
    charbuf out(4096);
    if (std::error_code ec; init(ec), ec) {
        out << "can't init: " << ec.message() << '\n';
        return EXIT_FAILURE;
    }

    out << "hello " << attrs().on(BOLD).fg(RED) << "RED BOLD" << attrs() << " text\n";
    terminal_write(out.flush());
    return EXIT_SUCCESS;
}
```

## Features

* CMake support
* No extra dependencies (only `libc` and `libc++`)
* Colors (including 8bit and RGB)
* A lot of helpful ANSI escapes
* Mouse support
* Fast terminal I/O routines (direct sys calls, no stdio) with non-blocking reading support
* `charbuf` for fast escape buffering and printing (it's like `std::stringstream`, but 10x faster)
* Automatic restore of terminal modes on `exit`, `SIGINT` (Ctrl+C), `SIGTERM` (excluding Windows)

## TODO

* Input escapes parsing

## Demos

### Snake

![snake](images/ansipp_demo_snake.gif)

### Dead pixels

![dead pixels](images/ansipp_demo_dead_pixels.gif)

### Simple gradient and colors

![simple](images/ansipp_demo_simple.png)

