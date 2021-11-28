# diablo

## What is this?

A collection of low-level, byte-array-oriented operations, designed to provide
efficient implementations for common operations in Haskell programs.

## Wait, it's not a video game?

No. I just thought `-ldiablo` was funny.

## Why is this needed?

Surely GHC is optimal enough? Well, no:

* GHC, as current, cannot consistently emit SIMD instructions. The only way to
  do this involves using the LLVM backend, and the optimizer can miss many
  opportunities, assuming your compiler of choice doesn't ruin your day.
* Auto-vectorization, even at the level of C, with all the help it can get, is
  unreliable. Even SIMD _intrinsics_ are 
  [not reliable](https://danluu.com/assembly-intrinsics). When it comes to
  Haskell, with all of its added complexities, this is a hard problem even in
  theory.
* Byte and bit bashing is hard, error-prone and highly counter-intuitive to the
  untrained. A lot of stuff is lacking implementations, which forces anyone in
  Haskell to write them _again_ themselves. And this is before contending with
  the kind of performance surprises GHC Haskell is known for!

This necessitates a _portable_, _complete_ and _well-tested and benchmarked_
solution for common bit and byte-level routines, which can be used via the FFI
or a wrapper, across many Haskell projects. This is what ``diablo`` aims to be.

## What are the goals of this project?

### Focus on Haskell integration

Our goal is to make working with GHC Haskell as easy as possible: this is our
one, and _only_ goal. We do this by checking [Tier 1 and Tier
2](https://gitlab.haskell.org/ghc/ghc/-/wikis/platforms) platforms via CI as far
as is possible for correctness and buildability. If you can get GHC on your
platform and architecture, ``diablo`` should work too. We also provide easier
integration by having an 'amalgamation' single-file build, similar to SQLite. No
need to worry about system libraries, build systems or any of that: just drop
one file, note it in your dependencies, and go.

Note that we use this both as a source of restriction _and_ a source of freedom.
For example, we don't support compilers other than GCC and Clang, as GHC cannot
be compiled without one or the other being available.

### No dependencies

Nothing outside of the C11-specified stdlib and one of our supported compilers
is required. The only dependencies ``diablo`` has are for tests and benchmarks:
if you don't need those, you don't need the dependencies for them either. We
also painstakingly avoid any stdlib extras (like the ones provided by `glibc`
for example), and our CI checks against multiple stdlibs (including `musl`) to
be sure.

### Documentation and performance

C is a nightmare of correctness issues even at the best of times, for experts.
Haskellers can and should have better things to do than figure out why the C
standard decided to be delusional that decade ago. Furthermore, even though C is
known for performance, what 'performance' means can vary on things like compiler
choice, phase of the moon and whether Cthulhu hates you today. This cannot and
should not be any business of any Haskell programmers - things should just go
fast.

``diablo`` aims to take away _all_ of these concerns. We provide detailed
documentation, as well as tests and benchmarks, using a modern build system and
tools, so that you can be sure that things are both correct _and_ fast, without
having to lift a finger. This includes SIMD support: whenever possible, we
provide SIMD acceleration, so you don't have to.

### Batteries included

Bit and byte-bashing operations are surprisingly common, showing up in areas
like text processing, serialization, cryptography, compression, and probably a
dozen other things. Unfortunately, Haskell's provision of such primitives is
limited, particularly where byte _sequences_ are concerned. This forces
Haskellers to either write their own, or try using someone else's, with their
own peculiar caveats.

We aim to provide as many bit and byte-oriented operations as is reasonable
here, including many operations on sequences. If you feel we could add
something, please suggest it!

## How do I use this?

The easiest thing to do is to drop the `amalgamation/diablo.c` file into your
Haskell project, then add it to your `c-sources`. You can then use the FFI
directly, with the confidence that things will Just Work.

If you want to use this as a _C_ library, your best bet is a [Meson
subproject](https://mesonbuild.com/Subprojects.html).

## How do I build this?

We use [Meson](https://mesonbuild.com/index.html). We support _only_ GCC and
Clang as compilers: in particular, should you want to build on Windows, the only
supported configuration is via MinGW.

If you also want to build the tests, you will need the following Python
dependencies:

* [CFFI](https://cffi.readthedocs.io/en/latest)
* [Hypothesis](https://hypothesis.readthedocs.io/en/latest/index.html)

If you want to build the benchmarks, you will also need the following:

* [PyTest](https://docs.pytest.org/en/6.2.x)
* [pytest-benchmark](https://pytest-benchmark.readthedocs.io/en/stable/index.html)

Use the standard approach to building a Meson project: this will build both a
static and a shared library by default.

## What's your platform support?

Our goal is supporting all of the [Tier 1 platforms for
GHC](https://gitlab.haskell.org/ghc/ghc/-/wikis/platforms#tier-1-platforms),
with as many of the [Tier
2](https://gitlab.haskell.org/ghc/ghc/-/wikis/platforms#tier-2-platforms) ones
as reasonable. Furthermore, we aim to be stdlib-neutral insofar as possible. 

The table below summarizes our efforts so far. We consider a
platform-architecture combo 'Supported' when we have CI to test it. 'WIP'
indicates that this is theoretically testable, but isn't currently done, or is
being worked on.

|Architecture|Platform|Tier|Supported?|Stdlibs|
|------------|--------|----|----------|-------|
|x86         |Windows (MinGW)|1|No way to test|N/A|
|x86-64      |Windows (MinGW)|1|Yes|`glibc`|
|x86         |Linux|1|No way to test|N/A|
|x86-64      |Linux|1|Yes|`glibc`, `musl`|
|x86-64      |MacOS|1|Yes|`libc`|
|aarch64     |Linux|2|Yes|`glibc`, `musl`|
|x86         |FreeBSD|2|WIP|N/A|
|x86-64      |FreeBSD|2|WIP|N/A|
|x86         |OpenBSD|2|No way to test|N/A|
|x86         |Solaris|2|No way to test|N/A|
|x86         |MacOS|2|No way to test|N/A|
|x86-64      |OpenBSD|2|No way to test|N/A|
|x86-64      |DragonFly|2|No way to test|N/A|
|PowerPC     |Linux|2|No way to test|N/A|
|PowerPC     |AIX|2|No way to test|N/A|
|PowerPC64   |Linux|2|No way to test|N/A|
|PowerPC64le |Linux|2|Yes|`glibc`, `musl`|
|Sparc       |Linux|2|No way to test|N/A|
|IA-64       |Linux|2|No way to test|N/A|
|Alpha       |Linux|2|No way to test|N/A|
|HPPA        |Linux|2|No way to test|N/A|
|S/390       |Linux|2|Yes|`glibc`, `musl`|
|m68k        |Linux|2|No way to test|N/A|
|mips        |Linux|2|No way to test|N/A|
|mipsel      |Linux|2|No way to test|N/A|
|ARMv7       |Linux|2|Yes|`glibc`, `musl`|
|ARMel       |Linux|2|No way to test|N/A|
|ARM         |iOS|2|No way to test|N/A|

We provide SIMDed implementations of most functionality on the following
architectures:

* Aarch64 (theoretically any NEON platform should work, but we can't benchmark
  it)
* x86 (all SSE)
* x86-64 (all SSE, AVX, AVX2)

For x86-64, we use runtime detection for SIMD instruction sets to allow for
portable binaries.

## What can I do with this?

The project is licensed Apache 2.0 (SPDX code
[`Apache-2.0`](https://spdx.org/licenses/Apache-2.0.html)). For more details,
please see the `LICENSE.md` file.
