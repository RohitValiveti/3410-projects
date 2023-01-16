# Overview

In this assignment, you will write a performance simulator: a C program that simulates the behavior of a cache.

Your cache simulator will take as input a memory trace: a list of load and store instructions that were executed during a single run of a single program. We refer to each load and store instruction as a memory reference. You will use a single-core memory trace to evaluate the impact of cache size, associativity, block size, etc. on cache performance. Once you extend your simulator to model multiple caches, you will use a 4-core memory trace to evaluate the impact of different cache coherence protocols.
