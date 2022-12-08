# Final Report
## CSCI 5900- Independent Study (Fall 2022)
## Sagar Pathare
## Date: December 6, 2022

## Title: Building metadata index and applying query filter

## Motivation
- The input files to [GIGGLE](https://github.com/ryanlayer/giggle) may contain additional metadata columns (importance, score, etc.) with arbitrary information (columns count, data types, and sizes). 
- Currently, GIGGLE ignores this metadata. 
- The only way to access the metadata after indexing is to open the original input files. 
- Could we store this metadata as a part of GIGGLE index while indexing the input files and load it while searching? 
- We would no longer need access to the input files after indexing
- We would no longer need the offset index used to access the input files
- This would be useful for STIX

## Method/implementation
- Designed `metadata.conf` file format to accept the metadata columns information- a list of `<column name, data type and size>`
- Stored the metadata types information in the header of `metadata_index.dat file`
- Stored the actual metadata information from the input files in the rest of the file
- Created a query parser which uses the metadata index to parse a simple query of the format `<column><operator><value>`
  - Query example- `score>=4.59`

### `metadata.conf` file format
The file contains a list of `<column number, column name, data type and size>`.
- column number- the index (1-based) of the column in the interval input files
- column name- the name declared for the column to be used for query filter
- data type- the list of supported types:  
  - char
  - int_8
  - int_16
  - int_32
  - int_64
  - float
  - double
  - string
- size (optional)- size of the string, only required if the data type is `string`.   

#### Example- `metadata.conf`
```
3 interval_length int_32
5 is_important int_8
6 feature string 10
4 score double
7 strand char
```

### `metadata_index.dat` file format
```
<7-byte file header (GIGLMET)> <3-byte version (000)> <6-byte extra (reserved)>
<1-byte uint8 num_cols> <1-byte uint8 col_width>
< array of  
  <256-byte char*, name>
  <1-byte uint8, width>
  <1-byte char, data type specifier>
>
<8-byte uint64 num_rows>
< array of  
  <data 1> <data 2> ... <data n>
>
```

## Results

### Time taken for Index (seconds)
| Operation              | Mean        | User       | System     |
|------------------------|-------------|------------|------------|
| Index without metadata | 2.301400604 | 2.18434688 | 0.11592476 |
| Index with metadata    | 3.217897652 | 2.90494334 |  0.3117602 |

### Time taken for Search Query (milliseconds)
| Operation                                                                        | Mean        | User        | System      |
|----------------------------------------------------------------------------------|-------------|-------------|-------------|
| Search using index without metadata                                              | 59.05192191 | 52.30880857 | 6.741488163 |
| Search using index with metadata without loading metadata                        | 58.88397734 |    53.15284 |     5.73212 |
| Search using index with metadata with loading metadata                           | 59.28336942 | 54.07237961 | 5.203470588 |
| Search using index with metadata with loading metadata and applying query filter | 61.46501216 | 53.66404174 | 7.794006087 |

### Time taken for Search Region (milliseconds)
| Operation                                                                        | Mean        | User        | System       |
|----------------------------------------------------------------------------------|-------------|-------------|--------------|
| Search using index without metadata                                              | 3.400331697 | 2.852696838 | 0.5706614229 |
| Search using index with metadata without loading metadata                        | 3.360303457 | 3.007352598 | 0.5151494662 |
| Search using index with metadata with loading metadata                           | 3.542046965 | 3.007217495 | 0.5688710866 |
| Search using index with metadata with loading metadata and applying query filter | 3.506091068 | 2.974596693 | 0.5560202362 |

**Note**: The time duration values are approximate as they are affected by other applications running in the background.
## Discussion
The time taken for indexing was increased by approximately 39%, which is fine for a one-time task. Even after building the metadata index, we still have the option not to load it, which doesn't affect the performance. Loading the metadata and applying the filter takes slightly increases the time by ~5%.

## Steps to Reproduce
1. Get the `split_stats.tar.gz` file from [https://drive.google.com/file/d/1LSK8ABVjk8ETxsaQykjGWgHwFuOtKjkr/view?usp=share_link](https://drive.google.com/file/d/1LSK8ABVjk8ETxsaQykjGWgHwFuOtKjkr/view?usp=share_link)
2. Extract the file using `tar -zxvf split_stats.tar.gz`
3. Install [hyperfine](https://github.com/sharkdp/hyperfine), a command-line benchmarking tool.
4. Run the following set of commands for performing the benchmarks.
```
$GIGGLE_ROOT/bin/giggle index -s -f -i "split_stats/*gz" -o split_stats_b
$GIGGLE_ROOT/bin/giggle index -s -f -i "split_stats/*gz" -o split_stats_b_m -m "split_stats_metadata.conf" 

$GIGGLE_ROOT/bin/giggle search -i split_stats_b -r 1:1-50000
$GIGGLE_ROOT/bin/giggle search -i split_stats_b_m -r 1:1-50000
$GIGGLE_ROOT/bin/giggle search -i split_stats_b_m -r 1:1-50000 -m
$GIGGLE_ROOT/bin/giggle search -i split_stats_b_m -r 1:1-50000 -m -u 'mean>0.0003'

hyperfine -w 3 '$GIGGLE_ROOT/bin/giggle index -s -f -i "split_stats/*gz" -o split_stats_b' --export-csv metadata_comparisons/i1.csv
hyperfine -w 3 '$GIGGLE_ROOT/bin/giggle index -s -f -i "split_stats/*gz" -o split_stats_b_m -m "split_stats_metadata.conf"' --export-csv metadata_comparisons/i2.csv

hyperfine -w 3 '$GIGGLE_ROOT/bin/giggle search -i split_stats_b -r 1:1-50000' --export-csv metadata_comparisons/s1.csv
hyperfine -w 3 '$GIGGLE_ROOT/bin/giggle search -i split_stats_b_m -r 1:1-50000' --export-csv metadata_comparisons/s2.csv
hyperfine -w 3 '$GIGGLE_ROOT/bin/giggle search -i split_stats_b_m -r 1:1-50000 -m' --export-csv metadata_comparisons/s3.csv
hyperfine -w 3 "$GIGGLE_ROOT/bin/giggle search -i split_stats_b_m -r 1:1-50000 -m -u 'mean>0.0003'" --export-csv metadata_comparisons/s4.csv

hyperfine -w 3 '$GIGGLE_ROOT/bin/giggle search -i split_stats_b -q GSM1218850_MB135DMMD.peak.q100.bed.gz' --export-csv metadata_comparisons/q1.csv
hyperfine -w 3 '$GIGGLE_ROOT/bin/giggle search -i split_stats_b_m -q GSM1218850_MB135DMMD.peak.q100.bed.gz' --export-csv metadata_comparisons/q2.csv
hyperfine -w 3 '$GIGGLE_ROOT/bin/giggle search -i split_stats_b_m -q GSM1218850_MB135DMMD.peak.q100.bed.gz -m' --export-csv metadata_comparisons/q3.csv
hyperfine -w 3 "$GIGGLE_ROOT/bin/giggle search -i split_stats_b_m -q GSM1218850_MB135DMMD.peak.q100.bed.gz -m -u 'mean>0.0003'" --export-csv metadata_comparisons/q4.csv
```
5. Combine the reports into a single CSV using  
```awk '(NR == 1) || (FNR > 1)' metadata_comparisons/*.csv > metadata_comparisons/combined.csv```
6. Analyze the results in the `metadata_comparisons/combined.csv`.
