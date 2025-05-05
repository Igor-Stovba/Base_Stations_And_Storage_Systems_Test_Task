## Project

Structure:

1) `tests` directory contains different tests.

2) `src` directory contains all source file `.cpp` and also `include/` with headers.

## Build
Everything is done through the ***cmake*** system

```bash 
mkdir build
cd build
cmake ..
```

In order to build the binary itself:

```bash 
make club
```

In order to build tests:

```bash 
make club_tests
```

## Run 

```bash
./club path-to-input-file
```

If ypu want to run tests, you should go to `build/tests` and

```bash
./club_tests
```