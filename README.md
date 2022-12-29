# Tree-Encoded Bitmaps

This repository contains the source code of
[*Tree-Encoded Bitmaps*](http://db.in.tum.de/~lang/papers/tebs.pdf)
and the short paper *In-Place Updates in Tree-Encoded Bitmaps*, which is an extension of the work done by Lang et al.
The original repository can be found [here](https://github.com/harald-lang/tree-encoded-bitmaps).
 
## Changes
- *src/dtl/bitmap/teb_flat.hpp* contains most of the changes and additions we made.
- The implementation for hybrid updates can be found in *src/dtl/bitmap/diff/diff.hpp*.
- *experiments/prune* contains the experiments that test the algorithms we proposed.
