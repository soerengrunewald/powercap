# powercap

This tool will set the power-limit on AMDGPU's to either min or max,
nothing more nothing less.

## Why

One could do the same thing by autostarting [corectrl](https://gitlab.com/corectrl/corectrl.git),
but this usually includes changing the GPU and Memory frequencies.

And this is something not what I want, my intent is to same power
during gameplay. The easiest way to do this, is to limit the power-budget
and let the GPU handle the rest.

One could achive the same thing using a couple lines of shell script.
But this would not no be as fun as coding a bit, also I like binaries.

## Dependencies

* [gcc-c++](https://gcc.gnu.org/) or [clang++](https://clang.llvm.org/)
* [meson](https://mesonbuild.com/) or [muon](https://muon.build/)
* [ninja](https://ninja-build.org/) or [samurai](https://github.com/michaelforney/samurai/)

## Configuraion

If a `systemdsystemunitdir` is provided a systemd service file is generated.
One can pass arguments to the service via `/etc/sysconfig/powercap` file.

By default the program will set the lowerst power-target. But one can change
this by supplying `--min`, `--max` or `--def` as argument by declaring the
variable `POWERCAP_ARGS` in `/etc/sysconfig/powercap`.

