# `wasmu` 🕸️🤏
A small, embeddable WebAssembly runtime for memory-constrained environments.

wasmu is primarily designed as a runtime for WebAssembly apps on the [DigiCron](https://digicron.devicefuture.org) retro smartwatch. wasmu focuses more on memory usage over performance, making it suitable for any environment where memory usage is a concern.

The name _wasmu_ is a play on words — _wasm_ is an initialism for _WebAssembly_, and _mu_ — or _µ_ — is the SI unit prefix for _micro_.

## Features
* **In-place interpretation**, meaning that wasmu doesn't try to recompile the WebAssembly code into native machine code or another intermediate representation
    * **Low memory footprint** — wasmu only needs to allocate a few tables and stacks alongside your WebAsesmbly code buffer
    * **Architecture-independent** — works on a wide range of CPU architectures
* **Embeddable** — single-header file that doesn't need `stdio` or `stdlib` to get working
* **Customisable** to fit your specific needs
    * **Define native modules** that can be called from WebAssembly code
* **Designed for microcontrollers**, but works on conventional computers and other platforms

## Building
Before building wasmu, you will need to install some dependencies:

```bash
sudo apt install gcc wabt
```

To build the library, run:

```bash
./build.sh
```

The built library will then be available at `dist/wasmu.h`, with the configuration header file at `dist/wasmu-config.h`.

## Testing
To run the unit tests, run:

```bash
./build.sh --test
```

Logs for each test are available at `test/$TEST/build/test.log`, where `$TEST` is the name of a test.