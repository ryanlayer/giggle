import click
import shutil
import os
import subprocess
from multiprocessing import Process, Pool


def getLogger(enable=True, prefix=None):
    if enable:
        return print
    if prefix:
        return lambda *x: print(prefix, *x)
    return lambda *_: None


@click.group()
def root():
    # check if giggle is installed
    installed = True
    try:
        FNULL = open(os.devnull, 'w')
        retcode = subprocess.call(['giggle'],
            stdout=FNULL,
            stderr=subprocess.STDOUT)
        installed = retcode == 0
    except:
        installed = False
    if not installed:
        raise click.ClickException('Make sure `giggle` is installed and in your PATH')


@root.command()
@click.option('-i', 'inputPath', type=click.Path(
        exists=True,
        file_okay=False,
        dir_okay=True,
        readable=True),
    required=True,
    help='Input directory')
@click.option('-o', 'outputPath', type=click.Path(
        file_okay=False,
        dir_okay=True,
        writable=True),
    required=True,
    help='Index output directory')
@click.option('-n', 'shardAmnt',
    required=True,
    type=click.INT,
    help='Amount of shards')
@click.option('-m', 'metadataPath', type=click.Path(
        file_okay=True,
        dir_okay=False,
        readable=True),
    help='Metadata config file')
@click.option('-k', 'keepTemp',
    is_flag=True,
    help='Keep temporary files after indexing')
@click.option('--verbose', 'verbose',
    is_flag=True,
    help='Extra logging for debugging')
def index(inputPath, outputPath, metadataPath, shardAmnt, keepTemp, verbose):
    vprint = getLogger(verbose)
    vprint("Using arguments:")
    vprint(" - inputPath:", inputPath)
    vprint(" - outputPath:", outputPath)
    vprint(" - metadataPath:", metadataPath)
    vprint(" - shardAmnt:", shardAmnt)
    vprint(" - keepTemp:", keepTemp)
    vprint(" - verbose:", verbose)

    if os.path.exists(outputPath):
        if os.listdir(outputPath):
            raise click.ClickException('Output directory already exists')
    else:
        os.mkdir(outputPath)
    
    if shardAmnt < 1:
        raise click.ClickException('Shard amount must be greater than 0')

    inputFiles = os.listdir(inputPath)
    inputFiles = list(map(lambda x: os.path.join(inputPath, x), inputFiles))
    # attempts to divide files into batches of equal size
    inputFileSizes = list(map(lambda x: os.path.getsize(x), inputFiles))
    
    vprint(len(inputFiles), "input files found.")

    inputTotalSize = sum(inputFileSizes)
    inputSizePerShard = inputTotalSize // shardAmnt
    
    threads = []
    
    i2 = 0
    for i in range(shardAmnt):
        total = 0
        filesSubset = []
        
        if i != shardAmnt - 1:
            while i2 < len(inputFiles):
                if total and total + inputFileSizes[i2] > inputSizePerShard:
                    break

                total += inputFileSizes[i2]
                filesSubset.append(inputFiles[i2])
                i2 += 1
        else:
            # catch rounding truncation
            while i2 < len(inputFiles):
                filesSubset.append(inputFiles[i2])
                i2 += 1
        
        if not filesSubset:
            break

        args = (i, outputPath, filesSubset, keepTemp, metadataPath, verbose)
        p = Process(target=launch_indexer, args=args)
        p.start()
        threads.append(p)
    
    # collect all
    for i, thread in enumerate(threads):
        thread.join()


def launch_indexer(shardIndex, outputPath, inputFiles, keepInputFiles, metadataPath, verbose):
    vprint = getLogger(verbose, '{shard ' + str(shardIndex) + '}')

    # 1. create shard directory
    shardPath = os.path.join(outputPath, str.format('shard_{0}', shardIndex))
    vprint("Operating within", shardPath)
    shardInputPath = os.path.join(shardPath, 'input')
    os.mkdir(shardPath)
    os.mkdir(shardInputPath)
    
    # 2. symlink input files

    for file in inputFiles:
        # unfortunately, giggle won't follow symlinks
        # os.symlink(file, os.path.join(shardInputPath, os.path.basename(file)))
        shutil.copy(file, os.path.join(shardInputPath, os.path.basename(file)))
    
    # 3. create giggle index
    metadataArg = str.format('-m {0}', metadataPath) if metadataPath else ''
    shardIndexPath = os.path.join(shardPath, 'index')
    giggleCmd = str.format('giggle index -i {0}/*.gz -o {1} {2}', shardInputPath, shardIndexPath, metadataArg)
    vprint("Working directory:", os.getcwd())
    vprint("Executing giggle ->", giggleCmd)
    os.system(giggleCmd)
    vprint("Giggle finished.")
    
    # 4. delete copied input files
    if not keepInputFiles:
        shutil.rmtree(shardInputPath)

    click.echo(str.format('Shard {0} indexed.', shardIndex))


