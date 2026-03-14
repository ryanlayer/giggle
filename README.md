<img src="https://raw.githubusercontent.com/ryanlayer/giggle/master/img/logo.png" width="300"/>

GIGGLE is a genomics search engine that identifies and ranks the significance of shared genomic loci between query features and thousands of genome interval files.

- [Nature Methods paper](https://www.nature.com/articles/nmeth.4556)
- [Mailing list](https://groups.google.com/d/forum/giggle-discuss)
- [Video presentation (14m)](https://www.youtube.com/watch?v=yw8H7PhtZoA)

## Quickstart

### Install

Choose one of the following methods:

**Conda/Mamba**

```bash
mamba env create -f environment.yml
mamba activate giggle-dev
export HTS_INC=$CONDA_PREFIX/include
export HTS_LIB=$CONDA_PREFIX/lib
```

**Nix**

```bash
nix develop
```

**Ubuntu/Debian**

```bash
sudo apt-get install gcc make zlib1g-dev libssl-dev libhts-dev
export HTS_INC=/usr/include
export HTS_LIB=/usr/lib
```

### Build

```bash
git clone https://github.com/ryanlayer/giggle.git
cd giggle
make
```

### Basic Example

```bash
# Create some test data
mkdir -p example/beds && cd example

# Download and prepare a small dataset
curl -s "http://hgdownload.soe.ucsc.edu/goldenPath/hg19/database/microsat.txt.gz" \
    | gunzip -c | cut -f 2,3,4 | sort -k1,1 -k2,2n | bgzip > beds/microsat.bed.gz

curl -s "http://hgdownload.soe.ucsc.edu/goldenPath/hg19/database/simpleRepeat.txt.gz" \
    | gunzip -c | cut -f 2,3,4 | sort -k1,1 -k2,2n | bgzip > beds/simpleRepeat.bed.gz

# Index the files
giggle index -i "beds/*.bed.gz" -o my_index -s -f

# Search a region
giggle search -i my_index -r chr1:1000000-2000000

# Search with full record output
giggle search -i my_index -r chr1:1000000-2000000 -v
```

## Usage

GIGGLE has two main commands: `index` and `search`.

### Indexing

```
giggle index -i <input files> -o <output dir> -f
    -i  Files to index (e.g. "data/*.gz")
    -o  Index output directory
    -s  Files are sorted (faster indexing)
    -f  Force reindex if output directory exists
    -m  Metadata config file (see experiments/metadata_index_query_filter)
```

**Note:** Input files must be bgzipped BED or VCF files.

### Searching

```
giggle search -i <index directory> [options]
    -i  GIGGLE index directory
    -r  Region(s) to search (e.g. "chr1:1000-2000" or CSV "chr1:1-100,chr2:1-100")
    -q  Query file (bgzipped BED or VCF)
    -c  Show counts by indexed file
    -s  Show significance statistics (requires -q)
    -v  Show full matching records
    -o  Group results by query record (use with -v)
    -f  Filter to files matching pattern (regex CSV)
    -g  Genome size for significance testing (default: 3095677412)
    -l  List files in the index
    -m  Load metadata index
    -u  Apply query filter
```

### Search Examples

**Count overlaps per file:**

```bash
giggle search -i my_index -r chr1:1000000-2000000
# Output:
# #microsat.bed.gz    size:41572  overlaps:5
# #simpleRepeat.bed.gz    size:962714 overlaps:23
```

**Get full records:**

```bash
giggle search -i my_index -r chr1:1000000-2000000 -v
```

**Filter to specific files:**

```bash
giggle search -i my_index -r chr1:1000000-2000000 -f "simple"
```

**Statistical analysis with query file:**

```bash
giggle search -i my_index -q query.bed.gz -s
# Output includes: odds_ratio, Fisher's tests, combo_score
```

**Group results by query interval:**

```bash
giggle search -i my_index -q query.bed.gz -v -o
```

## Testing

### Unit Tests

```bash
cd test/unit && make
```

### Functional Tests

Requires `bedtools` in PATH.

```bash
# you may need to up the ulimit on some systems
# ulimit -n 1300
cd test/func && ./giggle_tests.sh
```

## Example Analysis: Roadmap Epigenomics

```bash
# Download pre-built index
wget https://s3.amazonaws.com/layerlab/giggle/roadmap/roadmap_sort.tar.gz
tar -zxvf roadmap_sort.tar.gz

# Build index (if "Too many open files": ulimit -Sn 16384)
giggle index -s -f -i "roadmap_sort/*gz" -o roadmap_sort_b

# Download query data
wget ftp://ftp.ncbi.nlm.nih.gov/geo/samples/GSM1218nnn/GSM1218850/suppl/GSM1218850_MB135DMMD.peak.txt.gz

# Filter to top peaks
zcat GSM1218850_MB135DMMD.peak.txt.gz \
    | awk '$8>100' \
    | cut -f1,2,3 \
    | bgzip -c > query.bed.gz

# Search with significance
giggle search -s -i roadmap_sort_b/ -q query.bed.gz > results.txt
```

## Hosted Data

Pre-built indexes available for download:

- **Roadmap Epigenomics:** https://s3.amazonaws.com/layerlab/giggle/roadmap/roadmap_sort.tar.gz
- **UCSC Genome Browser:** https://s3.amazonaws.com/layerlab/giggle/ucsc/ucscweb_sort.tar.gz
- **FANTOM5:** https://s3.amazonaws.com/layerlab/giggle/fantom/fantom_sort.tar.gz

Interactive heatmap: http://ryanlayer.github.io/giggle/

## Web Server (Optional)

GIGGLE can run as a web service using [libmicrohttpd](http://www.gnu.org/software/libmicrohttpd/).

```bash
# Install dependencies
mkdir -p $HOME/usr/local/

# libmicrohttpd
wget http://ftpmirror.gnu.org/libmicrohttpd/libmicrohttpd-0.9.46.tar.gz
tar zxvf libmicrohttpd-0.9.46.tar.gz
cd libmicrohttpd-0.9.46
./configure --prefix=$HOME/usr/local/ && make && make install
cd ..

# json-c
wget https://github.com/json-c/json-c/archive/json-c-0.12.1-20160607.tar.gz
tar xvf json-c-0.12.1-20160607.tar.gz
cd json-c-json-c-0.12.1-20160607
./configure --prefix=$HOME/usr/local/ && make && make install
cd ..

export LD_LIBRARY_PATH=$HOME/usr/local/lib/

# Build server
cd giggle
make server
```

**Run servers:**

```bash
giggle/bin/server_enrichment -i roadmap_sort_b/ -u /tmp/ \
    -d giggle/examples/rme/data_def.json -p 8080 &

giggle/bin/server_enrichment -i ucscweb_sort_b/ -u /tmp/ \
    -d giggle/examples/ucsc/data_def.json -p 8081 &

# Access at:
# http://ryanlayer.github.io/giggle/?primary_index=localhost:8080&ucsc_index=localhost:8081
```

## Language Bindings

> **Note:** These community-maintained bindings may be outdated.

### Python

[python-giggle](https://github.com/brentp/python-giggle) by Brent Pedersen

```python
from giggle import Giggle

index = Giggle('my-index')  # or Giggle.create('new-index', 'files/*.bed')
print(index.files)

result = index.query('chr1', 9999, 20000)
print(result.n_total_hits)

for hit in result[0]:
    print(hit)
```

**Installation:**

```bash
# Requires: zlib, libcurl, libcrypto, libbz2, liblzma
git clone --recursive https://github.com/brentp/python-giggle
cd python-giggle
python setup.py install
```

### Go

[go-giggle](https://github.com/brentp/go-giggle) by Brent Pedersen
[![GoDoc](https://godoc.org/github.com/brentp/go-giggle?status.png)](https://godoc.org/github.com/brentp/go-giggle)

```go
import giggle "github.com/brentp/go-giggle"

index := giggle.Open("/path/to/index")
res := index.Query("1", 565657, 567999)

index.Files()      // all files in index
res.TotalHits()    // total count
res.Hits()         // []uint32 hits per file
res.Of(0)          // []string results from first file
```

## Containers

### Docker

[giggle-docker](https://github.com/kubor/giggle-docker) by Ryuichi Kubo

```bash
docker run kubor/giggle-docker giggle -h
```

### Singularity

[giggle-singularity](https://github.com/HugoGuillen/giggle-singularity) by Hugo Guillen

```bash
giggle.sh check   # verify configuration
giggle.sh pull    # create container
giggle.sh index   # create an index
giggle.sh search  # search an index
```
