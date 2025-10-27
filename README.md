# Crumple Quest

## Compile and Run

### Compile for Release Mode (Fast)

**Mac/Linux:**
```bash
cmake -DENABLE_DEBUG=OFF ..
cmake --build .
```

**Windows (MSVC):**
```bash
cmake ..
cmake --build . --config Release
```

### Compile for Debug Mode

**Mac/Linux:**
```bash
cmake -DENABLE_DEBUG=ON ..
cmake --build .
```

**Windows (MSVC):**
```bash
cmake ..
cmake --build . --config Debug
```

### Running 

**Mac/Linux:**
```bash
./engine
```

**Windows:**
```bash
Debug\engine.exe
# or
Release\engine.exe
```