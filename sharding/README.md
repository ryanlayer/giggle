# Multi Giggle

Utility for running a sharded giggle instance.
Builds and searches indices in parallel.

# Basic Usage

```bash
> py multi_giggle.py index --help
Usage: multi_giggle.py index [OPTIONS]

Options:
  -i DIRECTORY  Input directory  [required]
  -o DIRECTORY  Index output directory  [required]
  -n INTEGER    Amount of shards  [required]
  -m FILE       Metadata config file
  -k            Keep temporary files after indexing
  --help        Show this message and exit.


> py multi_giggle.py search --help
Usage: multi_giggle.py search [OPTIONS]

Options:
  -i DIRECTORY  (Sharded) index directory  [required]
  -q FILE       Query file  [required]
  -n INTEGER    Amount of jobs to run in parallel (default: 1)
  -s            Give significance by indexed file (requires query file).
  -m FILE       Metadata config file
  --help        Show this message and exit.
>
```

**Note:** multi_giggle.py requires `giggle` to be installed and on the PATH.

For example,
```bash
# Create an index with 10 shards
py multi_giggle.py index -i ./inputDataDir -o index -n 4

# Search the index with 2 jobs
py multi_giggle.py search -s -i index -q query.bed.gz -n 2
```

Where `inputDataDir` is a directory expected to contain `.bed.gz` files.