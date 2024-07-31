# ![ansipp](images/ansipp_logo.gif)

Simple, small, fast and portable library to use colors/drawing directly in terminal.

## Cross platform

It builds and works on 

[![Linux](https://github.com/acc15/ansipp/actions/workflows/linux.yml/badge.svg)](https://github.com/acc15/ansipp/actions/workflows/linux.yml)
[![MacOS](https://github.com/acc15/ansipp/actions/workflows/macos.yml/badge.svg)](https://github.com/acc15/ansipp/actions/workflows/macos.yml)
[![Windows](https://github.com/acc15/ansipp/actions/workflows/windows.yml/badge.svg)](https://github.com/acc15/ansipp/actions/workflows/windows.yml)

## Features

* CMake support
* No extra dependencies (only `libc` and `libc++`)
* Colors (including 8bit and RGB)
* A lot of helpful ANSI escapes
* Mouse support
* Fast terminal I/O routines (direct sys calls, no stdio) with non-blocking reading support
* `charbuf` for fast escape buffering and printing (it's like `std::stringstream`, but 10x faster)
* Automatic restore of terminal modes on `exit` and signals (`SIGINT`, `SIGTERM`, `SIGQUIT`)

## TODO

* Input escapes parsing

## Hello world

Build & install library: `./install.sh`

Add to your `CMakeLists.txt`:

```cmake
find_package(ansipp REQUIRED)
target_link_libraries(your_executable PRIVATE ansipp::ansipp)
```

Code:

```c++
#include <ansipp.hpp>

int main() {
    using namespace ansipp;
    init_or_exit();

    charbuf out(4096);
    out << "hello " << attrs().on(BOLD).fg(RED) << "RED BOLD" << attrs() << " text\n" << charbuf::to_stdout;
    return EXIT_SUCCESS;
}
```

## Demos

### Snake

![snake](images/ansipp_demo_snake.gif)

### Dead pixels

![dead pixels](images/ansipp_demo_dead_pixels.gif)

### Simple gradient and colors

![simple](images/ansipp_demo_simple.png)

