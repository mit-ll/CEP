[//]: # (Copyright 2020 Massachusetts Institute of Technology)
[//]: # (SPDX short identifier: BSD-2-Clause)

<p align="center">
   Copyright 2020 Massachusetts Institute of Technology
</p>

## Auto-FIR Script
The auto-fir.py script generates a randomized variant of the filter as described here: [Spiral Project Filter Gen](http://www.spiral.net/hardware/filter.html)

Executing `auto-fir.py -h` will print a complete list of script options.

Simulation performs cycle level comparisons and assumes Modelsim/Questasim is in the current path.

Example usage:
```
  ./auto-fir.py -b -v -a    <-- Build the FIR generator (if not available) and 
                                generate a filter with default settings.  Verilog 
                                identifiers will be randomized
  ./auto-fir.py -sg -v      <-- With the filter built, generate a series of random 
                                test vectors and save the results
  ./auto-fir.py -sv -v      <-- Re-run the simulation and verify the vectors still pass

```

All generated files are implicitly included in .gitignore and are thus not tracked by git.

Caution should be taken when running the auto-fir.py script with the `--clean` or `--allclean` switches.  

The following files will be deleted with `--clean`:
```
    ./firgen/*
    ./synth/*
    ./outputs/*
    ./work/*
    ./__pycache__/*
    transcript
    *.firlog
```

If `--allclean` is specified, then following ADDITIONAL files will be deleted:
```
    firgen.tgz
    rm -f synth-jan-14-2009.tar.gz
```

#### Return to the root CEP [README](../../README.md)
