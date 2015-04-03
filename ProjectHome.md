Nanopb is a plain-C implementation of Google's Protocol Buffers data format. It is targeted at 32 bit microcontrollers, but is also fit for other embedded systems with tight (2-10 kB ROM, <1 kB RAM) memory constraints. Nanopb supports static memory allocation, i.e. you don't need a malloc implementation and can be sure of the memory requirements of your code.

# Status #
The nanopb core is stable, well tested and supports the following features:

  * All protobuf data types
  * Required, repeated and optional fields
  * Packed arrays
  * Nested submessages
  * Default values for fields
  * Extension fields

# Releases #
Stable releases of the library are released approximately once a month, unless there are no new developments:

<table border='0'>
<tr><td><img src='https://koti.kapsi.fi/~jpa/nanopb/download.png' /></td><td><b><a href='http://koti.kapsi.fi/~jpa/nanopb/download/'>Download stable releases</a></b></td></tr>
</table>

The newest git master is also relatively stable. Any breaking development happens in separate branches. For testing the newest features, check out the repository:

```
git clone https://code.google.com/p/nanopb/
```

# Documentation and support #
<table border='0'>
<tr><td><img src='https://koti.kapsi.fi/~jpa/nanopb/documentation.png' /></td><td><b><a href='http://koti.kapsi.fi/~jpa/nanopb/docs/'>Read the latest documentation online</a></b></td></tr>
</table>

<table border='0'>
<tr><td><img src='https://koti.kapsi.fi/~jpa/nanopb/discussion.png' /></td><td><b><a href='https://groups.google.com/forum/#!forum/nanopb'>Ask questions on the discussion forum</a></b></td></tr>
</table>

<table border='0'>
<tr><td><img src='https://koti.kapsi.fi/~jpa/nanopb/bugs.png' /></td><td><b><a href='http://code.google.com/p/nanopb/issues/list'>Report bugs on the issue tracker</a></b></td></tr>
</table>

# Benchmarks #
[Here](http://koti.kapsi.fi/~jpa/nanopb/benchmark/) are some benchmarks of nanopb against other C-based protobuf libraries. Accuracy nor fairness is guaranteed, but full set of scripts to run the benchmarks are provided.

![https://koti.kapsi.fi/~jpa/nanopb/benchmark/arm_code.png](https://koti.kapsi.fi/~jpa/nanopb/benchmark/arm_code.png)

Also [code coverage](http://lakka.kapsi.fi:50140/job/nanopb%20code%20coverage/lastSuccessfulBuild/artifact/tests/build/coverage/workspace/nanopb%20code%20coverage/index.html) information is available.
# License #
License is the zlib license. See [LICENSE](http://code.google.com/p/nanopb/source/browse/LICENSE) in the source repository.