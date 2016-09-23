#!/usr/bin/env python3

import argparse
import concurrent.futures as cf
import math
import os
import shutil
import subprocess
import sys
import tempfile
import threading
import time

devnull = open(os.devnull, 'w')
adder = 'gadd'

def call_gadd(input_files, output_file, gadd_args=None):
    args = [adder]
    if gadd_args is not None:
        args.extend(gadd_args)
    args.append(output_file)
    args.extend(input_files)
    print(' '.join(args))
    subprocess.check_call(args, stdout=devnull, stderr=devnull)

class ThreadsafeCounter:
    def __init__(self):
        self.lock = threading.Lock()
        self.counter = 0

    def next(self):
        with self.lock:
            self.counter += 1
            return self.counter

class Merger:
    def __init__(self, max_workers=1, files_per_merge=2, gadd_args=None):
        self.executor = cf.ThreadPoolExecutor(max_workers=max_workers)
        self.temp_directory = tempfile.mkdtemp()
        self.files_per_merge = files_per_merge
        self._file_count = ThreadsafeCounter()
        self.gadd_args = gadd_args

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        shutil.rmtree(self.temp_directory)

    def merge(self, input_files, output_file):
        to_merge = [self.executor.submit(lambda x:x, filename)
                    for filename in input_files]

        while len(to_merge) > 1:
            ready_to_merge = [task for task in to_merge if task.done()]
            if (len(ready_to_merge) < self.files_per_merge and
                len(ready_to_merge) < len(to_merge)):
                # Wait for more merges to be finished.
                time.sleep(1)
                continue

            merge_this_iter = ready_to_merge[:self.files_per_merge]
            to_merge = [task for task in to_merge
                        if task not in merge_this_iter]

            merge_this_iter = [task.result() for task in merge_this_iter]
            future = self.executor.submit(self._submerge, merge_this_iter)
            to_merge.append(future)

        print(' '.join(['mv', to_merge[0].result(), output_file]))
        shutil.move(to_merge[0].result(), output_file)

    def _submerge(self, input_files):
        output_file = os.path.join(self.temp_directory, 'merge{}.root'.format(self._file_count.next()))
        call_gadd(input_files, output_file, self.gadd_args)
        return output_file

def main():
    parser = argparse.ArgumentParser(description='Recursive wrapper around {}'.format(adder))
    parser.add_argument('output_file', metavar='OUTPUT', type=str)
    parser.add_argument('input_files', metavar='INPUT', nargs='+')
    parser.add_argument('-j','--jobs', metavar='N',
                        nargs='?', default=1, type=int,
                        help='Run up to N subprocesses at once; max 8 with no arg')
    parser.add_argument('-m','--merge-number',
                        nargs=1, default=None, type=int,
                        help='The number of files to merge together at each step')
    args, unknown = parser.parse_known_args()

    if args.jobs is None:
        args.jobs = 8

    # If unspecified and 1-thread, merge them all directly.
    # If unspecified and N-thread, make it a 2-step process with even step-size.
    if args.merge_number is None:
        args.merge_number = math.ceil(len(args.input_files)/args.jobs)

    # print(args)
    # print(unknown)

    if ('-f' not in unknown and
        os.path.exists(args.output_file)):
        print('Output file "{}" already exists\n'
              'Pass -f to force re-creation of output file')
        return 1

    # return 0

    with Merger(max_workers=10, files_per_merge=5, gadd_args=unknown) as merger:
        merger.merge(args.input_files, args.output_file)

    return 0

if __name__=='__main__':
    sys.exit(main())
