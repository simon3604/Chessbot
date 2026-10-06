# Chessbot

## Build and debug in VS Code on Windows

Install the Microsoft C/C++ VS Code extension and the MSYS2 UCRT64 toolchain
(GCC and GDB). The build and debug configurations expect the tools at
`C:\msys64\ucrt64\bin\g++.exe` and `C:\msys64\ucrt64\bin\gdb.exe`.

- Press **Ctrl+Shift+B** to build the debug executable.
- Open **Run and Debug** and select **Debug Chessbot**. The debugger builds
  the executable before launching it in VS Code's integrated terminal.
- Enter engine commands in the terminal, such as `uci` and `isready`; enter
  `quit` to exit.