@root.command()
@click.option('-i', 'indexPath', type=click.Path(
        exists=True,
        file_okay=False,
        dir_okay=True,
        readable=True,
        writable=True),
    required=True,
    help='(Sharded) index directory')
@click.option('-q', 'queryFilePath', type=click.Path(
        exists=True,
        file_okay=True,
        dir_okay=False,
        readable=True),
    required=True,
    help='Query file')
@click.option('-n', 'nThreads',
    default=1,
    type=click.INT,
    help='Amount of jobs to run in parallel (default: 1)')
@click.option('-s', 'sFlag', is_flag=True, help='Give significance by indexed file (requires query file).')
@click.option('-m', 'metadataPath', type=click.Path(
        file_okay=True,
        dir_okay=False,
        readable=True),
    help='Metadata config file')
@click.option('--verbose', 'verbose',
    is_flag=True,
    help='Extra logging for debugging')
def search(indexPath, queryFilePath, nThreads, sFlag, metadataPath, verbose):
    vprint = getLogger(verbose)
    vprint("Using arguments:")
    vprint(" - indexPath:", indexPath)
    vprint(" - queryFilePath:", queryFilePath)
    vprint(" - nThreads:", nThreads)
    vprint(" - sFlag:", sFlag)
    vprint(" - metadataPath:", metadataPath)
    vprint(" - verbose:", verbose)

    # gather shard directories
    
    shardDirs = []
    
    for subDir in os.listdir(indexPath):
        path = os.path.join(indexPath, subDir)

        if os.path.isdir(path):
            if subDir.startswith('shard_'):
                try:
                    shardId = int(subDir.split('_')[1])
                    
                    foundIndex = False
                    for file in os.listdir(path):
                        if file == 'index':
                            foundIndex = True
                    if not foundIndex:
                        raise click.ClickException(
                            str.format('Shard {0} is missing index directory', shardId))

                    shardDirs.append((shardId, path))
                except:
                    pass
    
    if not shardDirs:
        raise click.ClickException('No shard indices found.')
    
    # dice query and launch searchers
    
    if nThreads < 1:
        raise click.ClickException('Thread amount must be greater than 0')

    results = []

    i = 0
    pool = Pool(processes=nThreads)
    while i < len(shardDirs):
        batchArgs = []
        
        for _ in range(nThreads):
            if i < len(shardDirs):
                args = (shardDirs[i][0], shardDirs[i][1], queryFilePath, metadataPath, sFlag, i != 0, verbose)
                batchArgs.append(args)
                i += 1
            else:
                break
        
        results += pool.map(launch_searcher, batchArgs)
    
    click.echo(''.join(results))


def launch_searcher(args):
    shardId, shardPath, queryFilePath, metadataPath, sFlag, stripHeader, verbose = args
    vprint = getLogger(verbose, '{shard ' + str(shardId) + '}')
    vprint("Operating within", shardPath)
    
    metadataArg = str.format('-m {0}', metadataPath) if metadataPath else ''
    indexPath = os.path.join(shardPath, 'index')
    sArg = '-s' if sFlag else ''
    cmd = str.format('giggle search -i {0} -q {1} {2} {3}', indexPath, queryFilePath, metadataArg, sArg)

    vprint("Executing giggle ->", cmd)
    p = subprocess.Popen(cmd,
         shell=True,
         stdout=subprocess.PIPE)
    out, _ = p.communicate()
    vprint("Giggle finished.")
    
    # strip out the header
    if stripHeader and sFlag:
        vprint("Striping header from output")
        out = out.split('\n', 1)[1]

    if isinstance(out, bytes):
        return out.decode('utf-8')
    return out


if __name__ == '__main__':
    root()
